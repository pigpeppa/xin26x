/***************************************************************************//**
 *
 * @file          h266_loop_filter_sse4.c
 * @brief         h266 loop filter implementation (SSE4).
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
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include "xin_typedef.h"
#include "basic_macro.h"
#include "h26x_block_transpose.h"

static const SINT32 tc7x2[7] =
{
    0x00060006, 0x00050005, 0x00040004, 0x00030003, 0x00020002, 0x00010001, 0x00010001
};

static const SINT32 tc3x2[3] =
{
    0x00060006, 0x00040004, 0x00020002
};

static const SINT32 dbCoeffs7x2[7] =
{
    0x003b003b, 0x00320032, 0x00290029, 0x00200020, 0x00170017, 0x000e000e, 0x00050005
};

static const SINT32 dbCoeffs3x2[3] =
{
    0x00350035, 0x00200020, 0x000b000b
};

static const SINT32 dbCoeffs5x2[5] =
{
    0x003a003a, 0x002d002d, 0x00200020, 0x00130013, 0x00060006
};

static const UINT8 tcMap[16] =
{
    0, 1, 0, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 3, 2, 3
};

static inline SINT32 calcDP (
    PIXEL    *src,
    intptr_t offset,
    BOOL     isChromaCtuBoundary)
{
    if (isChromaCtuBoundary)
    {
        return XIN_ABS(src[-offset*2] - 2*src[-offset*2] + src[-offset]);
    }
    else
    {
        return XIN_ABS(src[-offset*3] - 2*src[-offset*2] + src[-offset]);
    }
}

static inline SINT32 calcDQ (
    PIXEL    *src,
    intptr_t offset)
{
    return XIN_ABS(src[0] - 2*src[offset] + src[offset*2]);
}

static BOOL UseStrongFiltering (
    PIXEL    *src,
    intptr_t offset,
    SINT32   d,
    SINT32   beta,
    SINT32   tc,
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ,
    BOOL     isChromaHorCtuBoundary)
{
    PIXEL  m3, m4;
    PIXEL  m0, m7;
    PIXEL  m2;
    PIXEL  mP4, mP5, mP6, mP7;
    PIXEL  m8, m9, m10;
    PIXEL  m11;
    SINT32 sp3;
    SINT32 sq3;
    SINT32 strong;

    m4  = src[0];
    m3  = src[-offset];
    m7  = src[ offset*3];
    m0  = src[-offset*4];
    m2  = src[-offset*2];
    sp3 = XIN_ABS (m0 - m3);
    sq3 = XIN_ABS (m7 - m4);

    if (isChromaHorCtuBoundary)
    {
        sp3 = XIN_ABS (m2 - m3);
    }

    strong = sp3 + sq3;

    if (sidePisLarge || sideQisLarge)
    {
        if (sidePisLarge)
        {
            if (maxFilterLengthP == 7)
            {
                mP5 = src[-offset*5];
                mP6 = src[-offset*6];
                mP7 = src[-offset*7];
                mP4 = src[-offset*8];
                sp3 = sp3 + XIN_ABS (mP5 - mP6 - mP7 + mP4);
            }
            else
            {
                mP4 = src[-offset * 6];
            }

            sp3 = (sp3 + XIN_ABS (m0 - mP4) + 1) >> 1;
        }

        if (sideQisLarge)
        {
            if (maxFilterLengthQ == 7)
            {
                m8  = src[offset*4];
                m9  = src[offset*5];
                m10 = src[offset*6];
                m11 = src[offset*7];
                sq3 = sq3 + XIN_ABS (m8 - m9 - m10 + m11);
            }
            else
            {
                m11 = src[offset*5];
            }

            sq3 = (sq3 + XIN_ABS (m11 - m7) + 1) >> 1;
        }

        return ((sp3 + sq3) < (beta*3 >> 5)) && (d < (beta >> 4)) && (XIN_ABS(m3 - m4) < ((tc * 5 + 1) >> 1));

    }
    else
    {
        return (strong < (beta>>3)) && (d<(beta>>2)) && ( XIN_ABS(m3-m4) < ((tc*5+1)>>1));
    }

}

static void Xin266Filter1stLuma8_SSE4 (
    PIXEL    *src,
    intptr_t offset,
    SINT32   tc,
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ)
{
    static const UINT16 s_Const4[8] = {4, 4, 4, 4, 4, 4, 4, 4};
    const SINT32 *tcP;
    const SINT32 *tcQ;
    const SINT32 *dbCoeffsP;
    const SINT32 *dbCoeffsQ;

    __m128i p3x4, p2x4, p1x4, p0x4;
    __m128i q0x4, q1x4, q2x4, q3x4;
    __m128i p4x4, p5x4, p6x4, p7x4, q4x4, q5x4, q6x4, q7x4;
    __m128i nP3x4, nP2x4, nP1x4, nP0x4, nQ0x4, nQ1x4, nQ2x4, nQ3x4;
    __m128i nP6x4, nP5x4, nP4x4, nQ4x4, nQ5x4, nQ6x4;
    __m128i const4x4, const8x4, const64x4, const32x4;
    __m128i refPx4, refQx4;
    __m128i refMidx4;
    __m128i cValx4, coef0x4, coef1x4;
    __m128i tcMapx16, tc128;
    UINT32  numPSide, numQSide;
    PIXEL   *srcP, *srcQ;

    srcP = src - offset;
    srcQ = src;

    numPSide = sidePisLarge ? maxFilterLengthP : 3;
    numQSide = sideQisLarge ? maxFilterLengthQ : 3;

    dbCoeffsP = numPSide == 7 ? dbCoeffs7x2 : (numPSide == 5) ? dbCoeffs5x2 : dbCoeffs3x2;
    dbCoeffsQ = numQSide == 7 ? dbCoeffs7x2 : (numQSide == 5) ? dbCoeffs5x2 : dbCoeffs3x2;
    tcMapx16  = _mm_loadu_si128 ((__m128i *)tcMap);
    tc128     = _mm_cvtsi32_si128 (tc);

    const4x4  = _mm_loadu_si128 ((__m128i *)s_Const4);
    const8x4  = _mm_slli_epi16 (const4x4, 1);
    const64x4 = _mm_slli_epi16 (const8x4, 3);
    const32x4 = _mm_slli_epi16 (const8x4, 2);

    p3x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*3));
    p2x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*2));
    p1x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*1));
    p0x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*0));
    q0x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*0));
    q1x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*1));
    q2x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*2));
    q3x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*3));

    p3x4 = _mm_cvtepu8_epi16 (p3x4);
    p2x4 = _mm_cvtepu8_epi16 (p2x4);
    p1x4 = _mm_cvtepu8_epi16 (p1x4);
    p0x4 = _mm_cvtepu8_epi16 (p0x4);

    q0x4 = _mm_cvtepu8_epi16 (q0x4);
    q1x4 = _mm_cvtepu8_epi16 (q1x4);
    q2x4 = _mm_cvtepu8_epi16 (q2x4);
    q3x4 = _mm_cvtepu8_epi16 (q3x4);

    // kill off complier warnings
    p4x4 = _mm_setzero_si128 ();
    p5x4 = _mm_setzero_si128 ();
    p6x4 = _mm_setzero_si128 ();
    p7x4 = _mm_setzero_si128 ();

    q4x4 = _mm_setzero_si128 ();
    q5x4 = _mm_setzero_si128 ();
    q6x4 = _mm_setzero_si128 ();
    q7x4 = _mm_setzero_si128 ();

    if (numPSide == 7)
    {
        p7x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*7));
        p6x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*6));
        p5x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*5));
        p4x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*4));

        p7x4 = _mm_cvtepu8_epi16 (p7x4);
        p6x4 = _mm_cvtepu8_epi16 (p6x4);
        p5x4 = _mm_cvtepu8_epi16 (p5x4);
        p4x4 = _mm_cvtepu8_epi16 (p4x4);

        refPx4 = _mm_avg_epu16(p6x4, p7x4);
    }
    else if (numPSide == 5)
    {
        p5x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*5));
        p4x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*4));

        p5x4 = _mm_cvtepu8_epi16 (p5x4);
        p4x4 = _mm_cvtepu8_epi16 (p4x4);

        refPx4 = _mm_avg_epu16(p4x4, p5x4);
    }
    else
    {
        refPx4 = _mm_avg_epu16(p2x4, p3x4);
    }

    if (numQSide == 7)
    {
        q4x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*4));
        q5x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*5));
        q6x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*6));
        q7x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*7));

        q4x4 = _mm_cvtepu8_epi16 (q4x4);
        q5x4 = _mm_cvtepu8_epi16 (q5x4);
        q6x4 = _mm_cvtepu8_epi16 (q6x4);
        q7x4 = _mm_cvtepu8_epi16 (q7x4);

        refQx4 = _mm_avg_epu16(q6x4, q7x4);

    }
    else if (numQSide == 5)
    {
        q4x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*4));
        q5x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*5));

        q4x4 = _mm_cvtepu8_epi16 (q4x4);
        q5x4 = _mm_cvtepu8_epi16 (q5x4);

        refQx4 = _mm_avg_epu16(q4x4, q5x4);
    }
    else
    {
        refQx4 = _mm_avg_epu16(q2x4, q3x4);
    }

    if (numPSide == numQSide)
    {
        if (numPSide == 5)
        {
            refMidx4 = _mm_add_epi16 (q0x4,      p0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q2x4);
            refMidx4 = _mm_slli_epi16 (refMidx4, 1);
            refMidx4 = _mm_add_epi16 (refMidx4,  p3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  const8x4);
            refMidx4 = _mm_srai_epi16 (refMidx4, 4);
        }
        else
        {
            refMidx4 = _mm_add_epi16 (q0x4,      p0x4);
            refMidx4 = _mm_slli_epi16 (refMidx4, 1);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p5x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q5x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p6x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q6x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  const8x4);
            refMidx4 = _mm_srai_epi16 (refMidx4, 4);
        }

    }
    else
    {
        if ((numQSide == 7 && numPSide == 5) || (numQSide == 5 && numPSide == 7))
        {
            refMidx4 = _mm_add_epi16 (q0x4,      p0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_slli_epi16 (refMidx4, 1);
            refMidx4 = _mm_add_epi16 (refMidx4,  p2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p5x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q5x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  const8x4);
            refMidx4 = _mm_srai_epi16 (refMidx4, 4);
        }
        else if (numQSide == 3 && numPSide == 7)
        {
            refMidx4 = _mm_add_epi16 (q0x4,      p0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q2x4);
            refMidx4 = _mm_slli_epi16 (refMidx4, 1);
            refMidx4 = _mm_add_epi16 (refMidx4,  q0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p5x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p6x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  const8x4);
            refMidx4 = _mm_srai_epi16 (refMidx4, 4);
        }
        else if (numQSide == 7 && numPSide == 3)
        {
            refMidx4 = _mm_add_epi16 (q0x4,      p0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p2x4);
            refMidx4 = _mm_slli_epi16 (refMidx4, 1);
            refMidx4 = _mm_add_epi16 (refMidx4,  p0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q4x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q5x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q6x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  const8x4);
            refMidx4 = _mm_srai_epi16 (refMidx4, 4);
        }
        else
        {
            refMidx4 = _mm_add_epi16 (q0x4,      p0x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q1x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q2x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  p3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  q3x4);
            refMidx4 = _mm_add_epi16 (refMidx4,  const4x4);
            refMidx4 = _mm_srai_epi16 (refMidx4, 3);
        }

    }

    tcP = (numPSide == 3) ? tc3x2 : tc7x2;
    tcQ = (numQSide == 3) ? tc3x2 : tc7x2;

    // p0
    cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[0]));
    cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
    cValx4 = _mm_srai_epi16 (cValx4, 1);

    coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[0]);
    coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
    coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

    coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
    coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

    nP0x4 = _mm_add_epi16 (coef0x4, coef1x4);
    nP0x4 = _mm_add_epi16 (nP0x4,   const32x4);
    nP0x4 = _mm_srai_epi16 (nP0x4,  6);
    nP0x4 = _mm_max_epi16 (nP0x4, _mm_sub_epi16 (p0x4, cValx4));
    nP0x4 = _mm_min_epi16 (nP0x4, _mm_add_epi16 (p0x4, cValx4));

    // q0
    cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[0]));
    cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
    cValx4 = _mm_srai_epi16 (cValx4, 1);

    coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[0]);
    coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
    coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

    coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
    coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

    nQ0x4 = _mm_add_epi16 (coef0x4, coef1x4);
    nQ0x4 = _mm_add_epi16 (nQ0x4,   const32x4);
    nQ0x4 = _mm_srai_epi16 (nQ0x4,  6);
    nQ0x4 = _mm_max_epi16 (nQ0x4, _mm_sub_epi16 (q0x4, cValx4));
    nQ0x4 = _mm_min_epi16 (nQ0x4, _mm_add_epi16 (q0x4, cValx4));

    // p1
    cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[1]));
    cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
    cValx4 = _mm_srai_epi16 (cValx4, 1);

    coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[1]);
    coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
    coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

    coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
    coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

    nP1x4 = _mm_add_epi16 (coef0x4, coef1x4);
    nP1x4 = _mm_add_epi16 (nP1x4,   const32x4);
    nP1x4 = _mm_srai_epi16 (nP1x4,  6);
    nP1x4 = _mm_max_epi16 (nP1x4, _mm_sub_epi16 (p1x4, cValx4));
    nP1x4 = _mm_min_epi16 (nP1x4, _mm_add_epi16 (p1x4, cValx4));

    // q1
    cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[1]));
    cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
    cValx4 = _mm_srai_epi16 (cValx4, 1);

    coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[1]);
    coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
    coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

    coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
    coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

    nQ1x4 = _mm_add_epi16 (coef0x4, coef1x4);
    nQ1x4 = _mm_add_epi16 (nQ1x4,   const32x4);
    nQ1x4 = _mm_srai_epi16 (nQ1x4,  6);
    nQ1x4 = _mm_max_epi16 (nQ1x4, _mm_sub_epi16 (q1x4, cValx4));
    nQ1x4 = _mm_min_epi16 (nQ1x4, _mm_add_epi16 (q1x4, cValx4));

    // p2
    cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[2]));
    cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
    cValx4 = _mm_srai_epi16 (cValx4, 1);

    coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[2]);
    coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
    coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

    coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
    coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

    nP2x4 = _mm_add_epi16 (coef0x4, coef1x4);
    nP2x4 = _mm_add_epi16 (nP2x4,   const32x4);
    nP2x4 = _mm_srai_epi16 (nP2x4,  6);
    nP2x4 = _mm_max_epi16 (nP2x4, _mm_sub_epi16 (p2x4, cValx4));
    nP2x4 = _mm_min_epi16 (nP2x4, _mm_add_epi16 (p2x4, cValx4));

    // q2
    cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[2]));
    cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
    cValx4 = _mm_srai_epi16 (cValx4, 1);

    coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[2]);
    coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
    coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

    coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
    coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

    nQ2x4 = _mm_add_epi16 (coef0x4, coef1x4);
    nQ2x4 = _mm_add_epi16 (nQ2x4,   const32x4);
    nQ2x4 = _mm_srai_epi16 (nQ2x4,  6);
    nQ2x4 = _mm_max_epi16 (nQ2x4, _mm_sub_epi16 (q2x4, cValx4));
    nQ2x4 = _mm_min_epi16 (nQ2x4, _mm_add_epi16 (q2x4, cValx4));

    nP0x4 = _mm_packus_epi16 (nP0x4, nP0x4);
    nQ0x4 = _mm_packus_epi16 (nQ0x4, nQ0x4);
    nP1x4 = _mm_packus_epi16 (nP1x4, nP1x4);
    nQ1x4 = _mm_packus_epi16 (nQ1x4, nQ1x4);
    nP2x4 = _mm_packus_epi16 (nP2x4, nP2x4);
    nQ2x4 = _mm_packus_epi16 (nQ2x4, nQ2x4);

    _mm_storel_epi64 ((__m128i *)(srcP - offset*0), nP0x4);
    _mm_storel_epi64 ((__m128i *)(srcQ + offset*0), nQ0x4);
    _mm_storel_epi64 ((__m128i *)(srcP - offset*1), nP1x4);
    _mm_storel_epi64 ((__m128i *)(srcQ + offset*1), nQ1x4);
    _mm_storel_epi64 ((__m128i *)(srcP - offset*2), nP2x4);
    _mm_storel_epi64 ((__m128i *)(srcQ + offset*2), nQ2x4);

    if (numPSide >= 5)
    {
        // p3
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[3]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[3]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

        nP3x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nP3x4 = _mm_add_epi16 (nP3x4,   const32x4);
        nP3x4 = _mm_srai_epi16 (nP3x4,  6);
        nP3x4 = _mm_max_epi16 (nP3x4, _mm_sub_epi16 (p3x4, cValx4));
        nP3x4 = _mm_min_epi16 (nP3x4, _mm_add_epi16 (p3x4, cValx4));

        // p4
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[4]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[4]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

        nP4x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nP4x4 = _mm_add_epi16 (nP4x4,   const32x4);
        nP4x4 = _mm_srai_epi16 (nP4x4,  6);
        nP4x4 = _mm_max_epi16 (nP4x4, _mm_sub_epi16 (p4x4, cValx4));
        nP4x4 = _mm_min_epi16 (nP4x4, _mm_add_epi16 (p4x4, cValx4));

        nP3x4 = _mm_packus_epi16 (nP3x4, nP3x4);
        nP4x4 = _mm_packus_epi16 (nP4x4, nP4x4);

        _mm_storel_epi64 ((__m128i *)(srcP - offset*3), nP3x4);
        _mm_storel_epi64 ((__m128i *)(srcP - offset*4), nP4x4);

    }

    if (numPSide >= 7)
    {
        // p5
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[5]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[5]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

        nP5x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nP5x4 = _mm_add_epi16 (nP5x4,   const32x4);
        nP5x4 = _mm_srai_epi16 (nP5x4,  6);
        nP5x4 = _mm_max_epi16 (nP5x4, _mm_sub_epi16 (p5x4, cValx4));
        nP5x4 = _mm_min_epi16 (nP5x4, _mm_add_epi16 (p5x4, cValx4));

        // p6
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcP[6]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsP[6]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refPx4,   coef1x4);

        nP6x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nP6x4 = _mm_add_epi16 (nP6x4,   const32x4);
        nP6x4 = _mm_srai_epi16 (nP6x4,  6);
        nP6x4 = _mm_max_epi16 (nP6x4, _mm_sub_epi16 (p6x4, cValx4));
        nP6x4 = _mm_min_epi16 (nP6x4, _mm_add_epi16 (p6x4, cValx4));

        nP5x4 = _mm_packus_epi16 (nP5x4, nP5x4);
        nP6x4 = _mm_packus_epi16 (nP6x4, nP6x4);

        _mm_storel_epi64 ((__m128i *)(srcP - offset*5), nP5x4);
        _mm_storel_epi64 ((__m128i *)(srcP - offset*6), nP6x4);

    }

    if (numQSide >= 5)
    {
        // q3
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[3]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[3]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

        nQ3x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nQ3x4 = _mm_add_epi16 (nQ3x4,   const32x4);
        nQ3x4 = _mm_srai_epi16 (nQ3x4,  6);
        nQ3x4 = _mm_max_epi16 (nQ3x4, _mm_sub_epi16 (q3x4, cValx4));
        nQ3x4 = _mm_min_epi16 (nQ3x4, _mm_add_epi16 (q3x4, cValx4));

        // q4
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[4]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[4]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

        nQ4x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nQ4x4 = _mm_add_epi16 (nQ4x4,   const32x4);
        nQ4x4 = _mm_srai_epi16 (nQ4x4,  6);
        nQ4x4 = _mm_max_epi16 (nQ4x4, _mm_sub_epi16 (q4x4, cValx4));
        nQ4x4 = _mm_min_epi16 (nQ4x4, _mm_add_epi16 (q4x4, cValx4));

        nQ3x4 = _mm_packus_epi16 (nQ3x4, nQ3x4);
        nQ4x4 = _mm_packus_epi16 (nQ4x4, nQ4x4);

        _mm_storel_epi64 ((__m128i *)(srcQ + offset*3), nQ3x4);
        _mm_storel_epi64 ((__m128i *)(srcQ + offset*4), nQ4x4);

    }

    if (numQSide >= 7)
    {
        // q5
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[5]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[5]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

        nQ5x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nQ5x4 = _mm_add_epi16 (nQ5x4,   const32x4);
        nQ5x4 = _mm_srai_epi16 (nQ5x4,  6);
        nQ5x4 = _mm_max_epi16 (nQ5x4, _mm_sub_epi16 (q5x4, cValx4));
        nQ5x4 = _mm_min_epi16 (nQ5x4, _mm_add_epi16 (q5x4, cValx4));

        // q6
        cValx4 = _mm_mullo_epi16 (tc128, _mm_cvtsi32_si128 (tcQ[6]));
        cValx4 = _mm_shuffle_epi8 (cValx4, tcMapx16);
        cValx4 = _mm_srai_epi16 (cValx4, 1);

        coef0x4 = _mm_cvtsi32_si128 (dbCoeffsQ[6]);
        coef0x4 = _mm_shuffle_epi32 (coef0x4, 0);
        coef1x4 = _mm_sub_epi16 (const64x4, coef0x4);

        coef0x4 = _mm_mullo_epi16 (refMidx4, coef0x4);
        coef1x4 = _mm_mullo_epi16 (refQx4,   coef1x4);

        nQ6x4 = _mm_add_epi16 (coef0x4, coef1x4);
        nQ6x4 = _mm_add_epi16 (nQ6x4,   const32x4);
        nQ6x4 = _mm_srai_epi16 (nQ6x4,  6);
        nQ6x4 = _mm_max_epi16 (nQ6x4, _mm_sub_epi16 (q6x4, cValx4));
        nQ6x4 = _mm_min_epi16 (nQ6x4, _mm_add_epi16 (q6x4, cValx4));

        nQ5x4 = _mm_packus_epi16 (nQ5x4, nQ5x4);
        nQ6x4 = _mm_packus_epi16 (nQ6x4, nQ6x4);

        _mm_storel_epi64 ((__m128i *)(srcQ + offset*5), nQ5x4);
        _mm_storel_epi64 ((__m128i *)(srcQ + offset*6), nQ6x4);

    }

}

static void Xin266Filter2ndLuma8_SSE4 (
    PIXEL    *src,
    intptr_t offset,
    SINT32   tc,
    BOOL     strong,
    SINT32   filter2ndP,
    SINT32   filter2ndQ)
{
    static const UINT16 s_Const8[8]  = {8,  8,  8,  8,  8,  8,  8,  8};
    static const UINT16 s_Const4[8]  = {4,  4,  4,  4,  4,  4,  4,  4};
    static const UINT16 s_Const10[8] = {10, 10, 10, 10, 10, 10, 10, 10};

    __m128i allZero, const8x4, thrCutx4, allOnes, const10x4;
    __m128i deltax4, delta1x4, delta2x4;
    __m128i q0sP0x4, q1sP1x4;
    __m128i avgM1M3x4, avgM4M6x4;
    __m128i fltx4;
    __m128i p3x4, p2x4, p1x4, p0x4;
    __m128i q0x4, q1x4, q2x4, q3x4;
    __m128i dM0x4, tM1x4, tQ2x4, dQ3x4;
    __m128i nP2x4, nP1x4, nP0x4, nQ0x4, nQ1x4, nQ2x4;
    __m128i m2M3M4x4, m3M4M5x4;
    __m128i const2x4, const4x4;
    __m128i tc1x4, tc2x4, tc3x4;
    __m128i tcMapx16, tc128;
    __m128i fltPMask, fltQMask;
    PIXEL   *srcP, *srcQ;

    srcP = src - offset;
    srcQ = src;

    tc128    = _mm_cvtsi32_si128 (tc);
    tcMapx16 = _mm_loadu_si128 ((__m128i *)tcMap);
    tc1x4    = _mm_shuffle_epi8 (tc128, tcMapx16);

    if (strong)
    {
        p3x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*3));
        p2x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*2));
        p1x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*1));
        p0x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*0));
        q0x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*0));
        q1x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*1));
        q2x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*2));
        q3x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*3));

        p3x4 = _mm_cvtepu8_epi16 (p3x4);
        p2x4 = _mm_cvtepu8_epi16 (p2x4);
        p1x4 = _mm_cvtepu8_epi16 (p1x4);
        p0x4 = _mm_cvtepu8_epi16 (p0x4);
        q0x4 = _mm_cvtepu8_epi16 (q0x4);
        q1x4 = _mm_cvtepu8_epi16 (q1x4);
        q2x4 = _mm_cvtepu8_epi16 (q2x4);
        q3x4 = _mm_cvtepu8_epi16 (q3x4);

        dM0x4 = _mm_add_epi16 (p3x4, p3x4);
        tM1x4 = _mm_add_epi16 (p2x4, _mm_add_epi16 (p2x4, p2x4));
        tQ2x4 = _mm_add_epi16 (q2x4, _mm_add_epi16 (q2x4, q2x4));
        dQ3x4 = _mm_add_epi16 (q3x4, q3x4);

        tc2x4 = _mm_slli_epi16 (tc1x4, 1);
        tc3x4 = _mm_add_epi16 (tc1x4, tc2x4);

        m2M3M4x4 = _mm_add_epi16 (p1x4, _mm_add_epi16 (p0x4, q0x4));
        m3M4M5x4 = _mm_add_epi16 (p0x4, _mm_add_epi16 (q0x4, q1x4));

        const4x4 = _mm_loadu_si128 ((__m128i *)s_Const4);
        const2x4 = _mm_srai_epi16 (const4x4, 1);

        nP0x4 = _mm_add_epi16 (m2M3M4x4, m2M3M4x4);
        nP0x4 = _mm_add_epi16 (nP0x4,    p2x4);
        nP0x4 = _mm_add_epi16 (nP0x4,    q1x4);
        nP0x4 = _mm_add_epi16 (nP0x4,    const4x4);

        nQ0x4 = _mm_add_epi16 (m3M4M5x4, m3M4M5x4);
        nQ0x4 = _mm_add_epi16 (nQ0x4,    p1x4);
        nQ0x4 = _mm_add_epi16 (nQ0x4,    q2x4);
        nQ0x4 = _mm_add_epi16 (nQ0x4,    const4x4);

        nP1x4 = _mm_add_epi16 (m2M3M4x4, p2x4);
        nP1x4 = _mm_add_epi16 (nP1x4,    const2x4);

        nQ1x4 = _mm_add_epi16 (m3M4M5x4, q2x4);
        nQ1x4 = _mm_add_epi16 (nQ1x4,    const2x4);

        nP2x4 = _mm_add_epi16 (m2M3M4x4, dM0x4);
        nP2x4 = _mm_add_epi16 (nP2x4,    tM1x4);
        nP2x4 = _mm_add_epi16 (nP2x4,    const4x4);

        nQ2x4 = _mm_add_epi16 (m3M4M5x4, tQ2x4);
        nQ2x4 = _mm_add_epi16 (nQ2x4,    dQ3x4);
        nQ2x4 = _mm_add_epi16 (nQ2x4,    const4x4);

        nP0x4 = _mm_srai_epi16 (nP0x4, 3);
        nQ0x4 = _mm_srai_epi16 (nQ0x4, 3);
        nP1x4 = _mm_srai_epi16 (nP1x4, 2);
        nQ1x4 = _mm_srai_epi16 (nQ1x4, 2);
        nP2x4 = _mm_srai_epi16 (nP2x4, 3);
        nQ2x4 = _mm_srai_epi16 (nQ2x4, 3);

        nP0x4 = _mm_max_epi16 (nP0x4, _mm_sub_epi16 (p0x4, tc3x4));
        nP0x4 = _mm_min_epi16 (nP0x4, _mm_add_epi16 (p0x4, tc3x4));
        nQ0x4 = _mm_max_epi16 (nQ0x4, _mm_sub_epi16 (q0x4, tc3x4));
        nQ0x4 = _mm_min_epi16 (nQ0x4, _mm_add_epi16 (q0x4, tc3x4));
        nP1x4 = _mm_max_epi16 (nP1x4, _mm_sub_epi16 (p1x4, tc2x4));
        nP1x4 = _mm_min_epi16 (nP1x4, _mm_add_epi16 (p1x4, tc2x4));
        nQ1x4 = _mm_max_epi16 (nQ1x4, _mm_sub_epi16 (q1x4, tc2x4));
        nQ1x4 = _mm_min_epi16 (nQ1x4, _mm_add_epi16 (q1x4, tc2x4));
        nP2x4 = _mm_max_epi16 (nP2x4, _mm_sub_epi16 (p2x4, tc1x4));
        nP2x4 = _mm_min_epi16 (nP2x4, _mm_add_epi16 (p2x4, tc1x4));
        nQ2x4 = _mm_max_epi16 (nQ2x4, _mm_sub_epi16 (q2x4, tc1x4));
        nQ2x4 = _mm_min_epi16 (nQ2x4, _mm_add_epi16 (q2x4, tc1x4));

        nP0x4 = _mm_packus_epi16 (nP0x4, nP0x4);
        nQ0x4 = _mm_packus_epi16 (nQ0x4, nQ0x4);
        nP1x4 = _mm_packus_epi16 (nP1x4, nP1x4);
        nQ1x4 = _mm_packus_epi16 (nQ1x4, nQ1x4);
        nP2x4 = _mm_packus_epi16 (nP2x4, nP2x4);
        nQ2x4 = _mm_packus_epi16 (nQ2x4, nQ2x4);

        _mm_storel_epi64 ((__m128i *)(srcP + offset*0), nP0x4);
        _mm_storel_epi64 ((__m128i *)(srcQ + offset*0), nQ0x4);
        _mm_storel_epi64 ((__m128i *)(srcP - offset*1), nP1x4);
        _mm_storel_epi64 ((__m128i *)(srcQ + offset*1), nQ1x4);
        _mm_storel_epi64 ((__m128i *)(srcP - offset*2), nP2x4);
        _mm_storel_epi64 ((__m128i *)(srcQ + offset*2), nQ2x4);

    }
    else
    {
        allZero   = _mm_setzero_si128();
        const8x4  = _mm_loadu_si128 ((__m128i *)s_Const8);
        const10x4 = _mm_loadu_si128 ((__m128i *)s_Const10);
        thrCutx4  = _mm_mullo_epi16 (tc1x4, const10x4);
        allOnes   = _mm_cmpeq_epi16 (allZero, allZero);

        p2x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*2));
        p1x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*1));
        p0x4 = _mm_loadl_epi64 ((__m128i *)(srcP - offset*0));
        q0x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*0));
        q1x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*1));
        q2x4 = _mm_loadl_epi64 ((__m128i *)(srcQ + offset*2));

        p2x4 = _mm_cvtepu8_epi16 (p2x4);
        p1x4 = _mm_cvtepu8_epi16 (p1x4);
        p0x4 = _mm_cvtepu8_epi16 (p0x4);
        q0x4 = _mm_cvtepu8_epi16 (q0x4);
        q1x4 = _mm_cvtepu8_epi16 (q1x4);
        q2x4 = _mm_cvtepu8_epi16 (q2x4);

        /* Weak filter */
        q0sP0x4 = _mm_sub_epi16 (q0x4, p0x4);
        q1sP1x4 = _mm_sub_epi16 (q1x4, p1x4);
        q0sP0x4 = _mm_add_epi16 (q0sP0x4, _mm_slli_epi16 (q0sP0x4, 3));
        q1sP1x4 = _mm_add_epi16 (q1sP1x4, _mm_slli_epi16 (q1sP1x4, 1));
        deltax4 = _mm_sub_epi16 (q0sP0x4, q1sP1x4);
        deltax4 = _mm_add_epi16 (deltax4, const8x4);
        deltax4 = _mm_srai_epi16 (deltax4, 4);

        fltx4 = _mm_cmpgt_epi16 (thrCutx4, _mm_abs_epi16 (deltax4));

        if (!_mm_test_all_zeros (fltx4, allOnes))
        {
            deltax4 = _mm_min_epi16 (deltax4, tc1x4);
            deltax4 = _mm_max_epi16 (deltax4, _mm_sub_epi16 (allZero, tc1x4));

            nQ0x4 = _mm_sub_epi16 (q0x4, deltax4);
            nP0x4 = _mm_add_epi16 (p0x4, deltax4);

            nQ0x4 = _mm_blendv_epi8 (q0x4, nQ0x4, fltx4);
            nP0x4 = _mm_blendv_epi8 (p0x4, nP0x4, fltx4);

            nQ0x4 = _mm_packus_epi16 (nQ0x4, nQ0x4);
            nP0x4 = _mm_packus_epi16 (nP0x4, nP0x4);

            _mm_storel_epi64 ((__m128i *)(srcP - offset*0), nP0x4);
            _mm_storel_epi64 ((__m128i *)(srcQ + offset*0), nQ0x4);

            tc1x4 = _mm_srai_epi16 (tc1x4, 1);

            if (filter2ndP)
            {
                avgM1M3x4 = _mm_avg_epu16 (p2x4, p0x4);
                delta1x4  = _mm_sub_epi16 (deltax4, p1x4);
                delta1x4  = _mm_add_epi16 (delta1x4, avgM1M3x4);
                delta1x4  = _mm_srai_epi16 (delta1x4, 1);
                delta1x4  = _mm_min_epi16 (delta1x4, tc1x4);
                delta1x4  = _mm_max_epi16 (delta1x4, _mm_sub_epi16 (allZero, tc1x4));
                fltPMask  = _mm_cvtsi32_si128 (filter2ndP);
                fltPMask  = _mm_shuffle_epi8 (fltPMask, tcMapx16);
                fltPMask  = _mm_and_si128 (fltPMask, fltx4);

                nP1x4 = _mm_add_epi16 (p1x4, delta1x4);
                nP1x4 = _mm_blendv_epi8 (p1x4, nP1x4, fltPMask);
                nP1x4 = _mm_packus_epi16 (nP1x4, nP1x4);

                _mm_storel_epi64 ((__m128i *)(srcP - offset*1), nP1x4);

            }

            if (filter2ndQ)
            {
                avgM4M6x4 = _mm_avg_epu16 (q0x4, q2x4);
                delta2x4  = _mm_add_epi16 (deltax4, q1x4);
                delta2x4  = _mm_sub_epi16 (avgM4M6x4, delta2x4);
                delta2x4  = _mm_srai_epi16 (delta2x4, 1);
                delta2x4  = _mm_min_epi16 (delta2x4, tc1x4);
                delta2x4  = _mm_max_epi16 (delta2x4, _mm_sub_epi16 (allZero, tc1x4));
                fltQMask  = _mm_cvtsi32_si128 (filter2ndQ);
                fltQMask  = _mm_shuffle_epi8 (fltQMask, tcMapx16);
                fltQMask  = _mm_and_si128 (fltQMask, fltx4);

                nQ1x4 = _mm_add_epi16 (q1x4, delta2x4);
                nQ1x4 = _mm_blendv_epi8 (q1x4, nQ1x4, fltQMask);
                nQ1x4 = _mm_packus_epi16 (nQ1x4, nQ1x4);

                _mm_storel_epi64 ((__m128i *)(srcQ + offset*1), nQ1x4);
            }

        }

    }

}

