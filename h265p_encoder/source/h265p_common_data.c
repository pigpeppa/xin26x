/***************************************************************************//**
 *
 * @file          h265p_common_data.c
 * @brief         This file contains av1 common table data.
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
#include "h265p_constant.h"
#include "h265p_trans_context.h"
#include "h265p_definition.h"
#include "h265p_cabac_struct.h"

const UINT8 blockSize2BlockDim[XIN_BLOCK_SIZE_NUM][2] =
{
    {   4,   4 },     // XIN_BLOCK_4X4
    {   4,   8 },     // XIN_BLOCK_4X8
    {   8,   4 },     // XIN_BLOCK_8X4
    {   8,   8 },     // XIN_BLOCK_8X8
    {   8,  16 },     // XIN_BLOCK_8X16
    {  16,   8 },     // XIN_BLOCK_16X8
    {  16,  16 },     // XIN_BLOCK_16X16
    {  16,  32 },     // XIN_BLOCK_16X32
    {  32,  16 },     // XIN_BLOCK_32X16
    {  32,  32 },     // XIN_BLOCK_32X32
    {  32,  64 },     // XIN_BLOCK_32X64
    {  64,  32 },     // XIN_BLOCK_64X32
    {  64,  64 },     // XIN_BLOCK_64X64
    {  64, 128 },     // XIN_BLOCK_64X128
    { 128,  64 },     // XIN_BLOCK_128X64
    { 128, 128 },     // XIN_BLOCK_128X128
    {   4,  16 },     // XIN_BLOCK_4X16
    {  16,   4 },     // XIN_BLOCK_16X4
    {   8,  32 },     // XIN_BLOCK_8X32
    {  32,   8 },     // XIN_BLOCK_32X8
    {  16,  64 },     // XIN_BLOCK_16X64
    {  64,  16 },     // XIN_BLOCK_64X16
};

const UINT8 blockSize2LogDim[XIN_BLOCK_SIZE_NUM][2] =
{
    { 2, 2 },     // XIN_BLOCK_4X4
    { 2, 3 },     // XIN_BLOCK_4X8
    { 3, 2 },     // XIN_BLOCK_8X4
    { 3, 3 },     // XIN_BLOCK_8X8
    { 3, 4 },     // XIN_BLOCK_8X16
    { 4, 3 },     // XIN_BLOCK_16X8
    { 4, 4 },     // XIN_BLOCK_16X16
    { 4, 5 },     // XIN_BLOCK_16X32
    { 5, 4 },     // XIN_BLOCK_32X16
    { 5, 5 },     // XIN_BLOCK_32X32
    { 5, 6 },     // XIN_BLOCK_32X64
    { 6, 5 },     // XIN_BLOCK_64X32
    { 6, 6 },     // XIN_BLOCK_64X64
    { 6, 7 },     // XIN_BLOCK_64X128
    { 7, 6 },     // XIN_BLOCK_128X64
    { 7, 7 },     // XIN_BLOCK_128X128
    { 2, 4 },     // XIN_BLOCK_4X16
    { 4, 2 },     // XIN_BLOCK_16X4
    { 3, 5 },     // XIN_BLOCK_8X32
    { 5, 3 },     // XIN_BLOCK_32X8
    { 4, 6 },     // XIN_BLOCK_16X64
    { 6, 4 },     // XIN_BLOCK_64X16
};

const SINT32 av1IntraModeContext[XIN_INTRA_MODE_NUM] =
{
    0, 1, 2, 3, 4, 4, 4, 4, 3, 0, 1, 2, 0,
};

const UINT32 blockSize2TxSize[XIN_BLOCK_SIZE_NUM] =
{
    // 4X4
    XIN_TX_4X4,
    // 4X8,    8X4,      8X8
    XIN_TX_4X8,
    XIN_TX_8X4,
    XIN_TX_8X8,
    // 8X16,   16X8,     16X16
    XIN_TX_8X16,
    XIN_TX_16X8,
    XIN_TX_16X16,
    // 16X32,  32X16,    32X32
    XIN_TX_16X32,
    XIN_TX_32X16,
    XIN_TX_32X32,
    // 32X64,  64X32,
    XIN_TX_32X64,
    XIN_TX_64X32,
    // 64X64
    XIN_TX_64X64,
    // 64x128, 128x64,   128x128
    XIN_TX_64X64,
    XIN_TX_64X64,
    XIN_TX_64X64,
    // 4x16,   16x4,
    XIN_TX_4X16,
    XIN_TX_16X4,
    // 8x32,   32x8
    XIN_TX_8X32,
    XIN_TX_32X8,
    // 16x64,  64x16
    XIN_TX_16X64,
    XIN_TX_64X16
};

const UINT32 blockSize2TxSizeUv[XIN_BLOCK_SIZE_NUM] =
{
    // 4X4
    XIN_TX_4X4,
    // 4X8,    8X4,      8X8
    XIN_TX_4X4,
    XIN_TX_4X4,
    XIN_TX_4X4,
    // 8X16,   16X8,     16X16
    XIN_TX_4X8,
    XIN_TX_8X4,
    XIN_TX_8X8,
    // 16X32,  32X16,    32X32
    XIN_TX_8X16,
    XIN_TX_16X8,
    XIN_TX_16X16,
    // 32X64,  64X32,
    XIN_TX_16X32,
    XIN_TX_32X16,
    // 64X64
    XIN_TX_32X32,
    // 64x128, 128x64,   128x128
    XIN_TX_32X32,
    XIN_TX_32X32,
    XIN_TX_32X32,
    // 4x16,   16x4,
    XIN_TX_4X4,
    XIN_TX_4X4,
    // 8x32,   32x8
    XIN_TX_4X16,
    XIN_TX_16X4,
    // 16x64,  64x16
    XIN_TX_8X32,
    XIN_TX_32X8
};

const UINT32 blockSize2SqrTxSize[XIN_BLOCK_SIZE_NUM] =
{
    //                   4X4
    XIN_TX_4X4,
    // 4X8,    8X4,      8X8
    XIN_TX_4X4,
    XIN_TX_4X4,
    XIN_TX_8X8,
    // 8X16,   16X8,     16X16
    XIN_TX_8X8,
    XIN_TX_8X8,
    XIN_TX_16X16,
    // 16X32,  32X16,    32X32
    XIN_TX_16X16,
    XIN_TX_16X16,
    XIN_TX_32X32,
    // 32X64,  64X32,
    XIN_TX_32X32,
    XIN_TX_32X32,
    // 64X64
    XIN_TX_64X64,
    // 64x128, 128x64,   128x128
    XIN_TX_64X64,
    XIN_TX_64X64,
    XIN_TX_64X64,
    // 4x16,   16x4,     8x32
    XIN_TX_4X4,
    XIN_TX_4X4,
    XIN_TX_8X8,
    // 32x8,   16x64     64x16
    XIN_TX_8X8,
    XIN_TX_16X16,
    XIN_TX_16X16
};

const UINT32 blockDim2BlockSize[XIN_MAX_LG_BLOCK_SIZE+1][XIN_MAX_LG_BLOCK_SIZE+1] =
{
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID},
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID},
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_4X4,     XIN_BLOCK_4X8,     XIN_BLOCK_4X16,    XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID},
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_8X4,     XIN_BLOCK_8X8,     XIN_BLOCK_8X16,    XIN_BLOCK_8X32,    XIN_BLOCK_INVALID, XIN_BLOCK_INVALID},
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_16X4,    XIN_BLOCK_16X8,    XIN_BLOCK_16X16,   XIN_BLOCK_16X32,   XIN_BLOCK_16X64,   XIN_BLOCK_INVALID},
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_32X8,    XIN_BLOCK_32X16,   XIN_BLOCK_32X32,   XIN_BLOCK_32X64,   XIN_BLOCK_INVALID},
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_64X16,   XIN_BLOCK_64X32,   XIN_BLOCK_64X64,   XIN_BLOCK_64X128 },
    {XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_INVALID, XIN_BLOCK_128X64,  XIN_BLOCK_128X128},
};

const UINT32 txSizeSqrMap[XIN_TX_SIZE_NUM] =
{
    XIN_TX_4X4,    // TX_4X4
    XIN_TX_8X8,    // TX_8X8
    XIN_TX_16X16,  // TX_16X16
    XIN_TX_32X32,  // TX_32X32
    XIN_TX_64X64,  // TX_64X64
    XIN_TX_4X4,    // TX_4X8
    XIN_TX_4X4,    // TX_8X4
    XIN_TX_8X8,    // TX_8X16
    XIN_TX_8X8,    // TX_16X8
    XIN_TX_16X16,  // TX_16X32
    XIN_TX_16X16,  // TX_32X16
    XIN_TX_32X32,  // TX_32X64
    XIN_TX_32X32,  // TX_64X32
    XIN_TX_4X4,    // TX_4X16
    XIN_TX_4X4,    // TX_16X4
    XIN_TX_8X8,    // TX_8X32
    XIN_TX_8X8,    // TX_32X8
    XIN_TX_16X16,  // TX_16X64
    XIN_TX_16X16,  // TX_64X16
};

const UINT32 txSizeSqrUpMap[XIN_TX_SIZE_NUM] =
{
    XIN_TX_4X4,    // TX_4X4
    XIN_TX_8X8,    // TX_8X8
    XIN_TX_16X16,  // TX_16X16
    XIN_TX_32X32,  // TX_32X32
    XIN_TX_64X64,  // TX_64X64
    XIN_TX_8X8,    // TX_4X8
    XIN_TX_8X8,    // TX_8X4
    XIN_TX_16X16,  // TX_8X16
    XIN_TX_16X16,  // TX_16X8
    XIN_TX_32X32,  // TX_16X32
    XIN_TX_32X32,  // TX_32X16
    XIN_TX_64X64,  // TX_32X64
    XIN_TX_64X64,  // TX_64X32
    XIN_TX_16X16,  // TX_4X16
    XIN_TX_16X16,  // TX_16X4
    XIN_TX_32X32,  // TX_8X32
    XIN_TX_32X32,  // TX_32X8
    XIN_TX_64X64,  // TX_16X64
    XIN_TX_64X64,  // TX_64X16
};

const UINT32 txSizeLogMinus4[XIN_TX_SIZE_NUM] =
{
    0,  // TX_4X4
    2,  // TX_8X8
    4,  // TX_16X16
    6,  // TX_32X32
    6,  // TX_64X64
    1,  // TX_4X8
    1,  // TX_8X4
    3,  // TX_8X16
    3,  // TX_16X8
    5,  // TX_16X32
    5,  // TX_32X16
    6,  // TX_32X64
    6,  // TX_64X32
    2,  // TX_4X16
    2,  // TX_16X4
    4,  // TX_8X32
    4,  // TX_32X8
    5,  // TX_16X64
    5,  // TX_64X16
};

const UINT32 txType2Class[XIN_TX_2D_NUM] =
{
    XIN_TX_CLASS_2D,     // DCT_DCT
    XIN_TX_CLASS_2D,     // ADST_DCT
    XIN_TX_CLASS_2D,     // DCT_ADST
    XIN_TX_CLASS_2D,     // ADST_ADST
    XIN_TX_CLASS_2D,     // FLIPADST_DCT
    XIN_TX_CLASS_2D,     // DCT_FLIPADST
    XIN_TX_CLASS_2D,     // FLIPADST_FLIPADST
    XIN_TX_CLASS_2D,     // ADST_FLIPADST
    XIN_TX_CLASS_2D,     // FLIPADST_ADST
    XIN_TX_CLASS_2D,     // IDTX
    XIN_TX_CLASS_VERT,   // V_DCT
    XIN_TX_CLASS_HORIZ,  // H_DCT
    XIN_TX_CLASS_VERT,   // V_ADST
    XIN_TX_CLASS_HORIZ,  // H_ADST
    XIN_TX_CLASS_VERT,   // V_FLIPADST
    XIN_TX_CLASS_HORIZ,  // H_FLIPADST
};

const UINT32 txSize2LogScale[XIN_TX_SIZE_NUM] =
{
    0,      // 4x4 transform
    0,      // 8x8 transform
    0,      // 16x16 transform
    1,      // 32x32 transform
    2,      // 64x64 transform
    0,      // 4x8 transform
    0,      // 8x4 transform
    0,      // 8x16 transform
    0,      // 16x8 transform
    1,      // 16x32 transform
    1,      // 32x16 transform
    2,      // 32x64 transform
    2,      // 64x32 transform
    0,      // 4x16 transform
    0,      // 16x4 transform
    0,      // 8x32 transform
    0,      // 32x8 transform
    1,      // 16x64 transform
    1,      // 64x16 transform
};

const SINT32 av1ExtTxUsed[XIN_EXT_TX_SET_TYPE_NUM][XIN_TX_2D_NUM] =
{
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};

// Number of transform types in each set type
const SINT32 av1NumExtTxSet[XIN_EXT_TX_SET_TYPE_NUM] =
{
    1, 2, 5, 7, 12, 16,
};

// Maps tx set types to the indices.
const SINT32 av1ExtTxSetIndex[2][XIN_EXT_TX_SET_TYPE_NUM] =
{
    {
        // Intra
        0, -1, 2, 1, -1, -1
    },
    {
        // Inter
        0, 3, -1, -1, 2, 1
    },
};


