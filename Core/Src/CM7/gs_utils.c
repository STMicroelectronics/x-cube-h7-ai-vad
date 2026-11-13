/**
  ******************************************************************************
  * @file    gs_utils.c
  * @author  STMicroelectronics AIS application team
  * @version $Version$
  * @date    $Date$
  * @brief   utilities for audio and sensing getting started
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dpu.h"
#include "ai_utility.h"
#include "cpu_stats.h"

#include <stdio.h>

#ifdef APP_LCD
#include "stm32h747i_discovery_lcd.h"
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"
#include "stlogo.h"
#include "voiceicon.h"
#endif /* APP_LCD */
/* External functions --------------------------------------------------------*/

/**
* @brief  Displays System settings
* @param  None
* @retval None
*/
void PrintSystemSetting(void)
{
  LogSys("\n\n\r");
  LogSys(SEPARATION_LINE);
  LogSys("        System configuration (%s)\n\r",APP_CONF_STR);
  LogSys(SEPARATION_LINE);
  LogSys("\n\rLog Level: %s\n\n\r", getLogLevelStr(LOG_LEVEL));

  systemSettingLog();
  LogSys("\r\n");

#ifdef USE_HSP
  LogSys("\r\nHSP Acceleration used for Preprocessing\r\n\n");
#endif
  LogSys(SEPARATION_LINE);
  LogSys("\r\n");
  
#if defined (APP_DVFS) || defined (APP_LP)
  LogSys("\r\nLow Power Options:");
#ifdef APP_LP
  LogSys("\r\n DPS");
#endif /* APP_LP */
#ifdef APP_DVFS
  LogSys("\r\n DFVS");
#endif /* APP_DFVS */
  LogSys("\r\n");
#endif /* APP_DVFS || APP_LP */
}

/**
* @brief  Displays The main menu
* @param  None
* @retval None
*/
void PrintMenu(void)
{
  static  uint32_t prev_tick=0;
  
  uint32_t tick = HAL_GetTick();
  if (tick - prev_tick >= MENU_DISP_PERIOD )
  {
    time_stats_store(TIME_STAT_TOT, (float)(tick - prev_tick));
    prev_tick = tick ;
  }
}

/**
* @brief  Tracks whether voice activity is currently "held": false until
*         voice_proba_pct first crosses VAD_THRESHOLD_PCT, then true from
*         each crossing until APP_REFRESH_VOICE ms have elapsed with no
*         further crossing. Exposed so both the console line and the LCD
*         (APP_LCD) can share the same 1 s-hold state without duplicating
*         the timing logic.
* @param  voice_proba_pct voice probability, 0.0-100.0%
* @retval true while voice activity is held, false once the hold has expired
*/
bool UpdateVoiceActivityHold(float voice_proba_pct)
{
  static bool     held = false;
  static uint32_t last_voice_tick = 0U;
  uint32_t tick = HAL_GetTick();

  if (voice_proba_pct > VAD_THRESHOLD_PCT)
  {
    held = true;
    last_voice_tick = tick;
  }
  /* Unsigned tick difference: correct across the HAL tick wrap. */
  else if (held && ((tick - last_voice_tick) > (uint32_t)APP_REFRESH_VOICE))
  {
    held = false;
  }

  return held;
}

#ifdef APP_LCD
#define LCD_BYTES_PER_PIXEL (2U) /* LCD_PIXEL_FORMAT_RGB565, see LCD_Init() */

/**
* @brief  BSP_LCD_FillRGBRect() plus the D-cache maintenance it lacks.
*         DMA2D reads the source pixels straight from memory, bypassing the
*         CM7 D-cache, while stm32_lcd.c's DrawChar() builds each glyph row
*         in a cacheable stack buffer just before handing it over: clean the
*         source lines to memory first (see USE_BSP_CPU_CACHE_MAINTENANCE in
*         stm32h747i_discovery_conf.h for why the BSP's own clean is unused).
* @retval BSP status
*/
static int32_t App_LCD_FillRGBRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos,
                                   uint8_t *pData, uint32_t Width, uint32_t Height)
{
  SCB_CleanDCache_by_Addr(pData, (int32_t)(LCD_BYTES_PER_PIXEL * Width * Height));
  return BSP_LCD_FillRGBRect(Instance, Xpos, Ypos, pData, Width, Height);
}

