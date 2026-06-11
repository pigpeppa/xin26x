/***************************************************************************//**
 *
 * @file          h266_common_data.c
 * @brief         This file contains h266 common table data.
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
#include "xin_typedef.h"
#include "h266_constant.h"
#include "h26x_trans_context.h"

const UINT32 log2SbbSize[XIN_MAX_CU_DEPTH + 1][XIN_MAX_CU_DEPTH + 1][2] =
{
    { { 0,0 },{ 0,1 },{ 0,2 },{ 0,3 },{ 0,4 },{ 0,4 },{ 0,4 },{ 0,4 } },
    { { 1,0 },{ 1,1 },{ 1,1 },{ 1,3 },{ 1,3 },{ 1,3 },{ 1,3 },{ 1,3 } },
    { { 2,0 },{ 1,1 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 } },
    { { 3,0 },{ 3,1 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 } },
    { { 4,0 },{ 3,1 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 } },
    { { 4,0 },{ 3,1 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 } },
    { { 4,0 },{ 3,1 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 } },
    { { 4,0 },{ 3,1 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 },{ 2,2 } }
};

const SINT32 bcwWeights[XIN_BCW_NUM] =
{
    (-2), 3, 4, 5, 10
};

const SINT32 bcwSearchOrder[XIN_BCW_NUM] =
{
    XIN_BCW_DEFAULT, XIN_BCW_DEFAULT - 2, XIN_BCW_DEFAULT + 2, XIN_BCW_DEFAULT - 1, XIN_BCW_DEFAULT + 1
};

