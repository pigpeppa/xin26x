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

/* Gather decision samples 0, 3, 4 and 7 into the low four words, mirrored above. */
static const UINT8 dbPickBytes[16] =
{
    0, 1, 6, 7, 8, 9, 14, 15,
    0, 1, 6, 7, 8, 9, 14, 15
};

static inline __m128i dbLoad8 (
    const PIXEL *src)
{
    return _mm_cvtepu8_epi16 (_mm_loadl_epi64 ((const __m128i *)src));
}

/* abs(a - 2*b + c). Each input is an 8-bit pixel widened to i16. */
static inline __m128i dbLap (
    __m128i a,
    __m128i b,
    __m128i c)
{
    return _mm_abs_epi16 (_mm_sub_epi16 (_mm_add_epi16 (a, c), _mm_add_epi16 (b, b)));
}

static inline __m128i dbPick (
    __m128i v)
{
    return _mm_shuffle_epi8 (v, _mm_loadu_si128 ((const __m128i *)dbPickBytes));
}

/* beta/tc for the four decision samples: block0, block0, block1, block1. */
static inline __m128i dbPairValue (
    SINT32 v0,
    SINT32 v1)
{
    return _mm_set_epi16 (
        (SINT16)v1, (SINT16)v1, (SINT16)v0, (SINT16)v0,
        (SINT16)v1, (SINT16)v1, (SINT16)v0, (SINT16)v0);
}

static inline void dbPairSum (
    __m128i v,
    SINT32  *sum0,
    SINT32  *sum1)
{
    __m128i p = dbPick (v);
    __m128i h = _mm_hadd_epi16 (p, p);

    *sum0 = (SINT32)_mm_extract_epi16 (h, 0);
    *sum1 = (SINT32)_mm_extract_epi16 (h, 1);
}

static inline void dbPairAnd (
    __m128i mask,
    BOOL    *block0,
    BOOL    *block1)
{
    SINT32 bits = _mm_movemask_epi8 (mask);

    *block0 = (bits & 0x0F) == 0x0F;
    *block1 = (bits & 0xF0) == 0xF0;
}

/*
 * Strong-filter predicate on the four decision samples.
 * d4 is per-sample (dp + dq), already gathered. The scalar check compares 2*d.
 */
