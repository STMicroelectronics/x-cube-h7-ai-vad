/**
  ******************************************************************************
  * @file    network.h
  * @date    2026-10-27T16:17:37+0100
  * @brief   ST.AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef STAI_NETWORK_DETAILS_H
#define STAI_NETWORK_DETAILS_H

#include "stai.h"
#include "layers.h"

const stai_network_details g_network_details = {
  .tensors = (const stai_tensor[59]) {
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_c_inp_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_h_inp_output" },
   { .size_bytes = 1024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19RH__41_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_c_inp_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_h_inp_output" },
   { .size_bytes = 1024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15RH__5_output" },
   { .size_bytes = 492, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 3, 41, 1}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "input_1_output" },
   { .size_bytes = 123, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 3, 41, 1}}, .scale = {1, (const float[1]){0.02113228291273117}}, .zeropoint = {1, (const int16_t[1]){-4}}, .name = "input_1_QuantizeLinear_Output_output" },
   { .size_bytes = 39, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 39, 1}}, .scale = {1, (const float[1]){0.024968156591057777}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output" },
   { .size_bytes = 624, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 39, 16}}, .scale = {1, (const float[1]){0.01194307953119278}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output" },
   { .size_bytes = 304, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 19, 16}}, .scale = {1, (const float[1]){0.01194307953119278}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0" },
   { .size_bytes = 160, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 10, 16}}, .scale = {1, (const float[1]){0.021733799949288368}}, .zeropoint = {1, (const int16_t[1]){25}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output" },
   { .size_bytes = 160, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 10, 16}}, .scale = {1, (const float[1]){0.10068073123693466}}, .zeropoint = {1, (const int16_t[1]){32}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output" },
   { .size_bytes = 160, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 10, 16}}, .scale = {1, (const float[1]){0.03734692558646202}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output" },
   { .size_bytes = 80, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 5, 16}}, .scale = {1, (const float[1]){0.0387788750231266}}, .zeropoint = {1, (const int16_t[1]){-18}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output" },
   { .size_bytes = 80, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 5, 16}}, .scale = {1, (const float[1]){0.05319920554757118}}, .zeropoint = {1, (const int16_t[1]){28}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output" },
   { .size_bytes = 80, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 5, 16}}, .scale = {1, (const float[1]){0.02056172862648964}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output" },
   { .size_bytes = 320, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {3, (const int32_t[3]){1, 5, 16}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_0_conversion_output" },
   { .size_bytes = 1024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15WX__3_output" },
   { .size_bytes = 1024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15WXRH__7_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_0_9_output0" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_0_9_output1" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_0_9_output2" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_0_9_output3" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_c_t_20_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_f_t_18_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_ft_mul_C_t1_22_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_o_t_16_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_i_t_14_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_it_mul_c_t_24_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_c_out_26_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_tanh_C_t_28_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_h_out_30_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 1, 64, 1}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__730_to_chfirst_output" },
   { .size_bytes = 1024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19WX__39_output" },
   { .size_bytes = 1024, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19WXRH__43_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_0_45_output0" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_0_45_output1" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_0_45_output2" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_0_45_output3" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_c_t_56_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_f_t_54_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_ft_mul_C_t1_58_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_o_t_52_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_i_t_50_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_it_mul_c_t_60_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_c_out_62_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_tanh_C_t_64_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_h_out_66_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 1, 64, 1}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__1020_to_chfirst_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 64, 1}}, .scale = {1, (const float[1]){0.007823663763701916}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "StatefulPartitionedCall_vad_model_lstm_2_PartitionedCall_transpose_10_0_0_StatefulPartitionedCall_vad_model_concat_10_conversion_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 64, 1}}, .scale = {1, (const float[1]){0.007823663763701916}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "StatefulPartitionedCall_vad_model_lstm_PartitionedCall_transpose_10_0_conversion_output" },
   { .size_bytes = 128, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 128, 1}}, .scale = {1, (const float[1]){0.007823663763701916}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "StatefulPartitionedCall_vad_model_concat_10_output" },
   { .size_bytes = 32, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 32}}, .scale = {1, (const float[1]){0.051582761108875275}}, .zeropoint = {1, (const int16_t[1]){-21}}, .name = "StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output" },
   { .size_bytes = 32, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 32, 1}}, .scale = {1, (const float[1]){0.030516143888235092}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "StatefulPartitionedCall_vad_model_dense_3_Relu0_output" },
   { .size_bytes = 1, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 1}}, .scale = {1, (const float[1]){0.026384761556982994}}, .zeropoint = {1, (const int16_t[1]){-46}}, .name = "StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output" },
   { .size_bytes = 1, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 1, 1}}, .scale = {1, (const float[1]){0.026384761556982994}}, .zeropoint = {1, (const int16_t[1]){-45}}, .name = "StatefulPartitionedCall_vad_model_concat_20_output" },
   { .size_bytes = 1, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 1, 1}}, .scale = {1, (const float[1]){0.0038805054500699043}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "output_1_QuantizeLinear_Input_output" },
   { .size_bytes = 4, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {3, (const int32_t[3]){1, 1, 1}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "output_1_QuantizeLinear_Input_0_conversion_output" }
  },
  .nodes = (const stai_node_details[48]){
    {.id = 1, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){1}}, .output_tensors = {1, (const int32_t[1]){2}} }, /* LSTM__102_19RH__41 */
    {.id = 2, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){4}}, .output_tensors = {1, (const int32_t[1]){5}} }, /* LSTM__73_15RH__5 */
    {.id = 16, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){6}}, .output_tensors = {1, (const int32_t[1]){7}} }, /* input_1_QuantizeLinear_Output */
    {.id = 18, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){7}}, .output_tensors = {1, (const int32_t[1]){8}} }, /* StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0 */
    {.id = 21, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){8}}, .output_tensors = {1, (const int32_t[1]){9}} }, /* StatefulPartitionedCall_vad_model_separable_conv2d_Relu0 */
    {.id = 24, .type = AI_LAYER_POOL_TYPE, .input_tensors = {1, (const int32_t[1]){9}}, .output_tensors = {1, (const int32_t[1]){10}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270 */
    {.id = 27, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){10}}, .output_tensors = {1, (const int32_t[1]){11}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0 */
    {.id = 30, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){11}}, .output_tensors = {1, (const int32_t[1]){12}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0 */
    {.id = 33, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){12}}, .output_tensors = {1, (const int32_t[1]){13}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion */
    {.id = 39, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){13}}, .output_tensors = {1, (const int32_t[1]){14}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0 */
    {.id = 42, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){14}}, .output_tensors = {1, (const int32_t[1]){15}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0 */
    {.id = 45, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){15}}, .output_tensors = {1, (const int32_t[1]){16}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion */
    {.id = 45, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){16}}, .output_tensors = {1, (const int32_t[1]){17}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_0_conversion */
    {.id = 54, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){17}}, .output_tensors = {1, (const int32_t[1]){18}} }, /* LSTM__73_15WX__3 */
    {.id = 55, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){18, 5}}, .output_tensors = {1, (const int32_t[1]){19}} }, /* LSTM__73_15WXRH__7 */
    {.id = 56, .type = AI_LAYER_SPLIT_TYPE, .input_tensors = {1, (const int32_t[1]){19}}, .output_tensors = {4, (const int32_t[4]){20, 21, 22, 23}} }, /* LSTM__73_15_0_9 */
    {.id = 60, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){23}}, .output_tensors = {1, (const int32_t[1]){24}} }, /* LSTM__73_15_c_t_20 */
    {.id = 59, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){22}}, .output_tensors = {1, (const int32_t[1]){25}} }, /* LSTM__73_15_f_t_18 */
    {.id = 61, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){25, 3}}, .output_tensors = {1, (const int32_t[1]){26}} }, /* LSTM__73_15_ft_mul_C_t1_22 */
    {.id = 58, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){21}}, .output_tensors = {1, (const int32_t[1]){27}} }, /* LSTM__73_15_o_t_16 */
    {.id = 57, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){20}}, .output_tensors = {1, (const int32_t[1]){28}} }, /* LSTM__73_15_i_t_14 */
    {.id = 62, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){28, 24}}, .output_tensors = {1, (const int32_t[1]){29}} }, /* LSTM__73_15_it_mul_c_t_24 */
    {.id = 63, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){26, 29}}, .output_tensors = {1, (const int32_t[1]){30}} }, /* LSTM__73_15_c_out_26 */
    {.id = 64, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){30}}, .output_tensors = {1, (const int32_t[1]){31}} }, /* LSTM__73_15_tanh_C_t_28 */
    {.id = 65, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){27, 31}}, .output_tensors = {1, (const int32_t[1]){32}} }, /* LSTM__73_15_h_out_30 */
    {.id = 66, .type = AI_LAYER_TRANSPOSE_TYPE, .input_tensors = {1, (const int32_t[1]){32}}, .output_tensors = {1, (const int32_t[1]){33}} }, /* LSTM__730_to_chfirst */
    {.id = 70, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){33}}, .output_tensors = {1, (const int32_t[1]){34}} }, /* LSTM__102_19WX__39 */
    {.id = 71, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){34, 2}}, .output_tensors = {1, (const int32_t[1]){35}} }, /* LSTM__102_19WXRH__43 */
    {.id = 72, .type = AI_LAYER_SPLIT_TYPE, .input_tensors = {1, (const int32_t[1]){35}}, .output_tensors = {4, (const int32_t[4]){36, 37, 38, 39}} }, /* LSTM__102_19_0_45 */
    {.id = 76, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){39}}, .output_tensors = {1, (const int32_t[1]){40}} }, /* LSTM__102_19_c_t_56 */
    {.id = 75, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){38}}, .output_tensors = {1, (const int32_t[1]){41}} }, /* LSTM__102_19_f_t_54 */
    {.id = 77, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){41, 0}}, .output_tensors = {1, (const int32_t[1]){42}} }, /* LSTM__102_19_ft_mul_C_t1_58 */
    {.id = 74, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){37}}, .output_tensors = {1, (const int32_t[1]){43}} }, /* LSTM__102_19_o_t_52 */
    {.id = 73, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){36}}, .output_tensors = {1, (const int32_t[1]){44}} }, /* LSTM__102_19_i_t_50 */
    {.id = 78, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){44, 40}}, .output_tensors = {1, (const int32_t[1]){45}} }, /* LSTM__102_19_it_mul_c_t_60 */
    {.id = 79, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){42, 45}}, .output_tensors = {1, (const int32_t[1]){46}} }, /* LSTM__102_19_c_out_62 */
    {.id = 80, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){46}}, .output_tensors = {1, (const int32_t[1]){47}} }, /* LSTM__102_19_tanh_C_t_64 */
    {.id = 81, .type = AI_LAYER_ELTWISE_TYPE, .input_tensors = {2, (const int32_t[2]){43, 47}}, .output_tensors = {1, (const int32_t[1]){48}} }, /* LSTM__102_19_h_out_66 */
    {.id = 82, .type = AI_LAYER_TRANSPOSE_TYPE, .input_tensors = {1, (const int32_t[1]){48}}, .output_tensors = {1, (const int32_t[1]){49}} }, /* LSTM__1020_to_chfirst */
    {.id = 84, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){49}}, .output_tensors = {1, (const int32_t[1]){50}} }, /* StatefulPartitionedCall_vad_model_lstm_2_PartitionedCall_transpose_10_0_0_StatefulPartitionedCall_vad_model_concat_10_conversion */
    {.id = 68, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){33}}, .output_tensors = {1, (const int32_t[1]){51}} }, /* StatefulPartitionedCall_vad_model_lstm_PartitionedCall_transpose_10_0_conversion */
    {.id = 85, .type = AI_LAYER_CONCAT_TYPE, .input_tensors = {2, (const int32_t[2]){50, 51}}, .output_tensors = {1, (const int32_t[1]){52}} }, /* StatefulPartitionedCall_vad_model_concat_10 */
    {.id = 89, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){52}}, .output_tensors = {1, (const int32_t[1]){53}} }, /* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0 */
    {.id = 95, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {1, (const int32_t[1]){53}}, .output_tensors = {1, (const int32_t[1]){54}} }, /* StatefulPartitionedCall_vad_model_dense_3_Relu0 */
    {.id = 101, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){54}}, .output_tensors = {1, (const int32_t[1]){55}} }, /* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0 */
    {.id = 107, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {1, (const int32_t[1]){55}}, .output_tensors = {1, (const int32_t[1]){56}} }, /* StatefulPartitionedCall_vad_model_concat_20 */
    {.id = 110, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){56}}, .output_tensors = {1, (const int32_t[1]){57}} }, /* output_1_QuantizeLinear_Input */
    {.id = 110, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){57}}, .output_tensors = {1, (const int32_t[1]){58}} } /* output_1_QuantizeLinear_Input_0_conversion */
  },
  .n_nodes = 48
};
#endif

