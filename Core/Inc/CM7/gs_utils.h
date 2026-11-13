/**
  ******************************************************************************
  * @file    : gs_utils.h
  * @brief   : Utility Module
  * @version $Version$
  * @date    $Date$
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

#ifndef GS_AUDIO_SENSING_UTILS_H
#define GS_AUDIO_SENSING_UTILS_H

#include <stdbool.h>

extern void PrintHeader(void);
extern void PrintFooter(void);
extern void InitCpuStats(void);
extern void PrintCpuStatsSummary(void);
extern bool UpdateVoiceActivityHold(float voice_proba_pct);
extern void PrintVoiceActivity(float voice_proba_pct);
extern void PrintSystemSetting(void);
extern void PrintMenu(void);

#ifdef APP_LCD
extern void LCD_Init(void);
extern void DisplayLCD_VAD(void);
#endif /* APP_LCD */

#endif /* GS_AUDIO_SENSING_UTILS_H */
