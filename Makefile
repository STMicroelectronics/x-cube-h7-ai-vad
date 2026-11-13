##########################################################################
# Makefile for x-cube-h7-ai-vad
# Mirrors the STM32CubeIDE Debug/Release managed-build configuration.
# Toolchain: Cortex-M7 / STM32H747I-DISCO.
#
# Usage:
#   make              -> Debug build (default)
#   make DEBUG=1      -> Debug build (explicit)
#   make DEBUG=0      -> Release build
#   make clean        -> Remove build output
#
# Objects do not depend on the command-line options (AI_MODEL, EXTRA_DEFS,
# APP_*_STRING): run `make clean` whenever one of them changes.
##########################################################################

TARGET   := x-cube-h7-ai-vad
DEBUG    ?= 1
AI_MODEL ?= full

VALID_AI_MODELS := partial full
ifneq ($(words $(AI_MODEL)),1)
  $(error AI_MODEL must be exactly one of: $(VALID_AI_MODELS))
endif
ifeq ($(filter $(AI_MODEL),$(VALID_AI_MODELS)),)
  $(error Unsupported AI_MODEL '$(AI_MODEL)'; expected one of: $(VALID_AI_MODELS))
endif

# AI_MODEL_FULL selects the generated fully quantized network.
ifeq ($(AI_MODEL),full)
  AI_MODEL_DEFS := -DAI_MODEL_FULL
else
  AI_MODEL_DEFS :=
endif

ifeq ($(DEBUG),1)
  BUILD_DIR := Debug
else
  BUILD_DIR := Release
endif

######################################
# Toolchain
######################################
PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
AS       := $(PREFIX)gcc
CP       := $(PREFIX)objcopy
SZ       := $(PREFIX)size
HEX      := $(CP) -O ihex
BIN      := $(CP) -O binary -S

######################################
# CPU / FPU flags (Cortex-M7, double-precision hard FP)
######################################
CPU_FLAGS := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

######################################
# C defines
######################################
# EXTRA_DEFS: additional -D flags appended by the caller, e.g.
#   make clean && make EXTRA_DEFS=-DAPP_LOW_LAT_OFF   (opt out of the low-latency
#                                                     default, build the 48 ms cadence)
EXTRA_DEFS ?=

# Optional UART header strings (see app_display_info_header() in
# Core/Src/CM7/main.c). The values are computed by the caller, e.g.:
#   make APP_GIT_SHA1_STRING=$(git rev-parse --short HEAD) APP_VERSION_STRING=1.2.3
ifdef APP_GIT_SHA1_STRING
EXTRA_DEFS += -DAPP_GIT_SHA1_STRING=\"$(APP_GIT_SHA1_STRING)\"
endif
ifdef APP_VERSION_STRING
EXTRA_DEFS += -DAPP_VERSION_STRING=\"$(APP_VERSION_STRING)\"
endif

C_DEFS_COMMON := \
  -DUSE_HAL_DRIVER \
  -DSTM32H747xx \
  -DCORE_CM7 \
  -DHAVE_NETWORK_INFO \
  $(AI_MODEL_DEFS) \
  $(EXTRA_DEFS)

ifeq ($(DEBUG),1)
  C_DEFS := $(C_DEFS_COMMON) -DDEBUG
  AS_DEFS := -DDEBUG
  OPT    := -O0
  DBG    := -g3
else
  C_DEFS := $(C_DEFS_COMMON)
  AS_DEFS :=
  OPT    := -O3
  DBG    := -g0
endif

