/**
  ******************************************************************************
  * @file    network.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-10-27T16:17:44+0100
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
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

#include "ai_lite_inspect.h"
#include "ai_platform_interface.h"
#include "layers.h"
#include "core_convert.h"
#include "network.h"
#include "network_details.h"
#include "network_data.h"
#include "stai_events.h"

#include "lite_operators.h"

#include "ai_lite_inspect.h"
/*****************************************************************************/
#define STAI_INTERNAL_API_MAJOR               (1)
#define STAI_INTERNAL_API_MINOR               (0)
#define STAI_INTERNAL_API_MICRO               (0)

#define STAI_MAGIC                            (0xB1C00100)

/*****************************************************************************/
#define _STAI_CONCAT_ARG(a, b)     a ## b
#define STAI_CONCAT(a, b)         _STAI_CONCAT_ARG(a, b)

/*!  STAI_CAST SECTION                       *********************************/
#define STAI_CAST(type, expr) \
  ((type)(expr))


/*****************************************************************************/
#define STAI_SIZE(_size) \
  ((stai_size)(_size))

/*****************************************************************************/
#define STAI_INIT_BUFFER(_flags, _size, _address) \
  { \
    .size = (_size), \
    .address = (uintptr_t)(_address), \
    .flags = (_flags), \
  }

#define STAI_INIT_TENSOR(_name, _flags, _fmt, _size_bytes, _shape, _scale, _zeropoint) \
  { \
    .size_bytes = (_size_bytes), \
    .flags = (_flags), \
    .format = (stai_format)(_fmt), \
    .shape = STAI_PACK(_shape), \
    .scale = STAI_PACK(_scale), \
    .zeropoint = STAI_PACK(_zeropoint), \
    .name = (_name) \
  }

#define STAI_INIT_ARRAY(_size, _ptr) \
  { .size = STAI_SIZE(_size), .data = STAI_PACK(_ptr) }


#define STAI_CAST_ARRAY(_type, _size, _ptr) \
  { .size = STAI_SIZE(_size), .data = (_type)STAI_PACK(_ptr) }


#define STAI_DECLARE_ARRAY(_type, _size, ...) \
  { .size = STAI_SIZE(_size), .data = (_type[_size]) { STAI_PACK(__VA_ARGS__) } }


#define STAI_EMPTY_ARRAY() \
  { .size = 0, .data = NULL }


#define STAI_INIT_VERSION(_major, _minor, _micro) \
  { .major = (_major), .minor = (_minor), .micro = (_micro), .reserved = 0x0 }

/*****************************************************************************/
/**  Getters and setters  **/

#define STAI_GET_ARRAY_SIZE(nd_array) \
  (nd_array.size)


#define STAI_GET_ARRAY_ELEM(nd_array, pos) \
  (nd_array.data[(pos)])

#define _STAI_SET_ERROR(net_ctx, cond, value, exit) { \
  if (!(net_ctx)) { return STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE; } \
  if (((uintptr_t)net_ctx) & (_STAI_CONTEXT_ALIGNMENT-1)) { return STAI_ERROR_NETWORK_INVALID_CONTEXT_ALIGNMENT; } \
  if (((value) >= STAI_ERROR_GENERIC) && (cond)) { \
    if ((net_ctx)->_return_code == STAI_SUCCESS) { \
      (net_ctx)->_return_code = (value); \
    } \
    return (exit); \
  } \
}

/*****************************************************************************/
/* TODO REMOVE THESE TWO MACROS */
#define STAI_EVENT_NODE_START_CB
#define STAI_EVENT_NODE_STOP_CB

#ifdef STAI_EVENT_NODE_START_CB
#ifndef _STAI_NETWORK_EVENT_NODE_START_CB
  #define _STAI_NETWORK_EVENT_NODE_START_CB(_node_id, _buffers_size, ...) \
  if (net_ctx->_callback) { \
    const stai_event_node_start_stop _start_event = { \
      .node_id=(_node_id), \
      .buffers={ \
        .size=(_buffers_size), \
        .data=(stai_ptr const*)(const stai_ptr[_buffers_size])STAI_PACK(__VA_ARGS__) \
      } \
    }; \
    net_ctx->_callback(net_ctx->_callback_cookie, STAI_EVENT_NODE_START, (const void*)&_start_event); \
  }
#endif
#else
  #define _STAI_NETWORK_EVENT_NODE_START_CB(_node_id, _buffers_size, ...) \
    do { /* _STAI_NETWORK_EVENT_NODE_START_CB() */ } while(0);
#endif      /* STAI_EVENT_NODE_START_CB */

#ifdef STAI_EVENT_NODE_STOP_CB
#ifndef _STAI_NETWORK_EVENT_NODE_STOP_CB
  #define _STAI_NETWORK_EVENT_NODE_STOP_CB(_node_id, _buffers_size, ...) \
  if (net_ctx->_callback) { \
    const stai_event_node_start_stop _stop_event = { \
      .node_id=(_node_id), \
      .buffers={ \
        .size=(_buffers_size), \
        .data=(stai_ptr const*)(stai_ptr[_buffers_size])STAI_PACK(__VA_ARGS__) \
      } \
    }; \
    net_ctx->_callback(net_ctx->_callback_cookie, STAI_EVENT_NODE_STOP, (const void*)&_stop_event); \
  }
#endif
#else
  #define _STAI_NETWORK_EVENT_NODE_STOP_CB(_node_id, _buffers_size, ...) \
    do { /* _STAI_NETWORK_EVENT_NODE_STOP_CB() */ } while(0);
#endif      /* STAI_EVENT_NODE_STOP_CB */


/*****************************************************************************/
#define _STAI_NETWORK_MODEL_SIGNATURE     "0x7572e0684cb18a8ee0fb339fb12e89de"
#define _STAI_NETWORK_DATETIME            "2026-10-27T16:17:44+0100"
#define _STAI_NETWORK_COMPILE_DATETIME    __DATE__ " " __TIME__

#define _STAI_CONTEXT_ALIGNMENT        STAI_NETWORK_CONTEXT_ALIGNMENT

/*****************************************************************************/
#define g_network_activations_1     (NULL)




#if defined(HAVE_NETWORK_INFO)
/*****************************************************************************/
static const stai_network_info g_network_info = {
  .model_signature = _STAI_NETWORK_MODEL_SIGNATURE,
  .c_compile_datetime = _STAI_NETWORK_COMPILE_DATETIME,
  .c_model_name = STAI_NETWORK_MODEL_NAME,
  .c_model_datetime = _STAI_NETWORK_DATETIME,
  .c_model_signature = 0x0,
  .runtime_version = STAI_INIT_VERSION(12, 1, 0),
  .tool_version = STAI_INIT_VERSION(4, 1, 0),
  .api_version = STAI_INIT_VERSION(1, 0, 0),
  .n_macc = STAI_NETWORK_MACC_NUM,
  .n_nodes = STAI_NETWORK_NODES_NUM,
  .flags = STAI_NETWORK_FLAGS,
  .n_inputs = STAI_NETWORK_IN_NUM,
  .n_outputs = STAI_NETWORK_OUT_NUM,
  .n_activations = STAI_NETWORK_ACTIVATIONS_NUM,
  .n_weights = STAI_NETWORK_WEIGHTS_NUM,
  .n_states = STAI_NETWORK_STATES_NUM,
  .inputs = (stai_tensor[STAI_NETWORK_IN_NUM]) {
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_1_NAME,
      STAI_NETWORK_IN_1_FLAGS,
      STAI_NETWORK_IN_1_FORMAT,
      STAI_NETWORK_IN_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 4, 1, 1, 3, 41),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_2_NAME,
      STAI_NETWORK_IN_2_FLAGS,
      STAI_NETWORK_IN_2_FORMAT,
      STAI_NETWORK_IN_2_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_3_NAME,
      STAI_NETWORK_IN_3_FLAGS,
      STAI_NETWORK_IN_3_FORMAT,
      STAI_NETWORK_IN_3_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_4_NAME,
      STAI_NETWORK_IN_4_FLAGS,
      STAI_NETWORK_IN_4_FORMAT,
      STAI_NETWORK_IN_4_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_5_NAME,
      STAI_NETWORK_IN_5_FLAGS,
      STAI_NETWORK_IN_5_FORMAT,
      STAI_NETWORK_IN_5_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },
    .outputs = (stai_tensor[STAI_NETWORK_OUT_NUM]) {
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_1_NAME,
      STAI_NETWORK_OUT_1_FLAGS,
      STAI_NETWORK_OUT_1_FORMAT,
      STAI_NETWORK_OUT_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 3, 1, 1, 1),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_2_NAME,
      STAI_NETWORK_OUT_2_FLAGS,
      STAI_NETWORK_OUT_2_FORMAT,
      STAI_NETWORK_OUT_2_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_3_NAME,
      STAI_NETWORK_OUT_3_FLAGS,
      STAI_NETWORK_OUT_3_FORMAT,
      STAI_NETWORK_OUT_3_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_4_NAME,
      STAI_NETWORK_OUT_4_FLAGS,
      STAI_NETWORK_OUT_4_FORMAT,
      STAI_NETWORK_OUT_4_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_5_NAME,
      STAI_NETWORK_OUT_5_FLAGS,
      STAI_NETWORK_OUT_5_FORMAT,
      STAI_NETWORK_OUT_5_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 2, 1, 64),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },
  .activations = (stai_tensor[STAI_NETWORK_ACTIVATIONS_NUM]) {
    STAI_INIT_TENSOR(
      (NULL),
      STAI_NETWORK_ACTIVATION_1_FLAGS,
      STAI_FORMAT_U8,
      STAI_NETWORK_ACTIVATION_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 1, 1692),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },
  .weights = (stai_tensor[STAI_NETWORK_WEIGHTS_NUM]) {
    STAI_INIT_TENSOR(
      (NULL),
      STAI_NETWORK_WEIGHT_1_FLAGS,
      STAI_FORMAT_U8,
      STAI_NETWORK_WEIGHT_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 1, 78924),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },

  .states = NULL
};
#endif

#define _STAI_CONTEXT_ACQUIRE(_net_ctx, _net_handle) \
  _stai_network_context* _net_ctx = (_stai_network_context*)(_net_handle); \
  STAI_ASSERT(_net_ctx != NULL) \
  _STAI_SET_ERROR(_net_ctx, _net_ctx->_magic != STAI_MAGIC, \
                  STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE, _net_ctx->_return_code)


