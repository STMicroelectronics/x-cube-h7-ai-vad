/**
  ******************************************************************************
  * @file    audio_bm.c
  * @author  STMicroelectronics AIS application team
  * @brief   bare metal audio
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "preproc_dpu.h"
#include "audio_bm.h"
#include "cpu_stats.h"
#include "gs_utils.h"
#include "ai_device_adaptor.h"
#include "stm32h747i_discovery_audio.h"
#include "stm32h747i_discovery.h"   /* BSP_LED_On/Off, LED_GREEN */
    
/* Globals--------------------------------------------------------------------*/
AudPreProc_t audio_pre_proc_ctx ;
AudProc_t    audio_proc_ctx ;
volatile int audio_acq_buff_size;
volatile uint8_t * p_acquired_audio_samples;

/* Bytes of newly-acquired audio signalled per acquisition event (AUDIO_ACQ_LEN_MS,
 * 16 ms at the APP_LOW_LAT default / 48 ms at the APP_LOW_LAT_OFF opt-out) via
 * p_acquired_audio_samples/audio_acq_buff_size. Distinct from PATCH_LEN
 * (preproc_dpu.h): that one sizes the larger overlap-save window
 * AudioPreProc_DPU() actually consumes. */
#define AUDIO_ACQ_CHUNK_SIZE (AUDIO_ACQ_LEN * sizeof(int16_t))

/* Overlap-save sliding window: PATCH_LEN samples of context are needed per
 * inference, but only AUDIO_ACQ_LEN new samples arrive per acquisition event.
 * audio_exec_bm() shifts this buffer left by AUDIO_ACQ_LEN samples and
 * appends the new chunk before every AudioPreProc_DPU() call. Samples are
 * never dropped - the model's recurrent state depends on unbroken
 * continuity across inferences. */
static int16_t audio_patch_buf[PATCH_LEN];

#define AUDIO_BM_SAMPLE_RATE        AUDIO_FREQUENCY_16K
#define AUDIO_INPUT_CHANNELS        2U
#define AUDIO_PDM_PCM_PER_CALL      (AUDIO_BM_SAMPLE_RATE / 1000U)   /* 16 PCM samples / channel */
#define AUDIO_STEREO_TMP_SAMPLES    (2U * AUDIO_PDM_PCM_PER_CALL)

#define AUDIO_PREPROC_CHANNEL       0U   /* 0 = first channel, 1 = second channel. First channel must be used. */

/* Temporary stereo PCM produced by PDM->PCM conversion */
static int16_t audio_pcm_tmp[AUDIO_STEREO_TMP_SAMPLES]
    __attribute__((aligned(32)));

/* Raw-PDM circular buffer sizing (BSP_AUDIO_IN_RecordPDM):
 *  PDM_BYTES_PER_CH_PER_MS — raw PDM bytes per channel per 1 ms frame at
 *    decimation 64 (= 16 output samples/ms x decimation factor 64 / 8 bits).
 *  AUDIO_PDM_REF_PCM_RATE  — reference PCM rate (Hz) PDM_BYTES_PER_CH_PER_MS
 *    was derived at, so the buffer scales if AUDIO_BM_SAMPLE_RATE changes.
 *  AUDIO_INPUT_CHANNELS    — number of PDM channels. The digital-mic BSP
 *    path always delivers 2 interleaved channels even though the board has
 *    a single physical microphone; see AudioStereoToPreprocMono() below.
 * Result is a word (uint16_t) count for audio_pdm_raw_buf below. */
#define PDM_BYTES_PER_CH_PER_MS     128U
#define AUDIO_PDM_REF_PCM_RATE      16000U
#define AUDIO_PDM_RAW_WORDS         ((uint32_t)(PDM_BYTES_PER_CH_PER_MS * AUDIO_BM_SAMPLE_RATE \
                                     / AUDIO_PDM_REF_PCM_RATE * AUDIO_INPUT_CHANNELS))

static uint16_t audio_pdm_raw_buf[AUDIO_PDM_RAW_WORDS]
    __attribute__((section(".dma_buffer"), aligned(32)));

