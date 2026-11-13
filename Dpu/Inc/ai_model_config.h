/**
 ******************************************************************************
 * @file    ai_model_config.h
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
#ifndef AI_MODEL_CONFIG_H
#define AI_MODEL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported constants --------------------------------------------------------*/
/* TEN-VAD feature front-end. */
#define AI_SENSOR_ODR          (16000.0F)
#define AI_SPECT_NMEL          (40U)
#define AI_SPECT_COL           (3U)
#define AI_SPECT_HOP_LEN       (256U)
#define AI_SPECT_NFFT          (1024U)
#define AI_SPECT_WIN_LEN       (768U)
#define AI_SPECT_NORMALIZE     (0U)
#define AI_SPECT_FORMULA       (MEL_HTK)
#define AI_SPECT_FMIN          (125U)
#define AI_SPECT_FMAX          (7500U)
#define AI_SPECT_TYPE          (SPECTRUM_TYPE_POWER)
#define AI_SPECT_LOG_FORMULA   (LOGMELSPECTROGRAM_SCALE_LOG)
#define AI_SPECT_WIN           (user_win)
#define AI_SPECT_MEL_LUT       (user_melFiltersLut)
#define AI_SPECT_MEL_START_IDX (user_melFiltersStartIndices)
#define AI_SPECT_MEL_STOP_IDX  (user_melFiltersStopIndices)

#ifdef __cplusplus
}
#endif

#endif /* AI_MODEL_CONFIG_H*/