######################################
# Include paths
######################################
C_INCLUDES := \
  -ICore/Inc/CM7 \
  -IDpu/Inc \
  -IModel/$(AI_MODEL) \
  -IModel \
  -IModel/Utilities \
  -IMiddlewares/ST/stedgeai-lib/Inc \
  -IMiddlewares/ST/stm32-audio-sensing-processing/Inc \
  -ISTM32CubeH7/Drivers/BSP/Components/Common \
  -ISTM32CubeH7/Drivers/BSP/Components/otm8009a \
  -ISTM32CubeH7/Drivers/BSP/Components/nt35510 \
  -ISTM32CubeH7/Drivers/BSP/Components/is42s32800j \
  -ISTM32CubeH7/Drivers/BSP/STM32H747I-DISCO \
  -ISTM32CubeH7/Utilities/lcd \
  -ISTM32CubeH7/Utilities/Fonts \
  -IMiddlewares/ST/STM32_Audio/Addons/PDM/Inc \
  -ISTM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Inc \
  -ISTM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
  -ISTM32CubeH7/Drivers/CMSIS/Include \
  -ISTM32CubeH7/Drivers/CMSIS/Core/Include \
  -ISTM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Include \
  -ISTM32CubeH7/Drivers/CMSIS/DSP/Include \
  -ISTM32CubeH7/Drivers/CMSIS/DSP/Include/dsp

######################################
# C compiler flags
######################################
CFLAGS := $(CPU_FLAGS) $(OPT) $(DBG) $(C_DEFS) $(C_INCLUDES) \
  -Wall \
  -ffunction-sections \
  -fdata-sections \
  -fstack-usage \
  --param max-inline-insns-auto=10 \
  -std=gnu11 \
  -MMD -MP

######################################
# Assembler flags
######################################
ASFLAGS := $(CPU_FLAGS) $(DBG) $(AS_DEFS) \
  -x assembler-with-cpp \
  -MMD -MP

######################################
# Linker
######################################
LDSCRIPT := STM32CubeIDE/CM7/STM32H747XIHx_CM7_FLASH.ld

LDFLAGS := $(CPU_FLAGS) \
  -T$(LDSCRIPT) \
  --specs=nano.specs \
  -u _printf_float \
  -Wl,--no-warn-rwx-segments \
  -Wl,--gc-sections \
  -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
  -static \
  -Wl,--start-group -lc -lm -Wl,--end-group \
  -LMiddlewares/ST/stedgeai-lib/Lib/GCC/STM32H7 \
  -l:NetworkRuntime1210_CM7_GCC.a \
  -LMiddlewares/ST/STM32_Audio/Addons/PDM/Lib \
  -l:libPDMFilter_CM7_GCC_wc32.a

######################################
# Application – Core
######################################
C_SOURCES := \
  Core/Src/CM7/audio_bm.c \
  Core/Src/CM7/cpu_stats.c \
  Core/Src/CM7/gs_utils.c \
  Core/Src/CM7/logging.c \
  Core/Src/CM7/main.c \
  Core/Src/CM7/stm32_lcd_ex.c \
  Core/Src/CM7/stm32h7xx_hal_msp.c \
  Core/Src/CM7/stm32h7xx_it.c \
  Core/Src/CM7/system_stm32h7xx.c

######################################
# Application – DPU
######################################
C_SOURCES += \
  Dpu/Src/ai_dpu.c \
  Dpu/Src/preproc_dpu.c \
  Dpu/Src/user_mel_tables.c

######################################
# Application – AI Model
######################################
C_SOURCES += \
  Model/$(AI_MODEL)/network.c \
  Model/$(AI_MODEL)/network_data.c \
  Model/Utilities/ai_device_adaptor.c \
  Model/Utilities/ai_helper_ST_AI.c \
  Model/Utilities/ai_utility.c

######################################
# Middlewares – Audio Sensing Processing
######################################
C_SOURCES += \
  Middlewares/ST/stm32-audio-sensing-processing/Src/audio_din.c \
  Middlewares/ST/stm32-audio-sensing-processing/Src/feature_extraction.c \
  Middlewares/ST/stm32-audio-sensing-processing/Src/mel_filterbank.c \
  Middlewares/ST/stm32-audio-sensing-processing/Src/window.c

