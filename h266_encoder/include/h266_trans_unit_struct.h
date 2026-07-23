/***************************************************************************//**
 *
 * @file          h266_trans_unit_struct.h
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
#ifndef _h266_trans_unit_struct_h_
#define _h266_trans_unit_struct_h_

typedef struct xin_scan_pos
{
    UINT32  posX;
    UINT32  posY;
    UINT32  posIdx;
}xin_scan_pos;

typedef struct xin_tu_struct
{
    UINT32      tuIdx;
    UINT32      partIdx;
    UINT32      offsetX;    // x offet within current coding unit
    UINT32      offsetY;    // y offset within current coding unit

    UINT8       lgWidth[2];
    UINT8       lgHeight[2];
    UINT8       lgCGWidth[2];
    UINT8       lgCGHeight[2];

    BOOL        prevCbf;
    UINT8       yCbf;
    UINT8       uCbf;
    UINT8       vCbf;

    UINT32      yCbfContext;
    UINT32      uvCbfContext;

    UINT8       mtsIdx[PLANE_NUM];

    COEFF       *qCoeff[PLANE_NUM];
    intptr_t    coeffStride[2];
    
    UINT16      *gt0BitMap[PLANE_NUM];
    UINT64      nzCGMapEs[PLANE_NUM];
    UINT64      nzCGMapRs[PLANE_NUM];

    xin_scan_pos *scanOrderCG[2];
    xin_scan_pos *scanOrder[2];
    
}xin_tu_struct;

typedef struct xin_tu_list
{
    xin_tu_struct      *tuBuf;
    struct xin_tu_list *nextList;
}xin_tu_list;

#endif

