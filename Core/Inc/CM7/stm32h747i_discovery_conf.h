/**
  ******************************************************************************
  * @file    stm32h747i_discovery_conf.h
  * @brief   STM32H747I_DISCO BSP configuration for the VAD Getting Started app.
  *          The SAI4/PDM digital-mic audio-in path (BSP Instance 1) and the
  *          OTM8009A DSI LCD panel (BSP Instance 0) are enabled.  The WM8994
  *          codec (audio-out / analog audio-in), camera, SD, TS and JOY
  *          features are all disabled to keep the build footprint small.
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
#ifndef STM32H747I_DISCO_CONF_H
#define STM32H747I_DISCO_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* COM define */
#define USE_COM_LOG                         0U
/* Enables BSP_COM_Init()/COM1 for the console UART: the BSP's own MSP
 * configures PA10 (RX) with a pull-up so the line does not float when
 * nothing is driving the ST-LINK VCP. */
#define USE_BSP_COM_FEATURE                 1U

/* LCD controllers: the STM32H747I-DISCO panel is driven by the OTM8009A.
 * NT35510 is a different, mutually-exclusive panel option (not populated on
 * this board); ADV7533 is the separate HDMI bridge, not used here. */
#define USE_LCD_CTRL_NT35510                0U
#define USE_LCD_CTRL_OTM8009A               1U
#define USE_LCD_CTRL_ADV7533                0U

/* LTDC layer 0 framebuffer address: base of external SDRAM (BSP_LCD_InitEx
 * wires the default layer here). Layer 1 is unused by this application but
 * kept defined for parity with the vendor conf template. */
#define LCD_LAYER_0_ADDRESS                 0xD0000000U
#define LCD_LAYER_1_ADDRESS                 0xD0200000U

/* BSP_LCD_FillRGBRect()'s non-DMA2D path reads 4 bytes/pixel regardless of
 * the active pixel format, which corrupts a tightly-packed RGB565 source
 * (stlogo/voiceicon). The DMA2D path branches on Lcd_Ctx[].PixelFormat via
 * LL_ConvertLineToRGB() and handles RGB565 correctly, so it is mandatory
 * here, not just a performance option. */
#define USE_DMA2D_TO_FILL_RGB_RECT          1U

/* MANDATORY, not just a performance knob: DMA2D is a separate AXI bus master
 * that bypasses the CM7 D-cache, but BSP_LCD_FillRGBRect()'s DMA2D path reads
 * its *source* buffer directly from memory. stm32_lcd.c's DrawChar() builds
 * each glyph row in a local (stack = cacheable AXI_SRAM, see _estack in
 * STM32H747XIHx_CM7_FLASH.ld) buffer and immediately hands it to
 * UTIL_LCD_FillRGBRect() -> DMA2D: without a cache-clean in between, DMA2D
 * reads stale/garbage bytes instead of the freshly-computed pixels, rendering
 * every on-screen text string as colour-noise glyphs (the `const`,
 * flash-resident stlogo/voiceicon image blits are unaffected - flash isn't
 * written at runtime, so there is no stale-cache-line problem there).
 *
 * The vendor's own fix for this is USE_BSP_CPU_CACHE_MAINTENANCE, but its
 * BSP_LCD_FillRGBRect() implementation (STM32CubeH7 submodule) has a
 * one-character typo (SCB_CleanDCache_by_Addr((uint32_t *)pdata, ...) - an
 * undeclared identifier, not the function's own pData parameter) that fails
 * to compile, and we do not carry a patched submodule. Left at 0U here on
 * purpose: gs_utils.c's LCD_Init() instead registers App_LCD_FillRGBRect(),
 * which cleans the source buffer's D-cache lines and then calls
 * BSP_LCD_FillRGBRect(), so the D-cache stays enabled while drawing. This is
 * entirely application-side and needs no BSP/vendor change. */
#define USE_BSP_CPU_CACHE_MAINTENANCE       0U

/* Camera sensors — not used */
#define USE_CAMERA_SENSOR_OV5640            0U
#define USE_CAMERA_SENSOR_OV9655            0U

/* SD interface — not used */
#define USE_SD_BUS_WIDE_4B                  0U

/* Audio codecs: WM8994 is only required for audio-out and analog mic-in
 * (BSP Instance 0).  Digital-mic acquisition (BSP Instance 1, SAI4/PDM)
 * does not use the codec, so set to 0 to exclude I2C/WM8994 code. */
#define USE_AUDIO_CODEC_WM8994              0U

/* Touch-screen — not used */
#define USE_TS_GESTURE                      0U
#define USE_TS_MULTI_TOUCH                  0U
#define TS_TOUCH_NBR                        1U

/* IRQ priorities ---------------------------------------------------------- */
/* SAI4 / BDMA interrupt priority for digital-mic audio-in.
 * Must be low enough to allow SysTick (prio 0) and any higher-priority IRQs
 * to preempt.  Value 5 gives a comfortable margin in a bare-metal build. */
#define BSP_AUDIO_IN_IT_PRIORITY            5U
#define BSP_AUDIO_OUT_IT_PRIORITY           6U

/* Other BSP IRQ priorities (unused features — kept at safe defaults) */
#define BSP_SDRAM_IT_PRIORITY               15U
#define BSP_CAMERA_IT_PRIORITY              15U
#define BSP_BUTTON_WAKEUP_IT_PRIORITY       15U
#define BSP_SD_IT_PRIORITY                  14U
#define BSP_SD_RX_IT_PRIORITY               14U
#define BSP_SD_TX_IT_PRIORITY               15U
#define BSP_TS_IT_PRIORITY                  15U
#define BSP_JOY1_SEL_IT_PRIORITY            15U
#define BSP_JOY1_DOWN_IT_PRIORITY           15U
#define BSP_JOY1_LEFT_IT_PRIORITY           15U
#define BSP_JOY1_RIGHT_IT_PRIORITY          15U
#define BSP_JOY1_UP_IT_PRIORITY             15U

#ifdef __cplusplus
}
#endif

#endif /* STM32H747I_DISCO_CONF_H */
