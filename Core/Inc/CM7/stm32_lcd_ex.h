/**
  ******************************************************************************
  * @file    stm32_lcd_ex.h
  * @author  GPM Application Team
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32_LCD_EX_H
#define STM32_LCD_EX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* Includes ------------------------------------------------------------------*/
#include "stm32_lcd.h"

#define COL(x)  ((x) * (((sFONT *)UTIL_LCD_GetFont())->Width))

/* Exported functions ------------------------------------------------------- */
/**
 * @brief Formats and displays text at an LCD line.
 * @param line LCD line index.
 * @param format Printf-style format string.
 * @param ... Format arguments.
 */
void UTIL_LCDEx_PrintfAtLine(uint16_t line, const char *format, ...);

/**
 * @brief Formats and displays text at an LCD position.
 * @param x_pos Horizontal pixel position.
 * @param y_pos Vertical pixel position.
 * @param mode Text alignment mode.
 * @param format Printf-style format string.
 * @param ... Format arguments.
 */
void UTIL_LCDEx_PrintfAt(uint32_t x_pos, uint32_t y_pos, Text_AlignModeTypdef mode,
                         const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STM32_LCD_EX_H */
