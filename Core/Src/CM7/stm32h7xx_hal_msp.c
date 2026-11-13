/**
  ******************************************************************************
  * @file         stm32h7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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
#include "stm32h747i_discovery_audio.h"  /* MX_SAI_Config_t — for MX_SAI4_Block_A_Init override */

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* External functions --------------------------------------------------------*/

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* NOTE: On STM32H7 the PWR clock is always enabled; no explicit
   *       __HAL_RCC_PWR_CLK_ENABLE() call is required here.           */
}

/**
  * @brief UART MSP Initialization
  * GPIO (PA9/PA10 AF7, pull-up on RX) and the USART1 clock are configured by
  * the BSP's own USART1_MspInit() in stm32h747i_discovery.c, invoked by
  * BSP_COM_Init() before HAL_UART_Init() runs. This override only adds what
  * the BSP does not: the NVIC IRQ needed for interrupt-driven console Rx.
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART1)
  {
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }

}

/**
  * @brief UART MSP De-Initialization
  * GPIO/clock teardown is handled by the BSP's USART1_MspDeInit() (called via
  * BSP_COM_DeInit(), unused by this app); only the NVIC IRQ is undone here.
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART1)
  {
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }

}