static void AudioStereoToPreprocMono(const int16_t *src, int16_t *dst, uint32_t frames)
{
  for (uint32_t i = 0U; i < frames; ++i)
    dst[i] = src[(2U * i) + AUDIO_PREPROC_CHANNEL];
}

/*
 * Audio DMA acquisition double-buffer, cache-coherent via a non-cacheable
 * MPU region so it is a safe BDMA target.
 *
 * Placed in SRAM4 (D3 domain, 0x38000000) via the .dma_buffer linker section
 * defined in STM32H747XIHx_CM7_FLASH.ld.  SRAM4 is not covered by the CM7
 * D-Cache, so BDMA writes (SAI4 Block A, PDM mic) are immediately visible
 * to the CPU without any SCB_InvalidateDCache_by_Addr() call.
 *
 * Size: 2 x AUDIO_ACQ_CHUNK_SIZE bytes = 2 x (AUDIO_ACQ_LEN x sizeof(int16_t))
 *       = 2 x (256 x 2) = 1024 bytes at the APP_LOW_LAT default (2 x (768 x 2)
 *       = 3072 bytes at the APP_LOW_LAT_OFF opt-out); either way < SRAM4 64 KB.
 *       Each half holds one acquisition event's worth of new samples
 *       (overlap-save framing); the larger PATCH_LEN window consumed by
 *       AudioPreProc_DPU() lives in audio_patch_buf instead.
 *
 * 32-byte alignment satisfies the CM7 cache-line requirement should the
 * coherency strategy ever change to explicit SCB_InvalidateDCache_by_Addr()
 * maintenance.
 *
 * audio_pre_proc_ctx.acq_p is initialised to point here by audio_init_bm().
 */
static uint16_t audio_acq_buf[(AUDIO_ACQ_CHUNK_SIZE * 2) / sizeof(uint16_t)]
    __attribute__((section(".dma_buffer"), aligned(32)));

/*
 * PCM accumulation state — written exclusively from BDMA callback context.
 * pcm_write_half : 0 or 1, selects which audio_acq_buf half is being filled.
 * pcm_sample_count : number of int16_t samples written into the current half;
 *                    resets to 0 when a full half is signalled.
 */
static uint32_t pcm_write_half   = 0U;
static uint32_t pcm_sample_count = 0U;

/* Private function prototypes -----------------------------------------------*/
static void StartAudioSensors(void);
static void StopAudioSensors(void);
static void audio_pdm_callback(uint32_t pdm_half_idx);

int audio_de_init_bm(void)
{
  if (DPU_OK != AiDPUReleaseModel(&audio_proc_ctx.ai))
  {
    Error_Handler();
  }
  StopAudioSensors();
  return 0;
}

int audio_init_bm(void)
{
  /* get the AI model   */
  if (DPU_OK != AiDPULoadModel(&audio_proc_ctx.ai))
  {
    LogError("audio_init_bm: AI model load failed\n\r");
    Error_Handler();
  }
  if (DPU_OK != AudioPreProc_DPUInit(&audio_pre_proc_ctx.dpu))
  {
    LogError("audio_init_bm: pre-processing init failed\n\r");
    Error_Handler();
  }

  /* acq_p points at audio_acq_buf in SRAM4 (non-cacheable, BDMA-coherent). */
  audio_pre_proc_ctx.acq_p = (uint8_t *)audio_acq_buf;

  /* Preproc writes the float32 feature tensor straight into the model's own
   * input tensor - no intermediate scratch buffer needed. */
  audio_pre_proc_ctx.out_p = (float *) audio_proc_ctx.ai.p_stai_inputs[0];

  /* Start the overlap-save window from silence. */
  memset(audio_patch_buf, 0, sizeof(audio_patch_buf));

  BSP_LED_Off(LED_GREEN);

#ifdef APP_LCD
  LCD_Init();
  DisplayLCD_VAD();
#endif

  StartAudioSensors();

  return 0;
}

