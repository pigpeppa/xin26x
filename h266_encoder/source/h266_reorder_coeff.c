/***************************************************************************//**
 *
 * @file          h266_reorder_coeff.c
 * @brief         Rearrange quantized coefficient layout for entropy write.
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
#include "string.h"
#include "h266_definition.h"
#include "xin_typedef.h"
#include "basic_macro.h"
#include "h26x_definition.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_section_struct.h"
#include "h266_func_struct.h"

void Xin266CoeffScanCG (
    COEFF        *coefBuffer,
    intptr_t     coefStride,
    SINT32       cgWidth,
    SINT32       cgHeight,
    xin_scan_pos *scanOrder,
    UINT16       *coefSign,
    UINT16       *gt0Buf)
{
    SINT32       coefPosX;
    SINT32       coefPosY;
    SINT32       coefIndex;
    COEFF        coeff;
    LEVEL        absCoef;
    UINT16       gt0BitMap;
    UINT16       coeffSign;
    SINT32       coeffNum;

    gt0BitMap = 0;
    coeffNum  = cgWidth*cgHeight;
    coeffSign = 0;

    for (coefIndex = 0; coefIndex < coeffNum; coefIndex++)
    {
        coefPosX = scanOrder[coefIndex].posX;
        coefPosY = scanOrder[coefIndex].posY;
        coeff    = coefBuffer[coefPosY * coefStride + coefPosX];
        absCoef  = XIN_ABS (coeff);

        if (absCoef)
        {
            gt0BitMap |= 1 << coefIndex;
        }

        if (coeff < 0)
        {
            coeffSign |= 1 << coefIndex;
        }

    }

    *gt0Buf   = gt0BitMap;
    *coefSign = coeffSign;

}

void Xin266ReorderCoeff (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          compIdx)
{
    UINT32          grpIdx;
    UINT32          grpPos;
    UINT32          coeffGrpXRs;
    UINT32          coeffGrpYRs;
    UINT32          grpNum;
    COEFF           *coeffRs;
    UINT16          *gt0BitMap;
    UINT16          *coeffSign;
    UINT64          nzCGBitMapEs;
    UINT64          nzCGBitMapRs;
    UINT32          tuIdx;
    COEFF           *coefBuf;
    intptr_t        coeffStride;
    SINT32          cgWidth;
    SINT32          cgHeight;
    SINT32          width;
    SINT32          height;
    SINT32          cgSize;
    UINT32          compType;
    xin_scan_pos    *scanOrder;
    xin_scan_pos    *scanOrderCG;
    xin_func_struct *funcSet;

    tuIdx        = tu->tuIdx;
    compType     = compIdx != PLANE_LUMA;
    cgWidth      = 1 << tu->lgCGWidth[compType];
    cgHeight     = 1 << tu->lgCGHeight[compType];
    width        = 1 << tu->lgWidth[compType];
    height       = 1 << tu->lgHeight[compType];
    width        = XIN_MIN (width, 32);
    height       = XIN_MIN (height, 32);
    cgSize       = cgWidth*cgHeight;
    grpNum       = width*height/cgSize;
    gt0BitMap    = fullBuf->gt0BitMap[mtsIdx][compIdx] + partIdx;
    coeffSign    = fullBuf->coeffSign[mtsIdx][compIdx] + partIdx;
    coefBuf      = fullBuf->qCoefBuf[mtsIdx][compIdx] + coefAddr;
    coeffStride  = fullBuf->coeffStride[compType];
    nzCGBitMapEs = 0;
    nzCGBitMapRs = fullBuf->nzCGMapRs[mtsIdx][tuIdx][compIdx];
    scanOrder    = tu->scanOrder[compType];
    scanOrderCG  = tu->scanOrderCG[compType];
    funcSet      = secSet->funcSet;

    for (grpIdx = 0; grpIdx < grpNum; grpIdx++)
    {
        grpPos     = scanOrderCG[grpIdx].posIdx;
        *gt0BitMap = 0;

        if (nzCGBitMapRs & ((UINT64)1 << grpPos))
        {
            coeffGrpXRs   = scanOrderCG[grpIdx].posX;
            coeffGrpYRs   = scanOrderCG[grpIdx].posY;
            coeffRs       = coefBuf + (coeffGrpYRs*cgHeight*coeffStride + coeffGrpXRs*cgWidth);
            nzCGBitMapEs |= (UINT64)1 << grpIdx;

            funcSet->pfXinCoeffScanCG[(cgWidth == 4) && (cgHeight == 4)] (
                coeffRs,
                coeffStride,
                cgWidth,
                cgHeight,
                scanOrder,
                coeffSign,
                gt0BitMap);
        }

        gt0BitMap++;
        coeffSign++;

    }

    fullBuf->nzCGMapEs[mtsIdx][tuIdx][compIdx] = nzCGBitMapEs;

}