/*****************************************************************************/
static
void _stai_network_check(_stai_network_context* net_ctx)
{
  stai_size idx;

// Check activations status
  for (idx=0; idx<STAI_NETWORK_ACTIVATIONS_NUM; idx++) {
    if (net_ctx->_activations[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_ACTIVATIONS_NUM) ? STAI_FLAG_ACTIVATIONS : STAI_FLAG_NONE;
// Check inputs status
  for (idx=0; idx<STAI_NETWORK_IN_NUM; idx++) {
    if (net_ctx->_inputs[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_IN_NUM) ? STAI_FLAG_INPUTS : STAI_FLAG_NONE;

  // Check outputs status
  for (idx=0; idx<STAI_NETWORK_OUT_NUM; idx++) {
    if (net_ctx->_outputs[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_OUT_NUM) ? STAI_FLAG_OUTPUTS : STAI_FLAG_NONE;

// Check weights status
  for (idx=0; idx<STAI_NETWORK_WEIGHTS_NUM; idx++) {
    if (net_ctx->_weights[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_WEIGHTS_NUM) ? STAI_FLAG_WEIGHTS : STAI_FLAG_NONE;
STAI_PRINT("  [_stai_network_check] flags: 0x%08x\n", net_ctx->_flags)
}


/*****************************************************************************/
STAI_API_ENTRY
stai_return_code stai_network_init(
  stai_network* network)
{
  /* Memory where to store internal context is provided by applications as a raw byte buffer */
  _stai_network_context* net_ctx = (_stai_network_context*)(network);
  net_ctx->_return_code = STAI_SUCCESS;
  STAI_PRINT("[Entering Network Init] network(%p) context_size(%d)\n", net_ctx, (int32_t)sizeof(_stai_network_context))

  _STAI_SET_ERROR(net_ctx, STAI_NETWORK_CONTEXT_SIZE != sizeof(_stai_network_context),
                 STAI_ERROR_NETWORK_INVALID_CONTEXT_SIZE, net_ctx->_return_code)

  {
    const _stai_network_context _network_context = {
      ._magic = STAI_MAGIC,
      ._signature = STAI_NETWORK_MODEL_SIGNATURE,
      ._flags = STAI_NETWORK_FLAGS,
      ._return_code = STAI_SUCCESS,
      ._callback = NULL,
      ._callback_cookie = NULL,
      ._activations = {
      (stai_ptr)g_network_activations_1
      },
      ._weights = {
      (stai_ptr)g_network_weights_array
      },
      ._inputs = {
    NULL,NULL,NULL,NULL,NULL},
      ._outputs = {
    NULL,NULL,NULL,NULL,NULL},
    };

    // Deep copy of internal context to opaque buffer provided by app
    *net_ctx = _network_context;

    _stai_network_check(net_ctx);
  }

  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_deinit(
  stai_network* network)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  /*  Reset flags to initial state  */
  net_ctx->_flags = STAI_NETWORK_FLAGS;
  return net_ctx->_return_code;
}

/*****************************************************************************/



/* Int quant #0 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(input_1_QuantizeLinear_Output_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.02113228291273117f),
    AI_PACK_INTQ_ZP(-4)))

/* Int quant #1 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.024968156591057777f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #2 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0076367976143956184f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #3 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.01194307953119278f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #4 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0069281235337257385f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #5 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.01194307953119278f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #6 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.021733799949288368f),
    AI_PACK_INTQ_ZP(25)))

/* Int quant #7 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.008502530865371227f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #8 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.10068073123693466f),
    AI_PACK_INTQ_ZP(32)))

/* Int quant #9 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.018017422407865524f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #10 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.03734692558646202f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #11 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0387788750231266f),
    AI_PACK_INTQ_ZP(-18)))

/* Int quant #12 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007169339805841446f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #13 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.05319920554757118f),
    AI_PACK_INTQ_ZP(28)))

/* Int quant #14 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.011778467334806919f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #15 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.02056172862648964f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #16 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15RH__5_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.05647076293826103f),
    AI_PACK_INTQ_ZP(3)))

/* Int quant #17 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15WX__3_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2184317260980606f),
    AI_PACK_INTQ_ZP(3)))

/* Int quant #18 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15WXRH__7_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.21233989298343658f),
    AI_PACK_INTQ_ZP(-2)))

/* Int quant #19 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_0_9_output0_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.21233989298343658f),
    AI_PACK_INTQ_ZP(-2)))

/* Int quant #20 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_0_9_output1_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.21233989298343658f),
    AI_PACK_INTQ_ZP(-2)))

/* Int quant #21 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_0_9_output2_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.21233989298343658f),
    AI_PACK_INTQ_ZP(-2)))

/* Int quant #22 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_0_9_output3_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.21233989298343658f),
    AI_PACK_INTQ_ZP(-2)))

/* Int quant #23 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_i_t_14_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921568859368563f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #24 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_o_t_16_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921568859368563f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #25 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_f_t_18_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921568859368563f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #26 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_c_inp_QuantizeLinear_Output_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.17858465015888214f),
    AI_PACK_INTQ_ZP(56)))

/* Int quant #27 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_ft_mul_C_t1_22_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.17192235589027405f),
    AI_PACK_INTQ_ZP(57)))

/* Int quant #28 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_c_t_20_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007843137718737125f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #29 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_it_mul_c_t_24_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007842693477869034f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #30 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.17858465015888214f),
    AI_PACK_INTQ_ZP(56)))

/* Int quant #31 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_tanh_C_t_28_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0078431386500597f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #32 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007823663763701916f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #33 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__730_to_chfirst_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007823663763701916f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #34 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19RH__41_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.050968412309885025f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #35 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19WX__39_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.06258121132850647f),
    AI_PACK_INTQ_ZP(-17)))

/* Int quant #36 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19WXRH__43_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08544482290744781f),
    AI_PACK_INTQ_ZP(-6)))

/* Int quant #37 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_0_45_output0_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08544482290744781f),
    AI_PACK_INTQ_ZP(-6)))

/* Int quant #38 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_0_45_output1_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08544482290744781f),
    AI_PACK_INTQ_ZP(-6)))

/* Int quant #39 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_0_45_output2_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08544482290744781f),
    AI_PACK_INTQ_ZP(-6)))

/* Int quant #40 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_0_45_output3_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08544482290744781f),
    AI_PACK_INTQ_ZP(-6)))

/* Int quant #41 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_i_t_50_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921459894627333f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #42 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_o_t_52_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921522293239832f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #43 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_f_t_54_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003921275492757559f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #44 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_c_inp_QuantizeLinear_Output_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2930505871772766f),
    AI_PACK_INTQ_ZP(85)))

/* Int quant #45 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_ft_mul_C_t1_58_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.28806185722351074f),
    AI_PACK_INTQ_ZP(87)))

/* Int quant #46 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_c_t_56_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007843134924769402f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #47 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_it_mul_c_t_60_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007834051735699177f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #48 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.2950662076473236f),
    AI_PACK_INTQ_ZP(85)))

/* Int quant #49 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_tanh_C_t_64_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0078431386500597f),
    AI_PACK_INTQ_ZP(0)))

/* Int quant #50 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007782586384564638f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #51 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(LSTM__1020_to_chfirst_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007782586384564638f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #52 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_concat_10_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.007823663763701916f),
    AI_PACK_INTQ_ZP(-1)))

/* Int quant #53 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.051582761108875275f),
    AI_PACK_INTQ_ZP(-21)))

/* Int quant #54 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_dense_3_Relu0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.030516143888235092f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #55 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.003394300816580653f),
    AI_PACK_INTQ_ZP(53)))

/* Int quant #56 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.026384761556982994f),
    AI_PACK_INTQ_ZP(-46)))

/* Int quant #57 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_concat_20_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.026384761556982994f),
    AI_PACK_INTQ_ZP(-45)))

/* Int quant #58 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(5.085133307147771e-05f),
    AI_PACK_INTQ_ZP(127)))

/* Int quant #59 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(output_1_QuantizeLinear_Input_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.0038805054500699043f),
    AI_PACK_INTQ_ZP(-128)))



/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  input_1_QuantizeLinear_Output_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 123, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 39, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 9, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_bias_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 1, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_scratch0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 58, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 624, AI_STATIC)

/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 16, AI_STATIC)

/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_bias_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 16, AI_STATIC)

/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_scratch0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 4, AI_STATIC)

/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 304, AI_STATIC)

/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 160, AI_STATIC)

/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 48, AI_STATIC)

/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 16, AI_STATIC)

/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_scratch0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 145, AI_STATIC)

/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 160, AI_STATIC)

/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_bias_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 16, AI_STATIC)

/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_scratch0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#18 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 160, AI_STATIC)

/* Array#19 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 80, AI_STATIC)

/* Array#20 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 48, AI_STATIC)

/* Array#21 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_scratch0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 145, AI_STATIC)

/* Array#22 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 80, AI_STATIC)

/* Array#23 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#24 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_bias_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 16, AI_STATIC)

/* Array#25 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_scratch0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#26 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 80, AI_STATIC)

/* Array#27 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15RH__5_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#28 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15WX__3_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#29 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15WXRH__7_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#30 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_0_9_output0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#31 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_0_9_output1_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#32 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_0_9_output2_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#33 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_0_9_output3_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#34 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_0_9_num_or_size_splits_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 1, AI_STATIC)

/* Array#35 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_i_t_14_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#36 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_o_t_16_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#37 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_f_t_18_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#38 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_c_inp_QuantizeLinear_Output_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#39 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_ft_mul_C_t1_22_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#40 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_c_t_20_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#41 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_it_mul_c_t_24_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#42 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#43 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_tanh_C_t_28_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#44 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#45 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__730_to_chfirst_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#46 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19RH__41_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#47 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19WX__39_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#48 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19WXRH__43_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 256, AI_STATIC)

/* Array#49 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_0_45_output0_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#50 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_0_45_output1_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#51 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_0_45_output2_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#52 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_0_45_output3_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#53 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_0_45_num_or_size_splits_array, AI_ARRAY_FORMAT_S32,
  NULL, NULL, 1, AI_STATIC)

/* Array#54 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_i_t_50_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#55 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_o_t_52_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#56 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_f_t_54_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#57 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_c_inp_QuantizeLinear_Output_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#58 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_ft_mul_C_t1_58_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#59 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_c_t_56_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#60 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_it_mul_c_t_60_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#61 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#62 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_tanh_C_t_64_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#63 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#64 */
AI_ARRAY_OBJ_DECLARE(
  LSTM__1020_to_chfirst_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 64, AI_STATIC)

/* Array#65 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_10_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 128, AI_STATIC)

/* Array#66 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 32, AI_STATIC)

/* Array#67 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_Relu0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 32, AI_STATIC)

/* Array#68 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 32, AI_STATIC)

/* Array#69 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1, AI_STATIC)

/* Array#70 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_20_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1, AI_STATIC)

/* Array#71 */
AI_ARRAY_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1, AI_STATIC)

/* Array#72 */
AI_ARRAY_OBJ_DECLARE(
  output_1_QuantizeLinear_Input_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 1, AI_STATIC)



/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_bias, AI_STATIC,
  104, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_bias_array, NULL)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output, AI_STATIC,
  105, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 39, 1), AI_STRIDE_INIT(4, 1, 1, 1, 39),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array_intq)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_scratch0, AI_STATIC,
  106, 0x0,
  AI_SHAPE_INIT(4, 1, 58, 1, 1), AI_STRIDE_INIT(4, 1, 1, 58, 58),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_scratch0_array, NULL)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights, AI_STATIC,
  107, 0x1,
  AI_SHAPE_INIT(4, 1, 3, 3, 1), AI_STRIDE_INIT(4, 1, 1, 1, 3),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights_array, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights_array_intq)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  input_1_QuantizeLinear_Output_output, AI_STATIC,
  108, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 41, 3), AI_STRIDE_INIT(4, 1, 1, 1, 41),
  1, &input_1_QuantizeLinear_Output_output_array, &input_1_QuantizeLinear_Output_output_array_intq)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_bias, AI_STATIC,
  100, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_bias_array, NULL)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output, AI_STATIC,
  101, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 39, 1), AI_STRIDE_INIT(4, 1, 1, 16, 624),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output_array, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output_array_intq)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_scratch0, AI_STATIC,
  102, 0x0,
  AI_SHAPE_INIT(4, 1, 4, 1, 1), AI_STRIDE_INIT(4, 1, 1, 4, 4),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_scratch0_array, NULL)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights, AI_STATIC,
  103, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 16), AI_STRIDE_INIT(4, 1, 1, 16, 16),
  1, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights_array, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights_array_intq)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias, AI_STATIC,
  95, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias_array, NULL)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output, AI_STATIC,
  96, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 10, 1), AI_STRIDE_INIT(4, 1, 1, 16, 160),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array_intq)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_scratch0, AI_STATIC,
  97, 0x0,
  AI_SHAPE_INIT(4, 1, 145, 1, 1), AI_STRIDE_INIT(4, 1, 1, 145, 145),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_scratch0_array, NULL)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights, AI_STATIC,
  98, 0x1,
  AI_SHAPE_INIT(4, 16, 3, 1, 1), AI_STRIDE_INIT(4, 1, 16, 16, 48),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights_array, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights_array_intq)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0, AI_STATIC,
  99, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 19, 1), AI_STRIDE_INIT(4, 1, 1, 16, 304),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0_array, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0_array_intq)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_bias, AI_STATIC,
  88, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_bias_array, NULL)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output, AI_STATIC,
  89, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 10, 1), AI_STRIDE_INIT(4, 1, 1, 16, 160),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array_intq)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_scratch0, AI_STATIC,
  91, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_scratch0_array, NULL)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights, AI_STATIC,
  92, 0x1,
  AI_SHAPE_INIT(4, 16, 1, 1, 16), AI_STRIDE_INIT(4, 1, 16, 256, 256),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights_array, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights_array_intq)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output0, AI_STATIC,
  90, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 1, 10), AI_STRIDE_INIT(4, 1, 1, 16, 16),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array_intq)

