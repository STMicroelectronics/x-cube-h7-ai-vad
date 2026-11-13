/**
  ******************************************************************************
  * @file    preproc_dpu.c
  * @author  MCD Application Team
  * @brief   This file is implementing pre-processing functions that are making
  * 		 use of Audio pre-processing libraries
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


/* Includes ------------------------------------------------------------------*/
#include "preproc_dpu.h"
#include "main.h"
#include "app_config.h"
#include "ai_model_config.h"

#include "cpu_stats.h"
#include "ai_device_adaptor.h"
#include <string.h>
      
/* Private defines ---------------------------------------------------------- */

/* External functions --------------------------------------------------------*/

/**
 * @brief initializes preprocessing for Audio
 * @param pxCtx pointer to preprocessing context
 * @retval DPU_OK if success, DPU_ERROR else
 */
DPU_StatusTypeDef AudioPreProc_DPUInit(AudioPreProcCtx_t *pxCtx)
{
  assert_param( AI_SPECT_NFFT >= AI_SPECT_WIN_LEN );
  assert_param( AI_SPECT_NFFT >= AI_SPECT_NMEL );

  /* Init RFFT */
  arm_rfft_fast_init_f32(&pxCtx->S_Rfft, AI_SPECT_NFFT);

  /* Init Spectrogram */
  pxCtx->S_Spectr.pRfft                    = &pxCtx->S_Rfft;
  pxCtx->S_Spectr.Type                     = AI_SPECT_TYPE;
  pxCtx->S_Spectr.pWindow                  = (float32_t *) AI_SPECT_WIN;
  pxCtx->S_Spectr.SampRate                 = AI_SENSOR_ODR;
  pxCtx->S_Spectr.FrameLen                 = AI_SPECT_WIN_LEN;
  pxCtx->S_Spectr.FFTLen                   = AI_SPECT_NFFT;

  pxCtx->S_Spectr.pScratch1                = pxCtx->pSpectrScratchBuffer1;
  pxCtx->S_Spectr.pScratch2                = pxCtx->pSpectrScratchBuffer2;

  /* Zero-pad on the right only (center=False STFT, causal framing), up to
   * AI_SPECT_NFFT. pad_left + FrameLen + pad_right must equal FFTLen exactly:
   * audio_is16of32_pad() writes that many samples into pScratch1/2, which are
   * sized [AI_SPECT_NFFT]; a pad_right of AI_SPECT_WIN_LEN would sum to
   * 2 * AI_SPECT_WIN_LEN (1536) > AI_SPECT_NFFT (1024) and overflow those
   * buffers. */
  pxCtx->S_Spectr.pad_left                 = 0U;
  pxCtx->S_Spectr.pad_right                = AI_SPECT_NFFT - AI_SPECT_WIN_LEN;

  /* Init mel filterbank */
  pxCtx->S_MelFilter.pStartIndices         = (uint32_t *)  AI_SPECT_MEL_START_IDX;
  pxCtx->S_MelFilter.pCoefficients         = (float32_t *) AI_SPECT_MEL_LUT;
  pxCtx->S_MelFilter.pStopIndices          = (uint32_t *)  AI_SPECT_MEL_STOP_IDX;
  pxCtx->S_MelFilter.NumMels               = AI_SPECT_NMEL;
  pxCtx->S_MelFilter.FFTLen                = AI_SPECT_NFFT;
  pxCtx->S_MelFilter.SampRate              = (uint32_t) AI_SENSOR_ODR;
  pxCtx->S_MelFilter.FMin                  = (float32_t) AI_SPECT_FMIN;
  pxCtx->S_MelFilter.FMax                  = (float32_t) AI_SPECT_FMAX;
  pxCtx->S_MelFilter.Formula               = AI_SPECT_FORMULA;
  pxCtx->S_MelFilter.Normalize             = AI_SPECT_NORMALIZE;
  pxCtx->S_MelFilter.Mel2F                 = 1U;

  /* Init MelSpectrogram */
  pxCtx->S_MelSpectr.SpectrogramConf       = &pxCtx->S_Spectr;
  pxCtx->S_MelSpectr.MelFilter             = &pxCtx->S_MelFilter;

  /* Init LogMelSpectrogram */
  pxCtx->S_LogMelSpectr.MelSpectrogramConf = &pxCtx->S_MelSpectr;
  pxCtx->S_LogMelSpectr.LogFormula         = AI_SPECT_LOG_FORMULA;
  pxCtx->S_LogMelSpectr.Ref                = 1.0f;
  pxCtx->S_LogMelSpectr.TopdB              = HUGE_VALF;

  LogInfo("MEL spectrogram %d mel x %d col\n\r",AI_SPECT_NMEL,AI_SPECT_COL);
  LogInfo("- sampling freq : %u Hz\n\r",(uint32_t)AI_SENSOR_ODR);
  LogInfo("- acq period    : %u ms\n\r",(uint32_t)AUDIO_ACQ_LEN_MS);
  LogInfo("- window length : %u samples\n\r",AI_SPECT_WIN_LEN);
  LogInfo("- hop length    : %u samples\n\r",AI_SPECT_HOP_LEN);
#ifdef APP_LOW_LAT
  LogInfo("- low-latency feature-cache mode enabled\n\r");
#endif

  /* Start pre-emphasis and (in APP_LOW_LAT) the cached feature window from
   * silence. */
  pxCtx->preemp_prev = 0.0F;
#ifdef APP_LOW_LAT
  memset(pxCtx->feature_window, 0, sizeof(pxCtx->feature_window));
#endif

  return DPU_OK;
}

