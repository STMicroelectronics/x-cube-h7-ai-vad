/**
  ******************************************************************************
  * @file    user_mel_tables.h
  * @author  MCD Application Team
  * @brief   Header for user_mel_tables.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
#ifndef _USER_MEL_TABLES_H
#define _USER_MEL_TABLES_H

#include "arm_math.h"

extern const float32_t user_win[768];
extern const float32_t user_melFiltersLut[949];
extern const float32_t user_melFiltersmeans[40];
extern const float32_t user_melFiltersstds[40];
extern const uint32_t  user_melFiltersStartIndices[40];
extern const uint32_t  user_melFiltersStopIndices[40];

#endif /* _USER_MEL_TABLES_H */