/* Tensor #19 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output, AI_STATIC,
  93, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 1, 10), AI_STRIDE_INIT(4, 1, 1, 16, 16),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array_intq)

/* Tensor #20 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output, AI_STATIC,
  85, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 5, 1), AI_STRIDE_INIT(4, 1, 1, 16, 80),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array_intq)

/* Tensor #21 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_scratch0, AI_STATIC,
  86, 0x0,
  AI_SHAPE_INIT(4, 1, 145, 1, 1), AI_STRIDE_INIT(4, 1, 1, 145, 145),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_scratch0_array, NULL)

/* Tensor #22 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights, AI_STATIC,
  87, 0x1,
  AI_SHAPE_INIT(4, 16, 3, 1, 1), AI_STRIDE_INIT(4, 1, 16, 16, 48),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights_array, &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights_array_intq)

/* Tensor #23 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output0, AI_STATIC,
  94, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 10, 1), AI_STRIDE_INIT(4, 1, 1, 16, 160),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array_intq)

/* Tensor #24 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_bias, AI_STATIC,
  78, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_bias_array, NULL)

/* Tensor #25 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output, AI_STATIC,
  79, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 5, 1), AI_STRIDE_INIT(4, 1, 1, 16, 80),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array_intq)

/* Tensor #26 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_scratch0, AI_STATIC,
  81, 0x0,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_scratch0_array, NULL)

/* Tensor #27 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights, AI_STATIC,
  82, 0x1,
  AI_SHAPE_INIT(4, 16, 1, 1, 16), AI_STRIDE_INIT(4, 1, 16, 256, 256),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights_array, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights_array_intq)

/* Tensor #28 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output0, AI_STATIC,
  80, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 1, 5), AI_STRIDE_INIT(4, 1, 1, 16, 16),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array_intq)

/* Tensor #29 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output, AI_STATIC,
  83, 0x1,
  AI_SHAPE_INIT(4, 1, 16, 1, 5), AI_STRIDE_INIT(4, 1, 1, 16, 16),
  1, &StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output_array, &StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output_array_intq)

/* Tensor #30 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15RH__5_output, AI_STATIC,
  35, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 1, 1, 256, 256),
  1, &LSTM__73_15RH__5_output_array, &LSTM__73_15RH__5_output_array_intq)

/* Tensor #31 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15WXRH__7_output, AI_STATIC,
  38, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 1, 1, 256, 256),
  1, &LSTM__73_15WXRH__7_output_array, &LSTM__73_15WXRH__7_output_array_intq)

/* Tensor #32 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15WX__3_output, AI_STATIC,
  40, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 1, 1, 256, 256),
  1, &LSTM__73_15WX__3_output_array, &LSTM__73_15WX__3_output_array_intq)

/* Tensor #33 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_0_9_num_or_size_splits, AI_STATIC,
  43, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &LSTM__73_15_0_9_num_or_size_splits_array, NULL)

/* Tensor #34 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_0_9_output0, AI_STATIC,
  44, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_0_9_output0_array, &LSTM__73_15_0_9_output0_array_intq)

/* Tensor #35 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_0_9_output1, AI_STATIC,
  45, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_0_9_output1_array, &LSTM__73_15_0_9_output1_array_intq)

/* Tensor #36 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_0_9_output2, AI_STATIC,
  46, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_0_9_output2_array, &LSTM__73_15_0_9_output2_array_intq)

/* Tensor #37 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_0_9_output3, AI_STATIC,
  47, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_0_9_output3_array, &LSTM__73_15_0_9_output3_array_intq)

/* Tensor #38 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_i_t_14_output, AI_STATIC,
  59, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_i_t_14_output_array, &LSTM__73_15_i_t_14_output_array_intq)

/* Tensor #39 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_o_t_16_output, AI_STATIC,
  61, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_o_t_16_output_array, &LSTM__73_15_o_t_16_output_array_intq)

/* Tensor #40 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_f_t_18_output, AI_STATIC,
  53, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_f_t_18_output_array, &LSTM__73_15_f_t_18_output_array_intq)

/* Tensor #41 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_c_inp_QuantizeLinear_Output_output, AI_STATIC,
  48, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_c_inp_QuantizeLinear_Output_output_array, &LSTM__73_15_c_inp_QuantizeLinear_Output_output_array_intq)

/* Tensor #42 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_ft_mul_C_t1_22_output, AI_STATIC,
  54, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_ft_mul_C_t1_22_output_array, &LSTM__73_15_ft_mul_C_t1_22_output_array_intq)

/* Tensor #43 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_c_t_20_output, AI_STATIC,
  52, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_c_t_20_output_array, &LSTM__73_15_c_t_20_output_array_intq)

/* Tensor #44 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_it_mul_c_t_24_output, AI_STATIC,
  60, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_it_mul_c_t_24_output_array, &LSTM__73_15_it_mul_c_t_24_output_array_intq)

/* Tensor #45 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_c_out_26_QuantizeLinear_Input_output, AI_STATIC,
  51, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array, &LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array_intq)

/* Tensor #46 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_tanh_C_t_28_output, AI_STATIC,
  62, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_tanh_C_t_28_output_array, &LSTM__73_15_tanh_C_t_28_output_array_intq)

/* Tensor #47 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__73_15_h_out_30_QuantizeLinear_Input_output, AI_STATIC,
  58, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array, &LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array_intq)

/* Tensor #48 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__730_to_chfirst_output, AI_STATIC,
  31, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 64, 1), AI_STRIDE_INIT(4, 1, 1, 1, 64),
  1, &LSTM__730_to_chfirst_output_array, &LSTM__730_to_chfirst_output_array_intq)

/* Tensor #49 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19RH__41_output, AI_STATIC,
  3, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 1, 1, 256, 256),
  1, &LSTM__102_19RH__41_output_array, &LSTM__102_19RH__41_output_array_intq)

/* Tensor #50 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19WXRH__43_output, AI_STATIC,
  6, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 1, 1, 256, 256),
  1, &LSTM__102_19WXRH__43_output_array, &LSTM__102_19WXRH__43_output_array_intq)

/* Tensor #51 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19WX__39_output, AI_STATIC,
  8, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 1, 1), AI_STRIDE_INIT(4, 1, 1, 256, 256),
  1, &LSTM__102_19WX__39_output_array, &LSTM__102_19WX__39_output_array_intq)

/* Tensor #52 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_0_45_num_or_size_splits, AI_STATIC,
  11, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 4, 4, 4, 4),
  1, &LSTM__102_19_0_45_num_or_size_splits_array, NULL)

/* Tensor #53 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_0_45_output0, AI_STATIC,
  12, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_0_45_output0_array, &LSTM__102_19_0_45_output0_array_intq)

/* Tensor #54 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_0_45_output1, AI_STATIC,
  13, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_0_45_output1_array, &LSTM__102_19_0_45_output1_array_intq)

/* Tensor #55 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_0_45_output2, AI_STATIC,
  14, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_0_45_output2_array, &LSTM__102_19_0_45_output2_array_intq)

/* Tensor #56 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_0_45_output3, AI_STATIC,
  15, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_0_45_output3_array, &LSTM__102_19_0_45_output3_array_intq)

/* Tensor #57 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_i_t_50_output, AI_STATIC,
  27, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_i_t_50_output_array, &LSTM__102_19_i_t_50_output_array_intq)

/* Tensor #58 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_o_t_52_output, AI_STATIC,
  29, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_o_t_52_output_array, &LSTM__102_19_o_t_52_output_array_intq)

/* Tensor #59 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_f_t_54_output, AI_STATIC,
  21, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_f_t_54_output_array, &LSTM__102_19_f_t_54_output_array_intq)

/* Tensor #60 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_c_inp_QuantizeLinear_Output_output, AI_STATIC,
  16, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_c_inp_QuantizeLinear_Output_output_array, &LSTM__102_19_c_inp_QuantizeLinear_Output_output_array_intq)

/* Tensor #61 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_ft_mul_C_t1_58_output, AI_STATIC,
  22, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_ft_mul_C_t1_58_output_array, &LSTM__102_19_ft_mul_C_t1_58_output_array_intq)

/* Tensor #62 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_c_t_56_output, AI_STATIC,
  20, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_c_t_56_output_array, &LSTM__102_19_c_t_56_output_array_intq)

/* Tensor #63 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_it_mul_c_t_60_output, AI_STATIC,
  28, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_it_mul_c_t_60_output_array, &LSTM__102_19_it_mul_c_t_60_output_array_intq)

/* Tensor #64 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_c_out_62_QuantizeLinear_Input_output, AI_STATIC,
  19, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array, &LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array_intq)

/* Tensor #65 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_tanh_C_t_64_output, AI_STATIC,
  30, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_tanh_C_t_64_output_array, &LSTM__102_19_tanh_C_t_64_output_array_intq)

/* Tensor #66 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__102_19_h_out_66_QuantizeLinear_Input_output, AI_STATIC,
  26, 0x1,
  AI_SHAPE_INIT(4, 1, 64, 1, 1), AI_STRIDE_INIT(4, 1, 1, 64, 64),
  1, &LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array, &LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array_intq)

/* Tensor #67 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__1020_to_chfirst_output, AI_STATIC,
  0, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 64, 1), AI_STRIDE_INIT(4, 1, 1, 1, 64),
  1, &LSTM__1020_to_chfirst_output_array, &LSTM__1020_to_chfirst_output_array_intq)

/* Tensor #68 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__1020_to_chfirst_output0, AI_STATIC,
  1, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &LSTM__1020_to_chfirst_output_array, &LSTM__1020_to_chfirst_output_array_intq)

/* Tensor #69 */
AI_TENSOR_OBJ_DECLARE(
  LSTM__730_to_chfirst_output0, AI_STATIC,
  32, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 64), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &LSTM__730_to_chfirst_output_array, &LSTM__730_to_chfirst_output_array_intq)

/* Tensor #70 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_10_output, AI_STATIC,
  63, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 128), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_concat_10_output_array, &StatefulPartitionedCall_vad_model_concat_10_output_array_intq)

/* Tensor #71 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D, AI_STATIC,
  66, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 32), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array, &StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array_intq)

/* Tensor #72 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_Relu0_output, AI_STATIC,
  67, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 32), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_dense_3_Relu0_output_array, &StatefulPartitionedCall_vad_model_dense_3_Relu0_output_array_intq)

/* Tensor #73 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output0, AI_STATIC,
  71, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 32), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output_array, &StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output_array_intq)

/* Tensor #74 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_20_output, AI_STATIC,
  65, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_concat_20_output_array, &StatefulPartitionedCall_vad_model_concat_20_output_array_intq)

/* Tensor #75 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D, AI_STATIC,
  74, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array, &StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array_intq)

/* Tensor #76 */
AI_TENSOR_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output, AI_STATIC,
  75, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output_array, &StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output_array_intq)

/* Tensor #77 */
AI_TENSOR_OBJ_DECLARE(
  output_1_QuantizeLinear_Input_output, AI_STATIC,
  111, 0x1,
  AI_SHAPE_INIT(4, 1, 1, 1, 1), AI_STRIDE_INIT(4, 1, 1, 1, 1),
  1, &output_1_QuantizeLinear_Input_output_array, &output_1_QuantizeLinear_Input_output_array_intq)


AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&input_1_QuantizeLinear_Output_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_bias, (ai_tensor *)NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_layer, 34,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_integer_SSSA,
  &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_bias, (ai_tensor *)NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_layer, 41,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_integer_SSSA,
  &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias, (ai_tensor *)NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_layer, 47,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_integer_SSSA,
  &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_layer, AI_STATIC, 
  .groups = 16, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 1, 0, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_bias, (ai_tensor *)NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_layer, 50,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_integer_SSSA,
  &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_layer, 53,
  NL_TYPE, 0x0, NULL,
  nl, node_convert_integer,
  &StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias, (ai_tensor *)NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_layer, 59,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_integer_SSSA,
  &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_layer, AI_STATIC, 
  .groups = 16, 
  .filter_stride = AI_SHAPE_2D_INIT(2, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 1), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_bias, (ai_tensor *)NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_scratch0)
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_layer, 62,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_integer_SSSA,
  &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_layer, 65,
  NL_TYPE, 0x0, NULL,
  nl, node_convert_integer,
  &StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_chain,
  NULL, &StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15WXRH__7_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__73_15WX__3_output, (ai_tensor *)&LSTM__73_15RH__5_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15WXRH__7_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15WXRH__7_layer, 77,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__73_15WXRH__7_chain,
  NULL, &LSTM__73_15WXRH__7_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_0_9_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15WXRH__7_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 4, (ai_tensor *)&LSTM__73_15_0_9_output0, (ai_tensor *)&LSTM__73_15_0_9_output1, (ai_tensor *)&LSTM__73_15_0_9_output2, (ai_tensor *)&LSTM__73_15_0_9_output3),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_0_9_num_or_size_splits),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_0_9_layer, 80,
  SPLIT_TYPE, 0x0, NULL,
  split, forward_split,
  &LSTM__73_15_0_9_chain,
  NULL, &LSTM__73_15_0_9_layer, AI_STATIC, 
  .outer_elems = 1, 
  .outer_elems_stride = 256, 
)


AI_STATIC_CONST ai_i8 LSTM__73_15_i_t_14_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -126, -126, -126, -125, -124, -124, -123, -121, -120, -118, -116, -113, -109, -106, -101, -95, -89, -81, -72, -62, -52, -40, -27, -14, -1, 13, 26, 39, 51, 61, 71, 80, 88, 94, 100, 105, 108, 112, 115, 117, 119, 120, 122, 123, 123, 124, 125, 125, 125, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__73_15_i_t_14_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__73_15_i_t_14_nl_params_data, LSTM__73_15_i_t_14_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_i_t_14_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_0_9_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_i_t_14_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_i_t_14_layer, 89,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__73_15_i_t_14_chain,
  NULL, &LSTM__73_15_i_t_14_layer, AI_STATIC, 
  .nl_params = &LSTM__73_15_i_t_14_nl_params, 
)


AI_STATIC_CONST ai_i8 LSTM__73_15_o_t_16_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -126, -126, -126, -125, -124, -124, -123, -121, -120, -118, -116, -113, -109, -106, -101, -95, -89, -81, -72, -62, -52, -40, -27, -14, -1, 13, 26, 39, 51, 61, 71, 80, 88, 94, 100, 105, 108, 112, 115, 117, 119, 120, 122, 123, 123, 124, 125, 125, 125, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__73_15_o_t_16_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__73_15_o_t_16_nl_params_data, LSTM__73_15_o_t_16_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_o_t_16_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_0_9_output1),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_o_t_16_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_o_t_16_layer, 90,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__73_15_o_t_16_chain,
  NULL, &LSTM__73_15_o_t_16_layer, AI_STATIC, 
  .nl_params = &LSTM__73_15_o_t_16_nl_params, 
)


AI_STATIC_CONST ai_i8 LSTM__73_15_f_t_18_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -126, -126, -126, -125, -124, -124, -123, -121, -120, -118, -116, -113, -109, -106, -101, -95, -89, -81, -72, -62, -52, -40, -27, -14, -1, 13, 26, 39, 51, 61, 71, 80, 88, 94, 100, 105, 108, 112, 115, 117, 119, 120, 122, 123, 123, 124, 125, 125, 125, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__73_15_f_t_18_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__73_15_f_t_18_nl_params_data, LSTM__73_15_f_t_18_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_f_t_18_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_0_9_output2),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_f_t_18_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_f_t_18_layer, 91,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__73_15_f_t_18_chain,
  NULL, &LSTM__73_15_f_t_18_layer, AI_STATIC, 
  .nl_params = &LSTM__73_15_f_t_18_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_ft_mul_C_t1_22_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__73_15_f_t_18_output, (ai_tensor *)&LSTM__73_15_c_inp_QuantizeLinear_Output_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_ft_mul_C_t1_22_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_ft_mul_C_t1_22_layer, 101,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__73_15_ft_mul_C_t1_22_chain,
  NULL, &LSTM__73_15_ft_mul_C_t1_22_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)


AI_STATIC_CONST ai_i8 LSTM__73_15_c_t_20_nl_params_data[] = { -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -125, -124, -122, -119, -115, -109, -100, -88, -72, -51, -27, 0, 27, 51, 72, 88, 100, 109, 115, 119, 122, 124, 125, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__73_15_c_t_20_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__73_15_c_t_20_nl_params_data, LSTM__73_15_c_t_20_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_c_t_20_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_0_9_output3),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_c_t_20_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_c_t_20_layer, 92,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__73_15_c_t_20_chain,
  NULL, &LSTM__73_15_c_t_20_layer, AI_STATIC, 
  .nl_params = &LSTM__73_15_c_t_20_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_it_mul_c_t_24_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__73_15_i_t_14_output, (ai_tensor *)&LSTM__73_15_c_t_20_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_it_mul_c_t_24_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_it_mul_c_t_24_layer, 102,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__73_15_it_mul_c_t_24_chain,
  NULL, &LSTM__73_15_it_mul_c_t_24_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_c_out_26_QuantizeLinear_Input_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__73_15_ft_mul_C_t1_22_output, (ai_tensor *)&LSTM__73_15_it_mul_c_t_24_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_c_out_26_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_c_out_26_QuantizeLinear_Input_layer, 107,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__73_15_c_out_26_QuantizeLinear_Input_chain,
  NULL, &LSTM__73_15_c_out_26_QuantizeLinear_Input_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)