/**
* @brief  Initializes the DSI LCD panel and the text/graphics utility layer.
*         Must be called once, before DisplayLCD_VAD(). Framebuffer lives at
*         the fixed LTDC Layer 0 address in external SDRAM (BSP_LCD_InitEx()).
* @param  None
* @retval None
*/
void LCD_Init(void)
{
  LCD_UTILS_Drv_t lcd_driver = LCD_Driver;

  BSP_LCD_InitEx(0, LCD_ORIENTATION_LANDSCAPE, LCD_PIXEL_FORMAT_RGB565, 800, 480);
  lcd_driver.FillRGBRect = App_LCD_FillRGBRect;
  UTIL_LCD_SetFuncDriver(&lcd_driver);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetFont(&Font20);
}

/**
* @brief  Draws the static VAD screen: ST logo, title banner, model name and
*         the "Inf.: ___ ms | AI confidence: __ %%" label. Dynamic values and
*         the voice-activity status/icon are updated separately, once per
*         inference, by PrintVoiceActivity().
* @param  None
* @retval None
*/
void DisplayLCD_VAD(void)
{
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_ST_BLUE_DARK);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_FillRGBRect(650, 0, (uint8_t *)stlogo, 150, 103);
  UTIL_LCD_FillRect(150, 40, 500, 150, UTIL_LCD_COLOR_ST_BLUE_DARK);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCDEx_PrintfAt(0, 60, CENTER_MODE, "Voice Activity Detection");
  UTIL_LCD_SetFont(&Font20);
  UTIL_LCDEx_PrintfAt(0, 95, CENTER_MODE, "ST TenVAD model");
  UTIL_LCDEx_PrintfAt(0, 160, CENTER_MODE, "Inf.:      ms | AI confidence:   %%");

  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCDEx_PrintfAt(0, 430, CENTER_MODE, "Listening, waiting for speech");
}

/**
* @brief  Updates the dynamic part of the VAD screen: the inference-time/AI
*         confidence numbers refresh at MENU_DISP_PERIOD (matching the
*         console's own refresh cadence), and the status text/icon redraw
*         only when the held state changes - not every inference (~62 Hz) -
*         since neither needs updating that often and a status redraw costs
*         a few ms of CPU time. Called once per inference, after the console
*         line.
* @param  voice_proba_pct voice probability, 0.0-100.0%
* @param  held            current UpdateVoiceActivityHold() state
* @retval None
*/
static void UpdateLCD_VAD(float voice_proba_pct, bool held)
{
  static uint32_t prev_tick = 0;
  static bool prev_held = false;
  uint32_t tick = HAL_GetTick();
  bool refresh_numbers = (tick - prev_tick) >= MENU_DISP_PERIOD;
  bool status_changed = (held != prev_held);

  if (!refresh_numbers && !status_changed)
  {
    return;
  }

  if (refresh_numbers)
  {
    UTIL_LCD_SetFont(&Font20);
    UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_ST_BLUE_DARK);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
    UTIL_LCDEx_PrintfAt(232, 160, LEFT_MODE, "%5.2f", time_stats_get_avg(TIME_STAT_AI_PROC));
    UTIL_LCDEx_PrintfAt(585, 160, LEFT_MODE, "%3.0f", voice_proba_pct);
    prev_tick = tick;
  }

  if (status_changed)
  {
    UTIL_LCD_SetFont(&Font24);
    UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);

    if (held)
    {
      UTIL_LCDEx_PrintfAt(0, 430, CENTER_MODE, "    Voice Activity detected!   ");
      UTIL_LCD_FillRGBRect(300, 210, (uint8_t *)voiceicon, 200, 199);
    }
    else
    {
      UTIL_LCD_FillRect(300, 210, 200, 200, UTIL_LCD_COLOR_WHITE);
      UTIL_LCDEx_PrintfAt(0, 430, CENTER_MODE, "Listening, waiting for speech");
    }
    prev_held = held;
  }
}
#endif /* APP_LCD */

