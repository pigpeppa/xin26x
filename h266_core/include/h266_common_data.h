/***************************************************************************//**
 *
 * @file          h266_common_data.h
 * @brief         This file contains common tables for h266.
 *
 * @authors       Chao Zhou
 *
 * Xin26x Video Codec Library
 *
 * Copyright (C) 2020-2026 Chao Zhou <czhou2@qq.com>
 *
 * This file is part of Xin26x.
 *
 * Licensed under the GNU General Public License, Version 3 or later
 * (GPL-3.0-or-later). See the LICENSE file for details.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 *******************************************************************************/
#ifndef _h266_common_data_h_
#define _h266_common_data_h_

#include "h26x_common_data.h"

extern const UINT32 log2SbbSize[XIN_MAX_CU_DEPTH + 1][XIN_MAX_CU_DEPTH + 1][2];

extern const SINT32 bcwWeights[XIN_BCW_NUM];

extern const SINT32 bcwSearchOrder[XIN_BCW_NUM];

#endif

