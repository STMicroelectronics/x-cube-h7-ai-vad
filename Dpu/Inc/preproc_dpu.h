/**
  ******************************************************************************
  * @file    preproc_dpu.h
  * @author  MCD Application Team
  * @brief   Header for preproc_dpu.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023-2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PREPROC_DPU_H
#define _PREPROC_DPU_H

/* Includes ------------------------------------------------------------------*/
#include "dpu.h"
#include "app_config.h"
#include "feature_extraction.h"
#include "mel_filterbank.h"
#include "user_mel_tables.h"

/* Exported constants --------------------------------------------------------*/
/* PATCH_LEN = int16 audio samples spanned by one AudioPreProc_DPU() call
 * (overlap-save framing done by the caller in audio_bm.c) - NOT the
 * acquisition cadence (AUDIO_ACQ_LEN/AUDIO_ACQ_LEN_MS in dpu.h): the patch
 * deliberately spans more audio than arrives per acquisition event, which is
 * the whole point of overlap-save.
 * APP_LOW_LAT_OFF (opt-out): 1280 samples (80 ms of audio spanned per call,
 * acquisition cadence is 48 ms) - the full AI_SPECT_COL-column window, all
 * columns recomputed fresh each call, so AudioPreProc_DPU is stateless
 * across calls.
 * APP_LOW_LAT (default): 768 samples (48 ms of audio spanned per call,
 * acquisition cadence is 16 ms) == AI_SPECT_WIN_LEN - only enough for one
 * new column; AudioPreProc_DPU then keeps a cached feature window in
 * AudioPreProcCtx_t across calls (see below). */
#ifdef APP_LOW_LAT
#define PATCH_LEN            (AI_SPECT_WIN_LEN)
#else
#define PATCH_LEN            (AI_SPECT_WIN_LEN + (AI_SPECT_COL - 1) * AI_SPECT_HOP_LEN)
#endif

/* Exported types ------------------------------------------------------------*/
typedef struct {
  arm_rfft_fast_instance_f32 S_Rfft;
  float32_t pSpectrScratchBuffer1[AI_SPECT_NFFT];
  float32_t pSpectrScratchBuffer2[AI_SPECT_NFFT];

  MelFilterTypeDef           S_MelFilter;
  SpectrogramTypeDef         S_Spectr;
  MelSpectrogramTypeDef      S_MelSpectr;
  LogMelSpectrogramTypeDef   S_LogMelSpectr;

  /* Pre-emphasis carries x[-1] across calls, which avoids a discontinuity at
   * every acquisition event - 3x more often in APP_LOW_LAT mode. */
  float32_t preemp_prev;

#ifdef APP_LOW_LAT
  /* Cached AI_SPECT_COL x (AI_SPECT_NMEL+1) feature window: shifted left by
   * one column and refreshed with a single newly-computed column each call,
   * instead of recomputing all AI_SPECT_COL columns from scratch. */
  float32_t feature_window[AI_SPECT_COL * (AI_SPECT_NMEL + 1)];
#endif
}AudioPreProcCtx_t;


/* Exported functions --------------------------------------------------------*/
extern DPU_StatusTypeDef AudioPreProc_DPUInit(AudioPreProcCtx_t *pxCtx);
extern DPU_StatusTypeDef AudioPreProc_DPU(AudioPreProcCtx_t *pxCtx, uint8_t *pDataIn, float *p_features);

#endif /* _PREPROC_DPU_H*/
