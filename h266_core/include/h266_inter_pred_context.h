/***************************************************************************//**
 *
 * @file          h266_inter_pred_context.h
 * @brief         This file contains h266 constant definitions.
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
#ifndef _h266_inter_pred_context_h_
#define _h266_inter_pred_context_h_

#define XIN_MV_FRAC_BITS            4
#define XIN_MV_FRAC_ROUNDING        (1<<(XIN_MV_FRAC_BITS-1))
#define XIN_MV_FRAC_MASK            ((1<<XIN_MV_FRAC_BITS) - 1)

#define XIN_MV_UV_FRAC_BITS         5
#define XIN_MV_UV_FRAC_MASK         ((1<<XIN_MV_UV_FRAC_BITS) - 1)

#define XIN_INTERP_SUB_POS          16
#define XIN_INTERP_UV_SUB_POS       32
#define INTERP_SHIFT                6
#define INTERP_OFFSET               (1 << (INTERP_SHIFT-1))
#define INTERP_SHIFT2               12
#define INTERP_OFFSET2              ((1 << (INTERP_SHIFT2-1)) + (INTERP_PREC_OFFSET<<INTERP_SHIFT))
#define LUMA_INTERP_TAPS            8
#define CHROMA_INTERP_TAPS          4
#define INTERP_PRECISION            14
#define INTERP_PREC_OFFSET          (1 << (INTERP_PRECISION - 1))
#define INTERP_AVG_OFFSET           ((1 << INTERP_SHIFT) + 2*INTERP_PREC_OFFSET)

#define BILINEAR_PREC               10
#define BILINEAR_FILTER_PREC        4
#define BILI_INTERP_TAPS            2
#define BILI_SHIFT                  2
#define BILI_OFFSET                 (1 << (BILI_SHIFT-1))
#define BILI_SHIFT2                 4
#define BILI_OFFSET2                (1 << (BILI_SHIFT2-1))

#define DMVR_NUM_ITERATION          2
#define DMVR_MAX_SUB_SIZE           16
#define BILI_BUF_STRIDE             (XIN_MAX_CU_SIZE*2)
#define DMVR_PAD_BUF_STRIDE         32
#define DMVR_PADING_SIZE            (DMVR_NUM_ITERATION << 1)

#define XIN_INTERP_DEF_FILTER       0
#define XIN_INTERP_ALT_FILTER       1
#define XIN_INTERP_4x4_FILTER       2

#endif

