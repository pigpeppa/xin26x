/***************************************************************************//**
 *
 * @file          h26x_sao_context.h
 * @brief         This file contains SAO-related definitions.
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
#ifndef _h26x_sao_context_h_
#define _h26x_sao_context_h_

// Surrounding ctu availability 
#define XIN_LFT_CTU_AVAIL           (1 << 0)
#define XIN_RGT_CTU_AVAIL           (1 << 1)
#define XIN_TOP_CTU_AVAIL           (1 << 2)
#define XIN_BOT_CTU_AVAIL           (1 << 3)

#define XIN_NUM_SAO_BO_CLASS        32
#define XIN_NUM_SAO_BO_CLASSES_LOG2 5

#define XIN_SAO_EO_0                0
#define XIN_SAO_EO_1                1
#define XIN_SAO_EO_2                2
#define XIN_SAO_EO_3                3
#define XIN_SAO_BO                  4

#define XIN_NUM_SAO_EO              4

#define XIN_SAO_EO_CLASS_FV         0
#define XIN_SAO_EO_CLASS_HV         1
#define XIN_SAO_EO_CLASS_PL         2
#define XIN_SAO_EO_CLASS_HP         3
#define XIN_SAO_EO_CLASS_FP         4
#define XIN_NUM_SAO_EO_CLASS        5

#define XIN_MAX_SAO_OFFSET          7

#define XIN_SAO_GT_PERIOD           13
#define XIN_SAO_LUMA_FULL_MASK      0x1F
#define XIN_SAO_CHROMA_FULL_MASK    0x1F
#define XIN_SAO_LUMA_EO_MASK        0x0F
#define XIN_SAO_CHROMA_EO_MASK      0x0F

#endif

