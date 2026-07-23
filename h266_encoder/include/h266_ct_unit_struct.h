/***************************************************************************//**
 *
 * @file          h266_ct_unit_struct.h
 * @brief         This file defines coding tree unit related structures.
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

#ifndef _h266_ct_unit_struct_h_
#define _h266_ct_unit_struct_h_

#include "h26x_sao_context.h"

typedef struct xin_pic_struct xin_pic_struct;

typedef struct xin_ctu_struct
{
    UINT32      ctuX;
    UINT32      ctuY;
    
    UINT32      ctuPelX;
    UINT32      ctuPelY;

    UINT32      lgWidth;
    UINT32      width;
    UINT32      height;
    UINT32      pixelNum;

    UINT32      availField;

    UINT32      ctuIndex;
    UINT32      ctuAddr;

    UINT32      sliceIndex;

    SINT32      saoType[2];
    SINT8       saoOffset[PLANE_NUM][XIN_NUM_SAO_EO_CLASS];
    UINT32      saoBandPos[PLANE_NUM];

    BOOL        saoMergeLftFlag;
    BOOL        saoMergeTopFlag;

    UINT32      vaildCuCount;   // Count non-skip CU number in CTU
    UINT32      cuNumInCtu;

    UINT64      skipSse[6][6];
    UINT32      skipCount[6][6];

    UINT64      sseCost[6][6];
    UINT64      sadCost[6][6];
    UINT32      cuCount[6][6];

    UINT64      interSad[6][6];
    UINT32      interCount[6][6];

    UINT64      sadThr[6][6];
    UINT64      sseThr[6][6];

    UINT32      minDepth;
    UINT32      maxDepth;

    UINT32      bitPlan;
    SINT32      bitUsed;

    SINT32      ctuQp;   

    xin_pic_struct *picSet;
    xin_cu_struct  *cu;
    COEFF          *coeffBuf[PLANE_NUM];
    intptr_t       coeffStride[2];
    UINT16         *gt0Buf[PLANE_NUM];
    
}xin_ctu_struct;

#endif