int audio_exec_bm(void)
{
  if (audio_acq_buff_size >= (int)AUDIO_ACQ_CHUNK_SIZE)
  {
    audio_acq_buff_size -= (int)AUDIO_ACQ_CHUNK_SIZE;

    /* Overlap-save: shift the trailing (PATCH_LEN - AUDIO_ACQ_LEN) samples of
     * context to the front of the window, append the AUDIO_ACQ_LEN new ones.
     * Never drop samples here - the recurrent state's continuity across
     * inferences depends on it. */
    memmove(audio_patch_buf, audio_patch_buf + AUDIO_ACQ_LEN,
            (PATCH_LEN - AUDIO_ACQ_LEN) * sizeof(int16_t));
    memcpy(audio_patch_buf + (PATCH_LEN - AUDIO_ACQ_LEN),
           (const void *)p_acquired_audio_samples, AUDIO_ACQ_CHUNK_SIZE);

    /* Never display or act on a probability that this chunk did not produce. */
    if (DPU_OK != AudioPreProc_DPU(&audio_pre_proc_ctx.dpu, (uint8_t *)audio_patch_buf,
                                   audio_pre_proc_ctx.out_p))
    {
      LogError("\r\naudio_exec_bm: pre-processing failed\n\r");
      Error_Handler();
    }
    if (DPU_OK != AiDPUProcess(&audio_proc_ctx.ai))
    {
      LogError("\r\naudio_exec_bm: AI inference failed\n\r");
      Error_Handler();
    }

    PrintVoiceActivity(audio_proc_ctx.ai.out);
    if (audio_proc_ctx.ai.out > VAD_THRESHOLD_PCT)
    {
      BSP_LED_On(LED_GREEN);
    }
    else
    {
      BSP_LED_Off(LED_GREEN);
    }
  }
  return 0;
}

/* Callback Definition -------------------------------------------------------*/

/**
 * @brief  Shared PDM→PCM conversion helper, called from both BDMA callbacks.
 *
 * pdm_half_idx selects which half of audio_pdm_raw_buf was just filled by BDMA:
 *   0 → first half of audio_pdm_raw_buf  (half-transfer)
 *   1 → second half of audio_pdm_raw_buf (full-transfer)
 * (each half is AUDIO_PDM_RAW_WORDS/2 words).
 *
 * Each call converts one raw-PDM half into AUDIO_STEREO_TMP_SAMPLES interleaved
 * stereo PCM samples, keeps only channel AUDIO_PREPROC_CHANNEL (the model
 * expects mono input — see AudioStereoToPreprocMono()), and accumulates
 * AUDIO_PDM_PCM_PER_CALL mono samples into the current half of audio_acq_buf.
 * When the half is full (pcm_sample_count reaches AUDIO_ACQ_LEN samples -
 * 256/16 ms at the APP_LOW_LAT default, 768/48 ms at the APP_LOW_LAT_OFF
 * opt-out), the just-completed half is signalled to audio_exec_bm()
 * via p_acquired_audio_samples and audio_acq_buff_size, and writing
 * switches to the other half.
 *
 * Both audio_pdm_raw_buf and audio_acq_buf live in SRAM4 (non-cacheable MPU
 * region), so BDMA writes are immediately visible and CPU writes go
 * straight to memory — no SCB_InvalidateDCache_by_Addr() needed.
 */
static void audio_pdm_callback(uint32_t pdm_half_idx)
{
  /* Pointer into the just-filled PDM half */
  uint16_t *pdm_ptr = audio_pdm_raw_buf
                      + pdm_half_idx * (AUDIO_PDM_RAW_WORDS / 2U);

  /* Pointer to the current mono write position within audio_acq_buf */
  int16_t *pcm_ptr = (int16_t *)(
      audio_pre_proc_ctx.acq_p
      + pcm_write_half * AUDIO_ACQ_CHUNK_SIZE
      + pcm_sample_count * sizeof(int16_t)
  );

  /* PDM->PCM and the mono downmix are part of the pre-processing cost, but
   * this runs in ISR context once per millisecond: take a DWT delta rather
   * than port_dwt_reset(), which would zero the counter the main loop is
   * timing AudioPreProc_DPU()/AiDPUProcess() with. Own bucket, because these
   * land ~AUDIO_ACQ_LEN_MS times per acquisition event rather than once. */
  uint32_t pdm_cyc_start = port_dwt_get_cycles();
  BSP_AUDIO_IN_PDMToPCM(1U, pdm_ptr, (uint16_t *)audio_pcm_tmp);
  AudioStereoToPreprocMono(audio_pcm_tmp, pcm_ptr, AUDIO_PDM_PCM_PER_CALL);
  time_stats_store(TIME_STAT_PDM_PROC,
                   (port_dwt_get_cycles() - pdm_cyc_start) * 1000.0F
                   / port_hal_get_cpu_freq());

  pcm_sample_count += AUDIO_PDM_PCM_PER_CALL;

  if (pcm_sample_count >= AUDIO_ACQ_LEN)
  {
    /* Acquisition chunk complete - signal audio_exec_bm() */
    p_acquired_audio_samples = audio_pre_proc_ctx.acq_p
                               + pcm_write_half * AUDIO_ACQ_CHUNK_SIZE;
    audio_acq_buff_size     += (int)AUDIO_ACQ_CHUNK_SIZE;
    pcm_write_half          ^= 1U;
    pcm_sample_count         = 0U;
  }
}

