/*******************************************************************************
* Copyright 2019-2020 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <atomic>
#include <numeric>

#include "dnnl_common.hpp"
#include "dnnl_memory.hpp"
#include "dnnl_reorder.hpp"

#include "tests/test_thread.hpp"

int dnn_mem_t::reorder(const dnn_mem_t &rhs, const attr_bundle_t *attr_bundle) {
    if (this == &rhs) return OK;
    return execute_reorder(rhs, *this, attr_bundle);
}

#if defined(_WIN32) && !defined(__GNUC__)
#include "windows.h"

static size_t get_cpu_ram_size() {
    MEMORYSTATUSEX s {};
    s.dwLength = sizeof(s);
    GlobalMemoryStatusEx(&s);
    return s.ullTotalPhys;
}
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <unistd.h>
#include <sys/sysctl.h>

static size_t get_cpu_ram_size() {
#ifdef __APPLE__
    int query_ram[] = {CTL_HW, HW_MEMSIZE};
#else
    int query_ram[] = {CTL_HW, HW_PHYSMEM};
#endif
    int query_ram_len = sizeof(query_ram) / sizeof(*query_ram);
    size_t totalram = 0;
    size_t length = sizeof(totalram);

    sysctl(query_ram, query_ram_len, &totalram, &length, NULL, 0);
    return totalram;
}
#else
#include <sys/sysinfo.h>

static size_t get_cpu_ram_size() {
    struct sysinfo s {};
    sysinfo(&s);
    return s.totalram;
}
#endif

static size_t get_gpu_ram_size() {
// TODO: consider DPCPP run-time as well.
#if DNNL_GPU_RUNTIME == DNNL_RUNTIME_OCL
    cl_int status = CL_SUCCESS;
    cl_device_id ocl_device = 0;
    // Get single device attached to the engine.
    engine_t engine_tgt(engine_tgt_kind);
    dnnl_engine_get_ocl_device(engine_tgt, &ocl_device);

    cl_ulong ram_size = 0;
    status = clGetDeviceInfo(ocl_device, CL_DEVICE_GLOBAL_MEM_SIZE,
            sizeof(cl_ulong), &ram_size, NULL);
    if (status == CL_SUCCESS) return (size_t)ram_size;
#endif
    return 0;
}

int dnn_mem_t::check_mem_size(const_dnnl_primitive_desc_t const_pd) {
    if (!mem_check) return OK;

    static uint64_t cpu_device_capacity = get_cpu_ram_size();
    static uint64_t gpu_device_capacity = get_gpu_ram_size();

    const uint64_t devices_max_capacity = engine_tgt_kind == dnnl_cpu
            ? cpu_device_capacity
            : MIN2(cpu_device_capacity, gpu_device_capacity);
    // 0.75f is taken randomly. A subject to change in future.
    const double benchdnn_limit = 0.75f * devices_max_capacity;

    // get all amount of memories to collect mem_size over all of them
    const int n_memories = dnnl_primitive_desc_query_s32(
                                   const_pd, dnnl_query_num_of_inputs_s32, 0)
            + dnnl_primitive_desc_query_s32(
                    const_pd, dnnl_query_num_of_outputs_s32, 0);

    const auto get_mem_size = [const_pd](dnnl_query_t query, int index = 0) {
        const auto md = dnnl_primitive_desc_query_md(const_pd, query, index);
        auto mem_size = dnnl_memory_desc_get_size(md);
        // reference memories are always fp32, hence need rescaling factor
        size_t ref_mem_factor = 1;
        if (md->data_type != dnnl_data_type_undef)
            ref_mem_factor = ::sizeof_dt(dnnl_f32) / ::sizeof_dt(md->data_type);
        // runtime mem size is not defined
        if (mem_size == DNNL_RUNTIME_SIZE_VAL) mem_size = 0;
        return (1 + ref_mem_factor) * mem_size;
    };

    double total_mem_size = 0;

#define MD(name) dnnl_query_##name##_md
    for (auto query : {MD(src), MD(diff_src), MD(weights), MD(diff_weights),
                 MD(dst), MD(diff_dst)}) {
        for (int idx = 0; idx < n_memories; ++idx)
            total_mem_size += get_mem_size(query, idx);
    }

    for (auto query : {MD(workspace), MD(scratchpad)})
        total_mem_size += get_mem_size(query);
#undef MD

    int64_t library_internal_mem_size = 0;
    dnnl_primitive_desc_query(const_pd, dnnl_query_memory_consumption_s64, 0,
            &library_internal_mem_size);
    total_mem_size += library_internal_mem_size;

    const bool fits_device_ram = total_mem_size <= benchdnn_limit;
    if (!fits_device_ram) {
        auto GB = [](double bytes) { return bytes / powf(2, 30); };

        BENCHDNN_PRINT(2,
                "benchdnn: not enough RAM for a problem.\nRequested: %g GB, "
                "benchdnn limit: %g GB, CPU RAM capacity: %g GB, GPU RAM "
                "capacity: %g GB\n",
                GB(total_mem_size), GB(benchdnn_limit), GB(cpu_device_capacity),
                GB(gpu_device_capacity));
    }

    return fits_device_ram ? OK : FAIL;
}

// Returns physical offset by logical one. Logical offset is represented by an
// array pos. If is_pos_padded is true pos represents the position in already
// padded area.
static dnnl_dim_t md_off_v(const dnnl_memory_desc_t &md, const dnnl_dims_t pos,
        bool is_pos_padded = false) {
    assert(md.format_kind == dnnl_blocked);
    const auto &blk = md.format_desc.blocking;

    dnnl_dims_t pos_copy = {0};
    for (int d = 0; d < md.ndims; ++d)
        pos_copy[d] = pos[d] + (is_pos_padded ? 0 : md.padded_offsets[d]);

    dnnl_dim_t phys_offset = md.offset0;

    if (blk.inner_nblks > 0) {
        dnnl_dim_t blk_stride = 1;
        for (int iblk = blk.inner_nblks - 1; iblk >= 0; --iblk) {
            const int d = blk.inner_idxs[iblk];

            dnnl_dim_t p = pos_copy[d] % blk.inner_blks[iblk];
            pos_copy[d] /= blk.inner_blks[iblk];

            phys_offset += p * blk_stride;
            blk_stride *= blk.inner_blks[iblk];
        }
    }

    for (int d = 0; d < md.ndims; ++d) {
        const dnnl_dim_t p = pos_copy[d];
        phys_offset += p * blk.strides[d];
    }

    return phys_offset;
}

// Returns physical offset by logical one. logical offset is represented by a
// scalar l_offset. If is_pos_padded is true, l_offset represents logical
// offset in already padded area.
dnnl_dim_t md_off_l(dnnl_dims_t _pos, const dnnl_memory_desc_t &md,
        dnnl_dim_t l_offset, bool is_pos_padded = false) {
    dnnl_dims_t pos;
    for (int rd = 0; rd < md.ndims; ++rd) {
        const int d = md.ndims - 1 - rd;
        const dnnl_dim_t cur_dim
                = is_pos_padded ? md.padded_dims[d] : md.dims[d];
        pos[d] = l_offset % cur_dim;
        if (_pos) _pos[d] = pos[d];
        l_offset /= cur_dim;
    }
    return md_off_v(md, pos, is_pos_padded);
}

template <typename T>
int check_zero_padding_impl(const dnn_mem_t &mem, int arg) {
    const int ndims = mem.md_.ndims;
    const auto *dims = mem.md_.dims;
    const auto *pdims = mem.md_.padded_dims;

    if (ndims == 0) return OK;
    if (mem.md_.format_kind != dnnl_blocked) return OK;

    auto product = [](const dnnl_dim_t *beg, const dnnl_dim_t *end) {
        return std::accumulate(
                beg, end, (dnnl_dim_t)1, std::multiplies<dnnl_dim_t>());
    };

    int errors = 0;
    std::atomic<int> ok(true);

    const T *mem_ptr = (const T *)mem;

    for (int dim_m_idx = 0; dim_m_idx < ndims; ++dim_m_idx) {
        if (dims[dim_m_idx] == pdims[dim_m_idx]) continue;

        auto dim_l = product(pdims, pdims + dim_m_idx);
        auto dim_r = product(pdims + dim_m_idx + 1, pdims + ndims);

        dnnl::impl::parallel_nd(dim_l, dim_r, [&](dnnl_dim_t l, dnnl_dim_t r) {
            for (dnnl_dim_t m = dims[dim_m_idx]; m < pdims[dim_m_idx]; ++m) {
                auto l_idx = (l * pdims[dim_m_idx] + m) * dim_r + r;
                auto idx = md_off_l(nullptr, mem.md_, l_idx, true);
                if (!(mem_ptr[idx] == 0)) ok = false;
            }
        });

        // Run the check one more time to report incorrect elements. This check
        // is sequential.
        if (!ok) {
            for_(dnnl_dim_t l = 0; l < dim_l; ++l)
            for_(dnnl_dim_t m = dims[dim_m_idx]; m < pdims[dim_m_idx]; ++m)
            for (dnnl_dim_t r = 0; r < dim_r; ++r) {
                auto l_idx = (l * pdims[dim_m_idx] + m) * dim_r + r;
                dnnl_dims_t pos = {};
                auto idx = md_off_l(pos, mem.md_, l_idx, true);

                bool idx_ok = (mem_ptr[idx] == 0);
                if (!idx_ok) errors++;

                const bool dump = (!idx_ok && (errors < 10 || verbose >= 10))
                        || (verbose >= 99);
                if (dump) {
                    BENCHDNN_PRINT(0,
                            "[%4ld][arg:%d]"
                            "[" IFMT "," IFMT "," IFMT "," IFMT "," IFMT
                            "," IFMT "] fp:  0.f dt:% 9.6g \n",
                            (long)idx, arg, pos[5], pos[4], pos[3], pos[2],
                            pos[1], pos[0], mem.get_elem(l_idx));
                }
            }
        }
    }

    if (!ok) {
        BENCHDNN_PRINT(0,
                "@@@ [arg:%d] check_zero_padding failed (will turn into an "
                "error soon) err:%d\n",
                arg, (int)errors);
    }

    // TODO: Return FAIL in case of a failure. Temporarily, return OK
    // regardless of the check result until all failures are fixed.
    return OK;
}

int check_zero_padding(const dnn_mem_t &mem, int arg) {
#define CASE(dt, type) \
    case dt: return check_zero_padding_impl<type>(mem, arg);

    switch (mem.md_.data_type) {
        case dnnl_data_type_undef:
            return OK;

            CASE(dnnl_bf16, bfloat16_t);
            CASE(dnnl_f16, float16_t);
            CASE(dnnl_f32, float);
            CASE(dnnl_s32, int32_t);
            CASE(dnnl_s8, int8_t);
            CASE(dnnl_u8, uint8_t);

        default: assert(!"bad data_type");
    };
#undef CASE

    return FAIL;
}
