/***************************************************************************//**
 *
 * @file          h266_pred_unit_struct.h
 * @brief         This file contains h266 prediction unit data structures.
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
#ifndef _h266_pred_unit_struct_h_
#define _h266_pred_unit_struct_h_

#include "h266_intra_pred_context.h"

typedef struct xin_pu_struct
{
    
    UINT32          intraLumaMode;
    UINT32          intraChromaMode;

    UINT8           width;
    UINT8           height;

    UINT8           lgWidth;
    UINT8           lgHeight;

    SINT8           sortedIntraMPM[XIN_INTRA_MPM_NUM];
    SINT8           intraMPM[XIN_INTRA_MPM_NUM];
    SINT32          mpmNum;
    
    xin_mv32_u      mv[XIN_LIST_NUM];
    SINT8           refIdx[XIN_LIST_NUM];
    UINT32          mvpIndex[XIN_LIST_NUM];
    xin_mv32_u      predMv[XIN_LIST_NUM];

    BOOL            regularMergeFlag;
    BOOL            mergeFlag;
    BOOL            affine;
    UINT8           affineType;
    UINT8           mergeIndex;
    UINT8           imvIdx;
    UINT8           bcwIdx;
    
}xin_pu_struct;

#endif

