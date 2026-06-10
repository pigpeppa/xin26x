/***************************************************************************//**
 *
 * @file          h265p_trans_unit_struct.h
 * @brief         Transform unit struct definition.
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
#ifndef _h265p_trans_unit_struct_h_
#define _h265p_trans_unit_struct_h_

typedef struct xin_tu_struct
{
    UINT8       tuIdx;
    UINT8       txType;
    UINT8       txSize;
    UINT8       partIdx;
    UINT8       offsetX;    // x offet within current coding unit
    UINT8       offsetY;    // y offset within current coding unit

    UINT8       width;
    UINT8       height;
    UINT8       lgWidth;
    UINT8       lgHeight;
    UINT8       widthInMi;
    UINT8       heightInMi;

    BOOL        splitFlag;

    SINT32      eob;

    UINT8       culLevel;
    
    SINT32      dcSignCtx;
    SINT32      txSkipCtx;

    COEFF       *qCoeff;
    intptr_t    coeffStride;
    
}xin_tu_struct;

#endif