/**
 * @brief TEN-VAD spectral preprocessing of one PATCH_LEN-sample patch
 * @param pxCtx pointer to preprocessing context
 * @param pDataIn pointer to PATCH_LEN int16 audio samples
 * @param p_features pointer to the AI_SPECT_COL x (AI_SPECT_NMEL+1) float32
 *        output tensor (row-major, frame-major - matches the model's
 *        --optimize.merge_transposes False layout, no transpose needed)
 * @retval DPU_OK if success, DPU_ERROR else
 */
DPU_StatusTypeDef AudioPreProc_DPU(AudioPreProcCtx_t * pxCtx, uint8_t *pDataIn, float *p_features)
{
  assert_param( NULL != pxCtx );
  assert_param( NULL != pDataIn );
  assert_param( NULL != p_features );
  assert_param( AI_SPECT_NMEL == pxCtx->S_MelFilter.NumMels);

  /* File-static: AudioPreProc_DPU() is only ever called from audio_exec_bm()
   * in the main loop, never from an ISR, so it isn't reentrant. PATCH_LEN
   * int16 samples on the stack would blow the 4 KB MSP once the SAI/BDMA ISR
   * nests on top. */
  static int16_t p_preemp[PATCH_LEN];
  float32_t out[AI_SPECT_NMEL];
  const int16_t *p_in_preemp = (const int16_t *)pDataIn;
#ifndef APP_LOW_LAT
  int16_t   *p_in;
#endif

  FFTTypeDef_t sFFT = {
    .pRfft          = pxCtx->S_Spectr.pRfft,
    .pWindow        = pxCtx->S_Spectr.pWindow,
    .pScratch       = pxCtx->S_Spectr.pScratch1,
    .FFTLen         = pxCtx->S_Spectr.FFTLen,
    .sDin.FrameLen  = pxCtx->S_Spectr.FrameLen,
    .sDin.pad_left  = pxCtx->S_Spectr.pad_left,
    .sDin.pad_right = pxCtx->S_Spectr.pad_right
  };

  port_dwt_reset();

  /* Pre-emphasis over the whole patch: y[n] = x[n] - 0.97 * x[n-1].
   * x[-1] is carried across calls in pxCtx->preemp_prev, which avoids a
   * discontinuity at every acquisition event (3x more often in APP_LOW_LAT
   * mode). */
  float32_t xn_1 = pxCtx->preemp_prev;
  for (uint32_t n = 0; n < PATCH_LEN; n++)
  {
    float32_t xn = (float32_t)p_in_preemp[n];
    /* Saturate: a plain (int16_t) cast can wrap (e.g. xn=32767, xn_1=-32768
     * gives 64552 -> wraps to -993), turning a loud transient into a
     * spurious broadband spike right where the VAD is deciding. */
    p_preemp[n] = (int16_t)__SSAT((int32_t)(xn - 0.97F * xn_1), 16);
    xn_1 = xn;
  }
  /* The next call's window starts AUDIO_ACQ_LEN samples later (overlap-save),
   * so its true predecessor is p_in_preemp[AUDIO_ACQ_LEN - 1], not the last
   * sample of this patch. Inert with the current window: user_win[0] == 0.0
   * (Hann) zeroes y[0] before the FFT, so x[-1] never reaches the spectrum -
   * kept correct so this doesn't silently become a real bug if the window
   * type ever changes. */
  pxCtx->preemp_prev = (float32_t)p_in_preemp[AUDIO_ACQ_LEN - 1];

#ifdef APP_LOW_LAT
  /* One new AI_SPECT_WIN_LEN-sample window per call, i.e. exactly one new
   * mel column: shift the cached feature window left by one column, compute
   * the new column from p_preemp, append it, then copy the whole window to
   * the model input. The other AI_SPECT_COL-1 columns are reused unchanged. */
  memmove(pxCtx->feature_window, pxCtx->feature_window + (AI_SPECT_NMEL + 1),
          (AI_SPECT_COL - 1) * (AI_SPECT_NMEL + 1) * sizeof(float32_t));

  audio_is16of32_fft(p_preemp, &sFFT, pxCtx->S_Spectr.pScratch2);
  LogMelSpectrogramColumn(&pxCtx->S_LogMelSpectr, pxCtx->S_Spectr.pScratch2, out);

  float32_t *p_new_col = pxCtx->feature_window + (AI_SPECT_COL - 1) * (AI_SPECT_NMEL + 1);
  for (uint32_t j = 0; j < AI_SPECT_NMEL; j++)
  {
    p_new_col[j] = (out[j] - user_melFiltersmeans[j]) / user_melFiltersstds[j];
  }
  /* trailing "pitch" slot expected by the model input, always zero */
  p_new_col[AI_SPECT_NMEL] = 0.0F;

  memcpy(p_features, pxCtx->feature_window,
         AI_SPECT_COL * (AI_SPECT_NMEL + 1) * sizeof(float32_t));
#else
  for (uint32_t i = 0; i < AI_SPECT_COL; i++)
  {
    p_in = p_preemp + i * AI_SPECT_HOP_LEN;

    audio_is16of32_fft(p_in, &sFFT, pxCtx->S_Spectr.pScratch2);
    LogMelSpectrogramColumn(&pxCtx->S_LogMelSpectr, pxCtx->S_Spectr.pScratch2, out);

    /* Per-bin standardization against TEN-VAD training statistics */
    for (uint32_t j = 0; j < AI_SPECT_NMEL; j++)
    {
      p_features[i * (AI_SPECT_NMEL + 1) + j] =
          (out[j] - user_melFiltersmeans[j]) / user_melFiltersstds[j];
    }
    /* trailing "pitch" slot expected by the model input, always zero */
    p_features[i * (AI_SPECT_NMEL + 1) + AI_SPECT_NMEL] = 0.0F;
  }
#endif /* APP_LOW_LAT */

  /* No cache maintenance needed here: on H7 the AI runtime executes purely
   * on the CM7 core against this same buffer, there is no separate
   * NPU/bus-master needing cache coherency. */
  time_stats_store(TIME_STAT_PRE_PROC,port_dwt_get_cycles()*1000.0F/port_hal_get_cpu_freq());
  return DPU_OK;
}

