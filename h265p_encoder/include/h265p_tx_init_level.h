/***************************************************************************//**
 *
 * @file          h265p_tx_init_level.h
 * @brief         This file declares tx level initialization subroutines.
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
#ifndef _h265p_tx_init_level_h_
#define _h265p_tx_init_level_h_

void Xin265pTxInitLevel (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel4xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel8xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel16xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel32xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel64xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

#endif

