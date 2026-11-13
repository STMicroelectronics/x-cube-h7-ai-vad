/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
#include "ai_utility.h"
#include "gs_utils.h"    
#include "audio_bm.h"
#include "stm32h747i_discovery.h" /* BSP_COM_Init() / hcom_uart[] for the console UART */


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
/* SAI4/BDMA handles are managed by the BSP audio driver, not declared here. */
/* The USART1 handle is owned by the BSP (hcom_uart[COM1], declared in
 * stm32h747i_discovery.h) and set up via BSP_COM_Init().                    */

static __IO uint32_t exit_loop = 0;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
static void MPU_Config(void);
/* MPU_Config() + I/D-Cache enabled at top of main(); no ICACHE_Init() needed on H7. */
/* BSP_AUDIO_IN_Init() / BSP_AUDIO_IN_RecordPDM() called from StartAudioSensors() in audio_bm.c */
static void Console_UART_Init(void);
static void exec_bm(void);
static void app_display_info_header(void);

/* Private user code ---------------------------------------------------------*/


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Configure MPU, then enable Cortex-M7 caches.
   * Required order: MPU_Config() → SCB_EnableICache() → SCB_EnableDCache() → HAL_Init().
   * The MPU must be programmed before D-Cache is enabled so the non-cacheable
   * attribute on SRAM4 is already in effect before the first cached access.
   *
   * Coherency strategy — a non-cacheable MPU region:
   *   MPU Region 1 marks all of SRAM4 (0x38000000, 64 KB) as Normal
   *   Non-Cacheable (TEX=1, C=0, B=0).  The DMA audio buffer audio_acq_buf
   *   (.dma_buffer section) lives there, so BDMA writes from SAI4 are
   *   immediately visible to the CPU without any SCB_InvalidateDCache_by_Addr()
   *   call in the audio callbacks.
   *   Reference: STM32CubeH7 PWR/PWR_Domain3SystemControl/CM7/Src/main.c. */
  MPU_Config();
  SCB_EnableICache();
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  GPIO_Init();
  /* BSP_AUDIO_IN_Init() is called from audio_init_bm() → StartAudioSensors() */
  Console_UART_Init();
  app_display_info_header();
  PrintSystemSetting();

  exec_bm();
  
  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  *
  * Target frequencies for STM32H747I-DISCO (HSE = 25 MHz on-board):
  *   SYSCLK (CM7)  = 400 MHz   PLL1P:  HSE/M*N/P = 25/5*160/2
  *   HCLK          = 200 MHz   AHB prescaler /2
  *   APB1/2/3/4    = 100 MHz   APBx prescaler /2
  *   Flash latency = 4 WS      (AXI bus at 200 MHz, VOS1)
  *
  * Peripheral kernel clocks configured here:
  *   USART1 (VCP console) → PCLK2 (100 MHz).
  *
  * SAI4 kernel clock:
  *   Configured by the BSP weak function MX_SAI4_ClockConfig() which is called
  *   automatically by BSP_AUDIO_IN_Init(1, ...) in StartAudioSensors().
  *   For 16 kHz PCM (non-44.1 kHz group) the BSP sets:
  *     PLL2: HSE/M=25/25=1 MHz, N=344, P=7 → PLL2P ≈ 49.1 MHz → SAI4A clock.
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef       RCC_OscInitStruct   = {0};
  RCC_ClkInitTypeDef       RCC_ClkInitStruct   = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /* -----------------------------------------------------------------------
   * 1. Supply configuration + Voltage scaling
   *    The STM32H747I-DISCO hardware power path uses the internal SMPS
   *    directly (PWR_DIRECT_SMPS_SUPPLY).  HAL_PWREx_ConfigSupply() must be
   *    called before requesting VOS1 or PWR_FLAG_VOSRDY never asserts.
   *    VOLTAGE_SCALE1 allows SYSCLK up to 400 MHz.
   * --------------------------------------------------------------------- */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* -----------------------------------------------------------------------
   * 2. Enable HSE and configure PLL1
   *    HSE = 25 MHz  →  PLL1 VCO = 25/5 * 160 = 800 MHz
   *    PLL1P = 800/2 = 400 MHz  (SYSCLK)
   *    PLL1Q = 800/4 = 200 MHz  (USB / kernel clock consumer)
   *    PLL1R = 800/2 = 400 MHz  (unused here)
   * --------------------------------------------------------------------- */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState       = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState       = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM       = 5;
  RCC_OscInitStruct.PLL.PLLN       = 160;
  RCC_OscInitStruct.PLL.PLLFRACN   = 0;
  RCC_OscInitStruct.PLL.PLLP       = 2;
  RCC_OscInitStruct.PLL.PLLQ       = 4;
  RCC_OscInitStruct.PLL.PLLR       = 2;
  RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;    /* VCO range: 192–836 MHz */
  RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2; /* PLL input: 4–8 MHz (25/5=5 MHz) */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* -----------------------------------------------------------------------
   * 3. Select PLL1 as SYSCLK and configure bus dividers
   * --------------------------------------------------------------------- */
  RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                      RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 |
                                      RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;  /* D1 domain: SYSCLK = 400 MHz */
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;   /* HCLK = 200 MHz */
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;   /* APB3 = 100 MHz */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;   /* APB1 = 100 MHz */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;   /* APB2 = 100 MHz */
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;   /* APB4 = 100 MHz */
  /* FLASH_LATENCY_4: required for AXI bus at 200 MHz, VOS1 (RM0399 Table 17) */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /* -----------------------------------------------------------------------
   * 4. Peripheral kernel clocks
   * --------------------------------------------------------------------- */
  /* USART1 (VCP console): source = PCLK2 = 100 MHz. */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* SAI4 kernel clock: configured lazily by MX_SAI4_ClockConfig (BSP weak)
   * when BSP_AUDIO_IN_Init(1) is called from StartAudioSensors(). */
}

