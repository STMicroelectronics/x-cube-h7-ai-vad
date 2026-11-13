/**
  ******************************************************************************
  * @file    stm32h7xx_hal_conf.h
  * @author  MCD Application Team
  * @brief   HAL configuration file for STM32H747I-DISCO (CM7 core).
  *          Derived from stm32h7xx_hal_conf_template.h; only the modules used
  *          by the VAD getting-started application are enabled.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7xx_HAL_CONF_H
#define STM32H7xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ########################## Module Selection ############################## */
/**
  * @brief Enable only the HAL modules required by the VAD application.
  *        Disabled modules are not compiled, keeping code-size in check.
  *
  *  Required modules:
  *    CORTEX   – SCB, NVIC helpers
  *    DMA      – standard DMA (streams)
  *    MDMA     – master DMA (used by some BSP drivers)
  *    EXTI     – GPIO external interrupts
  *    FLASH    – flash programming / latency
  *    GPIO     – general-purpose I/O
  *    I2C      – BSP Discovery bus (codec control)
  *    PWR      – power / voltage-scaling
  *    RCC      – clocks
  *    SAI      – serial audio interface (PDM mic input)
  *    UART     – console (USART1 / ST-LINK VCP)
  *    LTDC     – LCD-TFT display controller (APP_LCD)
  *    DSI      – MIPI-DSI host, drives the OTM8009A panel (APP_LCD)
  *    DMA2D    – Chrom-ART, used by BSP_LCD_FillRGBRect() (APP_LCD)
  *    SDRAM    – external SDRAM used as the LCD framebuffer (APP_LCD)
  */
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_MDMA_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SAI_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_LTDC_MODULE_ENABLED
#define HAL_DSI_MODULE_ENABLED
#define HAL_DMA2D_MODULE_ENABLED
#define HAL_SDRAM_MODULE_ENABLED

/* ########################## Oscillator Values adaptation ################## */
/**
  * @brief External High Speed oscillator (HSE) value.
  *        STM32H747I-DISCO board crystal: 25 MHz.
  */
#if !defined  (HSE_VALUE)
#define HSE_VALUE    (25000000UL)
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    (100UL)
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief Internal oscillator (CSI) default value (4 MHz after reset).
  */
#if !defined  (CSI_VALUE)
  #define CSI_VALUE    (4000000UL)
#endif /* CSI_VALUE */

/**
  * @brief Internal High Speed oscillator (HSI) value (64 MHz).
  */
#if !defined  (HSI_VALUE)
  #define HSI_VALUE    (64000000UL)
#endif /* HSI_VALUE */

/**
  * @brief External Low Speed oscillator (LSE) value.
  */
#if !defined  (LSE_VALUE)
  #define LSE_VALUE    (32768UL)
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    (5000UL)
#endif /* LSE_STARTUP_TIMEOUT */

#if !defined  (LSI_VALUE)
  #define LSI_VALUE  (32000UL)
#endif /* LSI_VALUE */

/**
  * @brief External clock source for I2S/SAI peripheral.
  *        Not used by this application (PDM2PCM library drives decimation
  *        from the SAI bit clock).  Set to 0 to suppress the default.
  */
#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    (12288000UL)
#endif /* EXTERNAL_CLOCK_VALUE */

/* ########################## System Configuration ########################## */
/**
  * @brief Interrupt priority group: 4 preemption bits, 0 sub-priority bits.
  */
#define  TICK_INT_PRIORITY            (0x0FUL)
#define  USE_RTOS                          0U
#define  PREFETCH_ENABLE                   1U

