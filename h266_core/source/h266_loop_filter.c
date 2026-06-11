/***************************************************************************//**
 *
 * @file          h266_loop_filter.c
 * @brief         h266 loop filter implementation.
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
#include "basic_macro.h"
#include "assert.h"

static const SINT32 tc3[3] =
{
    3, 2, 1
};

static const SINT32 tc7[7] =
{
    6, 5, 4, 3, 2, 1, 1
};

static const SINT32 tc3x2[3] =
{
    6, 4, 2
};

static const SINT32 dbCoeffs7[7] =
{
    59, 50, 41,32, 23, 14, 5
};

static const SINT32 dbCoeffs3[3] =
{
    53, 32, 11
};

static const SINT32 dbCoeffs5[5] =
{
    58, 45, 32,19, 6
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


static void BilinearFilter (
    PIXEL    *srcP,
    PIXEL    *srcQ,
    intptr_t offset,
    SINT32   refMiddle,
    SINT32   refP,
    SINT32   refQ,
    SINT32   numPSide,
    SINT32   numQSide,
    SINT32   *dbCoeffsP,
    SINT32   *dbCoeffsQ,
    SINT32   tc)
{
    const SINT32  *tcP;
    const SINT32  *tcQ;
    SINT32  src;
    SINT32  cvalue;
    SINT32  pos;

    tcP  = (numPSide == 3) ? tc3x2 : tc7;
    tcQ  = (numQSide == 3) ? tc3x2 : tc7;

    for (pos = 0; pos < numPSide; pos++)
    {
        src    = srcP[-offset*pos];
        cvalue = (tc*tcP[pos]) >> 1;

        srcP[-offset*pos] = (PIXEL)XIN_CLIP(((refMiddle*dbCoeffsP[pos] + refP * (64 - dbCoeffsP[pos]) + 32) >> 6), src - cvalue, src + cvalue);
    }

    for (pos = 0; pos < numQSide; pos++)
    {
        src    = srcQ[offset*pos];
        cvalue = (tc*tcQ[pos]) >> 1;

        srcQ[offset*pos] = (PIXEL)XIN_CLIP(((refMiddle*dbCoeffsQ[pos] + refQ * (64 - dbCoeffsQ[pos]) + 32) >> 6), src - cvalue, src + cvalue);
    }

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

static void FilteringPandQ (
    PIXEL   *src,
    intptr_t offset,
    SINT32   numPSide,
    SINT32   numQSide,
    SINT32   tc)
{
    const SINT32   *dbCoeffsP;
    const SINT32   *dbCoeffsQ;
    PIXEL    *srcP;
    PIXEL    *srcQ;
    PIXEL    *srcPt;
    PIXEL    *srcQt;
    SINT32   refP;
    SINT32   refQ;
    SINT32   refMiddle;
    SINT32   newNumQSide;
    SINT32   newNumPSide;
    intptr_t offsetP;
    intptr_t offsetQ;

    refP      = 0;
    refQ      = 0;
    refMiddle = 0;
    srcP      = src-offset;
    srcQ      = src;
    dbCoeffsP = numPSide == 7 ? dbCoeffs7 : (numPSide==5) ? dbCoeffs5 : dbCoeffs3;
    dbCoeffsQ = numQSide == 7 ? dbCoeffs7 : (numQSide==5) ? dbCoeffs5 : dbCoeffs3;

    switch (numPSide)
    {
    case 7:
        refP = (srcP[-6*offset]   + srcP[-7 * offset] + 1) >> 1;
        break;
    case 3:
        refP = (srcP[-2 * offset] + srcP[-3 * offset] + 1) >> 1;
        break;
    case 5:
        refP = (srcP[-4 * offset] + srcP[-5 * offset] + 1) >> 1;
        break;
    }

    switch (numQSide)
    {
    case 7:
        refQ = (srcQ[6 * offset] + srcQ[7 * offset] + 1) >> 1;
        break;
    case 3:
        refQ = (srcQ[2 * offset] + srcQ[3 * offset] + 1) >> 1;
        break;
    case 5:
        refQ = (srcQ[4 * offset] + srcQ[5 * offset] + 1) >> 1;
        break;
    }

    if (numPSide == numQSide)
    {
        if (numPSide == 5)
        {
            refMiddle = (2 * (srcP[0] + srcQ[0] + srcP[-offset] + srcQ[offset] + srcP[-2 * offset] + srcQ[2 * offset]) + srcP[-3 * offset] + srcQ[3 * offset] + srcP[-4 * offset] + srcQ[4 * offset] + 8) >> 4;
        }
        else
        {
            refMiddle = (2 * (srcP[0] + srcQ[0]) + srcP[-offset] + srcQ[offset] + srcP[-2 * offset] + srcQ[2 * offset] + srcP[-3 * offset] + srcQ[3 * offset] + srcP[-4 * offset] + srcQ[4 * offset] + srcP[-5 * offset] + srcQ[5 * offset] + +srcP[-6 * offset] + srcQ[6 * offset] + 8) >> 4;
        }
    }
    else
    {
        srcPt   = srcP;
        srcQt   = srcQ;
        offsetP = -offset;
        offsetQ = offset;

        newNumQSide = numQSide;
        newNumPSide = numPSide;

        if (numQSide > numPSide)
        {
            XIN_SWAP (PIXEL*, srcPt, srcQt);
            XIN_SWAP (intptr_t, offsetP, offsetQ);

            newNumQSide = numPSide;
            newNumPSide = numQSide;
        }

        if (newNumPSide == 7 && newNumQSide == 5)
        {
            refMiddle = (2 * (srcP[0] + srcQ[0] + srcP[-offset] + srcQ[offset]) + srcP[-2 * offset] + srcQ[2 * offset] + srcP[-3 * offset] + srcQ[3 * offset] + srcP[-4 * offset] + srcQ[4 * offset] + srcP[-5 * offset] + srcQ[5 * offset] + 8) >> 4;
        }
        else if (newNumPSide == 7 && newNumQSide == 3)
        {
            refMiddle = (2 * (srcPt[0] + srcQt[0]) + srcQt[0] + 2 * (srcQt[offsetQ] + srcQt[2 * offsetQ]) + srcPt[offsetP] + srcQt[offsetQ] + srcPt[2 * offsetP] + srcPt[3 * offsetP] + srcPt[4 * offsetP] + srcPt[5 * offsetP] + srcPt[6 * offsetP] + 8) >> 4;
        }
        else
        {
            refMiddle = (srcP[0] + srcQ[0] + srcP[-offset] + srcQ[offset] + srcP[-2 * offset] + srcQ[2 * offset] + srcP[-3 * offset] + srcQ[3 * offset] + 4) >> 3;
        }

    }

    BilinearFilter (
        srcP,
        srcQ,
        offset,
        refMiddle,
        refP,
        refQ,
        numPSide,
        numQSide,
        (SINT32 *)dbCoeffsP,
        (SINT32 *)dbCoeffsQ,
        tc);

}

static void PelFilterStrongLuma (
    PIXEL    *src,
    intptr_t offset,
    SINT32   tc,
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ)
{
    PIXEL  m0, m1, m2, m3;
    PIXEL  m4, m5, m6, m7;

    m4  = src[0];
    m3  = src[-offset];
    m5  = src[ offset];
    m2  = src[-offset*2];
    m6  = src[ offset*2];
    m1  = src[-offset*3];
    m7  = src[ offset*3];
    m0  = src[-offset*4];

    if (sidePisLarge || sideQisLarge)
    {
        FilteringPandQ (
            src,
            offset,
            sidePisLarge ? maxFilterLengthP : 3,
            sideQisLarge ? maxFilterLengthQ : 3,
            tc);
    }
    else
    {
        src[-offset]   = (PIXEL)XIN_CLIP(((m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3), m3-tc*tc3[0], m3+tc*tc3[0]);
        src[0]         = (PIXEL)XIN_CLIP(((m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3), m4-tc*tc3[0], m4+tc*tc3[0]);
        src[-offset*2] = (PIXEL)XIN_CLIP(((m1 + m2 + m3 + m4 + 2)>>2), m2-tc*tc3[1], m2+tc*tc3[1]);
        src[ offset]   = (PIXEL)XIN_CLIP(((m3 + m4 + m5 + m6 + 2)>>2), m5-tc*tc3[1], m5+tc*tc3[1]);
        src[-offset*3] = (PIXEL)XIN_CLIP(((2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3), m1-tc*tc3[2], m1+tc*tc3[2]);
        src[ offset*2] = (PIXEL)XIN_CLIP(((m3 + m4 + m5 + 3*m6 + 2*m7 + 4 )>>3), m6-tc*tc3[2], m6+tc*tc3[2]);
    }

}

static void PelFilterWeakLuma (
    PIXEL    *src,
    intptr_t offset,
    SINT32   tc,
    SINT32   thrCut,
    BOOL     filter2ndP,
    BOOL     filter2ndQ)
{
    SINT32 delta;
    SINT32 delta1;
    SINT32 delta2;
    SINT32 tc2;
    PIXEL  m1, m2, m3;
    PIXEL  m4, m5, m6;

    m4  = src[0];
    m3  = src[-offset];
    m5  = src[ offset];
    m2  = src[-offset*2];
    m6  = src[ offset*2];
    m1  = src[-offset*3];

    /* Weak filter */
    delta = (9*(m4-m3) -3*(m5-m2) + 8)>>4 ;

    if (XIN_ABS(delta) < thrCut)
    {
        delta = XIN_CLIP(delta, -tc, tc);

        src[-offset] = (PIXEL)XIN_CLIP((m3+delta), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        src[0]       = (PIXEL)XIN_CLIP((m4-delta), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);

        tc2 = tc>>1;

        if (filter2ndP)
        {
            delta1 = XIN_CLIP((( ((m1+m3+1)>>1)- m2+delta)>>1), -tc2, tc2);

            src[-offset*2] = (PIXEL)XIN_CLIP((m2+delta1), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

        if (filter2ndQ)
        {
            delta2 = XIN_CLIP((( ((m6+m4+1)>>1)- m5-delta)>>1), -tc2, tc2);

            src[offset] = (PIXEL)XIN_CLIP((m5+delta2), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

    }

}


void Xin266LumaLoopFilter (
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
    UINT32   blockIdx;
    UINT32   idx;
    SINT32   dp0, dp3;
    SINT32   dq0, dq3;
    SINT32   d0, d3;
    SINT32   dp, dq;
    SINT32   dp0L, dq0L;
    SINT32   dp3L, dq3L;
    SINT32   d0L, d3L, dpL, dqL, dL;
    SINT32   d;
    intptr_t fltStride;
    intptr_t fltOffset;
    PIXEL    *fltPel;
    BOOL     strongFlt0;
    BOOL     strongFlt3;
    BOOL     strongFlt;
    SINT32   iTc;
    SINT32   sideThr;
    BOOL     filterP;
    BOOL     filterQ;

    if (isVert)
    {
        fltOffset = 1;
        fltStride = srcStride;
    }
    else
    {
        fltOffset = srcStride;
        fltStride = 1;
    }

    fltPel = src;

    for (blockIdx = 0; blockIdx < 2; blockIdx++)
    {
        iTc = tc[blockIdx];
        dp0 = calcDP (fltPel+fltStride*(blockIdx*4+0), fltOffset, FALSE);
        dq0 = calcDQ (fltPel+fltStride*(blockIdx*4+0), fltOffset);
        dp3 = calcDP (fltPel+fltStride*(blockIdx*4+3), fltOffset, FALSE);
        dq3 = calcDQ (fltPel+fltStride*(blockIdx*4+3), fltOffset);

        strongFlt = FALSE;
        sideThr   = (beta[blockIdx] + (beta[blockIdx]>>1))>>3;

        if (sidePisLarge)
        {
            dp0L = (dp0 + calcDP (fltPel + fltStride*(blockIdx*4 + 0) - 3*fltOffset, fltOffset, FALSE) + 1) >> 1;
            dp3L = (dp3 + calcDP (fltPel + fltStride*(blockIdx*4 + 3) - 3*fltOffset, fltOffset, FALSE) + 1) >> 1;
        }
        else
        {
            dp0L = dp0;
            dp3L = dp3;
        }

        if (sideQisLarge)
        {
            dq0L = (dq0 + calcDQ (fltPel + fltStride*(blockIdx*4 + 0) + 3*fltOffset, fltOffset) + 1) >> 1;
            dq3L = (dq3 + calcDQ (fltPel + fltStride*(blockIdx*4 + 3) + 3*fltOffset, fltOffset) + 1) >> 1;
        }
        else
        {
            dq0L = dq0;
            dq3L = dq3;
        }

        if (sidePisLarge || sideQisLarge)
        {
            d0L = dp0L + dq0L;
            d3L = dp3L + dq3L;
            dpL = dp0L + dp3L;
            dqL = dq0L + dq3L;
            dL  = d0L + d3L;

            if (dL < beta[blockIdx])
            {
                filterP = (dpL < sideThr);
                filterQ = (dqL < sideThr);

                strongFlt0 = UseStrongFiltering (fltPel+fltStride*(blockIdx*4+0), fltOffset, 2*d0L, beta[blockIdx], iTc, sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ, FALSE);
                strongFlt3 = UseStrongFiltering (fltPel+fltStride*(blockIdx*4+3), fltOffset, 2*d3L, beta[blockIdx], iTc, sidePisLarge, sideQisLarge, maxFilterLengthP, maxFilterLengthQ, FALSE);
                strongFlt  = strongFlt0 && strongFlt3;

                if (strongFlt)
                {
                    for (idx = 0; idx < 4; idx++)
                    {
                        PelFilterStrongLuma (
                            fltPel+fltStride*(blockIdx*4+idx),
                            fltOffset,
                            iTc,
                            sidePisLarge,
                            sideQisLarge,
                            maxFilterLengthP,
                            maxFilterLengthQ);
                    }
                }

            }

        }

        if (!strongFlt)
        {
            d0 = dp0 + dq0;
            d3 = dp3 + dq3;
            dp = dp0 + dp3;
            dq = dq0 + dq3;
            d  =  d0 + d3;

            if (d < beta[blockIdx])
            {
                if (maxFilterLengthP > 1 && maxFilterLengthQ > 1)
                {
                    filterP = (dp < sideThr);
                    filterQ = (dq < sideThr);
                }
                else
                {
                    filterP = FALSE;
                    filterQ = FALSE;
                }

                if (maxFilterLengthP > 2 && maxFilterLengthQ > 2)
                {
                    strongFlt0 = UseStrongFiltering(fltPel + fltStride*(blockIdx * 4 + 0), fltOffset, 2 * d0, beta[blockIdx], iTc, FALSE, FALSE, 0, 0, FALSE);
                    strongFlt3 = UseStrongFiltering(fltPel + fltStride*(blockIdx * 4 + 3), fltOffset, 2 * d3, beta[blockIdx], iTc, FALSE, FALSE, 0, 0, FALSE);
                    strongFlt  = strongFlt0 && strongFlt3;
                }
                else
                {
                    strongFlt = FALSE;
                }

                if (strongFlt)
                {
                    for (idx = 0; idx < 4; idx++)
                    {
                        PelFilterStrongLuma (
                            fltPel+fltStride*(blockIdx*4+idx),
                            fltOffset,
                            iTc,
                            FALSE,
                            FALSE,
                            0,
                            0);
                    }
                }
                else
                {
                    for (idx = 0; idx < 4; idx++)
                    {
                        PelFilterWeakLuma (
                            fltPel+fltStride*(blockIdx*4+idx),
                            fltOffset,
                            iTc,
                            iTc*10,
                            filterP,
                            filterQ);
                    }
                }

            }

        }

    }

}

static void PelFilterChroma (
    PIXEL    *src,
    intptr_t offset,
    SINT32   tc,
    BOOL     strong,
    BOOL     isChromaHorCtuBoundary)
{
    SINT32 delta;
    PIXEL  m0, m1;
    PIXEL  m2, m3;
    PIXEL  m4, m5;
    PIXEL  m6, m7;

    m0  = src[-offset*4];
    m1  = src[-offset*3];
    m2  = src[-offset*2];
    m3  = src[-offset];
    m4  = src[0];
    m5  = src[ offset];
    m6  = src[ offset*2];
    m7  = src[ offset*3];

    if (strong)
    {
        if (isChromaHorCtuBoundary)
        {
            src[-offset*1] = (PIXEL)XIN_CLIP(((3 * m2 + 2 * m3 + m4 + m5 + m6 + 4) >> 3), m3 - tc, m3 + tc);       // p0
            src[0]         = (PIXEL)XIN_CLIP(((2 * m2 + m3 + 2 * m4 + m5 + m6 + m7 + 4) >> 3), m4 - tc, m4 + tc);  // q0
            src[offset*1]  = (PIXEL)XIN_CLIP(((m2 + m3 + m4 + 2 * m5 + m6 + 2 * m7 + 4) >> 3), m5 - tc, m5 + tc);  // q1
            src[offset*2]  = (PIXEL)XIN_CLIP(((m3 + m4 + m5 + 2 * m6 + 3 * m7 + 4) >> 3), m6 - tc, m6 + tc);       // q2
        }
        else
        {
            src[-offset*3] = (PIXEL)XIN_CLIP(((3 * m0 + 2 * m1 + m2 + m3 + m4 + 4) >> 3), m1 - tc, m1 + tc);       // p2
            src[-offset*2] = (PIXEL)XIN_CLIP(((2 * m0 + m1 + 2 * m2 + m3 + m4 + m5 + 4) >> 3), m2 - tc, m2 + tc);  // p1
            src[-offset*1] = (PIXEL)XIN_CLIP(((m0 + m1 + m2 + 2 * m3 + m4 + m5 + m6 + 4) >> 3), m3 - tc, m3 + tc); // p0
            src[0]         = (PIXEL)XIN_CLIP(((m1 + m2 + m3 + 2 * m4 + m5 + m6 + m7 + 4) >> 3), m4 - tc, m4 + tc); // q0
            src[offset*1]  = (PIXEL)XIN_CLIP(((m2 + m3 + m4 + 2 * m5 + m6 + 2 * m7 + 4) >> 3), m5 - tc, m5 + tc);  // q1
            src[offset*2]  = (PIXEL)XIN_CLIP(((m3 + m4 + m5 + 2 * m6 + 3 * m7 + 4) >> 3), m6 - tc, m6 + tc);       // q2
        }
    }
    else
    {
        delta = XIN_CLIP((((( m4 - m3 ) << 2 ) + m2 - m5 + 4 ) >> 3), -tc, tc);

        src[-offset] = (PIXEL)XIN_CLIP((m3 + delta), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        src[0]       = (PIXEL)XIN_CLIP((m4 - delta), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
    }

}

void Xin266ChromaLoopFilter (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta[2],
    BOOL     largeBoundary,
    BOOL     isChromaHorCtuBoundary,
    BOOL     isVert)
{
    intptr_t fltStride;
    intptr_t fltOffset;
    PIXEL    *fltPel;
    UINT32   idx;
    UINT32   blockIdx;
    SINT32   dp0, dq0;
    SINT32   dp3, dq3;
    SINT32   d0, d3, d;
    BOOL     strongFlt0;
    BOOL     strongFlt3;
    BOOL     strongFlt;

    if (isVert)
    {
        fltOffset  = 1;
        fltStride  = srcStride;

    }
    else
    {
        fltOffset  = srcStride;
        fltStride  = 1;
    }

    fltPel = src;

    if (largeBoundary)
    {
        for (blockIdx = 0; blockIdx < 2; blockIdx++)
        {
            dp0 = calcDP (fltPel+fltStride*(blockIdx*2+0), fltOffset, isChromaHorCtuBoundary);
            dq0 = calcDQ (fltPel+fltStride*(blockIdx*2+0), fltOffset);
            dp3 = calcDP (fltPel+fltStride*(blockIdx*2+1), fltOffset, isChromaHorCtuBoundary);
            dq3 = calcDQ (fltPel+fltStride*(blockIdx*2+1), fltOffset);

            d0 = dp0 + dq0;
            d3 = dp3 + dq3;
            d  = d0 + d3;

            if (d < beta[blockIdx])
            {
                strongFlt0 = UseStrongFiltering (fltPel + fltStride*(blockIdx * 2 + 0), fltOffset, 2 * d0, beta[blockIdx], tc[blockIdx], FALSE, FALSE, 7, 7, isChromaHorCtuBoundary);
                strongFlt3 = UseStrongFiltering (fltPel + fltStride*(blockIdx * 2 + 1), fltOffset, 2 * d3, beta[blockIdx], tc[blockIdx], FALSE, FALSE, 7, 7, isChromaHorCtuBoundary);
                strongFlt  = strongFlt0 && strongFlt3;

                for (idx = 0; idx < 2; idx++)
                {
                    PelFilterChroma (
                        fltPel + fltStride*(blockIdx*2+idx),
                        fltOffset,
                        tc[blockIdx],
                        strongFlt,
                        isChromaHorCtuBoundary);
                }
            }
            else
            {
                for (idx = 0; idx < 2; idx++)
                {
                    PelFilterChroma (
                        fltPel + fltStride*(blockIdx*2+idx),
                        fltOffset,
                        tc[blockIdx],
                        FALSE,
                        isChromaHorCtuBoundary);
                }
            }

        }

    }
    else
    {
        for (idx = 0; idx < 4; idx++)
        {
            PelFilterChroma (
                fltPel + fltStride*idx,
                fltOffset,
                idx < 2 ? tc[0] : tc[1],
                FALSE,
                isChromaHorCtuBoundary);
        }
    }

}