/**
 * @brief  BSP audio-in half-transfer callback (BDMA first half filled).
 *         Instance 1 = SAI4/PDM digital mic.
 */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  if (Instance == 1U)
  {
    audio_pdm_callback(0U);
  }
}

/**
 * @brief  BSP audio-in complete-transfer callback (BDMA second half filled).
 *         Instance 1 = SAI4/PDM digital mic.
 */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  if (Instance == 1U)
  {
    audio_pdm_callback(1U);
  }
}

/**
 * @brief  BSP audio-in error callback — fatal, stop the application.
 */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
  (void)Instance;
  Error_Handler();
}

/**
 * @brief  Start SAI4/PDM digital-mic acquisition.
 *
 * Initialises BSP audio-in Instance 1 (SAI4_Block_A, AUDIO_IN_DEVICE_DIGITAL_MIC)
 * at 16 kHz 16-bit, AUDIO_INPUT_CHANNELS-channel (downmixed to mono after
 * PDM→PCM conversion, see audio_pdm_callback), which internally:
 *   - Calls MX_SAI4_ClockConfig (weak BSP function) to set PLL2 → SAI4A clock
 *   - Initialises the BDMA channel (BDMA_Channel1, AUDIO_IN_SAI_PDMx_DMAx_STREAM)
 *   - Calls BSP_AUDIO_IN_PDMToPCM_Init: configures the PDM filter with
 *     decimation_factor=PDM_FILTER_DEC_FACTOR_64, enables the CRC clock
 *     (required by the PDM2PCM library)
 * Then starts circular BDMA reception of raw PDM into audio_pdm_raw_buf.
 */
static void StartAudioSensors(void)
{
  BSP_AUDIO_Init_t audio_init = {0};

  audio_init.Device        = AUDIO_IN_DEVICE_DIGITAL_MIC;
  audio_init.SampleRate    = AUDIO_BM_SAMPLE_RATE;
  audio_init.BitsPerSample = AUDIO_RESOLUTION_16B;
  audio_init.ChannelsNbr   = AUDIO_INPUT_CHANNELS;   /* 2-channel path, like ST sample */
  audio_init.Volume        = 80U;                    /* digital-mic gain, 0-100% scale (BSP), no clipping observed */

  /* Reset PCM accumulation state before starting DMA */
  pcm_write_half = 0U;
  pcm_sample_count = 0U;
  audio_acq_buff_size = 0;
  p_acquired_audio_samples = audio_pre_proc_ctx.acq_p;

  if (BSP_AUDIO_IN_Init(1U, &audio_init) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Start circular BDMA of raw PDM bitstream into audio_pdm_raw_buf.
   * The callbacks (BSP_AUDIO_IN_HalfTransfer_CallBack /
   * BSP_AUDIO_IN_TransferComplete_CallBack) convert PDM→PCM and accumulate
   * PCM samples into audio_acq_buf, signalling audio_exec_bm() per half. */
  if (BSP_AUDIO_IN_RecordPDM(1U, (uint8_t *)audio_pdm_raw_buf,
                              sizeof(audio_pdm_raw_buf)) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
}

/**
 * @brief  Stop SAI4/PDM digital-mic acquisition.
 */
static void StopAudioSensors(void)
{
  if (BSP_AUDIO_IN_Stop(1U) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
}

