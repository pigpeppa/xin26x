/***************************************************************************//**
 *
 * @file          h265p_loop_filter.c
 * @brief         av1 loop filter implementation.
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
#include "h26x_definition.h"
#include "basic_macro.h"

#define XIN_S8_CLIP(X) XIN_CLIP(X, XIN_MIN_S8, XIN_MAX_S8)

// should we apply any filter at all: 11111111 yes, 00000000 no
static inline void Xin265pFilterMask2 (
    UINT8 limit,
    UINT8 blimit,
    UINT8 p1,
    UINT8 p0,
    UINT8 q0,
    UINT8 q1,
    SINT8 *mask)
{
    SINT8 fltMask;

    fltMask  = 0;
    fltMask |= (XIN_ABS (p1 - p0) > limit) * -1;
    fltMask |= (XIN_ABS (q1 - q0) > limit) * -1;
    fltMask |= (XIN_ABS (p0 - q0) * 2 + XIN_ABS (p1 - q1) / 2 > blimit) * -1;

    *mask = ~fltMask;
}

// should we apply any filter at all: 11111111 yes, 00000000 no
static inline void Xin265pFilterMask3 (
    UINT8 limit,
    UINT8 blimit,
    UINT8 p2,
    UINT8 p1,
    UINT8 p0,
    UINT8 q0,
    UINT8 q1,
    UINT8 q2,
    SINT8 *mask)
{
    SINT8 fltMask;

    fltMask  = 0;
    fltMask |= (XIN_ABS (p2 - p1) > limit) * -1;
    fltMask |= (XIN_ABS (p1 - p0) > limit) * -1;
    fltMask |= (XIN_ABS (q1 - q0) > limit) * -1;
    fltMask |= (XIN_ABS (q2 - q1) > limit) * -1;
    fltMask |= (XIN_ABS (p0 - q0) * 2 + XIN_ABS (p1 - q1) / 2 > blimit) * -1;

    *mask = ~fltMask;

}

static inline void Xin265pFilterMask4 (
    UINT8 limit,
    UINT8 blimit,
    UINT8 p3,
    UINT8 p2,
    UINT8 p1,
    UINT8 p0,
    UINT8 q0,
    UINT8 q1,
    UINT8 q2,
    UINT8 q3,
    SINT8 *mask)
{
    SINT8 fltMask;

    fltMask  = 0;
    fltMask |= (XIN_ABS (p3 - p2) > limit) * -1;
    fltMask |= (XIN_ABS (p2 - p1) > limit) * -1;
    fltMask |= (XIN_ABS (p1 - p0) > limit) * -1;
    fltMask |= (XIN_ABS (q1 - q0) > limit) * -1;
    fltMask |= (XIN_ABS (q2 - q1) > limit) * -1;
    fltMask |= (XIN_ABS (q3 - q2) > limit) * -1;
    fltMask |= (XIN_ABS (p0 - q0) * 2 + XIN_ABS (p1 - q1) / 2 > blimit) * -1;

    *mask = ~fltMask;

}


static inline void Xin265pFlatMask3 (
    UINT8 thresh,
    UINT8 p2,
    UINT8 p1,
    UINT8 p0,
    UINT8 q0,
    UINT8 q1,
    UINT8 q2,
    SINT8 *mask)
{
    SINT8 flatMask;

    flatMask  = 0;
    flatMask |= (XIN_ABS (p1 - p0) > thresh) * -1;
    flatMask |= (XIN_ABS (q1 - q0) > thresh) * -1;
    flatMask |= (XIN_ABS (p2 - p0) > thresh) * -1;
    flatMask |= (XIN_ABS (q2 - q0) > thresh) * -1;

    *mask = ~flatMask;

}

static inline void Xin265pFlatMask4 (
    UINT8 thresh,
    UINT8 p3,
    UINT8 p2,
    UINT8 p1,
    UINT8 p0,
    UINT8 q0,
    UINT8 q1,
    UINT8 q2,
    UINT8 q3,
    SINT8 *mask)
{
    SINT8 flatMask;

    flatMask  = 0;
    flatMask |= (XIN_ABS (p1 - p0) > thresh) * -1;
    flatMask |= (XIN_ABS (q1 - q0) > thresh) * -1;
    flatMask |= (XIN_ABS (p2 - p0) > thresh) * -1;
    flatMask |= (XIN_ABS (q2 - q0) > thresh) * -1;
    flatMask |= (XIN_ABS (p3 - p0) > thresh) * -1;
    flatMask |= (XIN_ABS (q3 - q0) > thresh) * -1;

    *mask = ~flatMask;

}

// is there high edge variance internal edge: 11111111 yes, 00000000 no
static inline SINT8 Xin265pHevMask (
    UINT8 thresh,
    UINT8 p1,
    UINT8 p0,
    UINT8 q0,
    UINT8 q1)
{
    UINT8 hev;

    hev  = 0;
    hev |= (XIN_ABS (p1 - p0) > thresh) * -1;
    hev |= (XIN_ABS (q1 - q0) > thresh) * -1;

    return hev;
}


static inline void Xin265pFilter4 (
    SINT8 fltMask,
    UINT8 thresh,
    PIXEL *op1,
    PIXEL *op0,
    PIXEL *oq0,
    PIXEL *oq1)
{
    SINT8   filter1, filter2;
    SINT8   ps0, ps1;
    SINT8   qs0, qs1;
    SINT32  hev;
    SINT8   filter;

    ps1 = (SINT8)(*op1 ^ 0x80);
    ps0 = (SINT8)(*op0 ^ 0x80);
    qs0 = (SINT8)(*oq0 ^ 0x80);
    qs1 = (SINT8)(*oq1 ^ 0x80);
    hev = Xin265pHevMask (thresh, *op1, *op0, *oq0, *oq1);

    // add outer taps if we have high edge variance
    filter = XIN_S8_CLIP (ps1 - qs1) & hev;

    // inner taps
    filter = XIN_S8_CLIP (filter + 3 * (qs0 - ps0)) & fltMask;

    // save bottom 3 bits so that we round one side +4 and the other +3
    // if it equals 4 we'll set to adjust by -1 to account for the fact
    // we'd round 3 the other way
    filter1 = XIN_S8_CLIP (filter + 4) >> 3;
    filter2 = XIN_S8_CLIP (filter + 3) >> 3;

    *oq0 = (PIXEL)(XIN_S8_CLIP (qs0 - filter1) ^ 0x80);
    *op0 = (PIXEL)(XIN_S8_CLIP (ps0 + filter2) ^ 0x80);

    // outer tap adjustments
    filter = XIN_ROUND_POWER2 (filter1, 1) & ~hev;

    *oq1 = (PIXEL)(XIN_S8_CLIP (qs1 - filter) ^ 0x80);
    *op1 = (PIXEL)(XIN_S8_CLIP (ps1 + filter) ^ 0x80);

}

static inline void Xin265pFilter6 (
    SINT8 fltMask,
    UINT8 thresh,
    SINT8 flatMask,
    PIXEL *op2,
    PIXEL *op1,
    PIXEL *op0,
    PIXEL *oq0,
    PIXEL *oq1,
    PIXEL *oq2)
{
    PIXEL p2, p1, p0;
    PIXEL q2, q1, q0;

    if (flatMask && fltMask)
    {
        p2 = *op2;
        p1 = *op1;
        p0 = *op0;
        q0 = *oq0;
        q1 = *oq1;
        q2 = *oq2;

        // 5-tap filter [1, 2, 2, 2, 1]
        *op1 = XIN_ROUND_POWER2 (p2 * 3 + p1 * 2 + p0 * 2 + q0,      3);
        *op0 = XIN_ROUND_POWER2 (p2 + p1 * 2 + p0 * 2 + q0 * 2 + q1, 3);
        *oq0 = XIN_ROUND_POWER2 (p1 + p0 * 2 + q0 * 2 + q1 * 2 + q2, 3);
        *oq1 = XIN_ROUND_POWER2 (p0 + q0 * 2 + q1 * 2 + q2 * 3,      3);
    }
    else
    {
        Xin265pFilter4 (
            fltMask,
            thresh,
            op1,
            op0,
            oq0,
            oq1);
    }

}

static inline void Xin265pFilter8 (
    SINT8 fltMask,
    UINT8 thresh,
    SINT8 flatMask,
    PIXEL *op3,
    PIXEL *op2,
    PIXEL *op1,
    PIXEL *op0,
    PIXEL *oq0,
    PIXEL *oq1,
    PIXEL *oq2,
    PIXEL *oq3)
{
    PIXEL p3, p2, p1, p0;
    PIXEL q3, q2, q1, q0;

    if (flatMask && fltMask)
    {
        p3 = *op3;
        p2 = *op2;
        p1 = *op1;
        p0 = *op0;
        q0 = *oq0;
        q1 = *oq1;
        q2 = *oq2;
        q3 = *oq3;

        // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
        *op2 = XIN_ROUND_POWER2 (p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
        *op1 = XIN_ROUND_POWER2 (p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
        *op0 = XIN_ROUND_POWER2 (p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
        *oq0 = XIN_ROUND_POWER2 (p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
        *oq1 = XIN_ROUND_POWER2 (p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
        *oq2 = XIN_ROUND_POWER2 (p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
    }
    else
    {
        Xin265pFilter4 (
            fltMask,
            thresh,
            op1,
            op0,
            oq0,
            oq1);
    }

}

static inline void Xin265pFilter14 (
    SINT8 fltMask,
    UINT8 thresh,
    SINT8 flatMask0,
    SINT8 flatMask1,
    PIXEL *op6,
    PIXEL *op5,
    PIXEL *op4,
    PIXEL *op3,
    PIXEL *op2,
    PIXEL *op1,
    PIXEL *op0,
    PIXEL *oq0,
    PIXEL *oq1,
    PIXEL *oq2,
    PIXEL *oq3,
    PIXEL *oq4,
    PIXEL *oq5,
    PIXEL *oq6)
{
    PIXEL p6, p5, p4, p3, p2, p1, p0;
    PIXEL q6, q5, q4, q3, q2, q1, q0;

    if (flatMask1 && flatMask0 && fltMask)
    {
        p6 = *op6;
        p5 = *op5;
        p4 = *op4;
        p3 = *op3;
        p2 = *op2;
        p1 = *op1;
        p0 = *op0;
        q0 = *oq0;
        q1 = *oq1;
        q2 = *oq2;
        q3 = *oq3;
        q4 = *oq4;
        q5 = *oq5;
        q6 = *oq6;

        // 13-tap filter [1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1]
        *op5 = XIN_ROUND_POWER2 (p6 * 7 + p5 * 2 + p4 * 2 + p3 + p2 + p1 + p0 + q0,                          4);
        *op4 = XIN_ROUND_POWER2 (p6 * 5 + p5 * 2 + p4 * 2 + p3 * 2 + p2 + p1 + p0 + q0 + q1,                 4);
        *op3 = XIN_ROUND_POWER2 (p6 * 4 + p5 + p4 * 2 + p3 * 2 + p2 * 2 + p1 + p0 + q0 + q1 + q2,            4);
        *op2 = XIN_ROUND_POWER2 (p6 * 3 + p5 + p4 + p3 * 2 + p2 * 2 + p1 * 2 + p0 + q0 + q1 + q2 + q3,       4);
        *op1 = XIN_ROUND_POWER2 (p6 * 2 + p5 + p4 + p3 + p2 * 2 + p1 * 2 + p0 * 2 + q0 + q1 + q2 + q3 + q4,  4);
        *op0 = XIN_ROUND_POWER2 (p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 * 2 + q0 * 2 + q1 + q2 + q3 + q4 + q5, 4);
        *oq0 = XIN_ROUND_POWER2 (p5 + p4 + p3 + p2 + p1 + p0 * 2 + q0 * 2 + q1 * 2 + q2 + q3 + q4 + q5 + q6, 4);
        *oq1 = XIN_ROUND_POWER2 (p4 + p3 + p2 + p1 + p0 + q0 * 2 + q1 * 2 + q2 * 2 + q3 + q4 + q5 + q6 * 2,  4);
        *oq2 = XIN_ROUND_POWER2 (p3 + p2 + p1 + p0 + q0 + q1 * 2 + q2 * 2 + q3 * 2 + q4 + q5 + q6 * 3,       4);
        *oq3 = XIN_ROUND_POWER2 (p2 + p1 + p0 + q0 + q1 + q2 * 2 + q3 * 2 + q4 * 2 + q5 + q6 * 4,            4);
        *oq4 = XIN_ROUND_POWER2 (p1 + p0 + q0 + q1 + q2 + q3 * 2 + q4 * 2 + q5 * 2 + q6 * 5,                 4);
        *oq5 = XIN_ROUND_POWER2 (p0 + q0 + q1 + q2 + q3 + q4 * 2 + q5 * 2 + q6 * 7,                          4);

    }
    else
    {
        Xin265pFilter8 (
            fltMask,
            thresh,
            flatMask0,
            op3,
            op2,
            op1,
            op0,
            oq0,
            oq1,
            oq2,
            oq3);
    }

}

void Xin265pLpfVert4 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh)
{
    SINT32  idx;
    PIXEL   p0, p1;
    PIXEL   q0, q1;
    SINT8   fltMask;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (idx = 0; idx < 4; ++idx)
    {
        p1 = src[-2];
        p0 = src[-1];
        q0 = src[0];
        q1 = src[1];

        Xin265pFilterMask2 (
            *limit,
            *blimit,
            p1,
            p0,
            q0,
            q1,
            &fltMask);

        Xin265pFilter4 (
            fltMask,
            *thresh,
            src - 2,
            src - 1,
            src,
            src + 1);

        src += srcStride;

    }

}

void Xin265pLpfVert6 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8   *blimit,
    UINT8   *limit,
    UINT8   *thresh)
{
    SINT32 idx;
    PIXEL  p0, p1, p2;
    PIXEL  q0, q1, q2;
    SINT8  fltMask;
    SINT8  flatMask;

    for (idx = 0; idx < 4; ++idx)
    {
        p2 = src[-3];
        p1 = src[-2];
        p0 = src[-1];
        q0 = src[0];
        q1 = src[1];
        q2 = src[2];

        Xin265pFilterMask3 (
            *limit,
            *blimit,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            &fltMask);

        Xin265pFlatMask3 (
            1,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            &flatMask);

        Xin265pFilter6 (
            fltMask,
            *thresh,
            flatMask,
            src - 3,
            src - 2,
            src - 1,
            src,
            src + 1,
            src + 2);

        src += srcStride;

    }

}

void Xin265pLpfVert8 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8   *blimit,
    UINT8   *limit,
    UINT8   *thresh)
{
    SINT32 idx;
    PIXEL  p0, p1, p2, p3;
    PIXEL  q0, q1, q2, q3;
    SINT8  fltMask;
    SINT8  flatMask;

    for (idx = 0; idx < 4; ++idx)
    {
        p3 = src[-4];
        p2 = src[-3];
        p1 = src[-2];
        p0 = src[-1];
        q0 = src[0];
        q1 = src[1];
        q2 = src[2];
        q3 = src[3];

        Xin265pFilterMask4 (
            *limit,
            *blimit,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &fltMask);

        Xin265pFlatMask4 (
            1,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &flatMask);

        Xin265pFilter8 (
            fltMask,
            *thresh,
            flatMask,
            src - 4,
            src - 3,
            src - 2,
            src - 1,
            src,
            src + 1,
            src + 2,
            src + 3);

        src += srcStride;

    }

}

void Xin265pLpfVert14 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8   *blimit,
    UINT8   *limit,
    UINT8   *thresh)
{
    SINT32 idx;
    PIXEL  p0, p1, p2, p3, p4, p5, p6;
    PIXEL  q0, q1, q2, q3, q4, q5, q6;
    SINT8  fltMask;
    SINT8  flatMask0;
    SINT8  flatMask1;

    for (idx = 0; idx < 4; ++idx)
    {
        p6 = src[-7]; 
        p5 = src[-6];
        p4 = src[-5]; 
        p3 = src[-4];
        p2 = src[-3];
        p1 = src[-2];
        p0 = src[-1];
        
        q0 = src[0];
        q1 = src[1];
        q2 = src[2];
        q3 = src[3];
        q4 = src[4];
        q5 = src[5];
        q6 = src[6];
        
        Xin265pFilterMask4 (
            *limit,
            *blimit,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &fltMask);

        Xin265pFlatMask4 (
            1,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &flatMask0);

        Xin265pFlatMask4 (
            1,
            p6,
            p5,
            p4,
            p0,
            q0,
            q4,
            q5,
            q6,
            &flatMask1);
        
        Xin265pFilter14 (
            fltMask, 
            *thresh, 
            flatMask0, 
            flatMask1, 
            src - 7, 
            src - 6, 
            src - 5, 
            src - 4, 
            src - 3,
            src - 2, 
            src - 1, 
            src, 
            src + 1, 
            src + 2, 
            src + 3, 
            src + 4, 
            src + 5, 
            src + 6);
        
        src += srcStride;
        
    }
    
}

void Xin265pLpfHorz4 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh)
{
    SINT32  idx;
    PIXEL   p0, p1;
    PIXEL   q0, q1;
    SINT8   fltMask;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (idx = 0; idx < 4; ++idx)
    {
        p1 = src[-2 * srcStride];
        p0 = src[-srcStride];
        q0 = src[0 * srcStride];
        q1 = src[1 * srcStride];

        Xin265pFilterMask2 (
            *limit,
            *blimit,
            p1,
            p0,
            q0,
            q1,
            &fltMask);

        Xin265pFilter4 (
            fltMask,
            *thresh,
            src - 2 * srcStride,
            src - 1 * srcStride,
            src,
            src + 1 * srcStride);

        src++;

    }

}

void Xin265pLpfHorz6 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh)
{
    SINT32 idx;
    PIXEL  p0, p1, p2;
    PIXEL  q0, q1, q2;
    SINT8  fltMask;
    SINT8  flatMask;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (idx = 0; idx < 4; ++idx)
    {
        p2 = src[-3 * srcStride];
        p1 = src[-2 * srcStride];
        p0 = src[-srcStride];

        q0 = src[0 * srcStride];
        q1 = src[1 * srcStride];
        q2 = src[2 * srcStride];

        Xin265pFilterMask3 (
            *limit,
            *blimit,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            &fltMask);

        Xin265pFlatMask3 (
            1,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            &flatMask);

        Xin265pFilter6 (
            fltMask,
            *thresh,
            flatMask,
            src - 3 * srcStride,
            src - 2 * srcStride,
            src - 1 * srcStride,
            src,
            src + 1 * srcStride,
            src + 2 * srcStride);

        src++;

    }

}

void Xin265pLpfHorz8 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh)
{
    SINT32 idx;
    PIXEL  p0, p1, p2, p3;
    PIXEL  q0, q1, q2, q3;
    SINT8  fltMask;
    SINT8  flatMask;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (idx = 0; idx < 4; ++idx)
    {
        p3 = src[-4 * srcStride];
        p2 = src[-3 * srcStride];
        p1 = src[-2 * srcStride];
        p0 = src[-1 * srcStride];

        q0 = src[0 * srcStride];
        q1 = src[1 * srcStride];
        q2 = src[2 * srcStride];
        q3 = src[3 * srcStride];


        Xin265pFilterMask4 (
            *limit,
            *blimit,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &fltMask);

        Xin265pFlatMask4 (
            1,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &flatMask);

        Xin265pFilter8 (
            fltMask,
            *thresh,
            flatMask,
            src - 4 * srcStride,
            src - 3 * srcStride,
            src - 2 * srcStride,
            src - 1 * srcStride,
            src,
            src + 1 * srcStride,
            src + 2 * srcStride,
            src + 3 * srcStride);

        src++;

    }

}

void Xin265pLpfHorz14 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh)
{
    SINT32 idx;
    PIXEL  p0, p1, p2, p3, p4, p5, p6;
    PIXEL  q0, q1, q2, q3, q4, q5, q6;
    SINT8  fltMask;
    SINT8  flatMask0;
    SINT8  flatMask1;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (idx = 0; idx < 4; ++idx)
    {
        p6 = src[-7 * srcStride];
        p5 = src[-6 * srcStride];
        p4 = src[-5 * srcStride];
        p3 = src[-4 * srcStride];
        p2 = src[-3 * srcStride];
        p1 = src[-2 * srcStride];
        p0 = src[-1 * srcStride];

        q0 = src[0 * srcStride];
        q1 = src[1 * srcStride];
        q2 = src[2 * srcStride];
        q3 = src[3 * srcStride];
        q4 = src[4 * srcStride];
        q5 = src[5 * srcStride];
        q6 = src[6 * srcStride];

        Xin265pFilterMask4 (
            *limit,
            *blimit,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &fltMask);

        Xin265pFlatMask4 (
            1,
            p3,
            p2,
            p1,
            p0,
            q0,
            q1,
            q2,
            q3,
            &flatMask0);

        Xin265pFlatMask4 (
            1,
            p6,
            p5,
            p4,
            p0,
            q0,
            q4,
            q5,
            q6,
            &flatMask1);

        Xin265pFilter14 (
            fltMask,
            *thresh,
            flatMask0,
            flatMask1,
            src - 7 * srcStride,
            src - 6 * srcStride,
            src - 5 * srcStride,
            src - 4 * srcStride,
            src - 3 * srcStride,
            src - 2 * srcStride,
            src - 1 * srcStride,
            src,
            src + 1 * srcStride,
            src + 2 * srcStride,
            src + 3 * srcStride,
            src + 4 * srcStride,
            src + 5 * srcStride,
            src + 6 * srcStride);

        src++;
    }

}


