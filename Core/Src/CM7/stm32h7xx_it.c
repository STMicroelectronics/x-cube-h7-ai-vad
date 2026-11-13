/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32h7xx_it.h"
#include "stm32h747i_discovery_audio.h"
#include "stm32h747i_discovery.h" /* hcom_uart[] / COM1 for the console UART IRQ */


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Private user code ---------------------------------------------------------*/


/* External variables --------------------------------------------------------*/
/* USART1 handle owned by the BSP: hcom_uart[COM1] (declared in
 * stm32h747i_discovery.h). */


/******************************************************************************/
/* Cortex-M7 Processor Exception Handlers                                     */
/******************************************************************************/
/* NOTE: SecureFault_Handler removed (TrustZone / M33 only, not on CM7).      */
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
   while (1)
  {
  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {

  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  while (1)
  {

  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{

}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{

}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{

}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h747xx.s).                  */
/******************************************************************************/

/**
  * @brief  SAI4 interrupt handler.
  *         Routes to the HAL SAI IRQ handler for SAI event processing
  *         (errors, underrun, etc.) on the digital-mic SAI4_Block_A path.
  */
void SAI4_IRQHandler(void)
{
  HAL_SAI_IRQHandler(&haudio_in_sai);
}

/**
  * @brief  BDMA Channel 1 interrupt handler.
  *         SAI4/PDM digital-mic DMA transfer: calls the BSP IRQ handler which
  *         dispatches HAL_DMA_IRQHandler → HAL_SAI_RxHalfCpltCallback /
  *         HAL_SAI_RxCpltCallback → BSP_AUDIO_IN_HalfTransfer_CallBack(1) /
  *         BSP_AUDIO_IN_TransferComplete_CallBack(1) → audio_pdm_callback().
  */
void BDMA_Channel1_IRQHandler(void)
{
  BSP_AUDIO_IN_IRQHandler(1U, AUDIO_IN_DEVICE_DIGITAL_MIC);
}
/**
  * @brief  USART1 interrupt handler (VCP console).
  *         Dispatches to HAL_UART_IRQHandler which triggers
  *         HAL_UART_RxCpltCallback (exit_loop flag) on single-byte Rx.
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&hcom_uart[COM1]);
}
