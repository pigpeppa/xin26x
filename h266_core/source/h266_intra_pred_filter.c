/***************************************************************************//**
 *
 * @file          h266_intra_pred_filter.c
 * @brief         h266 intra prediction filter subroutines.
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
#include "string.h"
#include "basic_macro.h"
#include "h266_intra_pred_context.h"
#include "xin_video_common.h"
#include "h26x_trans_context.h"
#include "h26x_common_data.h"
#include "assert.h"

static const SINT32 intraModeShift[] =
{
    0, 6, 10, 12, 14, 15
};

static SINT16 cubicFilter[32][4] =
{
    {  0, 64,  0,  0 },
    { -1, 63,  2,  0 },
    { -2, 62,  4,  0 },
    { -2, 60,  7, -1 },
    { -2, 58, 10, -2 },
    { -3, 57, 12, -2 },
    { -4, 56, 14, -2 },
    { -4, 55, 15, -2 },
    { -4, 54, 16, -2 },
    { -5, 53, 18, -2 },
    { -6, 52, 20, -2 },
    { -6, 49, 24, -3 },
    { -6, 46, 28, -4 },
    { -5, 44, 29, -4 },
    { -4, 42, 30, -4 },
    { -4, 39, 33, -4 },
    { -4, 36, 36, -4 },
    { -4, 33, 39, -4 },
    { -4, 30, 42, -4 },
    { -4, 29, 44, -5 },
    { -4, 28, 46, -6 },
    { -3, 24, 49, -6 },
    { -2, 20, 52, -6 },
    { -2, 18, 53, -5 },
    { -2, 16, 54, -4 },
    { -2, 15, 55, -4 },
    { -2, 14, 56, -4 },
    { -2, 12, 57, -3 },
    { -2, 10, 58, -2 },
    { -1,  7, 60, -2 },
    {  0,  4, 62, -2 },
    {  0,  2, 63, -1 },
};

static SINT16 linearFilter[32][4] =
{
    { 16, 32, 16, 0 },
    { 16, 32, 16, 0 },
    { 15, 31, 17, 1 },
    { 15, 31, 17, 1 },
    { 14, 30, 18, 2 },
    { 14, 30, 18, 2 },
    { 13, 29, 19, 3 },
    { 13, 29, 19, 3 },
    { 12, 28, 20, 4 },
    { 12, 28, 20, 4 },
    { 11, 27, 21, 5 },
    { 11, 27, 21, 5 },
    { 10, 26, 22, 6 },
    { 10, 26, 22, 6 },
    { 9, 25, 23, 7 },
    { 9, 25, 23, 7 },
    { 8, 24, 24, 8 },
    { 8, 24, 24, 8 },
    { 7, 23, 25, 9 },
    { 7, 23, 25, 9 },
    { 6, 22, 26, 10 },
    { 6, 22, 26, 10 },
    { 5, 21, 27, 11 },
    { 5, 21, 27, 11 },
    { 4, 20, 28, 12 },
    { 4, 20, 28, 12 },
    { 3, 19, 29, 13 },
    { 3, 19, 29, 13 },
    { 2, 18, 30, 14 },
    { 2, 18, 30, 14 },
    { 1, 17, 31, 15 },
    { 1, 17, 31, 15 },
};

static const SINT32 invIntraAngTable[32] =
{
    0,   16384, 8192, 5461, 4096, 2731, 2048, 1638, 1365, 1170, 1024, 910, 819, 712, 630, 565,
    512, 468,   420,  364,  321,  287,  256,  224,  191,  161,  128,  96,  64,  48,  32,  16
};

void Xin266IntraPredDc (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  width, height;
    SINT32  sum;
    UINT32  idx;
    UINT32  colIdx;
    UINT32  rowIdx;
    PIXEL   dcVal;
    SINT32  divOffset;
    SINT32  divShift;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    sum       = 0;
    divShift  = (width == height) ? lgWidth + 1 : XIN_MAX(lgWidth, lgHeight);
    divOffset = 1 << (divShift - 1);
    topBuf    = nBuf;
    lftBuf    = nBuf + 1 + multiRefIdx + width*4;

    if (width >= height)
    {
        for (idx = 0; idx < width; idx++ )
        {
            sum += topBuf[1 + idx + multiRefIdx];
        }
    }

    if (width <= height)
    {
        for (idx = 0; idx < height; idx++)
        {
            sum += lftBuf[1 + idx + multiRefIdx];
        }
    }

    dcVal = (PIXEL)((sum + divOffset) >> divShift);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            dst[rowIdx*dstStride+colIdx] = dcVal;
        }
    }

}

static inline SINT32 XinGetWideAngle (
    SINT32 lgWidth,
    SINT32 lgHeight,
    SINT32 predMode)
{
    SINT32 deltaSize;

    if ((predMode > XIN_DC_IDX) && (predMode <= XIN_VDIA_IDX))
    {
        deltaSize = XIN_ABS(lgWidth - lgHeight);

        if ((lgWidth > lgHeight) && (predMode < 2 + intraModeShift[deltaSize]))
        {
            predMode += (XIN_VDIA_IDX - 1);
        }
        else if ((lgHeight > lgWidth) && (predMode > XIN_VDIA_IDX - intraModeShift[deltaSize]))
        {
            predMode -= (XIN_VDIA_IDX - 1);
        }
    }

    return predMode;

}

void Xin266IntraPredPlanar (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  idx;
    PIXEL   *lftBuf;
    PIXEL   *topBuf;
    UINT32  col;
    UINT32  row;
    SINT32  finalShift;
    SINT32  botLft;
    SINT32  topRgt;
    SINT32  topRow[65];
    SINT32  lftColumn[65];
    SINT32  botRow[65];
    SINT32  rgtColumn[65];
    SINT32  horPred;
    SINT32  verPred;
    SINT32  offset;

    width      = 1 << lgWidth;
    height     = 1 << lgHeight;
    topBuf     = nBuf;
    lftBuf     = nBuf + 1 + multiRefIdx + width*4;
    finalShift = 1 + lgWidth + lgHeight;
    offset     = 1 << (lgWidth + lgHeight);

    // Get left and above reference column and row
    for (idx = 0; idx < width + 1; idx++)
    {
        topRow[idx] = topBuf[idx + 1];
    }

    for (idx = 0; idx < height + 1; idx++)
    {
        lftColumn[idx] = lftBuf[idx + 1];
    }

    // Prepare intermediate variables used in interpolation
    botLft = lftColumn[height];
    topRgt = topRow[width];

    for (idx = 0; idx < width; idx++)
    {
        botRow[idx] = botLft - topRow[idx];
        topRow[idx] = topRow[idx] << lgHeight;
    }

    for (idx = 0; idx < height; idx++)
    {
        rgtColumn[idx] = topRgt - lftColumn[idx];
        lftColumn[idx] = lftColumn[idx] << lgWidth;
    }

    for (row = 0; row < height; row++)
    {
        horPred = lftColumn[row];

        for (col = 0; col < width; col++)
        {
            topRow[col] += botRow[col];

            horPred += rgtColumn[row];
            verPred  = topRow[col];

            dst[row*dstStride + col] = (PIXEL)(((horPred << lgHeight) + (verPred << lgWidth) + offset) >> finalShift);
        }
    }

}

void Xin266LumaIntraFilter (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  deltaPos;
    SINT32  deltaInt;
    SINT32  deltaFrac;
    SINT16  *coeff;
    SINT32  pixelVal;
    PIXEL   fltPel[4];
    SINT16  (*filterCoef)[4];

    deltaPos   = intraPredAngle*(1 + multiRefIdx);
    filterCoef = interpFlag ? linearFilter : cubicFilter;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        deltaInt  = deltaPos >> 5;
        deltaFrac = deltaPos & 31;

        if (XIN_ABS(intraPredAngle) & 0x1F)
        {
            coeff = filterCoef[deltaFrac];

            for (colIdx = 0; colIdx < width; colIdx++)
            {
                fltPel[0] = refMain[deltaInt + colIdx + 0];
                fltPel[1] = refMain[deltaInt + colIdx + 1];
                fltPel[2] = refMain[deltaInt + colIdx + 2];
                fltPel[3] = refMain[deltaInt + colIdx + 3];

                pixelVal = (coeff[0] * fltPel[0] + coeff[1] * fltPel[1] + coeff[2] * fltPel[2] + coeff[3] * fltPel[3] + 32) >> 6;
                pred[colIdx] = (PIXEL)XIN_CLIP(pixelVal, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            }
        }
        else
        {
            // Just copy the integer samples
            for (colIdx = 0; colIdx < width; colIdx++ )
            {
                pred[colIdx] = refMain[colIdx + deltaInt + 1];
            }
        }

        pred     += predStride;
        deltaPos += intraPredAngle;

    }

}

void Xin266ChromaIntraFilter (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  deltaPos;
    SINT32  deltaInt;
    SINT32  deltaFrac;

    deltaPos = intraPredAngle;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        deltaInt  = deltaPos >> 5;
        deltaFrac = deltaPos & 31;

        if (XIN_ABS(intraPredAngle) & 0x1F)
        {
            // Do linear filtering
            for (colIdx = 0; colIdx < width; colIdx++)
            {
                pred[colIdx] = (PIXEL)(refMain[deltaInt + colIdx + 1] + ((deltaFrac * (refMain[deltaInt + colIdx + 2] - refMain[deltaInt + colIdx + 1]) + 16) >> 5));
            }
        }
        else
        {
            // Just copy the integer samples
            for (colIdx = 0; colIdx < width; colIdx++ )
            {
                pred[colIdx] = refMain[colIdx + deltaInt + 1];
            }
        }

        pred     += predStride;
        deltaPos += intraPredAngle;

    }

}

void Xin266ApplyAngPDPCHor (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   absAngleMode,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  invAngleSum;
    PIXEL   lft;
    SINT32  wl;
    SINT32  invAngle;

    invAngle    = invIntraAngTable[absAngleMode];
    height      = XIN_MIN (3 << angularScale, height);
    invAngleSum = 256;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        invAngleSum += invAngle;
        wl           = 32 >> (2 * rowIdx >> angularScale);

        for (colIdx = 0; colIdx < width; colIdx++)
        {
            lft = refSide[colIdx + (invAngleSum >> 9) + 1];

            pred[colIdx] = (PIXEL)XIN_CLIP(pred[colIdx] + ((wl * (lft - pred[colIdx]) + 32) >> 6), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

        pred += predStride;
    }

}

void Xin266ApplyAngPDPCVert (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   absAngleMode,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  invAngleSum;
    PIXEL   lft;
    SINT32  wl;
    SINT32  invAngle;

    invAngle = invIntraAngTable[absAngleMode];
    width    = XIN_MIN (3 << angularScale, width);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        invAngleSum = 256;

        for (colIdx = 0; colIdx < width; colIdx++)
        {
            invAngleSum += invAngle;

            wl  = 32 >> (2 * colIdx >> angularScale);
            lft = refSide[rowIdx + (invAngleSum >> 9) + 1];

            pred[colIdx] = (PIXEL)XIN_CLIP(pred[colIdx] + ((wl * (lft - pred[colIdx]) + 32) >> 6), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

        pred += predStride;
    }

}

void Xin266IntraPredHor (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32   width;
    SINT32   height;
    PIXEL    predBuf[64*64];
    PIXEL    *pred;
    intptr_t predStride;
    SINT32   colIdx;
    SINT32   rowIdx;
    PIXEL    *lftBuf;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    PIXEL    *refSide;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = lftBuf;
    refSide = topBuf;

    // swap width/height if we are doing a horizontal mode:
    XIN_SWAP (SINT32, width, height);

    predStride = width;
    pred       = predBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    refSide += multiRefIdx;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            pred[rowIdx*predStride + colIdx] = refMain[colIdx + 1];
        }
    }

    // Flip the block if this is the horizontal mode
    for (rowIdx = 0; rowIdx < height; rowIdx++ )
    {
        for (colIdx = 0; colIdx < width; colIdx++ )
        {
            dst[rowIdx + colIdx*dstStride] = predBuf[colIdx + rowIdx*predStride];
        }
    }

}

void Xin266IntraPredVer (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32   width;
    SINT32   height;
    SINT32   colIdx;
    SINT32   rowIdx;
    PIXEL    *lftBuf;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    PIXEL    *refSide;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = topBuf;
    refSide = lftBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    refSide += multiRefIdx;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            dst[rowIdx*dstStride + colIdx] = refMain[colIdx + 1];
        }
    }

}

void Xin266IntraPredBdpcm (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    SINT32   dirMode,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32 width;
    SINT32 height;
    SINT32 y;
    PIXEL  *lftBuf;
    PIXEL  *topBuf;

    assert ( dirMode == 1 || dirMode == 2);
    width  = 1 << lgWidth;
    height = 1 << lgHeight;
    topBuf = nBuf;
    lftBuf = nBuf + 1 + multiRefIdx + width*2;

    if (dirMode == 1)
    {
        for (y = 0; y < height; y++ )
        {
            memset (dst, lftBuf[y + 1], sizeof(PIXEL)*width);

            dst += dstStride;
        }
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            memcpy (dst, topBuf + 1, sizeof(PIXEL)*width);

            dst += dstStride;
        }
    }

}

void Xin266ApplyPDPC (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  rowIdx;
    UINT32  colIdx;
    UINT32  scale;
    SINT32  wT;
    SINT32  wL;
    PIXEL   lftPel;
    PIXEL   topPel;
    PIXEL   pelVal;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;

    width  = 1 << lgWidth;
    height = 1 << lgHeight;
    scale  = ((lgWidth - 2 + lgHeight - 2 + 2) >> 2);
    topBuf = nIntraBuf + 1;
    lftBuf = nIntraBuf + 2 + width*4;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        wT     = 32 >> XIN_MIN (31, ((rowIdx << 1) >> scale));
        lftPel = lftBuf[rowIdx];

        for (colIdx = 0; colIdx < width; colIdx++)
        {
            wL     = 32 >> XIN_MIN (31, ((colIdx << 1) >> scale));
            topPel = topBuf[colIdx];
            pelVal = pred[colIdx];

            pred[colIdx] = (PIXEL)(pelVal + ((wL * (lftPel - pelVal) + wT * (topPel - pelVal) + 32) >> 6));
        }

        pred += predStride;
    }

}

void Xin266ApplyPDPCHor (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    PIXEL  topLft;
    PIXEL  lft;
    SINT32 wl;
    SINT32 pixelVal;
    SINT32 angular;
    SINT32 width;
    SINT32 height;
    SINT32 rowIdx;
    SINT32 colIdx;
    PIXEL  *topBuf;
    PIXEL  *lftBuf;

    width  = 1 << lgWidth;
    height = 1 << lgHeight;
    topBuf = nIntraBuf;
    lftBuf = nIntraBuf + 1 + width*4;

    angular = ((lgWidth + lgHeight) - 2) >> 2;
    height  = XIN_MIN (3 << angular, height);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        topLft = lftBuf[0];
        wl     = 32 >> (2 * rowIdx >> angular);

        for (colIdx = 0; colIdx < width; colIdx++)
        {
            lft      = topBuf[1 + colIdx];
            pixelVal = pred[colIdx] + ((wl * (lft - topLft) + 32) >> 6);

            pred[colIdx] = (PIXEL)XIN_CLIP (pixelVal, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

        pred += predStride;
    }

}

void Xin266ApplyPDPCVer (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{

    PIXEL  topLft;
    PIXEL  lft;
    SINT32 wl;
    SINT32 pixelVal;
    SINT32 angular;
    SINT32 width;
    SINT32 height;
    SINT32 rowIdx;
    SINT32 colIdx;
    PIXEL  *topBuf;
    PIXEL  *lftBuf;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nIntraBuf;
    lftBuf  = nIntraBuf + 1 + width*4;
    angular = ((lgWidth + lgHeight) - 2) >> 2;
    width   = XIN_MIN (3 << angular, width);
    
    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        topLft = topBuf[0];
        lft    = lftBuf[1 + rowIdx];

        for (colIdx = 0; colIdx < XIN_MIN (3 << angular, width); colIdx++)
        {
            wl = 32 >> (2 * colIdx >> angular);

            pixelVal     = pred[colIdx] + ((wl * (lft - topLft) + 32) >> 6);
            pred[colIdx] = (PIXEL)XIN_CLIP (pixelVal, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

        pred += predStride;
    }

}