/* ########## SysTick helper ############################################### */
#define  USE_HAL_ADC_REGISTER_CALLBACKS         0U
#define  USE_HAL_CEC_REGISTER_CALLBACKS         0U
#define  USE_HAL_COMP_REGISTER_CALLBACKS        0U
#define  USE_HAL_CORDIC_REGISTER_CALLBACKS      0U
#define  USE_HAL_CRYP_REGISTER_CALLBACKS        0U
#define  USE_HAL_DAC_REGISTER_CALLBACKS         0U
#define  USE_HAL_DCMI_REGISTER_CALLBACKS        0U
#define  USE_HAL_DFSDM_REGISTER_CALLBACKS       0U
#define  USE_HAL_DMA2D_REGISTER_CALLBACKS       0U
#define  USE_HAL_DSI_REGISTER_CALLBACKS         0U
#define  USE_HAL_DTS_REGISTER_CALLBACKS         0U
#define  USE_HAL_ETH_REGISTER_CALLBACKS         0U
#define  USE_HAL_FDCAN_REGISTER_CALLBACKS       0U
#define  USE_HAL_FMAC_REGISTER_CALLBACKS        0U
#define  USE_HAL_GFXMMU_REGISTER_CALLBACKS      0U
#define  USE_HAL_HASH_REGISTER_CALLBACKS        0U
#define  USE_HAL_HCD_REGISTER_CALLBACKS         0U
#define  USE_HAL_HRTIM_REGISTER_CALLBACKS       0U
#define  USE_HAL_I2C_REGISTER_CALLBACKS         0U
#define  USE_HAL_I2S_REGISTER_CALLBACKS         0U
#define  USE_HAL_IRDA_REGISTER_CALLBACKS        0U
#define  USE_HAL_JPEG_REGISTER_CALLBACKS        0U
#define  USE_HAL_LPTIM_REGISTER_CALLBACKS       0U
#define  USE_HAL_LTDC_REGISTER_CALLBACKS        0U
#define  USE_HAL_MDIOS_REGISTER_CALLBACKS       0U
#define  USE_HAL_MMC_REGISTER_CALLBACKS         0U
#define  USE_HAL_NAND_REGISTER_CALLBACKS        0U
#define  USE_HAL_NOR_REGISTER_CALLBACKS         0U
#define  USE_HAL_OPAMP_REGISTER_CALLBACKS       0U
#define  USE_HAL_OSPI_REGISTER_CALLBACKS        0U
#define  USE_HAL_OTFDEC_REGISTER_CALLBACKS      0U
#define  USE_HAL_PCD_REGISTER_CALLBACKS         0U
#define  USE_HAL_PSSI_REGISTER_CALLBACKS        0U
#define  USE_HAL_QSPI_REGISTER_CALLBACKS        0U
#define  USE_HAL_RNG_REGISTER_CALLBACKS         0U
#define  USE_HAL_RTC_REGISTER_CALLBACKS         0U
#define  USE_HAL_SAI_REGISTER_CALLBACKS         0U
#define  USE_HAL_SD_REGISTER_CALLBACKS          0U
#define  USE_HAL_SDIO_REGISTER_CALLBACKS        0U
#define  USE_HAL_SDRAM_REGISTER_CALLBACKS       0U
#define  USE_HAL_SMARTCARD_REGISTER_CALLBACKS   0U
#define  USE_HAL_SMBUS_REGISTER_CALLBACKS       0U
#define  USE_HAL_SPI_REGISTER_CALLBACKS         0U
#define  USE_HAL_SPDIFRX_REGISTER_CALLBACKS     0U
#define  USE_HAL_SRAM_REGISTER_CALLBACKS        0U
#define  USE_HAL_SWPMI_REGISTER_CALLBACKS       0U
#define  USE_HAL_TIM_REGISTER_CALLBACKS         0U
#define  USE_HAL_UART_REGISTER_CALLBACKS        0U
#define  USE_HAL_USART_REGISTER_CALLBACKS       0U
#define  USE_HAL_WWDG_REGISTER_CALLBACKS        0U

/* ########################## Includes ###################################### */
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32h7xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32h7xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32h7xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_MDMA_MODULE_ENABLED
  #include "stm32h7xx_hal_mdma.h"
#endif /* HAL_MDMA_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32h7xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32h7xx_hal_exti.h"
#endif /* HAL_EXTI_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32h7xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
  #include "stm32h7xx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32h7xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_SAI_MODULE_ENABLED
  #include "stm32h7xx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
  #include "stm32h7xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_LTDC_MODULE_ENABLED
  #include "stm32h7xx_hal_ltdc.h"
#endif /* HAL_LTDC_MODULE_ENABLED */

#ifdef HAL_DSI_MODULE_ENABLED
  #include "stm32h7xx_hal_dsi.h"
#endif /* HAL_DSI_MODULE_ENABLED */

#ifdef HAL_DMA2D_MODULE_ENABLED
  #include "stm32h7xx_hal_dma2d.h"
#endif /* HAL_DMA2D_MODULE_ENABLED */

#ifdef HAL_SDRAM_MODULE_ENABLED
  #include "stm32h7xx_hal_sdram.h"
#endif /* HAL_SDRAM_MODULE_ENABLED */

/* Exported macro ----------------------------------------------------------- */
#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* STM32H7xx_HAL_CONF_H */
