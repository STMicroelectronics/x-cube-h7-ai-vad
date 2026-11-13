/**
  ******************************************************************************
  * @file    ai_dpu.h
  * @author  MCD Application Team
  * @brief   Header for ai_dpu.c module
  * @version v1.0.0
  * @date    2026-11-13
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _AI_DPU_H
#define _AI_DPU_H

/* Includes ------------------------------------------------------------------*/
#include "dpu.h"
#include "stai.h"     /* include ST Edge AI macros, types and data structures */
#include "network.h"
#include "network_data.h"

/* Exported constants --------------------------------------------------------*/
#define AI_MNETWORK_NUMBER         (1U)

#define AI_DPU_X_CUBE_AI_API_MAJOR (1)
#define AI_DPU_X_CUBE_AI_API_MINOR (0)
#define AI_DPU_X_CUBE_AI_API_MICRO (0)
#define AI_DPU_INPUT               (5U)
#define AI_DPU_OUTPUT              (5U)
#define AI_DPU_STATE_SIZE_BYTES    (256U)  /* one LSTM h or c state, 64 x float32 */

/* Exported types ------------------------------------------------------------*/

typedef struct {
  /* Global handle to reference the instantiated C-model */
  stai_network * p_network ;
  stai_ptr     * p_stai_inputs;
  stai_ptr     * p_stai_outputs;
  stai_ptr     * p_activations;
  
  stai_network_info info;

  float out;                    /* voice probability, 0.0-100.0% */
}AIProcCtx_t;

/* Exported functions --------------------------------------------------------*/
DPU_StatusTypeDef AiDPULoadModel(AIProcCtx_t * pxCtx);
DPU_StatusTypeDef AiDPUReleaseModel(AIProcCtx_t * pxCtx);
DPU_StatusTypeDef AiDPUProcess(AIProcCtx_t *pxCtx);

#endif /* _AI_DPU_H */
