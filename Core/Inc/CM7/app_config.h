/**
  ******************************************************************************
  * @file    app_config.h
  * @author  STMicroelectronics AIS application team
  * @version $Version$
  * @date    $Date$
  *
  * @brief
  *
  * <DESCRIPTIOM>
  *
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

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "logging.h"
//#define LOG_LEVEL LOG_DEBUG
#define LOG_LEVEL LOG_INFO
#define my_printf LogInfo

#define USE_UART_BAUDRATE               (115200) /* can up set up upto 921600 */
#define APP_CONF_STR "Bare Metal"
#define CPU_STATS

#define APP_NAME_STR "x-cube-h7-ai-vad"

#ifndef APP_VERSION_STRING
#define APP_VERSION_STRING "unversioned"
#endif /* !APP_VERSION_STRING */
#ifndef APP_GIT_SHA1_STRING
#define APP_GIT_SHA1_STRING "dev"
#endif /* !APP_GIT_SHA1_STRING */
/* LCD display support (~136 KB FLASH: assets, BSP, fonts). Opt out for
 * boards without a display, or to save FLASH, with EXTRA_DEFS=-DAPP_LCD_OFF
 * (see Makefile). */
#define APP_LCD
#ifdef APP_LCD_OFF
#undef APP_LCD
#endif
#define MENU_DISP_PERIOD (1000) /* ms between periodic CPU-stats menu refresh */
#define APP_REFRESH_VOICE (1000.0F) /* ms to retain the detected-voice status */

/* Low-latency feature-cache mode: one new mel column computed per 16 ms
 * acquisition event (LSTM state advances once per hop) instead of
 * recomputing all 3 columns every 48 ms. This is the default: TEN-VAD was
 * trained on a window sliding one hop at a time, so this is the framing the
 * model expects, not just an optimisation. Opt out to the 48 ms/three-columns
 * cadence with EXTRA_DEFS=-DAPP_LOW_LAT_OFF (see Makefile) - kept buildable
 * for CPU-constrained integrations. */
#define APP_LOW_LAT
#ifdef APP_LOW_LAT_OFF
#undef APP_LOW_LAT
#endif
#define VAD_THRESHOLD_PCT (80.0F) /* voice probability (%) above which the green LED lights and the console prints green */
#define SEPARATION_LINE "---------------------------------------------------------------\n\r"

#endif /* __APP_CONFIG_H__ */