AI_STATIC_CONST ai_i8 LSTM__73_15_tanh_C_t_28_nl_params_data[] = { -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -125, -124, -123, -121, -118, -114, -108, -101, -91, -78, -62, -44, -23, 0, 23, 44, 62, 78, 91, 101, 108, 114, 118, 121, 123, 124, 125, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__73_15_tanh_C_t_28_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__73_15_tanh_C_t_28_nl_params_data, LSTM__73_15_tanh_C_t_28_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_tanh_C_t_28_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_c_out_26_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_tanh_C_t_28_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_tanh_C_t_28_layer, 110,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__73_15_tanh_C_t_28_chain,
  NULL, &LSTM__73_15_tanh_C_t_28_layer, AI_STATIC, 
  .nl_params = &LSTM__73_15_tanh_C_t_28_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__73_15_h_out_30_QuantizeLinear_Input_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__73_15_o_t_16_output, (ai_tensor *)&LSTM__73_15_tanh_C_t_28_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_h_out_30_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__73_15_h_out_30_QuantizeLinear_Input_layer, 113,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__73_15_h_out_30_QuantizeLinear_Input_chain,
  NULL, &LSTM__73_15_h_out_30_QuantizeLinear_Input_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__730_to_chfirst_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__73_15_h_out_30_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__730_to_chfirst_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__730_to_chfirst_layer, 116,
  TRANSPOSE_TYPE, 0x0, NULL,
  transpose, forward_transpose,
  &LSTM__730_to_chfirst_chain,
  NULL, &LSTM__730_to_chfirst_layer, AI_STATIC, /* AI_LAYER_IS_STATEFUL flags=0x0 */
  .out_mapping = AI_SHAPE_INIT(6, AI_SHAPE_IN_CHANNEL, AI_SHAPE_HEIGHT, AI_SHAPE_CHANNEL, AI_SHAPE_WIDTH, AI_SHAPE_DEPTH, AI_SHAPE_EXTENSION), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19WXRH__43_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__102_19WX__39_output, (ai_tensor *)&LSTM__102_19RH__41_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19WXRH__43_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19WXRH__43_layer, 131,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__102_19WXRH__43_chain,
  NULL, &LSTM__102_19WXRH__43_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_0_45_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19WXRH__43_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 4, (ai_tensor *)&LSTM__102_19_0_45_output0, (ai_tensor *)&LSTM__102_19_0_45_output1, (ai_tensor *)&LSTM__102_19_0_45_output2, (ai_tensor *)&LSTM__102_19_0_45_output3),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_0_45_num_or_size_splits),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_0_45_layer, 134,
  SPLIT_TYPE, 0x0, NULL,
  split, forward_split,
  &LSTM__102_19_0_45_chain,
  NULL, &LSTM__102_19_0_45_layer, AI_STATIC, 
  .outer_elems = 1, 
  .outer_elems_stride = 256, 
)


AI_STATIC_CONST ai_i8 LSTM__102_19_i_t_50_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -124, -124, -124, -123, -123, -123, -122, -122, -121, -121, -120, -119, -118, -118, -117, -116, -115, -114, -112, -111, -110, -108, -107, -105, -103, -101, -99, -97, -94, -92, -89, -86, -83, -80, -76, -73, -69, -65, -61, -56, -52, -47, -42, -38, -32, -27, -22, -17, -11, -6, 0, 5, 10, 16, 21, 26, 31, 37, 41, 46, 51, 55, 60, 64, 68, 72, 75, 79, 82, 85, 88, 91, 93, 96, 98, 100, 102, 104, 106, 107, 109, 110, 111, 113, 114, 115, 116, 117, 117, 118, 119, 120, 120, 121, 121, 122, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__102_19_i_t_50_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__102_19_i_t_50_nl_params_data, LSTM__102_19_i_t_50_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_i_t_50_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_0_45_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_i_t_50_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_i_t_50_layer, 143,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__102_19_i_t_50_chain,
  NULL, &LSTM__102_19_i_t_50_layer, AI_STATIC, 
  .nl_params = &LSTM__102_19_i_t_50_nl_params, 
)


AI_STATIC_CONST ai_i8 LSTM__102_19_o_t_52_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -124, -124, -124, -123, -123, -123, -122, -122, -121, -121, -120, -119, -118, -118, -117, -116, -115, -114, -112, -111, -110, -108, -107, -105, -103, -101, -99, -97, -94, -92, -89, -86, -83, -80, -76, -73, -69, -65, -61, -56, -52, -47, -42, -38, -32, -27, -22, -17, -11, -6, 0, 5, 10, 16, 21, 26, 31, 37, 41, 46, 51, 55, 60, 64, 68, 72, 75, 79, 82, 85, 88, 91, 93, 96, 98, 100, 102, 104, 106, 107, 109, 110, 111, 113, 114, 115, 116, 117, 117, 118, 119, 120, 120, 121, 121, 122, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__102_19_o_t_52_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__102_19_o_t_52_nl_params_data, LSTM__102_19_o_t_52_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_o_t_52_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_0_45_output1),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_o_t_52_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_o_t_52_layer, 144,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__102_19_o_t_52_chain,
  NULL, &LSTM__102_19_o_t_52_layer, AI_STATIC, 
  .nl_params = &LSTM__102_19_o_t_52_nl_params, 
)


AI_STATIC_CONST ai_i8 LSTM__102_19_f_t_54_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -124, -124, -124, -123, -123, -123, -122, -122, -121, -121, -120, -119, -118, -118, -117, -116, -115, -114, -112, -111, -110, -108, -107, -105, -103, -101, -99, -97, -94, -92, -89, -86, -83, -80, -76, -73, -69, -65, -61, -56, -52, -47, -42, -38, -32, -27, -22, -17, -11, -6, 0, 5, 10, 16, 21, 26, 31, 37, 41, 46, 51, 55, 60, 64, 68, 72, 75, 79, 82, 85, 88, 91, 93, 96, 98, 100, 102, 104, 106, 107, 109, 110, 111, 113, 114, 115, 116, 117, 117, 118, 119, 120, 120, 121, 121, 122, 122, 123, 123, 123, 124, 124, 124, 124, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__102_19_f_t_54_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__102_19_f_t_54_nl_params_data, LSTM__102_19_f_t_54_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_f_t_54_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_0_45_output2),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_f_t_54_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_f_t_54_layer, 145,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__102_19_f_t_54_chain,
  NULL, &LSTM__102_19_f_t_54_layer, AI_STATIC, 
  .nl_params = &LSTM__102_19_f_t_54_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_ft_mul_C_t1_58_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__102_19_f_t_54_output, (ai_tensor *)&LSTM__102_19_c_inp_QuantizeLinear_Output_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_ft_mul_C_t1_58_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_ft_mul_C_t1_58_layer, 155,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__102_19_ft_mul_C_t1_58_chain,
  NULL, &LSTM__102_19_ft_mul_C_t1_58_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)


AI_STATIC_CONST ai_i8 LSTM__102_19_c_t_56_nl_params_data[] = { -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -127, -127, -127, -127, -126, -126, -126, -125, -124, -124, -123, -122, -120, -119, -117, -115, -113, -110, -107, -104, -99, -95, -89, -83, -77, -69, -61, -52, -43, -33, -23, -12, -1, 10, 21, 31, 41, 50, 59, 67, 75, 81, 87, 93, 97, 102, 105, 108, 111, 113, 115, 117, 118, 120, 121, 122, 122, 123, 124, 124, 124, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__102_19_c_t_56_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__102_19_c_t_56_nl_params_data, LSTM__102_19_c_t_56_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_c_t_56_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_0_45_output3),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_c_t_56_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_c_t_56_layer, 146,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__102_19_c_t_56_chain,
  NULL, &LSTM__102_19_c_t_56_layer, AI_STATIC, 
  .nl_params = &LSTM__102_19_c_t_56_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_it_mul_c_t_60_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__102_19_i_t_50_output, (ai_tensor *)&LSTM__102_19_c_t_56_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_it_mul_c_t_60_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_it_mul_c_t_60_layer, 156,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__102_19_it_mul_c_t_60_chain,
  NULL, &LSTM__102_19_it_mul_c_t_60_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_c_out_62_QuantizeLinear_Input_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__102_19_ft_mul_C_t1_58_output, (ai_tensor *)&LSTM__102_19_it_mul_c_t_60_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_c_out_62_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_c_out_62_QuantizeLinear_Input_layer, 161,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__102_19_c_out_62_QuantizeLinear_Input_chain,
  NULL, &LSTM__102_19_c_out_62_QuantizeLinear_Input_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)


AI_STATIC_CONST ai_i8 LSTM__102_19_tanh_C_t_64_nl_params_data[] = { -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -125, -123, -120, -115, -106, -90, -68, -37, 0, 37, 68, 90, 106, 115, 120, 123, 125, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    LSTM__102_19_tanh_C_t_64_nl_params, AI_ARRAY_FORMAT_S8,
    LSTM__102_19_tanh_C_t_64_nl_params_data, LSTM__102_19_tanh_C_t_64_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_tanh_C_t_64_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_c_out_62_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_tanh_C_t_64_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_tanh_C_t_64_layer, 164,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &LSTM__102_19_tanh_C_t_64_chain,
  NULL, &LSTM__102_19_tanh_C_t_64_layer, AI_STATIC, 
  .nl_params = &LSTM__102_19_tanh_C_t_64_nl_params, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__102_19_h_out_66_QuantizeLinear_Input_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__102_19_o_t_52_output, (ai_tensor *)&LSTM__102_19_tanh_C_t_64_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_h_out_66_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__102_19_h_out_66_QuantizeLinear_Input_layer, 167,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &LSTM__102_19_h_out_66_QuantizeLinear_Input_chain,
  NULL, &LSTM__102_19_h_out_66_QuantizeLinear_Input_layer, AI_STATIC, 
  .operation = ai_mul_f32, 
  .buffer_operation = ai_mul_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  LSTM__1020_to_chfirst_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__102_19_h_out_66_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&LSTM__1020_to_chfirst_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  LSTM__1020_to_chfirst_layer, 170,
  TRANSPOSE_TYPE, 0x0, NULL,
  transpose, forward_transpose,
  &LSTM__1020_to_chfirst_chain,
  NULL, &LSTM__1020_to_chfirst_layer, AI_STATIC, /* AI_LAYER_IS_STATEFUL flags=0x0 */
  .out_mapping = AI_SHAPE_INIT(6, AI_SHAPE_IN_CHANNEL, AI_SHAPE_HEIGHT, AI_SHAPE_CHANNEL, AI_SHAPE_WIDTH, AI_SHAPE_DEPTH, AI_SHAPE_EXTENSION), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_10_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&LSTM__1020_to_chfirst_output0, (ai_tensor *)&LSTM__730_to_chfirst_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_concat_10_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_10_layer, 179,
  CONCAT_TYPE, 0x0, NULL,
  concat, forward_concat,
  &StatefulPartitionedCall_vad_model_concat_10_chain,
  NULL, &StatefulPartitionedCall_vad_model_concat_10_layer, AI_STATIC, 
  .axis = AI_SHAPE_HEIGHT, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_Relu0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output0, (ai_tensor *)&StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_dense_3_Relu0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_dense_3_Relu0_layer, 191,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &StatefulPartitionedCall_vad_model_dense_3_Relu0_chain,
  NULL, &StatefulPartitionedCall_vad_model_dense_3_Relu0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_20_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, (ai_tensor *)&StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output, (ai_tensor *)&StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_concat_20_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  StatefulPartitionedCall_vad_model_concat_20_layer, 203,
  ELTWISE_INTEGER_TYPE, 0x0, NULL,
  eltwise_integer, forward_eltwise_integer_INT8,
  &StatefulPartitionedCall_vad_model_concat_20_chain,
  NULL, &StatefulPartitionedCall_vad_model_concat_20_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_INT8, 
)


AI_STATIC_CONST ai_i8 output_1_QuantizeLinear_Input_nl_params_data[] = { -102, -101, -101, -100, -99, -99, -98, -97, -97, -96, -95, -94, -94, -93, -92, -91, -90, -90, -89, -88, -87, -86, -85, -84, -83, -82, -81, -80, -79, -78, -77, -76, -75, -74, -72, -71, -70, -69, -68, -67, -65, -64, -63, -61, -60, -59, -57, -56, -55, -53, -52, -51, -49, -48, -46, -45, -43, -42, -40, -39, -37, -36, -34, -32, -31, -29, -28, -26, -24, -23, -21, -19, -18, -16, -14, -13, -11, -9, -8, -6, -4, -3, -1, 1, 3, 4, 6, 8, 9, 11, 13, 14, 16, 18, 19, 21, 23, 24, 26, 28, 29, 31, 32, 34, 36, 37, 39, 40, 42, 43, 45, 46, 48, 49, 51, 52, 54, 55, 56, 58, 59, 61, 62, 63, 64, 66, 67, 68, 69, 71, 72, 73, 74, 75, 76, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 90, 91, 92, 93, 94, 95, 95, 96, 97, 98, 98, 99, 100, 101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111, 112, 112, 113, 113, 113, 114, 114, 115, 115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 120, 121, 121, 121, 121, 121, 122, 122, 122, 122, 122, 123, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124, 124, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127 };
AI_ARRAY_OBJ_DECLARE(
    output_1_QuantizeLinear_Input_nl_params, AI_ARRAY_FORMAT_S8,
    output_1_QuantizeLinear_Input_nl_params_data, output_1_QuantizeLinear_Input_nl_params_data, 256, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  output_1_QuantizeLinear_Input_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&StatefulPartitionedCall_vad_model_concat_20_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, (ai_tensor *)&output_1_QuantizeLinear_Input_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  output_1_QuantizeLinear_Input_layer, 206,
  NL_TYPE, 0x0, NULL,
  nl, forward_nl_integer,
  &output_1_QuantizeLinear_Input_chain,
  NULL, &output_1_QuantizeLinear_Input_layer, AI_STATIC, 
  .nl_params = &output_1_QuantizeLinear_Input_nl_params, 
)
/**  Hybrid layers declarations section  *************************************/

