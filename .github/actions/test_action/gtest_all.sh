#!/bin/bash
#*******************************************************************************
# Copyright 2019 FUJITSU LIMITED
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# imitations under the License.
# *******************************************************************************/CMakeFiles/
cd /github/workspace/build/tests/gtests

/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_batch_normalization_f32 |tee check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_batch_normalization_s8 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_binary |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_concat |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_backward_data_f32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_backward_weights_f32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_eltwise_forward_f32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_eltwise_forward_x8s8f32s32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_format_any |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_forward_f32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_forward_u8s8fp |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_convolution_forward_u8s8s32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_cross_engine_reorder |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_deconvolution |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_dnnl_threading |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_eltwise |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_bf16bf16bf16 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_bf16bf16f32 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_f16 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_f16f16f32 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_f32 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_s8s8s32 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_s8u8s32 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_u8s8s32 |tee -a check.log
#/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_gemm_u8u8s32 |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_global_scratchpad |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_attr |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_handle |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_pd |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_pd_iter |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_primitive_cache |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_runtime_attr |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_runtime_dims |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_stream_attr |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_iface_wino_convolution |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_inner_product_backward_data |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_inner_product_backward_weights |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_inner_product_forward |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_layer_normalization |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_logsoftmax |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_lrn_backward |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_lrn_forward |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_matmul |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_pooling_backward |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_pooling_forward |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_reorder |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_resampling |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_rnn_forward |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_shuffle |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_softmax |tee -a check.log
/local_qemu_5.0.0/bin/qemu-aarch64 -cpu max,sve512=on ./test_sum |tee -a check.log

NUM_TP=`wc -l check.log | cut -f 1 -d " "`
NUM_OK=`grep PASSED check.log | wc -l | cut -f 1 -d " "`

echo "TP NUM: ${NUM_TP}"
echo "TP OK : ${NUM_OK}"

if [ ${NUM_TP} = ${NUM_OK} ] ; then
    echo "Congratulation!"
    exit 0
else
    echo "Something wrong!"
    exit 1
fi
