/**
  ******************************************************************************
  * @file    network.h
  * @date    2026-10-27T16:17:44+0100
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
  .tensors = (const stai_tensor[64]) {
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_c_inp_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.2930505871772766}}, .zeropoint = {1, (const int16_t[1]){85}}, .name = "LSTM__102_19_c_inp_QuantizeLinear_Output_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_h_inp_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007782586384564638}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__102_19_h_inp_QuantizeLinear_Output_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {1, (const float[1]){0.050968412309885025}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__102_19RH__41_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_c_inp_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.17858465015888214}}, .zeropoint = {1, (const int16_t[1]){56}}, .name = "LSTM__73_15_c_inp_QuantizeLinear_Output_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_h_inp_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.00782366469502449}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__73_15_h_inp_QuantizeLinear_Output_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {1, (const float[1]){0.05647076293826103}}, .zeropoint = {1, (const int16_t[1]){3}}, .name = "LSTM__73_15RH__5_output" },
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
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {1, (const float[1]){0.2184317260980606}}, .zeropoint = {1, (const int16_t[1]){3}}, .name = "LSTM__73_15WX__3_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {1, (const float[1]){0.21233989298343658}}, .zeropoint = {1, (const int16_t[1]){-2}}, .name = "LSTM__73_15WXRH__7_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.21233989298343658}}, .zeropoint = {1, (const int16_t[1]){-2}}, .name = "LSTM__73_15_0_9_output0" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.21233989298343658}}, .zeropoint = {1, (const int16_t[1]){-2}}, .name = "LSTM__73_15_0_9_output1" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.21233989298343658}}, .zeropoint = {1, (const int16_t[1]){-2}}, .name = "LSTM__73_15_0_9_output2" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.21233989298343658}}, .zeropoint = {1, (const int16_t[1]){-2}}, .name = "LSTM__73_15_0_9_output3" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.003921568859368563}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "LSTM__73_15_i_t_14_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.003921568859368563}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "LSTM__73_15_o_t_16_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.003921568859368563}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "LSTM__73_15_f_t_18_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.17192235589027405}}, .zeropoint = {1, (const int16_t[1]){57}}, .name = "LSTM__73_15_ft_mul_C_t1_22_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007843137718737125}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "LSTM__73_15_c_t_20_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007842693477869034}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__73_15_it_mul_c_t_24_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.17858465015888214}}, .zeropoint = {1, (const int16_t[1]){56}}, .name = "LSTM__73_15_c_out_26_QuantizeLinear_Input_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.0078431386500597}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "LSTM__73_15_tanh_C_t_28_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007823663763701916}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__73_15_h_out_30_QuantizeLinear_Input_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 64, 1}}, .scale = {1, (const float[1]){0.007823663763701916}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__730_to_chfirst_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {1, (const float[1]){0.06258121132850647}}, .zeropoint = {1, (const int16_t[1]){-17}}, .name = "LSTM__102_19WX__39_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 256}}, .scale = {1, (const float[1]){0.08544482290744781}}, .zeropoint = {1, (const int16_t[1]){-6}}, .name = "LSTM__102_19WXRH__43_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.08544482290744781}}, .zeropoint = {1, (const int16_t[1]){-6}}, .name = "LSTM__102_19_0_45_output0" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.08544482290744781}}, .zeropoint = {1, (const int16_t[1]){-6}}, .name = "LSTM__102_19_0_45_output1" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.08544482290744781}}, .zeropoint = {1, (const int16_t[1]){-6}}, .name = "LSTM__102_19_0_45_output2" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.08544482290744781}}, .zeropoint = {1, (const int16_t[1]){-6}}, .name = "LSTM__102_19_0_45_output3" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.003921459894627333}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "LSTM__102_19_i_t_50_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.003921522293239832}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "LSTM__102_19_o_t_52_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.003921275492757559}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "LSTM__102_19_f_t_54_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.28806185722351074}}, .zeropoint = {1, (const int16_t[1]){87}}, .name = "LSTM__102_19_ft_mul_C_t1_58_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007843134924769402}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__102_19_c_t_56_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007834051735699177}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__102_19_it_mul_c_t_60_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.2950662076473236}}, .zeropoint = {1, (const int16_t[1]){85}}, .name = "LSTM__102_19_c_out_62_QuantizeLinear_Input_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.0078431386500597}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "LSTM__102_19_tanh_C_t_64_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {1, (const float[1]){0.007782586384564638}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__102_19_h_out_66_QuantizeLinear_Input_output" },
   { .size_bytes = 256, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {2, (const int32_t[2]){1, 64}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_output" },
   { .size_bytes = 64, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 1, 64, 1}}, .scale = {1, (const float[1]){0.007782586384564638}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "LSTM__1020_to_chfirst_output" },
   { .size_bytes = 128, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 128, 1}}, .scale = {1, (const float[1]){0.007823663763701916}}, .zeropoint = {1, (const int16_t[1]){-1}}, .name = "StatefulPartitionedCall_vad_model_concat_10_output" },
   { .size_bytes = 32, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 32}}, .scale = {1, (const float[1]){0.051582761108875275}}, .zeropoint = {1, (const int16_t[1]){-21}}, .name = "StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output" },
   { .size_bytes = 32, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 32, 1}}, .scale = {1, (const float[1]){0.030516143888235092}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "StatefulPartitionedCall_vad_model_dense_3_Relu0_output" },
   { .size_bytes = 1, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {2, (const int32_t[2]){1, 1}}, .scale = {1, (const float[1]){0.026384761556982994}}, .zeropoint = {1, (const int16_t[1]){-46}}, .name = "StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output" },
   { .size_bytes = 1, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 1, 1}}, .scale = {1, (const float[1]){0.026384761556982994}}, .zeropoint = {1, (const int16_t[1]){-45}}, .name = "StatefulPartitionedCall_vad_model_concat_20_output" },
   { .size_bytes = 1, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {3, (const int32_t[3]){1, 1, 1}}, .scale = {1, (const float[1]){0.0038805054500699043}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "output_1_QuantizeLinear_Input_output" },
   { .size_bytes = 4, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {3, (const int32_t[3]){1, 1, 1}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "output_1_QuantizeLinear_Input_0_conversion_output" }
  },
  .nodes = (const stai_node_details[53]){
    {.id = 5, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){0}}, .output_tensors = {1, (const int32_t[1]){1}} }, /* LSTM__102_19_c_inp_QuantizeLinear_Output */
    {.id = 6, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){2}}, .output_tensors = {1, (const int32_t[1]){3}} }, /* LSTM__102_19_h_inp_QuantizeLinear_Output */
    {.id = 32, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){3}}, .output_tensors = {1, (const int32_t[1]){4}} }, /* LSTM__102_19RH__41 */
    {.id = 11, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){5}}, .output_tensors = {1, (const int32_t[1]){6}} }, /* LSTM__73_15_c_inp_QuantizeLinear_Output */
    {.id = 12, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){7}}, .output_tensors = {1, (const int32_t[1]){8}} }, /* LSTM__73_15_h_inp_QuantizeLinear_Output */
    {.id = 33, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){8}}, .output_tensors = {1, (const int32_t[1]){9}} }, /* LSTM__73_15RH__5 */
    {.id = 26, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){10}}, .output_tensors = {1, (const int32_t[1]){11}} }, /* input_1_QuantizeLinear_Output */
    {.id = 34, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){11}}, .output_tensors = {1, (const int32_t[1]){12}} }, /* StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0 */
    {.id = 41, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){12}}, .output_tensors = {1, (const int32_t[1]){13}} }, /* StatefulPartitionedCall_vad_model_separable_conv2d_Relu0 */
    {.id = 44, .type = AI_LAYER_POOL_TYPE, .input_tensors = {1, (const int32_t[1]){13}}, .output_tensors = {1, (const int32_t[1]){14}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270 */
    {.id = 47, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){14}}, .output_tensors = {1, (const int32_t[1]){15}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0 */
    {.id = 50, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){15}}, .output_tensors = {1, (const int32_t[1]){16}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0 */
    {.id = 53, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){16}}, .output_tensors = {1, (const int32_t[1]){17}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion */
    {.id = 59, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){17}}, .output_tensors = {1, (const int32_t[1]){18}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0 */
    {.id = 62, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){18}}, .output_tensors = {1, (const int32_t[1]){19}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0 */
    {.id = 65, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){19}}, .output_tensors = {1, (const int32_t[1]){20}} }, /* StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion */
    {.id = 74, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){20}}, .output_tensors = {1, (const int32_t[1]){21}} }, /* LSTM__73_15WX__3 */
    {.id = 77, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){21, 9}}, .output_tensors = {1, (const int32_t[1]){22}} }, /* LSTM__73_15WXRH__7 */
    {.id = 80, .type = AI_LAYER_SPLIT_TYPE, .input_tensors = {1, (const int32_t[1]){22}}, .output_tensors = {4, (const int32_t[4]){23, 24, 25, 26}} }, /* LSTM__73_15_0_9 */
    {.id = 89, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){23}}, .output_tensors = {1, (const int32_t[1]){27}} }, /* LSTM__73_15_i_t_14 */
    {.id = 90, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){24}}, .output_tensors = {1, (const int32_t[1]){28}} }, /* LSTM__73_15_o_t_16 */
    {.id = 91, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){25}}, .output_tensors = {1, (const int32_t[1]){29}} }, /* LSTM__73_15_f_t_18 */
    {.id = 101, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){29, 6}}, .output_tensors = {1, (const int32_t[1]){30}} }, /* LSTM__73_15_ft_mul_C_t1_22 */
    {.id = 92, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){26}}, .output_tensors = {1, (const int32_t[1]){31}} }, /* LSTM__73_15_c_t_20 */
    {.id = 102, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){27, 31}}, .output_tensors = {1, (const int32_t[1]){32}} }, /* LSTM__73_15_it_mul_c_t_24 */
    {.id = 107, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){30, 32}}, .output_tensors = {1, (const int32_t[1]){33}} }, /* LSTM__73_15_c_out_26_QuantizeLinear_Input */
    {.id = 107, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){33}}, .output_tensors = {1, (const int32_t[1]){34}} }, /* LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion */
    {.id = 110, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){33}}, .output_tensors = {1, (const int32_t[1]){35}} }, /* LSTM__73_15_tanh_C_t_28 */
    {.id = 113, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){28, 35}}, .output_tensors = {1, (const int32_t[1]){36}} }, /* LSTM__73_15_h_out_30_QuantizeLinear_Input */
    {.id = 113, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){36}}, .output_tensors = {1, (const int32_t[1]){37}} }, /* LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion */
    {.id = 116, .type = AI_LAYER_TRANSPOSE_TYPE, .input_tensors = {1, (const int32_t[1]){36}}, .output_tensors = {1, (const int32_t[1]){38}} }, /* LSTM__730_to_chfirst */
    {.id = 128, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){38}}, .output_tensors = {1, (const int32_t[1]){39}} }, /* LSTM__102_19WX__39 */
    {.id = 131, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){39, 4}}, .output_tensors = {1, (const int32_t[1]){40}} }, /* LSTM__102_19WXRH__43 */
    {.id = 134, .type = AI_LAYER_SPLIT_TYPE, .input_tensors = {1, (const int32_t[1]){40}}, .output_tensors = {4, (const int32_t[4]){41, 42, 43, 44}} }, /* LSTM__102_19_0_45 */
    {.id = 143, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){41}}, .output_tensors = {1, (const int32_t[1]){45}} }, /* LSTM__102_19_i_t_50 */
    {.id = 144, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){42}}, .output_tensors = {1, (const int32_t[1]){46}} }, /* LSTM__102_19_o_t_52 */
    {.id = 145, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){43}}, .output_tensors = {1, (const int32_t[1]){47}} }, /* LSTM__102_19_f_t_54 */
    {.id = 155, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){47, 1}}, .output_tensors = {1, (const int32_t[1]){48}} }, /* LSTM__102_19_ft_mul_C_t1_58 */
    {.id = 146, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){44}}, .output_tensors = {1, (const int32_t[1]){49}} }, /* LSTM__102_19_c_t_56 */
    {.id = 156, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){45, 49}}, .output_tensors = {1, (const int32_t[1]){50}} }, /* LSTM__102_19_it_mul_c_t_60 */
    {.id = 161, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){48, 50}}, .output_tensors = {1, (const int32_t[1]){51}} }, /* LSTM__102_19_c_out_62_QuantizeLinear_Input */
    {.id = 161, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){51}}, .output_tensors = {1, (const int32_t[1]){52}} }, /* LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion */
    {.id = 164, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){51}}, .output_tensors = {1, (const int32_t[1]){53}} }, /* LSTM__102_19_tanh_C_t_64 */
    {.id = 167, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {2, (const int32_t[2]){46, 53}}, .output_tensors = {1, (const int32_t[1]){54}} }, /* LSTM__102_19_h_out_66_QuantizeLinear_Input */
    {.id = 167, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){54}}, .output_tensors = {1, (const int32_t[1]){55}} }, /* LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion */
    {.id = 170, .type = AI_LAYER_TRANSPOSE_TYPE, .input_tensors = {1, (const int32_t[1]){54}}, .output_tensors = {1, (const int32_t[1]){56}} }, /* LSTM__1020_to_chfirst */
    {.id = 179, .type = AI_LAYER_CONCAT_TYPE, .input_tensors = {2, (const int32_t[2]){56, 38}}, .output_tensors = {1, (const int32_t[1]){57}} }, /* StatefulPartitionedCall_vad_model_concat_10 */
    {.id = 185, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){57}}, .output_tensors = {1, (const int32_t[1]){58}} }, /* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0 */
    {.id = 191, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {1, (const int32_t[1]){58}}, .output_tensors = {1, (const int32_t[1]){59}} }, /* StatefulPartitionedCall_vad_model_dense_3_Relu0 */
    {.id = 197, .type = AI_LAYER_DENSE_TYPE, .input_tensors = {1, (const int32_t[1]){59}}, .output_tensors = {1, (const int32_t[1]){60}} }, /* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0 */
    {.id = 203, .type = AI_LAYER_ELTWISE_INTEGER_TYPE, .input_tensors = {1, (const int32_t[1]){60}}, .output_tensors = {1, (const int32_t[1]){61}} }, /* StatefulPartitionedCall_vad_model_concat_20 */
    {.id = 206, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){61}}, .output_tensors = {1, (const int32_t[1]){62}} }, /* output_1_QuantizeLinear_Input */
    {.id = 206, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){62}}, .output_tensors = {1, (const int32_t[1]){63}} } /* output_1_QuantizeLinear_Input_0_conversion */
  },
  .n_nodes = 53
};
#endif

