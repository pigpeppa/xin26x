/***************************************************************************//**
*
* @file          h265p_intra_pred_filter_sse4.c
* @brief         av1 intra prediction subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "smmintrin.h"
#include "xin_typedef.h"
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "string.h"
#include "h265p_intra_pred_filter.h"

void Xin265pIntraPredVer16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  rowIdx;
    __m128i topx16;

    (void)lftBuf;
    (void)width;
    topx16 = _mm_loadu_si128 ((__m128i *)topBuf);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storeu_si128 ((__m128i *)(dst),             topx16);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), topx16);

        dst += 2*dstStride;
    }

}

void Xin265pIntraPredVer8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  rowIdx;
    __m128i topx8;

    (void)lftBuf;
    (void)width;
    topx8 = _mm_loadl_epi64 ((__m128i *)topBuf);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storel_epi64 ((__m128i *)(dst),             topx8);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride), topx8);

        dst += 2*dstStride;
    }

}

void Xin265pIntraPredVer4xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  rowIdx;
    UINT32  topx4;

    (void)lftBuf;
    (void)width;
    topx4 = *((UINT32 *)topBuf);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        *((UINT32 *)(dst))             = topx4;
        *((UINT32 *)(dst + dstStride)) = topx4;

        dst += 2*dstStride;
    }

}

void Xin265pIntraPredHor4xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  row;
    UINT32  lft0x4;
    UINT32  lft1x4;

    (void)topBuf;
    (void)width;

    for (row = 0; row < height; row += 2)
    {
        lft0x4 = lftBuf[row]*0x01010101;
        lft1x4 = lftBuf[row+1]*0x01010101;

        *((UINT32 *)(dst))             = lft0x4;
        *((UINT32 *)(dst + dstStride)) = lft1x4;

        dst += dstStride*2;
    }

}

void Xin265pIntraPredHor8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  row;
    UINT64  lft0x8;
    UINT64  lft1x8;

    (void)topBuf;
    (void)width;

    for (row = 0; row < height; row += 2)
    {
        lft0x8 = lftBuf[row]*0x0101010101010101;
        lft1x8 = lftBuf[row+1]*0x0101010101010101;

        *((UINT64 *)(dst))             = lft0x8;
        *((UINT64 *)(dst + dstStride)) = lft1x8;

        dst += dstStride*2;
    }

}

void Xin265pIntraPredHor16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  row;
    __m128i lft0x16;
    __m128i lft1x16;

    (void)topBuf;
    (void)width;

    for (row = 0; row < height; row += 2)
    {
        lft0x16 = _mm_set1_epi8 (lftBuf[row]);
        lft1x16 = _mm_set1_epi8 (lftBuf[row+1]);

        _mm_storeu_si128 ((__m128i *)(dst),             lft0x16);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), lft1x16);

        dst += dstStride*2;
    }

}

void Xin265pIntraPredPaeth16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32    rowIdx;
    __m128i   tlx8;
    __m128i   lftx8;
    __m128i   topx16;
    __m128i   top0x8;
    __m128i   top1x8;
    __m128i   base0x8;
    __m128i   base1x8;
    __m128i   plx8;
    __m128i   ptx8;
    __m128i   ptlx8;
    __m128i   allZero;
    __m128i   ltlx8;
    __m128i   mask0x8;
    __m128i   mask1x8;
    __m128i   dst0x8, dst1x8;
    __m128i   dstx16;

    (void)width;
    allZero = _mm_setzero_si128 ();
    tlx8    = _mm_set1_epi16 (topBuf[-1]);
    topx16  = _mm_loadu_si128 ((__m128i *)(topBuf));
    top0x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top1x8  = _mm_unpackhi_epi8 (topx16, allZero);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        lftx8 = _mm_set1_epi16 (lftBuf[rowIdx]);
        ltlx8 = _mm_sub_epi16 (lftx8, tlx8);

        base0x8 = _mm_add_epi16 (top0x8,  ltlx8);
        base1x8 = _mm_add_epi16 (top1x8,  ltlx8);

        plx8  = _mm_abs_epi16 (_mm_sub_epi16 (base0x8, lftx8));
        ptx8  = _mm_abs_epi16 (_mm_sub_epi16 (base0x8, top0x8));
        ptlx8 = _mm_abs_epi16 (_mm_sub_epi16 (base0x8, tlx8));

        mask0x8 = _mm_or_si128 (_mm_cmpgt_epi16 (plx8, ptx8), _mm_cmpgt_epi16 (plx8, ptlx8));
        mask1x8 = _mm_cmpgt_epi16 (ptx8, ptlx8);

        plx8  = _mm_andnot_si128 (mask0x8, lftx8);
        ptlx8 = _mm_and_si128 (mask1x8, tlx8);
        ptx8  = _mm_andnot_si128 (mask1x8, top0x8);
        ptx8  = _mm_or_si128 (ptx8, ptlx8);
        ptx8  = _mm_and_si128 (mask0x8, ptx8);

        dst0x8 = _mm_or_si128 (ptx8, plx8);

        plx8  = _mm_abs_epi16 (_mm_sub_epi16 (base1x8, lftx8));
        ptx8  = _mm_abs_epi16 (_mm_sub_epi16 (base1x8, top1x8));
        ptlx8 = _mm_abs_epi16 (_mm_sub_epi16 (base1x8, tlx8));

        mask0x8 = _mm_or_si128 (_mm_cmpgt_epi16 (plx8, ptx8), _mm_cmpgt_epi16 (plx8, ptlx8));
        mask1x8 = _mm_cmpgt_epi16 (ptx8, ptlx8);

        plx8  = _mm_andnot_si128 (mask0x8, lftx8);
        ptlx8 = _mm_and_si128 (mask1x8, tlx8);
        ptx8  = _mm_andnot_si128 (mask1x8, top1x8);
        ptx8  = _mm_or_si128 (ptx8, ptlx8);
        ptx8  = _mm_and_si128 (mask0x8, ptx8);

        dst1x8 = _mm_or_si128 (ptx8, plx8);

        dstx16 = _mm_packus_epi16 (dst0x8, dst1x8);

        _mm_storeu_si128 ((__m128i *)(dst), dstx16);

        dst += dstStride;

    }

}

void Xin265pIntraPredPaeth8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32    rowIdx;
    __m128i   tlx8;
    __m128i   lftx8;
    __m128i   topx8;
    __m128i   basex8;
    __m128i   plx8;
    __m128i   ptx8;
    __m128i   ptlx8;
    __m128i   allZero;
    __m128i   ltlx8;
    __m128i   mask0x8;
    __m128i   mask1x8;
    __m128i   dstx8;

    (void)width;
    allZero = _mm_setzero_si128 ();
    tlx8    = _mm_set1_epi16 (topBuf[-1]);
    topx8   = _mm_loadl_epi64 ((__m128i *)(topBuf));
    topx8   = _mm_unpacklo_epi8 (topx8, allZero);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        lftx8 = _mm_set1_epi16 (lftBuf[rowIdx]);
        ltlx8 = _mm_sub_epi16 (lftx8, tlx8);

        basex8 = _mm_add_epi16 (topx8,  ltlx8);

        plx8  = _mm_abs_epi16 (_mm_sub_epi16 (basex8, lftx8));
        ptx8  = _mm_abs_epi16 (_mm_sub_epi16 (basex8, topx8));
        ptlx8 = _mm_abs_epi16 (_mm_sub_epi16 (basex8, tlx8));

        mask0x8 = _mm_or_si128 (_mm_cmpgt_epi16 (plx8, ptx8), _mm_cmpgt_epi16 (plx8, ptlx8));
        mask1x8 = _mm_cmpgt_epi16 (ptx8, ptlx8);

        plx8  = _mm_andnot_si128 (mask0x8, lftx8);
        ptlx8 = _mm_and_si128 (mask1x8, tlx8);
        ptx8  = _mm_andnot_si128 (mask1x8, topx8);
        ptx8  = _mm_or_si128 (ptx8, ptlx8);
        ptx8  = _mm_and_si128 (mask0x8, ptx8);
        dstx8 = _mm_or_si128 (ptx8, plx8);

        _mm_storel_epi64 ((__m128i *)(dst), _mm_packus_epi16 (dstx8, allZero));

        dst += dstStride;

    }

}

void Xin265pIntraPredSmV8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m128i      botx8;
    __m128i      wghtx4;
    __m128i      topx8;
    __m128i      tb0x4;
    __m128i      tb1x4;
    __m128i      pred0x4;
    __m128i      pred1x4;
    __m128i      predx8;
    __m128i      rndx4;
    __m128i      allZero;

    (void)width;
    allZero  = _mm_setzero_si128 ();
    botx8    = _mm_set1_epi16 (lftBuf[height - 1]);
    topx8    = _mm_loadl_epi64 ((__m128i *)(topBuf));
    topx8    = _mm_unpacklo_epi8 (topx8, allZero);
    tb0x4    = _mm_unpacklo_epi16 (topx8, botx8);
    tb1x4    = _mm_unpackhi_epi16 (topx8, botx8);
    smWeight = intraSmWeightU16 + height;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    rndx4    = _mm_set1_epi32 (scale>>1);

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtx4 = _mm_set1_epi32 ((smWeight[rowIdx]) | ((scale - smWeight[rowIdx]) << 16));

        pred0x4 = _mm_madd_epi16 (tb0x4, wghtx4);
        pred1x4 = _mm_madd_epi16 (tb1x4, wghtx4);

        pred0x4 = _mm_add_epi32 (pred0x4, rndx4);
        pred1x4 = _mm_add_epi32 (pred1x4, rndx4);

        pred0x4 = _mm_srli_epi32 (pred0x4, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x4 = _mm_srli_epi32 (pred1x4, XIN_LOG_SM_WEIGHT_SCALE);

        predx8 = _mm_packus_epi32 (pred0x4, pred1x4);
        predx8 = _mm_packus_epi16 (predx8, allZero);

        _mm_storel_epi64 ((__m128i *)dst, predx8);

        dst += dstStride;
    }

}

void Xin265pIntraPredSm8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    __m128i      botx8;
    __m128i      topx8;
    __m128i      tb0x4;
    __m128i      tb1x4;
    __m128i      rndx4;
    __m128i      rlx4;
    __m128i      wghtHx4;
    __m128i      wghtWx8;
    __m128i      wghtW0x4;
    __m128i      wghtW1x4;
    __m128i      scalex8;
    __m128i      pred0x4;
    __m128i      pred1x4;
    __m128i      predx8;
    __m128i      allZero;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeightW;
    const UINT16 *smWeightH;

    rgt       = topBuf[width - 1];
    smWeightW = intraSmWeightU16 + width;
    smWeightH = intraSmWeightU16 + height;
    allZero   = _mm_setzero_si128 ();
    scale     = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    botx8     = _mm_set1_epi16 (lftBuf[height - 1]);
    topx8     = _mm_loadl_epi64 ((__m128i *)(topBuf));
    topx8     = _mm_unpacklo_epi8 (topx8, allZero);
    tb0x4     = _mm_unpacklo_epi16 (topx8, botx8);
    tb1x4     = _mm_unpackhi_epi16 (topx8, botx8);
    scalex8   = _mm_set1_epi16 ((SINT16)scale);
    rndx4     = _mm_set1_epi32 (scale);
    wghtWx8   = _mm_loadu_si128 ((__m128i *)(smWeightW));
    wghtW0x4  = _mm_unpacklo_epi16 (wghtWx8, _mm_sub_epi16 (scalex8, wghtWx8));
    wghtW1x4  = _mm_loadl_epi64 ((__m128i *)(smWeightW + 4));
    wghtW1x4  = _mm_unpackhi_epi16 (wghtWx8, _mm_sub_epi16 (scalex8, wghtWx8));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtHx4 = _mm_set1_epi32 ((smWeightH[rowIdx]) | ((scale - smWeightH[rowIdx]) << 16));
        rlx4    = _mm_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x4 = _mm_add_epi32 (_mm_madd_epi16 (rlx4, wghtW0x4), rndx4);
        pred1x4 = _mm_add_epi32 (_mm_madd_epi16 (rlx4, wghtW1x4), rndx4);
        pred0x4 = _mm_add_epi32 (pred0x4, _mm_madd_epi16 (tb0x4, wghtHx4));
        pred1x4 = _mm_add_epi32 (pred1x4, _mm_madd_epi16 (tb1x4, wghtHx4));

        pred0x4 = _mm_srli_epi32 (pred0x4, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred1x4 = _mm_srli_epi32 (pred1x4, XIN_LOG_SM_WEIGHT_SCALE + 1);

        predx8 = _mm_packus_epi32 (pred0x4, pred1x4);
        predx8 = _mm_packus_epi16 (predx8, allZero);

        _mm_storel_epi64 ((__m128i *)dst, predx8);

        dst += dstStride;

    }

}

void Xin265pIntraPredSmH8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m128i      wghtx8;
    __m128i      wght0x4, wght1x4;
    __m128i      pred0x4, pred1x4;
    __m128i      scalex8;
    __m128i      rlx4;
    __m128i      rndx4;
    __m128i      predx8;

    rgt      = topBuf[width - 1];
    smWeight = intraSmWeightU16 + width;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    scalex8  = _mm_set1_epi16 ((SINT16)scale);
    rndx4    = _mm_set1_epi32 (scale>>1);
    wghtx8   = _mm_loadu_si128 ((__m128i *)(smWeight));
    wght0x4  = _mm_unpacklo_epi16 (wghtx8, _mm_sub_epi16 (scalex8, wghtx8));
    wght1x4  = _mm_loadl_epi64 ((__m128i *)(smWeight + 4));
    wght1x4  = _mm_unpackhi_epi16 (wghtx8, _mm_sub_epi16 (scalex8, wghtx8));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        rlx4  = _mm_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x4 = _mm_add_epi32 (_mm_madd_epi16 (rlx4, wght0x4), rndx4);
        pred1x4 = _mm_add_epi32 (_mm_madd_epi16 (rlx4, wght1x4), rndx4);

        pred0x4 = _mm_srli_epi32 (pred0x4, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x4 = _mm_srli_epi32 (pred1x4, XIN_LOG_SM_WEIGHT_SCALE);


        predx8 = _mm_packus_epi32 (pred0x4, pred1x4);
        predx8 = _mm_packus_epi16 (predx8, predx8);

        _mm_storel_epi64 ((__m128i *)dst, predx8);

        dst += dstStride;
    }

}


