# Release Notes for x-cube-h7-ai-vad Application

## Purpose

This project provides an STM32 Microcontroler embedded real time environement
to execute
[STEdgeAI core](https://www.st.com/en/development-tools/stedgeai-core.html)
generated model targetting audio applications. The purpose of this package is
to stream physical data acquired by sensors into a processing chain including a
preprocessing step that typically would perform a first level of feature
extraction, the machine learning inference itself, and a post processing step
before exposing the results to the user in real time.

## Key Features

- Support of Voice Activity Detection using TEN-VAD
- Bare Metal (BM) Implementation

## Software components

| Name                                | Version     | Release notes
|-----                                | -------     | -------------
| STEdge AI runtime                   |  v4.1.0     |
| stm32-audio-sensing-processing      |  v1.6.0     | [release notes](Middlewares/ST/stm32-audio-sensing-processing/Release_Notes.html)
| STM32CubeH7                         |  V1.13.0    | [release notes](STM32CubeH7/Release_Notes.html)
| CMSIS                               |  v5.9.0     | [release notes](STM32CubeH7/Drivers/CMSIS/st_readme.txt)
| STM32H7xx CMSIS                     |  V1.10.7    | [release notes](STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Release_Notes.html)
| STM32H7xx_HAL_Drivers               |  V1.11.6    | [release notes](STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Release_Notes.html)
| STM32H747I-DISCO BSP                |  V3.6.4     | [release notes](STM32CubeH7/Drivers/BSP/STM32H747I-DISCO/Release_Notes.html)
| BSP Components - Common             |  V7.3.0     | [release notes](STM32CubeH7/Drivers/BSP/Components/Common/Release_Notes.html)
| BSP Components - IS42S32800J        |  V1.0.0     | [release notes](STM32CubeH7/Drivers/BSP/Components/is42s32800j/Release_Notes.html)
| BSP Components - NT35510            |  V2.0.2     | [release notes](STM32CubeH7/Drivers/BSP/Components/nt35510/Release_Notes.html)
| BSP Components - OTM8009A           |  V2.0.2     | [release notes](STM32CubeH7/Drivers/BSP/Components/otm8009a/Release_Notes.html)
| STM32 Utilities - Fonts             |  V2.0.4     | [release notes](STM32CubeH7/Utilities/Fonts/Release_Notes.html)
| STM32 Utilities - LCD               |  V2.0.3     | [release notes](STM32CubeH7/Utilities/lcd/Release_Notes.html)
| PDM2PCM library                     |  V3.3.0     | [release notes](STM32CubeH7/Middlewares/ST/STM32_Audio/Addons/PDM/Release_Notes.html)

## Update history

### V1.0.0 / October 2026

Initial release

## AI disclosure

*Parts of this software were developed with the assistance of generative AI tools.*
