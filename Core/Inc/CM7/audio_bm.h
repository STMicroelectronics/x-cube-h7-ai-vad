/**
  ******************************************************************************
  * @file   : audio_bm.h
  * @author : STMicroelectronics AIS application team
  * @brief  : Main program body
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
#ifndef __AUDIO_BM_H__
#define __AUDIO_BM_H__
#include "ai_dpu.h"
#include "preproc_dpu.h"

typedef struct _AudPreProc_t
{
   AudioPreProcCtx_t dpu;
   /* Pointer to the DMA acquisition double-buffer.
    * The actual storage (audio_acq_buf) is declared in audio_bm.c with
    * __attribute__((section(".dma_buffer"), aligned(32))) so it lands in
    * SRAM4 (D3 domain, 0x38000000).  MPU Region 1 (configured by
    * MPU_Config() in main.c before SCB_EnableDCache()) marks the whole
    * 64 KB SRAM4 as Normal Non-Cacheable (TEX=1, C=0, B=0), so BDMA
    * writes (SAI4 PDM path) are immediately visible to the CPU
    * without any SCB_InvalidateDCache_by_Addr() call.
    * Initialised to audio_acq_buf by audio_init_bm(). */
   uint8_t          *acq_p;
   float*            out_p;
}AudPreProc_t;

typedef struct _AudProc_t
{
  AIProcCtx_t ai;
}AudProc_t;

extern int audio_init_bm(void);
extern int audio_de_init_bm(void);
extern int audio_exec_bm(void);

#endif /* __AUDIO_BM_H__ */