void Xin266LumaLoopFilter_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta[2],
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ,
    BOOL     isVert)
{
    SINT32   dp00, dp30;
    SINT32   dp01, dp31;
    SINT32   dq00, dq30;
    SINT32   dq01, dq31;
    SINT32   d00, d30;
    SINT32   dp0, dq0;
    SINT32   d01, d31;
    SINT32   dp1, dq1;
    SINT32   dp00L, dq00L;
    SINT32   dp01L, dq01L;
    SINT32   dp30L, dq30L;
    SINT32   dp31L, dq31L;
    SINT32   d00L, d30L, d0L;
    SINT32   d01L, d31L, d1L;
    SINT32   d0, d1;
    intptr_t fltStride;
    intptr_t fltOffset;
    PIXEL    *fltPel;
    BOOL     strongFlt00;
    BOOL     strongFlt30;
    BOOL     fstStrongFlt0;
    BOOL     strongFlt01;
    BOOL     strongFlt31;
    BOOL     fstStrongFlt1;
    BOOL     sndStrongFlt0;
    BOOL     sndStrongFlt1;
    BOOL     sndFlt0, sndFlt1;
    SINT32   iTc0, iTc1;
    SINT32   iTc;
    SINT32   sideThr0, sideThr1;
    BOOL     filterP0, filterP1;
    BOOL     filterQ0, filterQ1;
    SINT32   filterP, filterQ;
    PIXEL    intermediateBuf[8*16];

    if (isVert)
    {
        Xin26xBlockTranspose_SSE2 (
            src - 4 - sidePisLarge*4,
            srcStride,
            intermediateBuf,
            8,
            8,
            8 + sidePisLarge*4 + sideQisLarge*4);

        fltOffset = 8;
        fltStride = 1;
        fltPel    = intermediateBuf + 4*8 + sidePisLarge*4*8;
    }
    else
    {
        fltOffset = srcStride;
        fltStride = 1;
        fltPel    = src;
    }

    iTc0 = tc[0];
    dp00 = calcDP (fltPel+fltStride*(0*4+0), fltOffset, FALSE);
    dq00 = calcDQ (fltPel+fltStride*(0*4+0), fltOffset);
    dp30 = calcDP (fltPel+fltStride*(0*4+3), fltOffset, FALSE);
    dq30 = calcDQ (fltPel+fltStride*(0*4+3), fltOffset);

    iTc1 = tc[1];
    dp01 = calcDP (fltPel+fltStride*(1*4+0), fltOffset, FALSE);
    dq01 = calcDQ (fltPel+fltStride*(1*4+0), fltOffset);
    dp31 = calcDP (fltPel+fltStride*(1*4+3), fltOffset, FALSE);
    dq31 = calcDQ (fltPel+fltStride*(1*4+3), fltOffset);

    fstStrongFlt0 = FALSE;
    fstStrongFlt1 = FALSE;
    sideThr0   = (beta[0] + (beta[0]>>1))>>3;
    sideThr1   = (beta[1] + (beta[1]>>1))>>3;

    if (sidePisLarge)
    {
        dp00L = (dp00 + calcDP (fltPel + fltStride*(0*4 + 0) - 3*fltOffset, fltOffset, FALSE) + 1) >> 1;
        dp30L = (dp30 + calcDP (fltPel + fltStride*(0*4 + 3) - 3*fltOffset, fltOffset, FALSE) + 1) >> 1;

        dp01L = (dp01 + calcDP (fltPel + fltStride*(1*4 + 0) - 3*fltOffset, fltOffset, FALSE) + 1) >> 1;
        dp31L = (dp31 + calcDP (fltPel + fltStride*(1*4 + 3) - 3*fltOffset, fltOffset, FALSE) + 1) >> 1;
    }
    else
    {
        dp00L = dp00;
        dp30L = dp30;

        dp01L = dp01;
        dp31L = dp31;
    }

    if (sideQisLarge)
    {
        dq00L = (dq00 + calcDQ (fltPel + fltStride*(0*4 + 0) + 3*fltOffset, fltOffset) + 1) >> 1;
        dq30L = (dq30 + calcDQ (fltPel + fltStride*(0*4 + 3) + 3*fltOffset, fltOffset) + 1) >> 1;

        dq01L = (dq01 + calcDQ (fltPel + fltStride*(1*4 + 0) + 3*fltOffset, fltOffset) + 1) >> 1;
        dq31L = (dq31 + calcDQ (fltPel + fltStride*(1*4 + 3) + 3*fltOffset, fltOffset) + 1) >> 1;
    }
    else
    {
        dq00L = dq00;
        dq30L = dq30;

        dq01L = dq01;
        dq31L = dq31;
    }

    if (sidePisLarge || sideQisLarge)
    {
        d00L = dp00L + dq00L;
        d30L = dp30L + dq30L;
        d0L  = d00L + d30L;


        d01L = dp01L + dq01L;
        d31L = dp31L + dq31L;
        d1L  = d01L + d31L;

        if ((d0L < beta[0]) || (d1L < beta[1]))
        {
            strongFlt00   = UseStrongFiltering (fltPel+fltStride*(0*4+0), fltOffset, 2*d00L, beta[0], iTc0, sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ, FALSE);
            strongFlt30   = UseStrongFiltering (fltPel+fltStride*(0*4+3), fltOffset, 2*d30L, beta[0], iTc0, sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ, FALSE);
            fstStrongFlt0 = strongFlt00 && strongFlt30 && (d0L < beta[0]);

            strongFlt01   = UseStrongFiltering (fltPel+fltStride*(1*4+0), fltOffset, 2*d01L, beta[1], iTc1, sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ, FALSE);
            strongFlt31   = UseStrongFiltering (fltPel+fltStride*(1*4+3), fltOffset, 2*d31L, beta[1], iTc1, sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ, FALSE);
            fstStrongFlt1 = strongFlt01 && strongFlt31 && (d1L < beta[1]);

            if (fstStrongFlt0 || fstStrongFlt1)
            {
                iTc = ((iTc1 * fstStrongFlt1) << 16) | (iTc0 * fstStrongFlt0);

                Xin266Filter1stLuma8_SSE4 (
                    fltPel+fltStride*0*4,
                    fltOffset,
                    iTc,
                    sidePisLarge,
                    sideQisLarge,
                    maxFilterLengthP,
                    maxFilterLengthQ);
            }

        }

    }

    if ((!fstStrongFlt0) || (!fstStrongFlt1))
    {
        d00 = dp00 + dq00;
        d30 = dp30 + dq30;
        dp0 = dp00 + dp30;
        dq0 = dq00 + dq30;
        d0  = d00 + d30;

        d01 = dp01 + dq01;
        d31 = dp31 + dq31;
        dp1 = dp01 + dp31;
        dq1 = dq01 + dq31;
        d1  = d01 + d31;

        if ((d0 < beta[0]) || (d1 < beta[1]))
        {
            if (maxFilterLengthP > 1 && maxFilterLengthQ > 1)
            {
                filterP0 = (dp0 < sideThr0);
                filterQ0 = (dq0 < sideThr0);

                filterP1 = (dp1 < sideThr1);
                filterQ1 = (dq1 < sideThr1);
            }
            else
            {
                filterP0 = FALSE;
                filterQ0 = FALSE;

                filterP1 = FALSE;
                filterQ1 = FALSE;
            }

            if (maxFilterLengthP > 2 && maxFilterLengthQ > 2)
            {
                strongFlt00   = UseStrongFiltering(fltPel + fltStride*(0 * 4 + 0), fltOffset, 2 * d00, beta[0], iTc0, FALSE, FALSE, 0, 0, FALSE);
                strongFlt30   = UseStrongFiltering(fltPel + fltStride*(0 * 4 + 3), fltOffset, 2 * d30, beta[0], iTc0, FALSE, FALSE, 0, 0, FALSE);
                sndStrongFlt0 = strongFlt00 && strongFlt30;

                strongFlt01   = UseStrongFiltering(fltPel + fltStride*(1 * 4 + 0), fltOffset, 2 * d01, beta[1], iTc1, FALSE, FALSE, 0, 0, FALSE);
                strongFlt31   = UseStrongFiltering(fltPel + fltStride*(1 * 4 + 3), fltOffset, 2 * d31, beta[1], iTc1, FALSE, FALSE, 0, 0, FALSE);
                sndStrongFlt1 = strongFlt01 && strongFlt31;
            }
            else
            {
                sndStrongFlt0 = FALSE;
                sndStrongFlt1 = FALSE;
            }

            sndFlt0 = (!fstStrongFlt0) && (d0 < beta[0]);
            sndFlt1 = (!fstStrongFlt1) && (d1 < beta[1]);

            if (sndStrongFlt0 || sndStrongFlt1)
            {
                iTc = ((iTc1 * (sndFlt1 && (sndStrongFlt1))) << 16) | (iTc0 * (sndFlt0 && (sndStrongFlt0)));

                Xin266Filter2ndLuma8_SSE4 (
                    fltPel+fltStride*0*4,
                    fltOffset,
                    iTc,
                    TRUE,
                    FALSE,
                    FALSE);
            }

            if (!sndStrongFlt0 || !sndStrongFlt1)
            {
                iTc = ((iTc1 * (sndFlt1 && (!sndStrongFlt1))) << 16) | (iTc0 * (sndFlt0 && (!sndStrongFlt0)));

                filterP = ((filterP1 * 0xFFFF) << 16) | (filterP0 * 0xFFFF);
                filterQ = ((filterQ1 * 0xFFFF) << 16) | (filterQ0 * 0xFFFF);

                Xin266Filter2ndLuma8_SSE4 (
                    fltPel+fltStride*0*4,
                    fltOffset,
                    iTc,
                    FALSE,
                    filterP,
                    filterQ);
            }

        }

    }

    if (isVert)
    {
        Xin26xBlockTranspose_SSE2 (
            intermediateBuf,
            8,
            src - 4 - sidePisLarge*4,
            srcStride,
            8 + sidePisLarge*4 + sideQisLarge*4,
            8);
    }

}