/**
* @brief  Displays live Cpu/Pre/AI load and the voice-activity probability on
*         a single self-refreshing console line, colored green above
*         VAD_THRESHOLD_PCT and red below, and updates the LCD in APP_LCD
*         builds. Called once per inference.
* @param  voice_proba_pct voice probability, 0.0-100.0%
* @retval None
*/
void PrintVoiceActivity(float voice_proba_pct)
{
  bool held = UpdateVoiceActivityHold(voice_proba_pct);

#ifdef CPU_STATS
  /* Run-long loads (not windowed): these share TIME_STAT_* with
   * PrintCpuStatsSummary()'s end-of-run figures, so this must not reset them.
   * Computed as sum/runtime rather than avg/cadence because PDM->PCM is stored
   * once per 1 ms DMA callback while feature extraction is stored once per
   * acquisition event - a per-call average cannot be scaled by a single
   * period once the buckets have different store rates. */
  float runtime_ms = time_stats_get_runtime();
  float pre_load = 0.0F;
  float ai_load  = 0.0F;

  if (runtime_ms > 0.0F)
  {
    pre_load = 100.0F * (time_stats_get_sum(TIME_STAT_PDM_PROC)
                         + time_stats_get_sum(TIME_STAT_PRE_PROC)) / runtime_ms;
    ai_load  = 100.0F * time_stats_get_sum(TIME_STAT_AI_PROC) / runtime_ms;
  }

  LogInfo("|%6.2f|%6.2f|%6.2f|", pre_load + ai_load, pre_load, ai_load);
#endif
  if (voice_proba_pct > VAD_THRESHOLD_PCT)
  {
    LogInfo("\033[1;32m %6.2f\033[0m| Voice activity detected", voice_proba_pct);
  }
  else
  {
    LogInfo("\033[0;31m %6.2f\033[0m|", voice_proba_pct);
  }

  if (!held)
  {
    LogInfo("                        ");
  }
  LogInfo("\r");
  fflush(stdout);

#ifdef APP_LCD
  UpdateLCD_VAD(voice_proba_pct, held);
#endif /* APP_LCD */
}

/**
* @brief  Displays Processing start message
* @param  None
* @retval None
*/
void PrintHeader(void)
{
  LogInfo("\r\n");
  LogInfo(SEPARATION_LINE);
  LogInfo("# Start Processing\n\r");
  LogInfo(SEPARATION_LINE);
#ifdef CPU_STATS
  LogInfo("|  Cpu |  Pre |  AI  | Voice |");
#endif
  LogInfo("\033[?25l\r\n"); /* hide cursor (\033[?25l) */
}

/**
* @brief  Displays Processing stop message
* @param  None
* @retval None
*/
void PrintFooter(void)
{
  LogInfo("\r\n\n\n\n\n");
  LogInfo(SEPARATION_LINE);
  LogInfo("# Stop Processing\n\r");
  LogInfo(SEPARATION_LINE);
  LogInfo("\033[?25h\r\n"); /* un-hide cursor (\033[?25h) */
}

/**
* @brief  Init CPU usage statistics
* @param  None
* @retval None
*/
void InitCpuStats(void)
{
  time_stats_init();
  port_dwt_init(); 
}
/**
* @brief  Displays CPU usage statistics
* @param  None
* @retval None
*/
void PrintCpuStatsSummary(void)
{
#ifdef CPU_STATS
  float runtime_ms = time_stats_get_runtime();
  float scale      = (runtime_ms > 0.0F) ? (100.0F / runtime_ms) : 0.0F;

  LogSys("                       CPU timing summary \n\r");
  LogSys(SEPARATION_LINE);
  LogSys("| %-26s | %8s | %8s | %8s |\r\n",
    "Statistics", "PDM->PCM", "Pre-proc", "AI infer");
  LogSys(SEPARATION_LINE);
  LogSys("| %-26s | %8u | %8u | %8u |\r\n", "Number of calls",
    time_stats_get_cnt(TIME_STAT_PDM_PROC),
    time_stats_get_cnt(TIME_STAT_PRE_PROC),
    time_stats_get_cnt(TIME_STAT_AI_PROC));
  LogSys("| %-26s | %8.3f | %8.3f | %8.3f |\r\n", "Average (ms)",
    time_stats_get_avg(TIME_STAT_PDM_PROC),
    time_stats_get_avg(TIME_STAT_PRE_PROC),
    time_stats_get_avg(TIME_STAT_AI_PROC));
  LogSys("| %-26s | %8.2f | %8.2f | %8.2f |\r\n", "Relative load (%)",
    scale * time_stats_get_sum(TIME_STAT_PDM_PROC),
    scale * time_stats_get_sum(TIME_STAT_PRE_PROC),
    scale * time_stats_get_sum(TIME_STAT_AI_PROC));
  LogSys(SEPARATION_LINE);
  LogSys("Pre-processing load reported live = PDM->PCM + Pre-proc = %.2f %%\r\n",
    scale * (time_stats_get_sum(TIME_STAT_PDM_PROC)
             + time_stats_get_sum(TIME_STAT_PRE_PROC)));
#endif
}

/* Private user code ---------------------------------------------------------*/
