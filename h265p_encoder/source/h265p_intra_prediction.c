/***************************************************************************//**
 *
 * @file          h265p_intra_prediction.c
 * @brief         av1 intra prediction subroutines.
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
#include "assert.h"
#include "string.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_picture_struct.h"
#include "h265p_seq_struct.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h265p_common_data.h"
#include "h265p_func_struct.h"
#include "h265p_intra_pred_filter.h"

static const UINT8 intraMode2Angle[] =
{
    0, 90, 180, 45, 135, 113, 157, 203, 67, 0, 0, 0, 0,
};

static const SINT16 drIntraDerivative[90] =
{
    // More evenly spread out angles and limited to 10-bit
    // Values that are 0 will never be used
    //                    Approx angle
    0,    0, 0,        //
    1023, 0, 0,        // 3, ...
    547,  0, 0,        // 6, ...
    372,  0, 0, 0, 0,  // 9, ...
    273,  0, 0,        // 14, ...
    215,  0, 0,        // 17, ...
    178,  0, 0,        // 20, ...
    151,  0, 0,        // 23, ... (113 & 203 are base angles)
    132,  0, 0,        // 26, ...
    116,  0, 0,        // 29, ...
    102,  0, 0, 0,     // 32, ...
    90,   0, 0,        // 36, ...
    80,   0, 0,        // 39, ...
    71,   0, 0,        // 42, ...
    64,   0, 0,        // 45, ... (45 & 135 are base angles)
    57,   0, 0,        // 48, ...
    51,   0, 0,        // 51, ...
    45,   0, 0, 0,     // 54, ...
    40,   0, 0,        // 58, ...
    35,   0, 0,        // 61, ...
    31,   0, 0,        // 64, ...
    27,   0, 0,        // 67, ... (67 & 157 are base angles)
    23,   0, 0,        // 70, ...
    19,   0, 0,        // 73, ...
    15,   0, 0, 0, 0,  // 76, ...
    11,   0, 0,        // 81, ...
    7,    0, 0,        // 84, ...
    3,    0, 0,        // 87, ...
};

BOOL Xin265pFilterIntraAllowed (
    BOOL    enableFilterIntra,
    UINT32  blockSize)
{
    if (!enableFilterIntra || blockSize == XIN_BLOCK_INVALID)
    {
        return FALSE;
    }

    return blockSize2BlockDim[blockSize][0] <= 32 && blockSize2BlockDim[blockSize][1] <= 32;
}

static void Xin265pGetIntraNBAvail (
    xin_mi_struct *mi,
    UINT32        sizeInMi,
    UINT32        *availField,
    intptr_t      miStride)
{
    intptr_t miIdx;
    UINT32   avail;

    avail = 0;

    for (miIdx = 0; miIdx < sizeInMi; miIdx++)
    {
        avail |= (mi->predMode != XIN_INVALID_MODE) << miIdx;

        mi += miStride;
    }

    *availField = avail;

}

void Xin265pGetIntraAvail (
    xin_mb_struct  *mb,
    UINT32         lgMiSize,
    intptr_t       miStride)
{
    SINT32   width;
    SINT32   height;
    SINT32   widthInMi;
    SINT32   heightInMi;
    SINT32   minDimInMi;
    UINT32   lftField;
    UINT32   topField;

    width      = mb->width[PLANE_LUMA];
    height     = mb->height[PLANE_LUMA];
    widthInMi  = width >> lgMiSize;
    heightInMi = height >> lgMiSize;
    minDimInMi = XIN_MIN (widthInMi, heightInMi);

    Xin265pGetIntraNBAvail (
        mb->lftMi,
        heightInMi + minDimInMi,
        &lftField,
        miStride);

    Xin265pGetIntraNBAvail (
        mb->topMi,
        widthInMi + minDimInMi,
        &topField,
        1);

    mb->intraLftAvail = lftField;
    mb->intraTopAvail = topField;

}

void Xin265pExtractIntraNBLuma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb)
{
    PIXEL    *lftRef;
    PIXEL    *topRef;
    PIXEL4   *topRef4;
    intptr_t srcStride;
    PIXEL    *lftIntra;
    PIXEL    *topIntra;
    PIXEL4   *topIntra4;
    SINT32   width;
    SINT32   height;
    SINT32   whInMi;
    SINT32   idx;
    UINT32   lftAvail;
    UINT32   topAvail;

    lftAvail   = mb->intraLftAvail;
    topAvail   = mb->intraTopAvail;
    srcStride  = secSet->reconStride[0];
    lftRef     = secSet->reconMb[0] - 1;
    topRef     = secSet->reconMb[0] - srcStride;
    topRef4    = (PIXEL4 *)topRef;
    width      = mb->width[PLANE_LUMA];
    height     = mb->height[PLANE_LUMA];
    whInMi     = (width + height) >> XIN_LOG_MI_SIZE;
    lftIntra   = secSet->intraLftBuf[PLANE_LUMA];
    topIntra   = secSet->intraTopBuf[PLANE_LUMA];
    topIntra4  = (PIXEL4 *)topIntra;

    if (lftAvail)
    {
        for (idx = 0; idx < whInMi; idx++)
        {
            if (lftAvail & (1 << idx))
            {
                lftIntra[idx*XIN_MI_SIZE + 0] = lftRef[srcStride * (idx * XIN_MI_SIZE + 0)];
                lftIntra[idx*XIN_MI_SIZE + 1] = lftRef[srcStride * (idx * XIN_MI_SIZE + 1)];
                lftIntra[idx*XIN_MI_SIZE + 2] = lftRef[srcStride * (idx * XIN_MI_SIZE + 2)];
                lftIntra[idx*XIN_MI_SIZE + 3] = lftRef[srcStride * (idx * XIN_MI_SIZE + 3)];
            }
            else
            {
                break;
            }
        }

        if (idx < whInMi)
        {
            memset (&lftIntra[idx*XIN_MI_SIZE], lftIntra[idx*XIN_MI_SIZE - 1], (whInMi - idx)*XIN_MI_SIZE);
        }

    }
    else
    {
        if (topAvail)
        {
            memset (lftIntra, topRef[0], whInMi*XIN_MI_SIZE);
        }
        else
        {
            memset (lftIntra, 129, whInMi*XIN_MI_SIZE);
        }
    }

    if (topAvail)
    {
        for (idx = 0; idx < whInMi; idx++)
        {
            if (topAvail & (1 << idx))
            {
                topIntra4[idx] = topRef4[idx];
            }
            else
            {
                break;
            }
        }

        if (idx < whInMi)
        {
            memset (&topIntra[idx*XIN_MI_SIZE], topIntra[idx*XIN_MI_SIZE - 1], (whInMi - idx)*XIN_MI_SIZE);
        }
    }
    else
    {
        if (lftAvail)
        {
            memset (topIntra, lftRef[0], whInMi*XIN_MI_SIZE);
        }
        else
        {
            memset (topIntra, 127, whInMi*XIN_MI_SIZE);
        }
    }

    if (topAvail && lftAvail)
    {
        topIntra[-1] = topRef[-1];
    }
    else if (topAvail)
    {
        topIntra[-1] = topRef[0];
    }
    else if (lftAvail)
    {
        topIntra[-1] = lftRef[0];
    }
    else
    {
        topIntra[-1] = 128;
    }

    lftIntra[-1] = topIntra[-1];

}

void Xin265pExtractIntraNBChroma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb)
{
    PIXEL    *lftRefU;
    PIXEL    *topRefU;
    PIXEL    *lftRefV;
    PIXEL    *topRefV;
    PIXEL2   *topRef2U;
    PIXEL2   *topRef2V;
    intptr_t srcStride;
    PIXEL    *lftIntraU;
    PIXEL    *topIntraU;
    PIXEL    *lftIntraV;
    PIXEL    *topIntraV;
    PIXEL2   *topIntra2U;
    PIXEL2   *topIntra2V;
    SINT32   width;
    SINT32   height;
    SINT32   whInMi;
    SINT32   idx;
    UINT32   lftAvail;
    UINT32   topAvail;

    lftAvail   = mb->intraLftAvail;
    topAvail   = mb->intraTopAvail;
    srcStride  = secSet->reconStride[PLANE_CHROMA];
    lftRefU    = secSet->reconMb[PLANE_CHROMA_U] - 1;
    topRefU    = secSet->reconMb[PLANE_CHROMA_U] - srcStride;
    lftRefV    = secSet->reconMb[PLANE_CHROMA_V] - 1;
    topRefV    = secSet->reconMb[PLANE_CHROMA_V] - srcStride;
    topRef2U   = (PIXEL2 *)topRefU;
    topRef2V   = (PIXEL2 *)topRefV;
    width      = mb->width[PLANE_LUMA];
    height     = mb->height[PLANE_LUMA];
    whInMi     = (width + height) >> XIN_LOG_MI_SIZE;
    lftIntraU  = secSet->intraLftBuf[PLANE_CHROMA_U];
    topIntraU  = secSet->intraTopBuf[PLANE_CHROMA_U];
    lftIntraV  = secSet->intraLftBuf[PLANE_CHROMA_V];
    topIntraV  = secSet->intraTopBuf[PLANE_CHROMA_V];
    topIntra2U = (PIXEL2 *)topIntraU;
    topIntra2V = (PIXEL2 *)topIntraV;

    if (lftAvail)
    {
        for (idx = 0; idx < whInMi; idx++)
        {
            if (lftAvail & (1 << idx))
            {
                lftIntraU[idx*2 + 0] = lftRefU[srcStride * (idx * 2 + 0)];
                lftIntraU[idx*2 + 1] = lftRefU[srcStride * (idx * 2 + 1)];
                lftIntraV[idx*2 + 0] = lftRefV[srcStride * (idx * 2 + 0)];
                lftIntraV[idx*2 + 1] = lftRefV[srcStride * (idx * 2 + 1)];
            }
            else
            {
                break;
            }
        }

        if (idx < whInMi)
        {
            memset (&lftIntraU[idx*2], lftIntraU[idx*2 - 1], (whInMi - idx)*2);
            memset (&lftIntraV[idx*2], lftIntraV[idx*2 - 1], (whInMi - idx)*2);
        }

    }
    else
    {
        if (topAvail)
        {
            memset (lftIntraU, topRefU[0], whInMi*2);
            memset (lftIntraV, topRefV[0], whInMi*2);
        }
        else
        {
            memset (lftIntraU, 129, whInMi*2);
            memset (lftIntraV, 129, whInMi*2);
        }
    }

    if (topAvail)
    {
        for (idx = 0; idx < whInMi; idx++)
        {
            if (topAvail & (1 << idx))
            {
                topIntra2U[idx] = topRef2U[idx];
                topIntra2V[idx] = topRef2V[idx];
            }
            else
            {
                break;
            }
        }

        if (idx < whInMi)
        {
            memset (&topIntraU[idx*2], topIntraU[idx*2 - 1], (whInMi - idx)*2);
            memset (&topIntraV[idx*2], topIntraV[idx*2 - 1], (whInMi - idx)*2);
        }

    }
    else
    {
        if (lftAvail)
        {
            memset (topIntraU, topRefU[0], whInMi*2);
            memset (topIntraV, topRefV[0], whInMi*2);
        }
        else
        {
            memset (topIntraU, 127, whInMi*2);
            memset (topIntraV, 127, whInMi*2);
        }
    }

    if (topAvail && lftAvail)
    {
        topIntraU[-1] = topRefU[-1];
        topIntraV[-1] = topRefV[-1];
    }
    else if (topAvail)
    {
        topIntraU[-1] = topRefU[0];
        topIntraV[-1] = topRefV[0];
    }
    else if (lftAvail)
    {
        topIntraU[-1] = lftRefU[0];
        topIntraV[-1] = lftRefV[0];
    }
    else
    {
        topIntraU[-1] = 128;
        topIntraV[-1] = 128;
    }

    lftIntraU[-1] = topIntraU[-1];
    lftIntraV[-1] = topIntraV[-1];

}

static inline SINT32 XinGetDx (SINT32 angle)
{
    if (angle > 0 && angle < 90)
    {
        return drIntraDerivative[angle];
    }
    else if (angle > 90 && angle < 180)
    {
        return drIntraDerivative[180 - angle];
    }
    else
    {
        // In this case, we are not really going to use dx. We may return any value.
        return 1;
    }
}

static inline int XinGetDy (SINT32 angle)
{
    if (angle > 90 && angle < 180)
    {
        return drIntraDerivative[angle - 90];
    }
    else if (angle > 180 && angle < 270)
    {
        return drIntraDerivative[270 - angle];
    }
    else
    {
        // In this case, we are not really going to use dy. We may return any value.
        return 1;
    }
}

void Xin265pIntraPred (
    xin_sec_struct *secSet,
    UINT32         planeIdx,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         mode,
    SINT32         deltaAngle)
{
    xin_mb_struct   *mb;
    xin_func_struct *funcSet;
    PIXEL           *lftBuf;
    PIXEL           *topBuf;
    SINT32          modeAngle;
    BOOL            isDrMode;
    SINT32          width;
    SINT32          height;
    SINT32          lgWidth;
    SINT32          dx;
    SINT32          dy;

    mb        = secSet->mb;
    funcSet   = secSet->funcSet;
    width     = planeIdx ? mb->width[PLANE_CHROMA] : mb->width[PLANE_LUMA];
    height    = planeIdx ? mb->height[PLANE_CHROMA] : mb->height[PLANE_LUMA];
    lgWidth   = calcLog2[width];
    lftBuf    = secSet->intraLftBuf[planeIdx];
    topBuf    = secSet->intraTopBuf[planeIdx];
    modeAngle = intraMode2Angle[mode] + deltaAngle;
    isDrMode  = (mode >= XIN_V_PRED) && (mode <= XIN_D67_PRED);

    if (isDrMode)
    {
        dx = XinGetDx (modeAngle);
        dy = XinGetDy (modeAngle);

        assert(modeAngle > 0 && modeAngle < 270);

        if (modeAngle > 0 && modeAngle < 90)
        {
            funcSet->pfXinIntraPredDrZ1[lgWidth] (
                pred,
                predStride,
                width,
                height,
                topBuf,
                lftBuf,
                FALSE,
                dx,
                dy);
        }
        else if (modeAngle > 90 && modeAngle < 180)
        {
            funcSet->pfXinIntraPredDrZ2[lgWidth] (
                pred,
                predStride,
                width,
                height,
                topBuf,
                lftBuf,
                FALSE,
                FALSE,
                dx,
                dy);
        }
        else if (modeAngle > 180 && modeAngle < 270)
        {
            Xin265pIntraPredDrZ3 (
                pred,
                predStride,
                width,
                height,
                topBuf,
                lftBuf,
                FALSE,
                dx,
                dy);
        }
        else if (modeAngle == 90)
        {
            funcSet->pfXinIntraPredVer[lgWidth] (
                pred,
                predStride,
                width,
                height,
                topBuf,
                lftBuf);

        }
        else if (modeAngle == 180)
        {
            Xin265pIntraPredHor (
                pred,
                predStride,
                width,
                height,
                topBuf,
                lftBuf);

        }

    }
    else if (mode == XIN_DC_PRED)
    {

        lftBuf = mb->intraLftAvail ? lftBuf : NULL;
        topBuf = mb->intraTopAvail ? topBuf : NULL;

        Xin265pIntraPredDc (
            pred,
            predStride,
            width,
            height,
            topBuf,
            lftBuf);
    }
    else if (mode == XIN_SMOOTH_PRED)
    {
        funcSet->pfXinIntraPredSm[lgWidth] (
            pred,
            predStride,
            width,
            height,
            topBuf,
            lftBuf);
    }
    else if (mode == XIN_SMOOTH_V_PRED)
    {
        funcSet->pfXinIntraPredSmV[lgWidth] (
            pred,
            predStride,
            width,
            height,
            topBuf,
            lftBuf);
    }
    else if (mode == XIN_SMOOTH_H_PRED)
    {
        funcSet->pfXinIntraPredSmH[lgWidth] (
            pred,
            predStride,
            width,
            height,
            topBuf,
            lftBuf);
    }
    else
    {
        funcSet->pfXinIntraPredPaeth[lgWidth] (
            pred,
            predStride,
            width,
            height,
            topBuf,
            lftBuf);
    }

}

