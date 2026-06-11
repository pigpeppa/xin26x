/***************************************************************************//**
 *
 * @file          h26x_trans_context.h
 * @brief         This file contains h26x constant definitions.
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
#ifndef _h26x_trans_context_h_
#define _h26x_trans_context_h_

// definitions for h26x transform
#define XIN_TR_COEFF_ZERO_OUT_TH    32
#define XIN_TR_MATRIX_SHIFT         6
#define XIN_TR_MAX_LG_RANGE         15
#define XIN_MAX_LG_TU_SIZE          6
#define XIN_MIN_LG_TU_SIZE          2
#define XIN_MAX_TU_SIZE             (1 << XIN_MAX_LG_TU_SIZE)
#define XIN_MIN_TU_SIZE             (1 << XIN_MIN_LG_TU_SIZE)

//EMT transform tags
typedef enum xin_trans_type
{
    XIN_DCT2       = 0,
    XIN_DCT8       = 1,
    XIN_DST7       = 2,
    XIN_TRANS_TYPE = 3,
    XIN_SKIP       = 4
} xin_trans_type;

typedef enum xin_mts_idx
{
    XIN_MTS_DCT2_DCT2 = 0,
    XIN_MTS_SKIP      = 1,
    XIN_MTS_DST7_DST7 = 2,
    XIN_MTS_DCT8_DST7 = 3,
    XIN_MTS_DST7_DCT8 = 4,
    XIN_MTS_DCT8_DCT8 = 5,
    XIN_MTS_IDX_NUM   = 6
} xin_mts_idx;

extern const COEFF trCoreDCT2P2[2][2][2];
extern const COEFF trCoreDCT2P4[2][4][4];
extern const COEFF trCoreDCT2P8[2][8][8];
extern const COEFF trCoreDCT2P16[2][16][16];
extern const COEFF trCoreDCT2P32[2][32][32];
extern const COEFF trCoreDCT2P64[2][64][64];
extern const COEFF trCoreDCT8P4[2][4][4];
extern const COEFF trCoreDCT8P8[2][8][8];
extern const COEFF trCoreDCT8P16[2][16][16];
extern const COEFF trCoreDCT8P32[2][32][32];
extern const COEFF trCoreDST7P4[2][4][4];
extern const COEFF trCoreDST7P8[2][8][8];
extern const COEFF trCoreDST7P16[2][16][16];
extern const COEFF trCoreDST7P32[2][32][32];
extern const SINT8 mtsIdx2txType[XIN_MTS_IDX_NUM][2];

#endif