static inline __m128i dbStrongMask (
    __m128i p0,
    __m128i p3,
    __m128i q0,
    __m128i q3,
    __m128i p4,
    __m128i p5,
    __m128i p6,
    __m128i p7,
    __m128i q4,
    __m128i q5,
    __m128i q6,
    __m128i q7,
    __m128i d4,
    __m128i beta,
    __m128i tc,
    BOOL    sidePisLarge,
    BOOL    sideQisLarge,
    SINT32  maxFilterLengthP,
    SINT32  maxFilterLengthQ)
{
    __m128i one = _mm_set1_epi16 (1);
    __m128i sp  = _mm_abs_epi16 (_mm_sub_epi16 (p3, p0));
    __m128i sq  = _mm_abs_epi16 (_mm_sub_epi16 (q3, q0));
    __m128i thrHi;
    __m128i thrD;
    __m128i sum;
    __m128i twoD;
    __m128i pq;
    __m128i tcThr;
    __m128i mask;

    if (sidePisLarge || sideQisLarge)
    {
        if (sidePisLarge)
        {
            __m128i ref = (maxFilterLengthP == 7) ? p7 : p5;

            if (maxFilterLengthP == 7)
            {
                __m128i acc = _mm_sub_epi16 (_mm_add_epi16 (p4, p7), _mm_add_epi16 (p5, p6));

                sp = _mm_add_epi16 (sp, _mm_abs_epi16 (acc));
            }

            sp = _mm_add_epi16 (sp, _mm_abs_epi16 (_mm_sub_epi16 (p3, ref)));
            sp = _mm_srai_epi16 (_mm_add_epi16 (sp, one), 1);
        }

        if (sideQisLarge)
        {
            __m128i ref = (maxFilterLengthQ == 7) ? q7 : q5;

            if (maxFilterLengthQ == 7)
            {
                __m128i acc = _mm_sub_epi16 (_mm_add_epi16 (q4, q7), _mm_add_epi16 (q5, q6));

                sq = _mm_add_epi16 (sq, _mm_abs_epi16 (acc));
            }

            sq = _mm_add_epi16 (sq, _mm_abs_epi16 (_mm_sub_epi16 (ref, q3)));
            sq = _mm_srai_epi16 (_mm_add_epi16 (sq, one), 1);
        }

        thrHi = _mm_srai_epi16 (_mm_mullo_epi16 (beta, _mm_set1_epi16 (3)), 5);
        thrD  = _mm_srai_epi16 (beta, 4);
    }
    else
    {
        thrHi = _mm_srai_epi16 (beta, 3);
        thrD  = _mm_srai_epi16 (beta, 2);
    }

    sum   = _mm_add_epi16 (sp, sq);
    twoD  = _mm_add_epi16 (d4, d4);
    pq    = _mm_abs_epi16 (_mm_sub_epi16 (p0, q0));
    tcThr = _mm_srai_epi16 (_mm_add_epi16 (_mm_mullo_epi16 (tc, _mm_set1_epi16 (5)), one), 1);
    mask  = _mm_and_si128 (_mm_cmpgt_epi16 (thrHi, sum), _mm_cmpgt_epi16 (thrD, twoD));
    mask  = _mm_and_si128 (mask, _mm_cmpgt_epi16 (tcThr, pq));

    return mask;
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
    intptr_t fltOffset;
    PIXEL    *fltPel;
    PIXEL    *rowP;
    BOOL     fstStrongFlt0;
    BOOL     fstStrongFlt1;
    BOOL     sndStrongFlt0;
    BOOL     sndStrongFlt1;
    BOOL     sndFlt0, sndFlt1;
    BOOL     filterP0, filterP1;
    BOOL     filterQ0, filterQ1;
    SINT32   iTc0, iTc1;
    SINT32   iTc;
    SINT32   sideThr0, sideThr1;
    SINT32   dp0, dp1, dq0, dq1;
    SINT32   d0L, d1L, d0, d1;
    SINT32   filterP, filterQ;
    __m128i  one;
    __m128i  p0, p1, p2, p3, q0, q1, q2, q3;
    __m128i  p4, p5, p6, p7, q4, q5, q6, q7;
    __m128i  dp, dq, dpL, dqL;
    __m128i  beta4, tc4;
    __m128i  zero;
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
        fltPel    = intermediateBuf + 4*8 + sidePisLarge*4*8;
    }
    else
    {
        fltOffset = srcStride;
        fltPel    = src;
    }

    rowP = fltPel - fltOffset;
    p0   = dbLoad8 (rowP);
    p1   = dbLoad8 (rowP - fltOffset);
    p2   = dbLoad8 (rowP - fltOffset*2);
    p3   = dbLoad8 (rowP - fltOffset*3);
    q0   = dbLoad8 (fltPel);
    q1   = dbLoad8 (fltPel + fltOffset);
    q2   = dbLoad8 (fltPel + fltOffset*2);
    q3   = dbLoad8 (fltPel + fltOffset*3);

    one  = _mm_set1_epi16 (1);
    zero = _mm_setzero_si128 ();
    dp   = dbLap (p2, p1, p0);
    dq   = dbLap (q0, q1, q2);
    dpL  = dp;
    dqL  = dq;
    p4   = zero;
    p5   = zero;
    p6   = zero;
    p7   = zero;
    q4   = zero;
    q5   = zero;
    q6   = zero;
    q7   = zero;

    if (sidePisLarge)
    {
        p4  = dbLoad8 (rowP - fltOffset*4);
        p5  = dbLoad8 (rowP - fltOffset*5);
        dpL = _mm_srai_epi16 (_mm_add_epi16 (_mm_add_epi16 (dp, dbLap (p5, p4, p3)), one), 1);

        if (maxFilterLengthP == 7)
        {
            p6 = dbLoad8 (rowP - fltOffset*6);
            p7 = dbLoad8 (rowP - fltOffset*7);
        }
    }

    if (sideQisLarge)
    {
        q4  = dbLoad8 (fltPel + fltOffset*4);
        q5  = dbLoad8 (fltPel + fltOffset*5);
        dqL = _mm_srai_epi16 (_mm_add_epi16 (_mm_add_epi16 (dq, dbLap (q3, q4, q5)), one), 1);

        if (maxFilterLengthQ == 7)
        {
            q6 = dbLoad8 (fltPel + fltOffset*6);
            q7 = dbLoad8 (fltPel + fltOffset*7);
        }
    }

    dbPairSum (_mm_add_epi16 (dpL, dqL), &d0L, &d1L);
    dbPairSum (_mm_add_epi16 (dp, dq), &d0, &d1);
    dbPairSum (dp, &dp0, &dp1);
    dbPairSum (dq, &dq0, &dq1);

    iTc0         = tc[0];
    iTc1         = tc[1];
    beta4        = dbPairValue (beta[0], beta[1]);
    tc4          = dbPairValue (iTc0, iTc1);
    sideThr0     = (beta[0] + (beta[0]>>1))>>3;
    sideThr1     = (beta[1] + (beta[1]>>1))>>3;
    fstStrongFlt0 = FALSE;
    fstStrongFlt1 = FALSE;

    if (sidePisLarge || sideQisLarge)
    {
        if ((d0L < beta[0]) || (d1L < beta[1]))
        {
            __m128i d4 = _mm_add_epi16 (dbPick (dpL), dbPick (dqL));
            __m128i strong;

            strong = dbStrongMask (
                dbPick (p0), dbPick (p3), dbPick (q0), dbPick (q3),
                dbPick (p4), dbPick (p5), dbPick (p6), dbPick (p7),
                dbPick (q4), dbPick (q5), dbPick (q6), dbPick (q7),
                d4, beta4, tc4,
                sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ);

            dbPairAnd (strong, &fstStrongFlt0, &fstStrongFlt1);
            fstStrongFlt0 = fstStrongFlt0 && (d0L < beta[0]);
            fstStrongFlt1 = fstStrongFlt1 && (d1L < beta[1]);

            if (fstStrongFlt0 || fstStrongFlt1)
            {
                iTc = ((iTc1 * fstStrongFlt1) << 16) | (iTc0 * fstStrongFlt0);

                Xin266Filter1stLuma8_SSE4 (
                    fltPel,
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
                __m128i d4 = _mm_add_epi16 (dbPick (dp), dbPick (dq));
                __m128i strong;

                strong = dbStrongMask (
                    dbPick (p0), dbPick (p3), dbPick (q0), dbPick (q3),
                    zero, zero, zero, zero,
                    zero, zero, zero, zero,
                    d4, beta4, tc4,
                    FALSE, FALSE, 0, 0);

                dbPairAnd (strong, &sndStrongFlt0, &sndStrongFlt1);
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
                    fltPel,
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
                    fltPel,
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

