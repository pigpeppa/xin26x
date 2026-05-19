/***************************************************************************//**
*
* @file          h265p_intra_pred_filter.c
* @brief         av1 intra prediction subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "string.h"
#include "assert.h"
#include "h265p_intra_pred_filter.h"

const UINT8 intraSmWeightU8[2 * 64] =
{
    // Unused, because we always offset by bs, which is at least 2.
    0, 0,
    // bs = 2
    255, 128,
    // bs = 4
    255, 149, 85, 64,
    // bs = 8
    255, 197, 146, 105, 73, 50, 37, 32,
    // bs = 16
    255, 225, 196, 170, 145, 123, 102, 84, 68, 54, 43, 33, 26, 20, 17, 16,
    // bs = 32
    255, 240, 225, 210, 196, 182, 169, 157, 145, 133, 122, 111, 101, 92, 83, 74,
    66, 59, 52, 45, 39, 34, 29, 25, 21, 17, 14, 12, 10, 9, 8, 8,
    // bs = 64
    255, 248, 240, 233, 225, 218, 210, 203, 196, 189, 182, 176, 169, 163, 156,
    150, 144, 138, 133, 127, 121, 116, 111, 106, 101, 96, 91, 86, 82, 77, 73, 69,
    65, 61, 57, 54, 50, 47, 44, 41, 38, 35, 32, 29, 27, 25, 22, 20, 18, 16, 15,
    13, 12, 10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 4,
};

const UINT16 intraSmWeightU16[2 * 64] =
{
    // Unused, because we always offset by bs, which is at least 2.
    0, 0,
    // bs = 2
    255, 128,
    // bs = 4
    255, 149, 85, 64,
    // bs = 8
    255, 197, 146, 105, 73, 50, 37, 32,
    // bs = 16
    255, 225, 196, 170, 145, 123, 102, 84, 68, 54, 43, 33, 26, 20, 17, 16,
    // bs = 32
    255, 240, 225, 210, 196, 182, 169, 157, 145, 133, 122, 111, 101, 92, 83, 74,
    66, 59, 52, 45, 39, 34, 29, 25, 21, 17, 14, 12, 10, 9, 8, 8,
    // bs = 64
    255, 248, 240, 233, 225, 218, 210, 203, 196, 189, 182, 176, 169, 163, 156,
    150, 144, 138, 133, 127, 121, 116, 111, 106, 101, 96, 91, 86, 82, 77, 73, 69,
    65, 61, 57, 54, 50, 47, 44, 41, 38, 35, 32, 29, 27, 25, 22, 20, 18, 16, 15,
    13, 12, 10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 4,
};

void Xin265pIntraPredDcRect (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  sumPel;
    SINT32  idx;
    SINT32  multiplier;
    SINT32  shift;
    SINT32  dcVal;

    assert (width != height);

    sumPel = 0;

    for (idx = 0; idx < width; idx++)
    {
        sumPel += topBuf[idx];
    }

    for (idx = 0; idx < height; idx++)
    {
        sumPel += lftBuf[idx];
    }

    sumPel += ((width + height) >> 1);

    if (width < height)
    {
        multiplier = (width*2 == height) ? 0x5556 : 0x3334;
        shift      = calcLog2[width];
    }
    else
    {
        multiplier = (height*2 == width) ? 0x5556 : 0x3334;
        shift      = calcLog2[height];
    }

    dcVal = ((sumPel >> shift)*multiplier) >> 16;

    for (idx = 0; idx < height; idx++)
    {
        memset (dst, dcVal, width);
        dst += dstStride;
    }

}

void Xin265pIntraPredDc128 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32 idx;

    (void)topBuf;
    (void)lftBuf;

    for (idx = 0; idx < height; idx++)
    {
        memset (dst, 128, width);
        dst += dstStride;
    }
}

void Xin265pIntraPredDcLft (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32 idx;
    SINT32 dcVal;
    SINT32 sumPel;

    (void)topBuf;

    sumPel = 0;

    for (idx = 0; idx < height; idx++)
    {
        sumPel += lftBuf[idx];
    }

    dcVal = (sumPel + (height >> 1)) / height;

    for (idx = 0; idx < height; idx++)
    {
        memset (dst, dcVal, width);
        dst += dstStride;
    }

}

void Xin265pIntraPredDcTop (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32 idx;
    SINT32 sumPel;
    SINT32 dcVal;

    (void)lftBuf;

    sumPel = 0;

    for (idx = 0; idx < width; idx++)
    {
        sumPel += topBuf[idx];
    }

    dcVal = (sumPel + (width >> 1)) / width;

    for (idx = 0; idx < height; idx++)
    {
        memset (dst, dcVal, width);
        dst += dstStride;
    }

}

void Xin265pIntraPredDcSqua (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32 idx;
    SINT32 sumPel;
    SINT32 dcVal;

    assert (width == height);

    sumPel = 0;

    for (idx = 0; idx < width; idx++)
    {
        sumPel += topBuf[idx];
    }

    for (idx = 0; idx < height; idx++)
    {
        sumPel += lftBuf[idx];
    }

    dcVal = (sumPel + width) / (width + height);

    for (idx = 0; idx < height; idx++)
    {
        memset (dst, dcVal, width);
        dst += dstStride;
    }

}

void Xin265pIntraPredDc (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    if ((topBuf != NULL) && (lftBuf != NULL))
    {
        if (width == height)
        {
            Xin265pIntraPredDcSqua(
                dst,
                dstStride,
                width,
                height,
                topBuf,
                lftBuf);
        }
        else
        {
            Xin265pIntraPredDcRect(
                dst,
                dstStride,
                width,
                height,
                topBuf,
                lftBuf);
        }
    }
    else if (topBuf != NULL)
    {
        Xin265pIntraPredDcTop (
            dst,
            dstStride,
            width,
            height,
            topBuf,
            lftBuf);
    }
    else if (lftBuf != NULL)
    {
        Xin265pIntraPredDcLft (
            dst,
            dstStride,
            width,
            height,
            topBuf,
            lftBuf);
    }
    else
    {

        Xin265pIntraPredDc128(
            dst,
            dstStride,
            width,
            height,
            topBuf,
            lftBuf);
    }

}

void Xin265pIntraPredVer (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32 idx;

    (void)lftBuf;

    for (idx = 0; idx < height; idx++)
    {
        memcpy (dst, topBuf, width);
        dst += dstStride;
    }
}

void Xin265pIntraPredHor (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32 idx;

    (void)topBuf;

    for (idx = 0; idx < height; idx++)
    {
        memset (dst, lftBuf[idx], width);

        dst += dstStride;
    }
}

// Directional prediction, zone 1: 0 < angle < 90
void Xin265pIntraPredDrZ1 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy)
{
    SINT32 rowIdx;
    SINT32 colIdx;
    SINT32 maxBaseX;
    SINT32 fracBits;
    SINT32 baseInc;
    SINT32 x;
    SINT32 base;
    SINT32 shift;

    (void)lftBuf;
    (void)dy;

    assert(dy == 1);
    assert(dx > 0);

    x        = dx;
    maxBaseX = ((width + height) - 1) << upSampleTop;
    fracBits = 6 - upSampleTop;
    baseInc  = 1 << upSampleTop;

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        base  = x >> fracBits;
        shift = ((x << upSampleTop) & 0x3F) >> 1;

        if (base >= maxBaseX)
        {
            for (; rowIdx < height; ++rowIdx)
            {
                memset (dst, topBuf[maxBaseX], width*sizeof(PIXEL));
                dst += dstStride;
            }

            return;
        }

        for (colIdx = 0; colIdx < width; ++colIdx)
        {
            if (base < maxBaseX)
            {
                dst[colIdx] = (UINT8)XIN_ROUND_POWER2 (topBuf[base] * (32 - shift) + topBuf[base + 1] * shift, 5);
            }
            else
            {
                dst[colIdx] = topBuf[maxBaseX];
            }

            base += baseInc;
        }

        dst += dstStride;
        x   += dx;

    }

}

// Directional prediction, zone 2: 90 < angle < 180
void Xin265pIntraPredDrZ2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy)
{
    SINT32 rowIdx;
    SINT32 colIdx;
    SINT32 minBaseX;
    SINT32 fracBitsX;
    SINT32 fracBitsY;
    SINT32 baseX;
    SINT32 baseY;
    SINT32 x, y;
    SINT32 shift;
    SINT32 val;

    assert(dx > 0);
    assert(dy > 0);

    minBaseX  = -(1 << upSampleTop);
    fracBitsX = 6 - upSampleTop;
    fracBitsY = 6 - upSampleLft;

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        for (colIdx = 0; colIdx < width; ++colIdx)
        {
            y     = rowIdx + 1;
            x     = (colIdx << 6) - y * dx;
            baseX = x >> fracBitsX;

            if (baseX >= minBaseX)
            {
                shift = ((x * (1 << upSampleTop)) & 0x3F) >> 1;
                val   = topBuf[baseX] * (32 - shift) + topBuf[baseX + 1] * shift;
                val   = XIN_ROUND_POWER2 (val, 5);
            }
            else
            {
                x     = colIdx + 1;
                y     = (rowIdx << 6) - x * dy;
                baseY = y >> fracBitsY;

                shift = ((y * (1 << upSampleLft)) & 0x3F) >> 1;
                val   = lftBuf[baseY] * (32 - shift) + lftBuf[baseY + 1] * shift;
                val   = XIN_ROUND_POWER2 (val, 5);
            }

            dst[colIdx] = (UINT8)val;

        }

        dst += dstStride;

    }

}

// Directional prediction, zone 3: 180 < angle < 270
void Xin265pIntraPredDrZ3 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy)
{
    SINT32 rowIdx;
    SINT32 colIdx;
    SINT32 maxBaseY;
    SINT32 fracBits;
    SINT32 baseInc;
    SINT32 y;
    SINT32 base;
    SINT32 shift;
    SINT32 val;

    (void)topBuf;
    (void)dx;
    assert(dx == 1);
    assert(dy > 0);

    y        = dy;
    maxBaseY = ((width + height) - 1) << upSampleLft;
    fracBits = 6 - upSampleLft;
    baseInc  = 1 << upSampleLft;

    for (colIdx = 0; colIdx < width; ++colIdx, y += dy)
    {
        base  = y >> fracBits;
        shift = ((y << upSampleLft) & 0x3F) >> 1;

        for (rowIdx = 0; rowIdx < height; ++rowIdx, base += baseInc)
        {
            if (base < maxBaseY)
            {
                val = lftBuf[base]*(32 - shift) + lftBuf[base + 1]*shift;

                dst[rowIdx*dstStride + colIdx] = (UINT8)XIN_ROUND_POWER2(val, 5);
            }
            else
            {
                for (; rowIdx < height; ++rowIdx)
                {
                    dst[rowIdx * dstStride + colIdx] = lftBuf[maxBaseY];
                }

                break;
            }
        }

    }

}

static PIXEL XinPaethPredSingle (
    PIXEL   lft,
    PIXEL   top,
    PIXEL topLft)
{
    SINT32  base;
    SINT32  pLft;
    SINT32  pTop;
    SINT32  pTopLft;

    base    = top + lft - topLft;
    pLft    = XIN_ABS (base - lft);
    pTop    = XIN_ABS (base - top);
    pTopLft = XIN_ABS (base - topLft);

    // Return nearest to base of left, top and top_left.
    return (pLft <= pTop && pLft <= pTopLft)
           ? lft
           : (pTop <= pTopLft) ? top : topLft;

}

void Xin265pIntraPredPaeth (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32    rowIdx;
    SINT32    colIdx;
    PIXEL     topLft;

    topLft = topBuf[-1];

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            dst[colIdx] = XinPaethPredSingle(lftBuf[rowIdx], topBuf[colIdx], topLft);
        }

        dst += dstStride;
    }

}

void Xin265pIntraPredSmH (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL       rgt;
    UINT16      scale;
    SINT32      rowIdx;
    SINT32      colIdx;
    UINT32      predVal;
    const UINT8 *smWeight;

    rgt      = topBuf[width - 1];
    smWeight = intraSmWeightU8 + width;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        for (colIdx = 0; colIdx < width; ++colIdx)
        {
            predVal     = smWeight[colIdx]*lftBuf[rowIdx] + (scale - smWeight[colIdx])*rgt;
            dst[colIdx] = (PIXEL)XIN_ROUND_POWER2 (predVal, XIN_LOG_SM_WEIGHT_SCALE);
        }

        dst += dstStride;
    }

}

void Xin265pIntraPredSmV (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL       bot;
    UINT16      scale;
    SINT32      rowIdx;
    SINT32      colIdx;
    UINT32      predVal;
    const UINT8 *smWeight;

    bot      = lftBuf[height - 1];
    smWeight = intraSmWeightU8 + height;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        for (colIdx = 0; colIdx < width; ++colIdx)
        {
            predVal     = smWeight[rowIdx]*topBuf[colIdx] + (scale - smWeight[rowIdx])*bot;
            dst[colIdx] = (PIXEL)XIN_ROUND_POWER2 (predVal, XIN_LOG_SM_WEIGHT_SCALE);
        }

        dst += dstStride;
    }

}

void Xin265pIntraPredSm (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL       bot;
    PIXEL       rgt;
    UINT16      scale;
    SINT32      rowIdx;
    SINT32      colIdx;
    UINT32      predVal;
    const UINT8 *smWeightW;
    const UINT8 *smWeightH;

    rgt       = topBuf[width - 1];
    bot       = lftBuf[height - 1];
    smWeightW = intraSmWeightU8 + width;
    smWeightH = intraSmWeightU8 + height;
    scale     = 1 << XIN_LOG_SM_WEIGHT_SCALE;

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        for (colIdx = 0; colIdx < width; ++colIdx)
        {

            predVal  = topBuf[colIdx]*smWeightH[rowIdx] + (scale - smWeightH[rowIdx])*bot;
            predVal += lftBuf[rowIdx]*smWeightW[colIdx] + (scale - smWeightW[colIdx])*rgt;

            dst[colIdx] = (PIXEL)XIN_ROUND_POWER2 (predVal, XIN_LOG_SM_WEIGHT_SCALE + 1);

        }

        dst += dstStride;
    }

}