######################################
# STM32H747I-DISCO BSP (audio-in digital mic, LCD/SDRAM for APP_LCD)
######################################
C_SOURCES += \
  STM32CubeH7/Drivers/BSP/STM32H747I-DISCO/stm32h747i_discovery_audio.c \
  STM32CubeH7/Drivers/BSP/STM32H747I-DISCO/stm32h747i_discovery.c \
  STM32CubeH7/Drivers/BSP/STM32H747I-DISCO/stm32h747i_discovery_bus.c \
  STM32CubeH7/Drivers/BSP/STM32H747I-DISCO/stm32h747i_discovery_lcd.c \
  STM32CubeH7/Drivers/BSP/STM32H747I-DISCO/stm32h747i_discovery_sdram.c \
  STM32CubeH7/Drivers/BSP/Components/otm8009a/otm8009a.c \
  STM32CubeH7/Drivers/BSP/Components/otm8009a/otm8009a_reg.c \
  STM32CubeH7/Drivers/BSP/Components/nt35510/nt35510.c \
  STM32CubeH7/Drivers/BSP/Components/nt35510/nt35510_reg.c \
  STM32CubeH7/Drivers/BSP/Components/is42s32800j/is42s32800j.c

######################################
# Utilities — LCD text/graphics layer (APP_LCD)
######################################
# stm32_lcd.c #includes every Fonts/font*.c directly (see its own source) -
# do NOT also list the font .c files here, or their symbols are defined twice.
C_SOURCES += \
  STM32CubeH7/Utilities/lcd/stm32_lcd.c

######################################
# STM32H7xx HAL Driver
######################################
C_SOURCES += \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
  STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c

######################################
# CMSIS DSP (from STM32CubeH7)
######################################
C_SOURCES += \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_dot_prod_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_mult_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_scale_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/CommonTables/arm_const_structs.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_squared_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/SupportFunctions/arm_q15_to_float.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_init_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix8_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_rfft_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_rfft_fast_f32.c \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_rfft_fast_init_f32.c

######################################
# Startup / syscalls (from STM32CubeIDE/)
######################################
C_SOURCES += \
  STM32CubeIDE/CM7/syscalls.c \
  STM32CubeIDE/CM7/sysmem.c

######################################
# Assembly sources
######################################
ASM_SOURCES := \
  STM32CubeIDE/startup_stm32h747xx.s \
  STM32CubeIDE/startup_cm4.s \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S

######################################
# Build rules
######################################
# C objects
C_OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))

# Assembly objects – handle both .s (lowercase) and .S (uppercase) extensions
ASM_OBJECTS := $(addprefix $(BUILD_DIR)/,$(patsubst %.S,%.o,$(patsubst %.s,%.o,$(ASM_SOURCES))))

ALL_OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)
DEPS        := $(ALL_OBJECTS:.o=.d)

.PHONY: all clean size hex bin

all: $(BUILD_DIR)/$(TARGET).elf size

# Link
$(BUILD_DIR)/$(TARGET).elf: $(ALL_OBJECTS) $(LDSCRIPT)
	$(CC) $(ALL_OBJECTS) $(LDFLAGS) -o $@

# C source -> object
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MF"$(@:.o=.d)" -MT"$@" -c $< -o $@

# Assembly (.s lowercase)
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -MF"$(@:.o=.d)" -MT"$@" -c $< -o $@

# Assembly (.S uppercase)
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -MF"$(@:.o=.d)" -MT"$@" -c $< -o $@

# arm_bitreversal2 exists as both .c and .S in the same directory; without this
# explicit rule the generic "%.o: %.c" rule above silently wins (declared
# first, and the .c also happens to exist), building the wrong implementation
# of the ASM_SOURCES entry listed below. Pin it to the .S explicitly.
$(BUILD_DIR)/STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.o: \
  STM32CubeH7/Drivers/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -MF"$(@:.o=.d)" -MT"$@" -c $< -o $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

hex: $(BUILD_DIR)/$(TARGET).elf
	$(HEX) $< $(BUILD_DIR)/$(TARGET).hex

bin: $(BUILD_DIR)/$(TARGET).elf
	$(BIN) $< $(BUILD_DIR)/$(TARGET).bin

clean:
	rm -rf Debug Release

-include $(DEPS)
