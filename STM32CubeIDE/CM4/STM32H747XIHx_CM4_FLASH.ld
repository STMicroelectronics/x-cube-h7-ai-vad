/*
******************************************************************************
**
** @file        : STM32H747XIHx_CM4_FLASH.ld
**
** @brief       : Linker script for the CM4 structural placeholder project.
**                Links the SAME startup_cm4.s spin-loop stub already baked
**                into the CM7 image (see STM32CubeIDE/CM7/STM32H747XIHx_CM7_
**                FLASH.ld, design principle P1) so this project builds a
**                standalone, harmless .elf for IDE parity only -- it is not
**                meant to be flashed separately from the CM7 image.
**
**  Memory used:
**    FLASH_BK2 0x0810 0000 — 1024 KB  CM4 boot bank (.isr_vector_m4 + code)
**    D2_SRAM1  0x3000 0000 —  128 KB  CM4-side RAM, used only for _estack
**
******************************************************************************
** @attention
**
** Copyright (c) 2026 STMicroelectronics.
** All rights reserved.
**
** This software component is licensed by ST under Apache License, Version 2.0,
** the "License"; You may not use this file except in compliance with the
** License. You may obtain a copy of the License at:
**                        opensource.org/licenses/Apache-2.0
**
******************************************************************************
*/

ENTRY(Reset_Handler_m4)

_estack = ORIGIN(D2_SRAM1) + LENGTH(D2_SRAM1);

MEMORY
{
  FLASH_BK2 (rx)  : ORIGIN = 0x08100000, LENGTH = 1024K
  D2_SRAM1  (xrw) : ORIGIN = 0x30000000, LENGTH = 128K
}

SECTIONS
{
  .isr_vector_m4 :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector_m4))
    . = ALIGN(4);
  } >FLASH_BK2

  .text :
  {
    . = ALIGN(4);
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH_BK2

  /DISCARD/ :
  {
    *(.ARM.exidx*)
    *(.ARM.extab*)
  }
}