LITE_FORCE_INLINE void forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_network(_stai_network_context* net_ctx)
{
  input_1_QuantizeLinear_Output_output_array.data = AI_PTR(net_ctx->_activations[0] + 1136);
  input_1_QuantizeLinear_Output_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1136);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights_array.data = AI_PTR(net_ctx->_weights[0] + 34852);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 34852);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_bias_array.data = AI_PTR(net_ctx->_weights[0] + 34864);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 34864);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 1388);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 1388);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array.data = AI_PTR(net_ctx->_activations[0] + 1476);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1476);
  _STAI_NETWORK_EVENT_NODE_START_CB(34, 1, { input_1_QuantizeLinear_Output_output.data->data});
  forward_conv2d_integer_SSSA((ai_layer *)&StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(34, 1, { StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array.data = AI_PTR(net_ctx->_activations[0] + 1476);
  StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1476);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights_array.data = AI_PTR(net_ctx->_weights[0] + 34868);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 34868);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_bias_array.data = AI_PTR(net_ctx->_weights[0] + 34884);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 34884);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 1516);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 1516);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output_array.data = AI_PTR(net_ctx->_activations[0] + 636);
  StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 636);
  _STAI_NETWORK_EVENT_NODE_START_CB(41, 1, { StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_output.data->data});
  forward_conv2d_integer_SSSA((ai_layer *)&StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(41, 1, { StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0_array.data = AI_PTR(net_ctx->_activations[0] + 1388);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0_array.data_start = AI_PTR(net_ctx->_activations[0] + 1388);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights_array.data = AI_PTR(net_ctx->_weights[0] + 34948);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 34948);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias_array.data = AI_PTR(net_ctx->_weights[0] + 34996);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 34996);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array.data = AI_PTR(net_ctx->_activations[0] + 660);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 660);
  _STAI_NETWORK_EVENT_NODE_START_CB(47, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_output0.data->data});
  forward_conv2d_integer_SSSA((ai_layer *)&StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(47, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array.data = AI_PTR(net_ctx->_activations[0] + 660);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 660);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights_array.data = AI_PTR(net_ctx->_weights[0] + 35060);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 35060);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_bias_array.data = AI_PTR(net_ctx->_weights[0] + 35316);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 35316);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array.data = AI_PTR(net_ctx->_activations[0] + 820);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 820);
  _STAI_NETWORK_EVENT_NODE_START_CB(50, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_output.data->data});
  forward_conv2d_integer_SSSA((ai_layer *)&StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(50, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_node_convert_integer_StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array.data = AI_PTR(net_ctx->_activations[0] + 820);
  StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 820);
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  _STAI_NETWORK_EVENT_NODE_START_CB(53, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_output0.data->data});
  node_convert_integer((ai_node *)&StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(53, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights_array.data = AI_PTR(net_ctx->_weights[0] + 35380);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 35380);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias_array.data = AI_PTR(net_ctx->_weights[0] + 34996);
  StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 34996);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 672);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 672);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array.data = AI_PTR(net_ctx->_activations[0] + 820);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 820);
  _STAI_NETWORK_EVENT_NODE_START_CB(59, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_output0.data->data});
  forward_conv2d_integer_SSSA((ai_layer *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(59, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array.data = AI_PTR(net_ctx->_activations[0] + 820);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 820);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights_array.data = AI_PTR(net_ctx->_weights[0] + 35428);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_weights_array.data_start = AI_PTR(net_ctx->_weights[0] + 35428);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_bias_array.data = AI_PTR(net_ctx->_weights[0] + 35684);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_bias_array.data_start = AI_PTR(net_ctx->_weights[0] + 35684);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_scratch0_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_scratch0_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array.data = AI_PTR(net_ctx->_activations[0] + 576);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 576);
  _STAI_NETWORK_EVENT_NODE_START_CB(62, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_output.data->data});
  forward_conv2d_integer_SSSA((ai_layer *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(62, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_node_convert_integer_StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array.data = AI_PTR(net_ctx->_activations[0] + 576);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 576);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output_array.data = AI_PTR(net_ctx->_activations[0] + 656);
  StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 656);
  _STAI_NETWORK_EVENT_NODE_START_CB(65, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_output0.data->data});
  node_convert_integer((ai_node *)&StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(65, 1, { StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__73_15WXRH__7_network(_stai_network_context* net_ctx)
{
  LSTM__73_15WX__3_output_array.data = AI_PTR(net_ctx->_activations[0] + 896);
  LSTM__73_15WX__3_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 896);
  LSTM__73_15RH__5_output_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15RH__5_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15WXRH__7_output_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  LSTM__73_15WXRH__7_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  _STAI_NETWORK_EVENT_NODE_START_CB(77, 2, { LSTM__73_15WX__3_output.data->data,LSTM__73_15RH__5_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__73_15WXRH__7_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(77, 1, { LSTM__73_15WXRH__7_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_split_LSTM__73_15_0_9_network(_stai_network_context* net_ctx)
{
  LSTM__73_15WXRH__7_output_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  LSTM__73_15WXRH__7_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  LSTM__73_15_0_9_num_or_size_splits_array.data = AI_PTR(net_ctx->_weights[0] + 57252);
  LSTM__73_15_0_9_num_or_size_splits_array.data_start = AI_PTR(net_ctx->_weights[0] + 57252);
  LSTM__73_15_0_9_output3_array.data = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_0_9_output3_array.data_start = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_0_9_output2_array.data = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_0_9_output2_array.data_start = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_0_9_output1_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_0_9_output1_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_0_9_output0_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15_0_9_output0_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  _STAI_NETWORK_EVENT_NODE_START_CB(80, 1, { LSTM__73_15WXRH__7_output.data->data});
  forward_split((ai_layer *)&LSTM__73_15_0_9_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(80, 4, { LSTM__73_15_0_9_output0.data->data,LSTM__73_15_0_9_output1.data->data,LSTM__73_15_0_9_output2.data->data,LSTM__73_15_0_9_output3.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__73_15_i_t_14_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_0_9_output0_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15_0_9_output0_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15_i_t_14_output_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  LSTM__73_15_i_t_14_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  _STAI_NETWORK_EVENT_NODE_START_CB(89, 1, { LSTM__73_15_0_9_output0.data->data});
  forward_nl_integer((ai_layer *)&LSTM__73_15_i_t_14_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(89, 1, { LSTM__73_15_i_t_14_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__73_15_o_t_16_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_0_9_output1_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_0_9_output1_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_o_t_16_output_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15_o_t_16_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  _STAI_NETWORK_EVENT_NODE_START_CB(90, 1, { LSTM__73_15_0_9_output1.data->data});
  forward_nl_integer((ai_layer *)&LSTM__73_15_o_t_16_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(90, 1, { LSTM__73_15_o_t_16_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__73_15_f_t_18_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_0_9_output2_array.data = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_0_9_output2_array.data_start = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_f_t_18_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_f_t_18_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  _STAI_NETWORK_EVENT_NODE_START_CB(91, 1, { LSTM__73_15_0_9_output2.data->data});
  forward_nl_integer((ai_layer *)&LSTM__73_15_f_t_18_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(91, 1, { LSTM__73_15_f_t_18_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__73_15_ft_mul_C_t1_22_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_f_t_18_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_f_t_18_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_c_inp_QuantizeLinear_Output_output_array.data = AI_PTR(net_ctx->_activations[0] + 1324);
  LSTM__73_15_c_inp_QuantizeLinear_Output_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1324);
  LSTM__73_15_ft_mul_C_t1_22_output_array.data = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_ft_mul_C_t1_22_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 384);
  _STAI_NETWORK_EVENT_NODE_START_CB(101, 2, { LSTM__73_15_f_t_18_output.data->data,LSTM__73_15_c_inp_QuantizeLinear_Output_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__73_15_ft_mul_C_t1_22_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(101, 1, { LSTM__73_15_ft_mul_C_t1_22_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__73_15_c_t_20_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_0_9_output3_array.data = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_0_9_output3_array.data_start = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_c_t_20_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_c_t_20_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  _STAI_NETWORK_EVENT_NODE_START_CB(92, 1, { LSTM__73_15_0_9_output3.data->data});
  forward_nl_integer((ai_layer *)&LSTM__73_15_c_t_20_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(92, 1, { LSTM__73_15_c_t_20_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__73_15_it_mul_c_t_24_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_i_t_14_output_array.data = AI_PTR(net_ctx->_activations[0] + 512);
  LSTM__73_15_i_t_14_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 512);
  LSTM__73_15_c_t_20_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_c_t_20_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_it_mul_c_t_24_output_array.data = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_it_mul_c_t_24_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 448);
  _STAI_NETWORK_EVENT_NODE_START_CB(102, 2, { LSTM__73_15_i_t_14_output.data->data,LSTM__73_15_c_t_20_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__73_15_it_mul_c_t_24_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(102, 1, { LSTM__73_15_it_mul_c_t_24_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__73_15_c_out_26_QuantizeLinear_Input_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_ft_mul_C_t1_22_output_array.data = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_ft_mul_C_t1_22_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 384);
  LSTM__73_15_it_mul_c_t_24_output_array.data = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_it_mul_c_t_24_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 448);
  LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  _STAI_NETWORK_EVENT_NODE_START_CB(107, 2, { LSTM__73_15_ft_mul_C_t1_22_output.data->data,LSTM__73_15_it_mul_c_t_24_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__73_15_c_out_26_QuantizeLinear_Input_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(107, 1, { LSTM__73_15_c_out_26_QuantizeLinear_Input_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__73_15_tanh_C_t_28_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_c_out_26_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_tanh_C_t_28_output_array.data = AI_PTR(net_ctx->_activations[0] + 640);
  LSTM__73_15_tanh_C_t_28_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 640);
  _STAI_NETWORK_EVENT_NODE_START_CB(110, 1, { LSTM__73_15_c_out_26_QuantizeLinear_Input_output.data->data});
  forward_nl_integer((ai_layer *)&LSTM__73_15_tanh_C_t_28_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(110, 1, { LSTM__73_15_tanh_C_t_28_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__73_15_h_out_30_QuantizeLinear_Input_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_o_t_16_output_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15_o_t_16_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__73_15_tanh_C_t_28_output_array.data = AI_PTR(net_ctx->_activations[0] + 640);
  LSTM__73_15_tanh_C_t_28_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 640);
  LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  _STAI_NETWORK_EVENT_NODE_START_CB(113, 2, { LSTM__73_15_o_t_16_output.data->data,LSTM__73_15_tanh_C_t_28_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__73_15_h_out_30_QuantizeLinear_Input_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(113, 1, { LSTM__73_15_h_out_30_QuantizeLinear_Input_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_transpose_LSTM__730_to_chfirst_network(_stai_network_context* net_ctx)
{
  LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__73_15_h_out_30_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__730_to_chfirst_output_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__730_to_chfirst_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  _STAI_NETWORK_EVENT_NODE_START_CB(116, 1, { LSTM__73_15_h_out_30_QuantizeLinear_Input_output.data->data});
  forward_transpose((ai_layer *)&LSTM__730_to_chfirst_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(116, 1, { LSTM__730_to_chfirst_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__102_19WXRH__43_network(_stai_network_context* net_ctx)
{
  LSTM__102_19WX__39_output_array.data = AI_PTR(net_ctx->_activations[0] + 1324);
  LSTM__102_19WX__39_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1324);
  LSTM__102_19RH__41_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19RH__41_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19WXRH__43_output_array.data = AI_PTR(net_ctx->_activations[0] + 896);
  LSTM__102_19WXRH__43_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 896);
  _STAI_NETWORK_EVENT_NODE_START_CB(131, 2, { LSTM__102_19WX__39_output.data->data,LSTM__102_19RH__41_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__102_19WXRH__43_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(131, 1, { LSTM__102_19WXRH__43_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_split_LSTM__102_19_0_45_network(_stai_network_context* net_ctx)
{
  LSTM__102_19WXRH__43_output_array.data = AI_PTR(net_ctx->_activations[0] + 896);
  LSTM__102_19WXRH__43_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 896);
  LSTM__102_19_0_45_num_or_size_splits_array.data = AI_PTR(net_ctx->_weights[0] + 74664);
  LSTM__102_19_0_45_num_or_size_splits_array.data_start = AI_PTR(net_ctx->_weights[0] + 74664);
  LSTM__102_19_0_45_output3_array.data = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_0_45_output3_array.data_start = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_0_45_output2_array.data = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_0_45_output2_array.data_start = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_0_45_output1_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_0_45_output1_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_0_45_output0_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19_0_45_output0_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  _STAI_NETWORK_EVENT_NODE_START_CB(134, 1, { LSTM__102_19WXRH__43_output.data->data});
  forward_split((ai_layer *)&LSTM__102_19_0_45_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(134, 4, { LSTM__102_19_0_45_output0.data->data,LSTM__102_19_0_45_output1.data->data,LSTM__102_19_0_45_output2.data->data,LSTM__102_19_0_45_output3.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__102_19_i_t_50_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_0_45_output0_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19_0_45_output0_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19_i_t_50_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__102_19_i_t_50_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  _STAI_NETWORK_EVENT_NODE_START_CB(143, 1, { LSTM__102_19_0_45_output0.data->data});
  forward_nl_integer((ai_layer *)&LSTM__102_19_i_t_50_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(143, 1, { LSTM__102_19_i_t_50_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__102_19_o_t_52_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_0_45_output1_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_0_45_output1_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_o_t_52_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19_o_t_52_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  _STAI_NETWORK_EVENT_NODE_START_CB(144, 1, { LSTM__102_19_0_45_output1.data->data});
  forward_nl_integer((ai_layer *)&LSTM__102_19_o_t_52_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(144, 1, { LSTM__102_19_o_t_52_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__102_19_f_t_54_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_0_45_output2_array.data = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_0_45_output2_array.data_start = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_f_t_54_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_f_t_54_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  _STAI_NETWORK_EVENT_NODE_START_CB(145, 1, { LSTM__102_19_0_45_output2.data->data});
  forward_nl_integer((ai_layer *)&LSTM__102_19_f_t_54_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(145, 1, { LSTM__102_19_f_t_54_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__102_19_ft_mul_C_t1_58_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_f_t_54_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_f_t_54_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_c_inp_QuantizeLinear_Output_output_array.data = AI_PTR(net_ctx->_activations[0] + 1260);
  LSTM__102_19_c_inp_QuantizeLinear_Output_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 1260);
  LSTM__102_19_ft_mul_C_t1_58_output_array.data = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_ft_mul_C_t1_58_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 128);
  _STAI_NETWORK_EVENT_NODE_START_CB(155, 2, { LSTM__102_19_f_t_54_output.data->data,LSTM__102_19_c_inp_QuantizeLinear_Output_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__102_19_ft_mul_C_t1_58_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(155, 1, { LSTM__102_19_ft_mul_C_t1_58_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__102_19_c_t_56_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_0_45_output3_array.data = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_0_45_output3_array.data_start = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_c_t_56_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_c_t_56_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  _STAI_NETWORK_EVENT_NODE_START_CB(146, 1, { LSTM__102_19_0_45_output3.data->data});
  forward_nl_integer((ai_layer *)&LSTM__102_19_c_t_56_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(146, 1, { LSTM__102_19_c_t_56_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__102_19_it_mul_c_t_60_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_i_t_50_output_array.data = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__102_19_i_t_50_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 320);
  LSTM__102_19_c_t_56_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_c_t_56_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_it_mul_c_t_60_output_array.data = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_it_mul_c_t_60_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 192);
  _STAI_NETWORK_EVENT_NODE_START_CB(156, 2, { LSTM__102_19_i_t_50_output.data->data,LSTM__102_19_c_t_56_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__102_19_it_mul_c_t_60_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(156, 1, { LSTM__102_19_it_mul_c_t_60_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__102_19_c_out_62_QuantizeLinear_Input_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_ft_mul_C_t1_58_output_array.data = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_ft_mul_C_t1_58_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_it_mul_c_t_60_output_array.data = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_it_mul_c_t_60_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 192);
  LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  _STAI_NETWORK_EVENT_NODE_START_CB(161, 2, { LSTM__102_19_ft_mul_C_t1_58_output.data->data,LSTM__102_19_it_mul_c_t_60_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__102_19_c_out_62_QuantizeLinear_Input_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(161, 1, { LSTM__102_19_c_out_62_QuantizeLinear_Input_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_LSTM__102_19_tanh_C_t_64_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_c_out_62_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_tanh_C_t_64_output_array.data = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_tanh_C_t_64_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 128);
  _STAI_NETWORK_EVENT_NODE_START_CB(164, 1, { LSTM__102_19_c_out_62_QuantizeLinear_Input_output.data->data});
  forward_nl_integer((ai_layer *)&LSTM__102_19_tanh_C_t_64_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(164, 1, { LSTM__102_19_tanh_C_t_64_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_LSTM__102_19_h_out_66_QuantizeLinear_Input_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_o_t_52_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19_o_t_52_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__102_19_tanh_C_t_64_output_array.data = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_tanh_C_t_64_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 128);
  LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  _STAI_NETWORK_EVENT_NODE_START_CB(167, 2, { LSTM__102_19_o_t_52_output.data->data,LSTM__102_19_tanh_C_t_64_output.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&LSTM__102_19_h_out_66_QuantizeLinear_Input_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(167, 1, { LSTM__102_19_h_out_66_QuantizeLinear_Input_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_transpose_LSTM__1020_to_chfirst_network(_stai_network_context* net_ctx)
{
  LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__102_19_h_out_66_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  LSTM__1020_to_chfirst_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__1020_to_chfirst_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  _STAI_NETWORK_EVENT_NODE_START_CB(170, 1, { LSTM__102_19_h_out_66_QuantizeLinear_Input_output.data->data});
  forward_transpose((ai_layer *)&LSTM__1020_to_chfirst_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(170, 1, { LSTM__1020_to_chfirst_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_concat_StatefulPartitionedCall_vad_model_concat_10_network(_stai_network_context* net_ctx)
{
  LSTM__1020_to_chfirst_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__1020_to_chfirst_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  LSTM__730_to_chfirst_output_array.data = AI_PTR(net_ctx->_activations[0] + 256);
  LSTM__730_to_chfirst_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 256);
  StatefulPartitionedCall_vad_model_concat_10_output_array.data = AI_PTR(net_ctx->_activations[0] + 64);
  StatefulPartitionedCall_vad_model_concat_10_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 64);
  _STAI_NETWORK_EVENT_NODE_START_CB(179, 2, { LSTM__1020_to_chfirst_output0.data->data,LSTM__730_to_chfirst_output0.data->data});
  forward_concat((ai_layer *)&StatefulPartitionedCall_vad_model_concat_10_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(179, 1, { StatefulPartitionedCall_vad_model_concat_10_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_StatefulPartitionedCall_vad_model_dense_3_Relu0_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array.data = AI_PTR(net_ctx->_weights[0] + 4);
  StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array.data_start = AI_PTR(net_ctx->_weights[0] + 4);
  StatefulPartitionedCall_vad_model_dense_3_Relu0_output_array.data = AI_PTR(net_ctx->_activations[0] + 32);
  StatefulPartitionedCall_vad_model_dense_3_Relu0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 32);
  _STAI_NETWORK_EVENT_NODE_START_CB(191, 2, { StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_output0.data->data,StatefulPartitionedCall_vad_model_dense_3_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&StatefulPartitionedCall_vad_model_dense_3_Relu0_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(191, 1, { StatefulPartitionedCall_vad_model_dense_3_Relu0_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_eltwise_integer_INT8_StatefulPartitionedCall_vad_model_concat_20_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array.data = AI_PTR(net_ctx->_weights[0] + 0);
  StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D_array.data_start = AI_PTR(net_ctx->_weights[0] + 0);
  StatefulPartitionedCall_vad_model_concat_20_output_array.data = AI_PTR(net_ctx->_activations[0] + 4);
  StatefulPartitionedCall_vad_model_concat_20_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 4);
  _STAI_NETWORK_EVENT_NODE_START_CB(203, 2, { StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_output.data->data,StatefulPartitionedCall_vad_model_dense_5_BiasAdd_ReadVariableOp0_DequantizeLinear_Output_const_3D.data->data});
  forward_eltwise_integer_INT8((ai_layer *)&StatefulPartitionedCall_vad_model_concat_20_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(203, 1, { StatefulPartitionedCall_vad_model_concat_20_output.data->data});
}

LITE_FORCE_INLINE void forward_lite_nl_integer_output_1_QuantizeLinear_Input_network(_stai_network_context* net_ctx)
{
  StatefulPartitionedCall_vad_model_concat_20_output_array.data = AI_PTR(net_ctx->_activations[0] + 4);
  StatefulPartitionedCall_vad_model_concat_20_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 4);
  output_1_QuantizeLinear_Input_output_array.data = AI_PTR(net_ctx->_activations[0] + 0);
  output_1_QuantizeLinear_Input_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 0);
  _STAI_NETWORK_EVENT_NODE_START_CB(206, 1, { StatefulPartitionedCall_vad_model_concat_20_output.data->data});
  forward_nl_integer((ai_layer *)&output_1_QuantizeLinear_Input_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(206, 1, { output_1_QuantizeLinear_Input_output.data->data});
}

/*****************************************************************************/


static const ai_u32 LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32 = 0.2930505871772766f;
static const ai_i8 LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8 = 85;

static const ai_u32 LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32 = 0.007782586384564638f;
static const ai_i8 LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8 = -1;

static const ai_i8 LSTM__102_19RH__41_t_in_0_fmt_zero_const_s8 = -1;
static const ai_i8 LSTM__102_19RH__41_t_out_0_fmt_zero_const_s8 = -1;
static const ai_u16 LSTM__102_19RH__41_t_in_0_shape_ch_const_u16 = 64;
static const ai_u16 LSTM__102_19RH__41_t_out_0_shape_ch_const_u16 = 256;
static const ai_u32 LSTM__102_19RH__41_t_out_0_shape_h_w_prod_const_u32 = 1;
static const ai_float LSTM__102_19RH__41_t_in_0_fmt_scale_const_f32 = 0.007782586384564638f;
static const ai_float LSTM__102_19RH__41_t_out_0_fmt_scale_const_f32 = 0.050968412309885025f;
static const ai_float LSTM__102_19RH__41_t_weight_0_fmt_scale_const_f32 = 0.011019394733011723f;

static const ai_u32 LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32 = 0.17858465015888214f;
static const ai_i8 LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8 = 56;

static const ai_u32 LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32 = 0.00782366469502449f;
static const ai_i8 LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8 = -1;

static const ai_i8 LSTM__73_15RH__5_t_in_0_fmt_zero_const_s8 = -1;
static const ai_i8 LSTM__73_15RH__5_t_out_0_fmt_zero_const_s8 = 3;
static const ai_u16 LSTM__73_15RH__5_t_in_0_shape_ch_const_u16 = 64;
static const ai_u16 LSTM__73_15RH__5_t_out_0_shape_ch_const_u16 = 256;
static const ai_u32 LSTM__73_15RH__5_t_out_0_shape_h_w_prod_const_u32 = 1;
static const ai_float LSTM__73_15RH__5_t_in_0_fmt_scale_const_f32 = 0.00782366469502449f;
static const ai_float LSTM__73_15RH__5_t_out_0_fmt_scale_const_f32 = 0.05647076293826103f;
static const ai_float LSTM__73_15RH__5_t_weight_0_fmt_scale_const_f32 = 0.012531476095318794f;

static const ai_u32 input_1_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32 = 123;
static const ai_float input_1_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32 = 0.02113228291273117f;
static const ai_i8 input_1_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8 = -4;



static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_shape_w_const_u16 = 39;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_shape_h_const_u16 = 1;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_size_1_const_u16 = 3;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_size_0_const_u16 = 1;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_legacy_pool_pad_1_const_u16 = 0;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_legacy_pool_pad_0_const_u16 = 0;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_stride_1_const_u16 = 2;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_stride_0_const_u16 = 1;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_shape_w_const_u16 = 19;
static const ai_u16 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_shape_h_const_u16 = 1;
static const ai_i8 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_fmt_zero_const_s8 = -128;







static const ai_i8 LSTM__73_15WX__3_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 LSTM__73_15WX__3_t_out_0_fmt_zero_const_s8 = 3;
static const ai_u16 LSTM__73_15WX__3_t_in_0_shape_ch_const_u16 = 80;
static const ai_u16 LSTM__73_15WX__3_t_out_0_shape_ch_const_u16 = 256;
static const ai_u32 LSTM__73_15WX__3_t_out_0_shape_h_w_prod_const_u32 = 1;
static const ai_float LSTM__73_15WX__3_t_in_0_fmt_scale_const_f32 = 0.02056172862648964f;
static const ai_float LSTM__73_15WX__3_t_out_0_fmt_scale_const_f32 = 0.2184317260980606f;
static const ai_float LSTM__73_15WX__3_t_weight_0_fmt_scale_const_f32 = 0.017174772918224335f;










static const ai_u32 LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32 = 0.17858465015888214f;
static const ai_i8 LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8 = 56;



static const ai_u32 LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32 = 0.007823663763701916f;
static const ai_i8 LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8 = -1;


static const ai_i8 LSTM__102_19WX__39_t_in_0_fmt_zero_const_s8 = -1;
static const ai_i8 LSTM__102_19WX__39_t_out_0_fmt_zero_const_s8 = -17;
static const ai_u16 LSTM__102_19WX__39_t_in_0_shape_ch_const_u16 = 64;
static const ai_u16 LSTM__102_19WX__39_t_out_0_shape_ch_const_u16 = 256;
static const ai_u32 LSTM__102_19WX__39_t_out_0_shape_h_w_prod_const_u32 = 1;
static const ai_float LSTM__102_19WX__39_t_in_0_fmt_scale_const_f32 = 0.007823663763701916f;
static const ai_float LSTM__102_19WX__39_t_out_0_fmt_scale_const_f32 = 0.06258121132850647f;
static const ai_float LSTM__102_19WX__39_t_weight_0_fmt_scale_const_f32 = 0.015963705256581306f;










static const ai_u32 LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32 = 0.2950662076473236f;
static const ai_i8 LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8 = 85;



static const ai_u32 LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32 = 64;
static const ai_float LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32 = 0.007782586384564638f;
static const ai_i8 LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8 = -1;



static const ai_i8 StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_fmt_zero_const_s8 = -1;
static const ai_i8 StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_fmt_zero_const_s8 = -21;
static const ai_u16 StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_shape_ch_const_u16 = 128;
static const ai_u16 StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_shape_ch_const_u16 = 32;
static const ai_u32 StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_shape_h_w_prod_const_u32 = 1;
static const ai_float StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_fmt_scale_const_f32 = 0.007823663763701916f;
static const ai_float StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_fmt_scale_const_f32 = 0.051582761108875275f;
static const ai_float StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_weight_0_fmt_scale_const_f32 = 0.00910287257283926f;


static const ai_i8 StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_fmt_zero_const_s8 = -46;
static const ai_u16 StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_shape_ch_const_u16 = 32;
static const ai_u16 StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_shape_ch_const_u16 = 1;
static const ai_u32 StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_shape_h_w_prod_const_u32 = 1;
static const ai_float StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_fmt_scale_const_f32 = 0.030516143888235092f;
static const ai_float StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_fmt_scale_const_f32 = 0.026384761556982994f;
static const ai_float StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_weight_0_fmt_scale_const_f32 = 0.0031123824883252382f;



static const ai_u32 output_1_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32 = 1;
static const ai_float output_1_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32 = 0.0038805054500699043f;
static const ai_i8 output_1_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8 = -128;
STAI_API_ENTRY
stai_return_code stai_network_run(
  stai_network* network,
  const stai_run_mode mode)
{
   STAI_UNUSED(mode)
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_ACTIVATIONS) != STAI_FLAG_ACTIVATIONS,
        STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR, net_ctx->_return_code)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_INPUTS) != STAI_FLAG_INPUTS,
                  STAI_ERROR_NETWORK_INVALID_IN_PTR, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_OUTPUTS) != STAI_FLAG_OUTPUTS,
                  STAI_ERROR_NETWORK_INVALID_OUT_PTR, net_ctx->_return_code)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_WEIGHTS) != STAI_FLAG_WEIGHTS,
                  STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR, net_ctx->_return_code)


  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_c_inp_QuantizeLinear_Output */
  {
      const ai_float* LSTM__102_19_c_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32 = (ai_float*)(net_ctx->_inputs[4] + 0);
    ai_i8* LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1260);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(5, 1, {(stai_ptr) LSTM__102_19_c_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32});
    
  forward_lite_node_convert_integer_if32os8(LSTM__102_19_c_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32, LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_ptr_s8, LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32, LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(5, 1, {(stai_ptr) LSTM__102_19_c_inp_QuantizeLinear_Output_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_c_inp_QuantizeLinear_Output */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_h_inp_QuantizeLinear_Output */
  {
      const ai_float* LSTM__102_19_h_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32 = (ai_float*)(net_ctx->_inputs[3] + 0);
    ai_i8* LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1324);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(6, 1, {(stai_ptr) LSTM__102_19_h_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32});
    
  forward_lite_node_convert_integer_if32os8(LSTM__102_19_h_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32, LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_ptr_s8, LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32, LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(6, 1, {(stai_ptr) LSTM__102_19_h_inp_QuantizeLinear_Output_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_h_inp_QuantizeLinear_Output */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19RH__41 */
  {
      ai_i8* LSTM__102_19RH__41_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    const ai_i8* LSTM__102_19RH__41_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1324);
    const ai_i8* LSTM__102_19RH__41_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 36);
    const ai_i32* LSTM__102_19RH__41_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 16420);
    ai_i16* LSTM__102_19RH__41_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1388);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(32, 1, {(stai_ptr) LSTM__102_19RH__41_t_in_0_ptr_const_s8});
    
  forward_lite_dense_is8os8ws8(LSTM__102_19RH__41_t_out_0_ptr_s8, LSTM__102_19RH__41_t_in_0_ptr_const_s8, LSTM__102_19RH__41_t_weight_0_ptr_const_s8, LSTM__102_19RH__41_t_weight_1_ptr_const_s32, LSTM__102_19RH__41_t_in_0_fmt_zero_const_s8, LSTM__102_19RH__41_t_out_0_fmt_zero_const_s8, LSTM__102_19RH__41_t_in_0_shape_ch_const_u16, LSTM__102_19RH__41_t_out_0_shape_ch_const_u16, LSTM__102_19RH__41_t_out_0_shape_h_w_prod_const_u32, LSTM__102_19RH__41_t_in_0_fmt_scale_const_f32, LSTM__102_19RH__41_t_out_0_fmt_scale_const_f32, LSTM__102_19RH__41_t_weight_0_fmt_scale_const_f32, LSTM__102_19RH__41_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(32, 1, {(stai_ptr) LSTM__102_19RH__41_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19RH__41 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_c_inp_QuantizeLinear_Output */
  {
      const ai_float* LSTM__73_15_c_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32 = (ai_float*)(net_ctx->_inputs[2] + 0);
    ai_i8* LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1324);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(11, 1, {(stai_ptr) LSTM__73_15_c_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32});
    
  forward_lite_node_convert_integer_if32os8(LSTM__73_15_c_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32, LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_ptr_s8, LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32, LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(11, 1, {(stai_ptr) LSTM__73_15_c_inp_QuantizeLinear_Output_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_c_inp_QuantizeLinear_Output */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_h_inp_QuantizeLinear_Output */
  {
      const ai_float* LSTM__73_15_h_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32 = (ai_float*)(net_ctx->_inputs[1] + 0);
    ai_i8* LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1004);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(12, 1, {(stai_ptr) LSTM__73_15_h_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32});
    
  forward_lite_node_convert_integer_if32os8(LSTM__73_15_h_inp_QuantizeLinear_Output_t_in_0_ptr_const_f32, LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_ptr_s8, LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32, LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(12, 1, {(stai_ptr) LSTM__73_15_h_inp_QuantizeLinear_Output_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_h_inp_QuantizeLinear_Output */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15RH__5 */
  {
      ai_i8* LSTM__73_15RH__5_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 256);
    const ai_i8* LSTM__73_15RH__5_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1004);
    const ai_i8* LSTM__73_15RH__5_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 17444);
    const ai_i32* LSTM__73_15RH__5_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 33828);
    ai_i16* LSTM__73_15RH__5_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1068);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(33, 1, {(stai_ptr) LSTM__73_15RH__5_t_in_0_ptr_const_s8});
    
  forward_lite_dense_is8os8ws8(LSTM__73_15RH__5_t_out_0_ptr_s8, LSTM__73_15RH__5_t_in_0_ptr_const_s8, LSTM__73_15RH__5_t_weight_0_ptr_const_s8, LSTM__73_15RH__5_t_weight_1_ptr_const_s32, LSTM__73_15RH__5_t_in_0_fmt_zero_const_s8, LSTM__73_15RH__5_t_out_0_fmt_zero_const_s8, LSTM__73_15RH__5_t_in_0_shape_ch_const_u16, LSTM__73_15RH__5_t_out_0_shape_ch_const_u16, LSTM__73_15RH__5_t_out_0_shape_h_w_prod_const_u32, LSTM__73_15RH__5_t_in_0_fmt_scale_const_f32, LSTM__73_15RH__5_t_out_0_fmt_scale_const_f32, LSTM__73_15RH__5_t_weight_0_fmt_scale_const_f32, LSTM__73_15RH__5_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(33, 1, {(stai_ptr) LSTM__73_15RH__5_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15RH__5 */
  /* LITE_KERNEL_SECTION BEGIN input_1_QuantizeLinear_Output */
  {
      const ai_float* input_1_QuantizeLinear_Output_t_in_0_ptr_const_f32 = (ai_float*)(net_ctx->_inputs[0] + 0);
    ai_i8* input_1_QuantizeLinear_Output_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1136);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(26, 1, {(stai_ptr) input_1_QuantizeLinear_Output_t_in_0_ptr_const_f32});
    
  forward_lite_node_convert_integer_if32os8(input_1_QuantizeLinear_Output_t_in_0_ptr_const_f32, input_1_QuantizeLinear_Output_t_out_0_ptr_s8, input_1_QuantizeLinear_Output_t_out_0_shape_h_w_ch_d_prod_const_u32, input_1_QuantizeLinear_Output_t_out_0_fmt_scale_const_f32, input_1_QuantizeLinear_Output_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(26, 1, {(stai_ptr) input_1_QuantizeLinear_Output_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END input_1_QuantizeLinear_Output */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0 */
  {
    
  forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv2d_separable_conv2d_depthwise0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv2d_Relu0 */
  {
    
  forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv2d_Relu0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv2d_Relu0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270 */
  {
      const ai_i8* StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 636);
    ai_i8* StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1388);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(44, 1, {(stai_ptr) StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_ptr_const_s8});
    
  forward_lite_maxpool_is8os8_scalepos(StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_ptr_const_s8, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_ptr_s8, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_shape_w_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_shape_h_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_shape_ch_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_size_1_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_size_0_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_legacy_pool_pad_1_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_legacy_pool_pad_0_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_stride_1_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_l_pool_stride_0_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_shape_w_const_u16, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_shape_h_const_u16, 1.0f, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_in_0_fmt_zero_const_s8, StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(44, 1, {(stai_ptr) StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise__1270 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0 */
  {
    
  forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_separable_conv2d_depthwise0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0 */
  {
    
  forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_BiasAdd0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion */
  {
    
  forward_lite_node_convert_integer_StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_Relu0_0_conversion */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0 */
  {
    
  forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_1_separable_conv2d_depthwise0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0 */
  {
    
  forward_lite_conv2d_integer_SSSA_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion */
  {
    
  forward_lite_node_convert_integer_StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_separable_conv1d_1_Relu0_0_0_StatefulPartitionedCall_vad_model_separable_conv1d_1_BiasAdd__1470_conversion */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15WX__3 */
  {
      ai_i8* LSTM__73_15WX__3_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 896);
    const ai_i8* LSTM__73_15WX__3_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 656);
    const ai_i8* LSTM__73_15WX__3_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 35748);
    const ai_i32* LSTM__73_15WX__3_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 56228);
    ai_i16* LSTM__73_15WX__3_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 736);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(74, 1, {(stai_ptr) LSTM__73_15WX__3_t_in_0_ptr_const_s8});
    
  forward_lite_dense_is8os8ws8(LSTM__73_15WX__3_t_out_0_ptr_s8, LSTM__73_15WX__3_t_in_0_ptr_const_s8, LSTM__73_15WX__3_t_weight_0_ptr_const_s8, LSTM__73_15WX__3_t_weight_1_ptr_const_s32, LSTM__73_15WX__3_t_in_0_fmt_zero_const_s8, LSTM__73_15WX__3_t_out_0_fmt_zero_const_s8, LSTM__73_15WX__3_t_in_0_shape_ch_const_u16, LSTM__73_15WX__3_t_out_0_shape_ch_const_u16, LSTM__73_15WX__3_t_out_0_shape_h_w_prod_const_u32, LSTM__73_15WX__3_t_in_0_fmt_scale_const_f32, LSTM__73_15WX__3_t_out_0_fmt_scale_const_f32, LSTM__73_15WX__3_t_weight_0_fmt_scale_const_f32, LSTM__73_15WX__3_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(74, 1, {(stai_ptr) LSTM__73_15WX__3_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15WX__3 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15WXRH__7 */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__73_15WXRH__7_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15WXRH__7 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_0_9 */
  {
    
  forward_lite_split_LSTM__73_15_0_9_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_0_9 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_i_t_14 */
  {
    
  forward_lite_nl_integer_LSTM__73_15_i_t_14_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_i_t_14 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_o_t_16 */
  {
    
  forward_lite_nl_integer_LSTM__73_15_o_t_16_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_o_t_16 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_f_t_18 */
  {
    
  forward_lite_nl_integer_LSTM__73_15_f_t_18_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_f_t_18 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_ft_mul_C_t1_22 */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__73_15_ft_mul_C_t1_22_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_ft_mul_C_t1_22 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_c_t_20 */
  {
    
  forward_lite_nl_integer_LSTM__73_15_c_t_20_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_c_t_20 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_it_mul_c_t_24 */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__73_15_it_mul_c_t_24_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_it_mul_c_t_24 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_c_out_26_QuantizeLinear_Input */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__73_15_c_out_26_QuantizeLinear_Input_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_c_out_26_QuantizeLinear_Input */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion */
  {
      const ai_i8* LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 320);
    ai_float* LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32 = (ai_float*)(net_ctx->_outputs[2] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(107, 1, {(stai_ptr) LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8});
    
  forward_lite_node_convert_integer_is8of32(LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8, LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32, LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32, LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(107, 1, {(stai_ptr) LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32});
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_c_out_26_QuantizeLinear_Input_0_conversion */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_tanh_C_t_28 */
  {
    
  forward_lite_nl_integer_LSTM__73_15_tanh_C_t_28_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_tanh_C_t_28 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_h_out_30_QuantizeLinear_Input */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__73_15_h_out_30_QuantizeLinear_Input_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_h_out_30_QuantizeLinear_Input */
  /* LITE_KERNEL_SECTION BEGIN LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion */
  {
      const ai_i8* LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 320);
    ai_float* LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32 = (ai_float*)(net_ctx->_outputs[1] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(113, 1, {(stai_ptr) LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8});
    
  forward_lite_node_convert_integer_is8of32(LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8, LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32, LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32, LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(113, 1, {(stai_ptr) LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32});
  }
  /* LITE_KERNEL_SECTION END LSTM__73_15_h_out_30_QuantizeLinear_Input_0_conversion */
  /* LITE_KERNEL_SECTION BEGIN LSTM__730_to_chfirst */
  {
    
  forward_lite_transpose_LSTM__730_to_chfirst_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__730_to_chfirst */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19WX__39 */
  {
      ai_i8* LSTM__102_19WX__39_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1324);
    const ai_i8* LSTM__102_19WX__39_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 256);
    const ai_i8* LSTM__102_19WX__39_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 57256);
    const ai_i32* LSTM__102_19WX__39_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 73640);
    ai_i16* LSTM__102_19WX__39_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 896);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(128, 1, {(stai_ptr) LSTM__102_19WX__39_t_in_0_ptr_const_s8});
    
  forward_lite_dense_is8os8ws8(LSTM__102_19WX__39_t_out_0_ptr_s8, LSTM__102_19WX__39_t_in_0_ptr_const_s8, LSTM__102_19WX__39_t_weight_0_ptr_const_s8, LSTM__102_19WX__39_t_weight_1_ptr_const_s32, LSTM__102_19WX__39_t_in_0_fmt_zero_const_s8, LSTM__102_19WX__39_t_out_0_fmt_zero_const_s8, LSTM__102_19WX__39_t_in_0_shape_ch_const_u16, LSTM__102_19WX__39_t_out_0_shape_ch_const_u16, LSTM__102_19WX__39_t_out_0_shape_h_w_prod_const_u32, LSTM__102_19WX__39_t_in_0_fmt_scale_const_f32, LSTM__102_19WX__39_t_out_0_fmt_scale_const_f32, LSTM__102_19WX__39_t_weight_0_fmt_scale_const_f32, LSTM__102_19WX__39_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(128, 1, {(stai_ptr) LSTM__102_19WX__39_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19WX__39 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19WXRH__43 */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__102_19WXRH__43_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19WXRH__43 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_0_45 */
  {
    
  forward_lite_split_LSTM__102_19_0_45_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_0_45 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_i_t_50 */
  {
    
  forward_lite_nl_integer_LSTM__102_19_i_t_50_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_i_t_50 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_o_t_52 */
  {
    
  forward_lite_nl_integer_LSTM__102_19_o_t_52_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_o_t_52 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_f_t_54 */
  {
    
  forward_lite_nl_integer_LSTM__102_19_f_t_54_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_f_t_54 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_ft_mul_C_t1_58 */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__102_19_ft_mul_C_t1_58_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_ft_mul_C_t1_58 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_c_t_56 */
  {
    
  forward_lite_nl_integer_LSTM__102_19_c_t_56_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_c_t_56 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_it_mul_c_t_60 */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__102_19_it_mul_c_t_60_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_it_mul_c_t_60 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_c_out_62_QuantizeLinear_Input */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__102_19_c_out_62_QuantizeLinear_Input_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_c_out_62_QuantizeLinear_Input */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion */
  {
      const ai_i8* LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 64);
    ai_float* LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32 = (ai_float*)(net_ctx->_outputs[4] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(161, 1, {(stai_ptr) LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8});
    
  forward_lite_node_convert_integer_is8of32(LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8, LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32, LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32, LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(161, 1, {(stai_ptr) LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32});
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_c_out_62_QuantizeLinear_Input_0_conversion */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_tanh_C_t_64 */
  {
    
  forward_lite_nl_integer_LSTM__102_19_tanh_C_t_64_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_tanh_C_t_64 */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_h_out_66_QuantizeLinear_Input */
  {
    
  forward_lite_eltwise_integer_INT8_LSTM__102_19_h_out_66_QuantizeLinear_Input_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_h_out_66_QuantizeLinear_Input */
  /* LITE_KERNEL_SECTION BEGIN LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion */
  {
      const ai_i8* LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 64);
    ai_float* LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32 = (ai_float*)(net_ctx->_outputs[3] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(167, 1, {(stai_ptr) LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8});
    
  forward_lite_node_convert_integer_is8of32(LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8, LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32, LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32, LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32, LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(167, 1, {(stai_ptr) LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32});
  }
  /* LITE_KERNEL_SECTION END LSTM__102_19_h_out_66_QuantizeLinear_Input_0_conversion */
  /* LITE_KERNEL_SECTION BEGIN LSTM__1020_to_chfirst */
  {
    
  forward_lite_transpose_LSTM__1020_to_chfirst_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END LSTM__1020_to_chfirst */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_concat_10 */
  {
    
  forward_lite_concat_StatefulPartitionedCall_vad_model_concat_10_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_concat_10 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0 */
  {
      ai_i8* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    const ai_i8* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 64);
    const ai_i8* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 74668);
    const ai_i32* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 78764);
    ai_i16* StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 1408);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(185, 1, {(stai_ptr) StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_ptr_const_s8});
    
  forward_lite_dense_is8os8ws8(StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_ptr_s8, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_ptr_const_s8, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_weight_0_ptr_const_s8, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_weight_1_ptr_const_s32, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_fmt_zero_const_s8, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_fmt_zero_const_s8, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_shape_ch_const_u16, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_shape_ch_const_u16, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_shape_h_w_prod_const_u32, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_in_0_fmt_scale_const_f32, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_fmt_scale_const_f32, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_weight_0_fmt_scale_const_f32, StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(185, 1, {(stai_ptr) StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_dense_3_Tensordot_MatMul0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_dense_3_Relu0 */
  {
    
  forward_lite_eltwise_integer_INT8_StatefulPartitionedCall_vad_model_dense_3_Relu0_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_dense_3_Relu0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0 */
  {
      ai_i8* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    const ai_i8* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 32);
    const ai_i8* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 78892);
    const ai_i32* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 34864);
    ai_i16* StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 64);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(197, 1, {(stai_ptr) StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_ptr_const_s8});
    
  forward_lite_dense_is8os8ws8(StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_ptr_s8, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_ptr_const_s8, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_weight_0_ptr_const_s8, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_weight_1_ptr_const_s32, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_fmt_zero_const_s8, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_fmt_zero_const_s8, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_shape_ch_const_u16, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_shape_ch_const_u16, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_shape_h_w_prod_const_u32, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_in_0_fmt_scale_const_f32, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_fmt_scale_const_f32, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_weight_0_fmt_scale_const_f32, StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(197, 1, {(stai_ptr) StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_dense_5_Tensordot_MatMul0 */
  /* LITE_KERNEL_SECTION BEGIN StatefulPartitionedCall_vad_model_concat_20 */
  {
    
  forward_lite_eltwise_integer_INT8_StatefulPartitionedCall_vad_model_concat_20_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END StatefulPartitionedCall_vad_model_concat_20 */
  /* LITE_KERNEL_SECTION BEGIN output_1_QuantizeLinear_Input */
  {
    
  forward_lite_nl_integer_output_1_QuantizeLinear_Input_network(net_ctx);
  }
  /* LITE_KERNEL_SECTION END output_1_QuantizeLinear_Input */
  /* LITE_KERNEL_SECTION BEGIN output_1_QuantizeLinear_Input_0_conversion */
  {
      const ai_i8* output_1_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    ai_float* output_1_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32 = (ai_float*)(net_ctx->_outputs[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(206, 1, {(stai_ptr) output_1_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8});
    
  forward_lite_node_convert_integer_is8of32(output_1_QuantizeLinear_Input_0_conversion_t_in_0_ptr_const_s8, output_1_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32, output_1_QuantizeLinear_Input_0_conversion_t_out_0_shape_h_w_ch_d_prod_const_u32, output_1_QuantizeLinear_Input_0_conversion_t_in_0_fmt_scale_const_f32, output_1_QuantizeLinear_Input_0_conversion_t_in_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(206, 1, {(stai_ptr) output_1_QuantizeLinear_Input_0_conversion_t_out_0_ptr_f32});
  }
  /* LITE_KERNEL_SECTION END output_1_QuantizeLinear_Input_0_conversion */
  return net_ctx->_return_code;
}

/*****************************************************************************/
/*  Getters APIs Section  */
STAI_API_ENTRY
stai_size stai_network_get_context_size()
{
  return (stai_size)STAI_NETWORK_CONTEXT_SIZE;
}

#if defined(HAVE_NETWORK_INFO)
STAI_API_ENTRY
stai_return_code stai_network_get_info(
  stai_network* network,
  stai_network_info* info)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, info==NULL, STAI_ERROR_NETWORK_INVALID_INFO, net_ctx->_return_code)

  // Copy of network info struct
  *info = g_network_info;

  return STAI_SUCCESS;
}
#endif


STAI_API_ENTRY
stai_return_code stai_network_get_activations(
  stai_network* network, stai_ptr* activations, stai_size* n_activations)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  _STAI_SET_ERROR(net_ctx, !n_activations, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_activations = STAI_NETWORK_ACTIVATIONS_NUM;
for (stai_size idx=0; activations && (idx<STAI_NETWORK_ACTIVATIONS_NUM); idx++) {
    // get address of the activations buffers
    activations[idx] = net_ctx->_activations[idx];
  }return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_weights(
  stai_network* network, stai_ptr* weights, stai_size* n_weights)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_weights, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_weights = STAI_NETWORK_WEIGHTS_NUM;
for (stai_size idx=0; weights && (idx<STAI_NETWORK_WEIGHTS_NUM); idx++) {
    // get address of the weights buffers
    weights[idx] = net_ctx->_weights[idx];
  }return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_inputs(
  stai_network* network, stai_ptr* inputs, stai_size* n_inputs)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_inputs, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_inputs = STAI_NETWORK_IN_NUM;
  for (stai_size idx=0; inputs && (idx<STAI_NETWORK_IN_NUM); idx++) {
    inputs[idx] = net_ctx->_inputs[idx];
  }
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_outputs(
  stai_network* network, stai_ptr* outputs, stai_size* n_outputs)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_outputs, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_outputs = STAI_NETWORK_OUT_NUM;
  for (stai_size idx=0; outputs && (idx<STAI_NETWORK_OUT_NUM); idx++) {
    outputs[idx] = net_ctx->_outputs[idx];
  }
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_error(
  stai_network* network)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  /* return 1st generated error or STAI_SUCCESS if no errors so far */
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_states(
  stai_network* network, stai_ptr* states, stai_size* n_states)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_states, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  /* get the number of internals states (supporting multi-heap also for internal states) */
  *n_states = STAI_NETWORK_STATES_NUM;

  STAI_UNUSED(states)
return net_ctx->_return_code;
}


/*****************************************************************************/
/*  Setters APIs Section  */

STAI_API_ENTRY
stai_return_code stai_network_set_activations(
  stai_network* network,
  const stai_ptr* activations,
  const stai_size n_activations)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
const uintptr_t _activations_alignment[] = STAI_NETWORK_ACTIVATIONS_ALIGNMENTS;
  STAI_PRINT("  [stai_network_set_activations] network(%p) activations[%d]: %p\n\n", net_ctx, n_activations, activations)
  _STAI_SET_ERROR(net_ctx, !activations,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_activations!=STAI_NETWORK_ACTIVATIONS_NUM,
                  STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_NUM, net_ctx->_return_code)

  for (stai_size idx=0; activations && idx<STAI_NETWORK_ACTIVATIONS_NUM; idx++) {
    STAI_PRINT("  activation[%d]: %p\n", idx, activations[idx])
    _STAI_SET_ERROR(net_ctx, activations[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)activations[idx]) & (_activations_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_activations[idx] = activations[idx];
  }
  net_ctx->_inputs[0] = activations[0] + 512;

  net_ctx->_inputs[1] = activations[0] + 256;

  net_ctx->_inputs[2] = activations[0] + 1004;

  net_ctx->_inputs[3] = activations[0] + 0;

  net_ctx->_inputs[4] = activations[0] + 1260;

  net_ctx->_outputs[0] = activations[0] + 4;

  net_ctx->_outputs[1] = activations[0] + 640;

  net_ctx->_outputs[2] = activations[0] + 384;

  net_ctx->_outputs[3] = activations[0] + 1152;

  net_ctx->_outputs[4] = activations[0] + 896;
_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_weights(
  stai_network* network,
  const stai_ptr* weights,
  const stai_size n_weights)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
const uintptr_t _weights_alignment[] = STAI_NETWORK_WEIGHTS_ALIGNMENTS;
  _STAI_SET_ERROR(net_ctx, !weights,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_weights!=STAI_NETWORK_WEIGHTS_NUM,
                  STAI_ERROR_NETWORK_INVALID_WEIGHTS_NUM, net_ctx->_return_code)
  for (stai_size idx=0; weights && idx<STAI_NETWORK_WEIGHTS_NUM; idx++) {
    STAI_PRINT("  weight[%d]: %p\n", idx, weights[idx])
    _STAI_SET_ERROR(net_ctx, weights[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)weights[idx]) & (_weights_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_weights[idx] = weights[idx];
  }_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_inputs(
  stai_network* network,
  const stai_ptr* inputs,
  const stai_size n_inputs)
{
  const uintptr_t _inputs_alignment[] = STAI_NETWORK_IN_ALIGNMENTS;
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !inputs,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_inputs!=STAI_NETWORK_IN_NUM,
                  STAI_ERROR_NETWORK_INVALID_IN_NUM, net_ctx->_return_code)

  for (stai_size idx=0; inputs && idx<STAI_NETWORK_IN_NUM; idx++) {
    STAI_PRINT("  input[%d]: %p\n", idx, inputs[idx])
    _STAI_SET_ERROR(net_ctx, inputs[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_IN_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)inputs[idx]) & (_inputs_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_inputs[idx] = inputs[idx];
  }

  _stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_outputs(
  stai_network* network,
  const stai_ptr* outputs,
  const stai_size n_outputs)
{
  const uintptr_t _outputs_alignment[] = STAI_NETWORK_OUT_ALIGNMENTS;
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !outputs,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_outputs!=STAI_NETWORK_OUT_NUM,
                  STAI_ERROR_NETWORK_INVALID_OUT_NUM, net_ctx->_return_code)

  for (stai_size idx=0; outputs && idx<n_outputs; idx++) {
    STAI_PRINT("  output[%d]: %p\n", idx, outputs[idx])
    _STAI_SET_ERROR(net_ctx, outputs[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_OUT_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)outputs[idx]) & (_outputs_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_outputs[idx] = outputs[idx];
  }

  _stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_states(
  stai_network* network,
  const stai_ptr* states,
  const stai_size n_states)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  STAI_UNUSED(states)
  STAI_UNUSED(n_states)
_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}

STAI_API_ENTRY
stai_return_code stai_network_set_callback(
  stai_network* network, const stai_event_cb cb, void* cb_cookie)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  STAI_PRINT("  set_callback %p cb %p cookie %p\n", net_ctx, cb, cb_cookie)
  // _STAI_SET_ERROR(net_ctx, cb==NULL, STAI_ERROR_NETWORK_INVALID_CALLBACK, net_ctx->_return_code)
  net_ctx->_callback = cb;
  net_ctx->_callback_cookie = cb_cookie;
  return net_ctx->_return_code;
}

#undef _STAI_SET_ERROR
#undef _STAI_CONTEXT_ALIGNMENT
#undef _STAI_CONTEXT_ACQUIRE
#undef _STAI_NETWORK_EVENT_NODE_START_CB
#undef _STAI_NETWORK_EVENT_NODE_STOP_CB
#undef _STAI_NETWORK_MODEL_SIGNATURE
#undef _STAI_NETWORK_DATETIME
#undef _STAI_NETWORK_COMPILE_DATETIME

