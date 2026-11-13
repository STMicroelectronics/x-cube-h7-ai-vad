/**
  ******************************************************************************
  * @file      startup_cm4.s
  * @author    MCD Application Team
  * @brief     STM32H747xx Devices: Cortex-M4 vector table for GCC based
  *            toolchain.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software component is licensed by ST under Apache License, Version 2.0,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/Apache-2.0
  *
  ******************************************************************************
  * This module performs:
  * - Set the initial SP
  * - Set the initial PC == Reset_Handler_m4,
  * - Reset_Handler consists in an infinite loop so to boot CM4 core properly
  *   and keep it in a known state.
  *
  * Design principle P1: The CM4 core is NOT used by this application.
  * Under default option bytes (BOOT_CM4 = 1) the CM4 boots from FLASH_BK2
  * (0x08100000).  This stub places a minimal CM4 vector table at that address
  * whose Reset_Handler is an infinite loop, so the CM4 spins safely without
  * touching any shared peripheral or memory.
  *
  * No CM4 application, RTOS, or inter-core (HSEM/OpenAMP) logic is required.
  * The single CM7 image includes this stub so that one .elf covers both cores.
  */

  .global  g_pfnVectors_m4

/**
 * @brief  This is the code that gets called when the CM4 processor first
 *         starts execution following a reset event. The CM4 stays in an
 *         infinite loop.
 * @param  None
 * @retval : None
 */
    .section  .text.Reset_Handler_m4
  .weak  Reset_Handler_m4
  .type  Reset_Handler_m4, %function
Reset_Handler_m4:
Infinite_Loop:
  wfi                 /* sleep between spins instead of busy-looping */
  b  Infinite_Loop

.size  Reset_Handler_m4, .-Reset_Handler_m4

/******************************************************************************
*
* The minimal vector table for the Cortex-M4.  The section .isr_vector_m4 is
* placed in FLASH_BK2 (0x08100000) by the linker script, so the CM4 finds its
* vector table at the right address when it boots.
*
*******************************************************************************/
   .section  .isr_vector_m4,"a",%progbits
  .type  g_pfnVectors_m4, %object
  .size  g_pfnVectors_m4, .-g_pfnVectors_m4

g_pfnVectors_m4:
  .word  _estack
  .word  Reset_Handler_m4

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
