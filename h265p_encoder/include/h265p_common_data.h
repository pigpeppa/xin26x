/***************************************************************************//**
 *
 * @file          h265p_common_data.h
 * @brief         This file contains common tables for av1.
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
#ifndef _h265p_common_data_h_
#define _h265p_common_data_h_

#include "h26x_common_data.h"

extern const UINT8  blockSize2BlockDim[XIN_BLOCK_SIZE_NUM][2];
extern const UINT8  blockSize2LogDim[XIN_BLOCK_SIZE_NUM][2];
extern const SINT32 av1IntraModeContext[XIN_INTRA_MODE_NUM];
extern const UINT32 blockSize2TxSize[XIN_BLOCK_SIZE_NUM];
extern const UINT32 blockSize2TxSizeUv[XIN_BLOCK_SIZE_NUM];
extern const UINT32 blockSize2SqrTxSize[XIN_BLOCK_SIZE_NUM];
extern const UINT32 blockDim2BlockSize[XIN_MAX_LG_BLOCK_SIZE+1][XIN_MAX_LG_BLOCK_SIZE+1];
extern const UINT32 txSizeSqrMap[XIN_TX_SIZE_NUM];
extern const UINT32 txSizeSqrUpMap[XIN_TX_SIZE_NUM];
extern const UINT32 txSizeLogMinus4[XIN_TX_SIZE_NUM];
extern const UINT32 txType2Class[XIN_TX_2D_NUM];
extern const UINT32 txSize2LogScale[XIN_TX_SIZE_NUM];
extern const SINT32 av1ExtTxUsed[XIN_EXT_TX_SET_TYPE_NUM][XIN_TX_2D_NUM];
extern const SINT32 av1NumExtTxSet[XIN_EXT_TX_SET_TYPE_NUM];
extern const SINT32 av1ExtTxSetIndex[2][XIN_EXT_TX_SET_TYPE_NUM];

#endif
