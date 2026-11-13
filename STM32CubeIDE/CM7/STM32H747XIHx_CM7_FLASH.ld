/*
******************************************************************************
**
** @file        : STM32H747XIHx_CM7_FLASH.ld
**
** @brief       : Linker script for STM32H747xI device — single CM7 image.
**                Based on the STM32H7-GettingStarted-ImageClassification
**                reference script (design principle P1).
**
**  Memory layout (STM32H747I-DISCO):
**    FLASH_BK1 0x0800 0000 — 1024 KB  CM7 .isr_vector, code, rodata, data LMA
**    FLASH_BK2 0x0810 0000 — 1024 KB  CM4 .isr_vector_m4 spin-loop stub
**    AXI_SRAM  0x2400 0000 —  512 KB  D1 domain — CM7 main stack/data/bss
**    DTCM_RAM  0x2000 0000 —  128 KB  CM7 tightly-coupled (zero wait-state)
**    SRAM4     0x3800 0000 —   64 KB  D3 domain — BDMA-accessible DMA buffers
**
**  CM4 boot (design principle P1):
**    Under default STM32H747 option bytes BOOT_CM4 = 1, the CM4 boots from
**    FLASH_BK2 (0x08100000) immediately after reset.  startup_cm4.s places
**    a minimal vector table in .isr_vector_m4 (-> FLASH_BK2) whose
**    Reset_Handler_m4 is an infinite loop, so the CM4 spins in a defined
**    safe state without touching any shared resource.
**    No option-byte change or separate CM4 image is required -- the single
**    CM7 .elf produced by this project covers both cores.
**    Confirm BOOT_CM4 option byte = 1 (factory default) before flashing.
**
**  DMA coherency note:
**    The SAI4 PDM audio path uses BDMA, which can only access D3 SRAM
**    (SRAM4 at 0x38000000).  Buffers declared with
**    __attribute__((section(".dma_buffer"))) are placed here so BDMA can
**    reach them without cache-coherency workarounds.
**
******************************************************************************
** @attention
**
** Copyright (c) 2025 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the CM7 main stack (top of AXI_SRAM) */
_estack = ORIGIN(AXI_SRAM) + LENGTH(AXI_SRAM);  /* 0x24080000 */

/* Minimum heap / stack sizes; linker errors if these don't fit */
_Min_Heap_Size  = 0x800;   /* 2 KB */
_Min_Stack_Size = 0x1000;  /* 4 KB */

/* ---- Memory regions ------------------------------------------------------- */
MEMORY
{
  FLASH_BK1 (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K  /* CM7 code            */
  FLASH_BK2 (rx)  : ORIGIN = 0x08100000, LENGTH = 1024K  /* CM4 spin-loop stub  */
  DTCM_RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 128K   /* CM7 DTCM (fast)     */
  AXI_SRAM  (xrw) : ORIGIN = 0x24000000, LENGTH = 512K   /* D1 AXI SRAM         */
  SRAM4     (xrw) : ORIGIN = 0x38000000, LENGTH = 64K    /* D3 SRAM, BDMA-only  */
}

/* ---- Output sections ------------------------------------------------------ */
SECTIONS
{
  /* CM7 interrupt vector table -- must be the very first thing in FLASH_BK1 */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH_BK1

  /* CM4 spin-loop stub vector table -- placed at start of FLASH_BK2.
   * startup_cm4.s emits .isr_vector_m4 with Reset_Handler_m4 = infinite loop.
   * Under default option bytes (BOOT_CM4=1) the CM4 fetches its reset vector
   * from 0x08100004 and spins safely without touching any shared resource.    */
  .isr_vector_m4 :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector_m4))
    . = ALIGN(4);
  } >FLASH_BK2

  /* Program code */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH_BK1

  /* Read-only data (constants, strings) */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH_BK1

  /* C++ exception tables (READONLY: GCC 11+; safe to remove for GCC 10) */
  .ARM.extab (READONLY) :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH_BK1

  .ARM (READONLY) :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH_BK1

  /* Constructor / destructor tables */
  .preinit_array (READONLY) :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH_BK1

  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH_BK1

  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH_BK1

  /* Initialised data: LMA in FLASH_BK1, VMA in AXI_SRAM */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    *(.RamFunc)    /* Functions declared to run from RAM */
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >AXI_SRAM AT> FLASH_BK1

  /* Zero-initialised data */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >AXI_SRAM

  /* Heap / stack size check -- must come after .bss in the same region */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >AXI_SRAM

  /*
   * DMA audio buffers -- placed in SRAM4 (D3 domain).
   *
   * BDMA (used by SAI4 Block A for PDM microphone acquisition) can ONLY
   * access D3 SRAM.  Buffers placed here require no cache-maintenance because
   * they are outside the AXI SRAM cache domain seen by the CM7 D-Cache.
   * Alignment to 32 bytes satisfies the CM7 cache-line requirement in case a
   * future strategy moves buffers back to cacheable RAM.
   *
   * Usage:
   *   uint16_t __attribute__((section(".dma_buffer"))) audio_buf[N];
   */
  .dma_buffer (NOLOAD) :
  {
    . = ALIGN(32);
    *(.dma_buffer)
    *(.dma_buffer*)
    . = ALIGN(32);
  } >SRAM4

  /* Strip standard-library debugging symbols */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
