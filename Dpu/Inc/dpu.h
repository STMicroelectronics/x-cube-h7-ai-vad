/**
 ******************************************************************************
 * @file    dpu.h
 * @author  STMicroelectronics - AIS - MCD Team
 * @version v1.0.0
 * @date    2026-11-13
 * @brief   Configure the getting started functionality
 *
 * Each logic module of the application should define a DEBUG control byte
 * used to turn on/off the log for the module.
 *
 *********************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *********************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DPU_H_
#define DPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Exported constants --------------------------------------------------------*/
#include "ai_model_config.h"
#include "app_config.h"   /* APP_LOW_LAT */

/* One acquisition event supplies new samples that audio_bm.c frames into the
 * larger overlap-save window the model needs.
 * APP_LOW_LAT_OFF (opt-out): a full AI_SPECT_COL-column window's worth
 * (48 ms) - all columns are recomputed fresh every inference.
 * APP_LOW_LAT (default): a single hop's worth (16 ms) - only the newest
 * column is computed each inference, the other AI_SPECT_COL-1 are reused
 * from the previous call (see preproc_dpu.c). */
#ifdef APP_LOW_LAT
#define AUDIO_ACQ_LEN     (AI_SPECT_HOP_LEN)                  /* 256 samples = 16 ms */
#else
#define AUDIO_ACQ_LEN     (AI_SPECT_COL * AI_SPECT_HOP_LEN)   /* 768 samples = 48 ms */
#endif
#define AUDIO_ACQ_LEN_MS  (uint32_t)(1000.0F * AUDIO_ACQ_LEN / AI_SENSOR_ODR)

typedef enum
{
  DPU_OK       = 0x00,
  DPU_ERROR    = 0x01,
} DPU_StatusTypeDef;

#ifdef __cplusplus
}
#endif

#endif /* DPU_H_ */