/**
  * @brief Console UART Initialization — ST-LINK VCP (PA9 TX, PA10 RX) via the
  *        STM32H747I-DISCO BSP (BSP_COM_Init/COM1). The BSP's own MSP
  *        (stm32h747i_discovery.c) applies a pull-up to PA10 (RX) so the
  *        line does not float when nothing is driving the VCP.
  *        Kernel clock = PCLK2 = 100 MHz.
  * @param None
  * @retval None
  */
static void Console_UART_Init(void)
{
  COM_InitTypeDef COM_Init;

  COM_Init.BaudRate   = USE_UART_BAUDRATE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.StopBits   = COM_STOPBITS_1;
  COM_Init.Parity     = COM_PARITY_NONE;
  COM_Init.HwFlowCtl  = COM_HWCONTROL_NONE;

  if (BSP_COM_Init(COM1, &COM_Init) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();   /* PE2/PE4/PE5: SAI4 PDM clock/data/FS (BSP MSP) */
  __HAL_RCC_GPIOI_CLK_ENABLE();   /* PI12: LED1 */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Configure GPIO pin Output Level: LED1 is active-low, start it off */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /* Configure GPIO pin : USER_BUTTON_Pin (plain input; unused by this app) */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief Executes main loop.
 */
static void exec_bm(void)
{
  unsigned char c;
  
  InitCpuStats();

  audio_init_bm();

  PrintHeader();
  UART_Start_Receive_IT(&hcom_uart[COM1], &c, 1);
  while (exit_loop == 0)
  {
    PrintMenu();
    
    audio_exec_bm();
  
  }
  
  audio_de_init_bm();

  PrintFooter();
  PrintCpuStatsSummary();
}

/**
 * @brief Displays the application information header on the output device.
 *
 * This function is responsible for presenting the header information
 * related to the application, such as version, name, or other relevant
 * details, typically at the start of the program execution.
 */
static void app_display_info_header(void)
{
  printf("========================================\r\n");
  printf("%s %s (%s)\r\n", APP_NAME_STR, APP_VERSION_STRING, APP_GIT_SHA1_STRING);
  printf("Build date & time: %s %s\r\n", __DATE__, __TIME__);
#if defined(__GNUC__)
  printf("Compiler: GCC %d.%d.%d\r\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__ICCARM__)
  printf("Compiler: IAR EWARM %d.%d.%d\r\n", __VER__ / 1000000, (__VER__ / 1000) % 1000 ,__VER__ % 1000);
#else
  printf("Compiler: Unknown\r\n");
#endif
  uint32_t hal_ver = HAL_GetHalVersion();
  printf("HAL: %lu.%lu.%lu\r\n",
         (hal_ver >> 24) & 0xFFUL,
         (hal_ver >> 16) & 0xFFUL,
         (hal_ver >>  8) & 0xFFUL);
  printf("STEdgeAI Tools: %d.%d.%d\r\n", STAI_TOOLS_VERSION_MAJOR, STAI_TOOLS_VERSION_MINOR, STAI_TOOLS_VERSION_MICRO);
  printf("NN model: %s\r\n", STAI_NETWORK_ORIGIN_MODEL_NAME);
  printf("========================================\r\n");
}

/**
 * @brief Stdout redirection helpers for different toolchains.
 *
 * - IAR: Uses __write to transmit each byte over USART1 VCP.
 * - GCC/STM32CubeIDE: Uses _write to transmit the full buffer over USART1 VCP.
 */
#if defined(__ICCARM__) /* For IAR */
/* Redirects stdout to USART1 VCP. */
size_t __write(int Handle, const unsigned char *Buf, size_t Bufsize)
{
  int i;

  for(i=0; i<Bufsize; i++)
  {
    (void)HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)&Buf[i], 1, 1000);
  }

  return Bufsize;
}
#elif defined(__GNUC__)   /* For GCC / STM32CubeIDE */
/* Redirects stdout to USART1 VCP. */
int _write(int fd, char * ptr, int len)
{
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}
#endif


/**
 * @brief  Rx Transfer completed callback
 * @param  UartHandle: UART handle
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  exit_loop = 1;
}

/**
  * @brief  Configure the MPU regions.
  *
  * Region 0 — background catch-all (4 GB, Strongly Ordered, no-access).
  *   SubRegionDisable = 0x87 leaves sub-regions 0/1/2/7 governed by the
  *   default ARMv7-M memory map (SRAM / Flash / peripherals / PPB) so normal
  *   code execution is unaffected.
  *
  * Region 1 — SRAM4 D3 domain (0x38000000, 64 KB), Normal Non-Cacheable
  *   (TEX=1, C=0, B=0).  The audio DMA double-buffer audio_acq_buf
  *   (.dma_buffer section, 62848 B) is placed here.  BDMA-driven SAI4 PDM
  *   writes are immediately visible to the CM7 without cache maintenance.
  *
  * Region 2 — External SDRAM (0xD0000000, 32 MB), Normal Write-Through
  *   Cacheable (TEX=0, C=1, B=0), enabled only when APP_LCD is defined.  The
  *   LTDC layer-0 framebuffer (LCD_LAYER_0_ADDRESS) lives here; CPU writes
  *   (BSP_LCD_* / UTIL_LCD_*) are pushed to memory immediately (write-through)
  *   so the LTDC DMA read side never needs an explicit cache-clean, matching
  *   the vendor LCD_DSI_CmdMode_SingleBuffer example's MPU_Config().
  *
  * Reference: STM32CubeH7 Projects/STM32H747I-DISCO/Examples/PWR/
  *            PWR_Domain3SystemControl/CM7/Src/main.c  MPU_Config().
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disable the MPU before reconfiguring */
  HAL_MPU_Disable();

  /* Region 0: strongly-ordered background for undefined external ranges.
   * BaseAddress = 0x00, Size = 4 GB.
   * SubRegionDisable = 0x87 disables sub-regions 0/1/2 and 7 so the
   * default ARMv7-M attributes still apply to SRAM, Flash, and peripherals. */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x00000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 1: SRAM4 D3 domain — Normal Non-Cacheable (TEX=1, C=0, B=0).
   * Covers the full 64 KB SRAM4 starting at 0x38000000.  The audio DMA
   * double-buffer (audio_acq_buf, .dma_buffer) is the primary occupant; any
   * future BDMA descriptors placed here are also coherent. */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x38000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

#ifdef APP_LCD
  /* Region 2: external SDRAM — Normal Write-Through Cacheable (TEX=0, C=1,
   * B=0).  Holds the LTDC layer-0 framebuffer at LCD_LAYER_0_ADDRESS. */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0xD0000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif /* APP_LCD */

  /* Enable MPU; privileged default allows background region for unmatched
   * privileged accesses (falls through to ARMv7-M default map). */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  /* Do not leave a stale voice-activity indication lit (LED1 is active-low). */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
