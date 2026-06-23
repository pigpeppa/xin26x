/***************************************************************************//**
 *
 * @file          h266_alf_rdo_avx2.c
 * @brief         Get ALF statistics (AVX2).
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
#include "memory.h"
#include "basic_macro.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_constant.h"
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "h266_alf_context.h"
#include "h266_alf.h"
#include "memory.h"
#include "immintrin.h"
#include "h266_alf_subroutines.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

#define maxFilterSamples  4

static void XinCalcCovariance4 (
    SINT16   *eLocal,
    PIXEL    *rec,
    intptr_t stride,
    UINT32   channel,
    int      transpose_idx,
    int      clip_top_row,
    int      clip_bot_row)
{
    static const int alf_pattern_5[13] =
    {
        0*16,
        1*16,  2*16,  3*16,
        4*16,  5*16,  6*16,  5*16,  4*16,
        3*16,  2*16,  1*16,
        0*16
    };

    static const int alf_pattern_7[25] =
    {
        0*16,
        1*16,  2*16,  3*16,
        4*16,  5*16,  6*16,  7*16,  8*16,
        9*16, 10*16, 11*16, 12*16, 11*16, 10*16, 9*16,
        8*16,  7*16,  6*16,  5*16,  4*16,
        3*16,  2*16,  1*16,
        0*16
    };

    const BOOL is_luma = channel == PLANE_LUMA;
    const int* filter_pattern = is_luma ? alf_pattern_7 : alf_pattern_5;
    const int half_filter_length = (is_luma ? 7 : 5) >> 1;

    int k = 0;

    __m128i xrec;
    __m128i xcur;
    __m128i xval0;
    __m128i xval1;

    xrec = _mm_cvtsi32_si128 (*((UINT32 *)(rec)));
    xrec = _mm_cvtepu8_epi16 (xrec);
    xcur = _mm_slli_epi16 (xrec, 1);

    if (transpose_idx == 0)
    {
        for (int i = -half_filter_length; i < 0; i++)
        {
            PIXEL *rec0 = rec + XIN_MAX(i, clip_top_row) * stride;
            PIXEL *rec1 = rec - XIN_MAX(i, -clip_bot_row) * stride;

            for( int j = -half_filter_length - i; j <= half_filter_length + i; j++, k++ )
            {
                xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(rec0 + j)));
                xval0 = _mm_cvtepu8_epi16 (xval0);
                xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(rec1 - j)));
                xval1 = _mm_cvtepu8_epi16 (xval1);

                xval0 = _mm_add_epi16( xval0, xval1 );
                xval0 = _mm_sub_epi16( xval0, xcur );

                _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
            }
        }

        for( int j = -half_filter_length; j < 0; j++, k++ )
        {
            xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(rec + j)));
            xval0 = _mm_cvtepu8_epi16 (xval0);
            xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(rec - j)));
            xval1 = _mm_cvtepu8_epi16 (xval1);

            xval0 = _mm_add_epi16( xval0, xval1 );
            xval0 = _mm_sub_epi16( xval0, xcur );

            _mm_storel_epi64( ( __m128i* ) & eLocal[filter_pattern[k]], xval0 );
        }

    }
    else if (transpose_idx == 1)
    {
        for( int j = -half_filter_length; j < 0; j++ )
        {
            PIXEL *rec0 = rec + j;
            PIXEL *rec1 = rec - j;

            for( int i = -half_filter_length - j; i <= half_filter_length + j; i++, k++ )
            {
                const PIXEL *vptr0 = &rec0[XIN_MAX(i, clip_top_row) * stride];
                const PIXEL *vptr1 = &rec1[-XIN_MAX(i, -clip_bot_row) * stride];

                xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr0)));
                xval0 = _mm_cvtepu8_epi16 (xval0);
                xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr1)));
                xval1 = _mm_cvtepu8_epi16 (xval1);

                xval0 = _mm_add_epi16( xval0, xval1 );
                xval0 = _mm_sub_epi16( xval0, xcur );

                _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
            }

        }

        for( int i = -half_filter_length; i < 0; i++, k++ )
        {
            const PIXEL *vptr0 = &rec[XIN_MAX(i, clip_top_row) * stride];
            const PIXEL *vptr1 = &rec[-XIN_MAX(i, -clip_bot_row) * stride];

            xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr0)));
            xval0 = _mm_cvtepu8_epi16 (xval0);
            xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr1)));
            xval1 = _mm_cvtepu8_epi16 (xval1);

            xval0 = _mm_add_epi16( xval0, xval1 );
            xval0 = _mm_sub_epi16( xval0, xcur );

            _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
        }

    }
    else if (transpose_idx == 2)
    {
        for( int i = -half_filter_length; i < 0; i++ )
        {
            PIXEL *rec0 = rec + XIN_MAX(i, clip_top_row) * stride;
            PIXEL *rec1 = rec - XIN_MAX(i, -clip_bot_row) * stride;

            for( int j = half_filter_length + i; j >= -half_filter_length - i; j--, k++ )
            {
                xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(&rec0[ j])));
                xval0 = _mm_cvtepu8_epi16 (xval0);
                xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(&rec1[-j])));
                xval1 = _mm_cvtepu8_epi16 (xval1);

                xval0 = _mm_add_epi16( xval0, xval1 );
                xval0 = _mm_sub_epi16( xval0, xcur );

                _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
            }
        }
        for( int j = -half_filter_length; j < 0; j++, k++ )
        {
            xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(&rec[ j])));
            xval0 = _mm_cvtepu8_epi16 (xval0);
            xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(&rec[-j])));
            xval1 = _mm_cvtepu8_epi16 (xval1);

            xval0 = _mm_add_epi16( xval0, xval1 );
            xval0 = _mm_sub_epi16( xval0, xcur );

            _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
        }

    }
    else
    {
        for( int j = -half_filter_length; j < 0; j++ )
        {
            PIXEL *rec0 = rec + j;
            PIXEL *rec1 = rec - j;

            for( int i = half_filter_length + j; i >= -half_filter_length - j; i--, k++ )
            {
                const PIXEL *vptr0 = &rec0[XIN_MAX(i, clip_top_row) * stride];
                const PIXEL *vptr1 = &rec1[-XIN_MAX(i, -clip_bot_row) * stride];

                xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr0)));
                xval0 = _mm_cvtepu8_epi16 (xval0);
                xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr1)));
                xval1 = _mm_cvtepu8_epi16 (xval1);

                xval0 = _mm_add_epi16( xval0, xval1 );
                xval0 = _mm_sub_epi16( xval0, xcur );

                _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
            }
        }

        for( int i = -half_filter_length; i < 0; i++, k++ )
        {
            const PIXEL *vptr0 = &rec[XIN_MAX(i, clip_top_row) * stride];
            const PIXEL *vptr1 = &rec[-XIN_MAX(i, -clip_bot_row) * stride];

            xval0 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr0)));
            xval0 = _mm_cvtepu8_epi16 (xval0);
            xval1 = _mm_cvtsi32_si128 (*((UINT32 *)(vptr1)));
            xval1 = _mm_cvtepu8_epi16 (xval1);

            xval0 = _mm_add_epi16( xval0, xval1 );
            xval0 = _mm_sub_epi16( xval0, xcur );

            _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xval0 );
        }

    }

    _mm_storel_epi64( ( __m128i* ) &eLocal[filter_pattern[k]], xrec );

}

void Xin266GetPreBlkStats_AVX2 (
    xin_alf_cov    *alfCov,
    BOOL           useNonLinearAlf,
    xin_alf_filter *alfFilter,
    xin_alf_class  *alfClass,
    intptr_t       alfClassStride,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *recon,
    intptr_t       reconStride,
    UINT32         planeIdx,
    SINT32         width,
    SINT32         height,
    SINT16         alfClippingValues[PLANE_TYPE][XIN_ALF_CLIP_NUM],
    SINT32         vbCtuHeight,
    SINT32         vbPos)
{
    int transposeIdx = 0;
    int classIdx = 0;
    intptr_t classPos;

    (void)useNonLinearAlf;
    (void)alfClippingValues;

    for (int i = 0; i < height; i += 4)
    {

        int clipTopRow[4] = { -maxFilterSamples, -maxFilterSamples, -maxFilterSamples, -maxFilterSamples };
        int clipBotRow[4] = {  maxFilterSamples,  maxFilterSamples,  maxFilterSamples,  maxFilterSamples };

        for (int ii = 0; ii < 4; ii++)
        {
            int vbDistance = ((i + ii) % vbCtuHeight) - vbPos;

            if (vbDistance >= -3 && vbDistance < 0)
            {
                clipBotRow[ii] = -vbDistance - 1;
                clipTopRow[ii] = -clipBotRow[ii]; // symmetric
            }
            else if( vbDistance >= 0 && vbDistance < 3 )
            {
                clipTopRow[ii] = -vbDistance;
                clipBotRow[ii] = -clipTopRow[ii]; // symmetric
            }
        }

        for (int j = 0; j < width; j += 4)
        {
            classPos = (i / 4) * alfClassStride + j / 4;

            if (alfClass && alfClass[classPos].classIdx == XIN_ALF_UNUSED_CLASS && alfClass[classPos].transposeIdx == XIN_ALF_UNUSED_TRANSPOS)
            {
                continue;
            }

            if (alfClass)
            {
                transposeIdx = alfClass[classPos].transposeIdx;
                classIdx     = alfClass[classPos].classIdx;
            }

            SINT16 ELocal[XIN_ALF_MAX_LUMA_COEF_NUM*16];
            SINT16 yLocal[4][4];
            __m128i inputx4;
            __m128i reconx4;
            __m128i diffx4;
            __m128i localL;
            __m128i localK;
            __m128i localY;
            __m128i sumx4;

            for (int ii = 0; ii < 4; ii++)
            {
                inputx4 = _mm_cvtsi32_si128 (*((UINT32 *)(input + j + ii * inputStride)));
                inputx4 = _mm_cvtepu8_epi16 (inputx4);
                reconx4 = _mm_cvtsi32_si128 (*((UINT32 *)(recon + j + ii * reconStride)));
                reconx4 = _mm_cvtepu8_epi16 (reconx4);

                diffx4 = _mm_sub_epi16 (inputx4, reconx4);

                _mm_storel_epi64( ( __m128i* ) (&yLocal[ii][0]), diffx4);

            }

            for (int ii = 0; ii < 4; ii++)
            {
                XinCalcCovariance4 (
                    ELocal + (ii << 2),
                    recon + j + ii * reconStride,
                    reconStride,
                    planeIdx != PLANE_LUMA,
                    transposeIdx,
                    clipTopRow[ii],
                    clipBotRow[ii]);
            }

            for( int k = 0; k < alfFilter->numCoeff; k++ )
            {

                const short *Elocalk = &ELocal[k<<4];

                for( int l = k; l < alfFilter->numCoeff; l++ )
                {
                    const short *Elocall = &ELocal[l<<4];

                    localL = _mm_loadu_si128 ((__m128i *)(Elocall));
                    localK = _mm_loadu_si128 ((__m128i *)(Elocalk));

                    sumx4 = _mm_madd_epi16 (localL, localK);

                    localL = _mm_loadu_si128 ((__m128i *)(Elocall + 8));
                    localK = _mm_loadu_si128 ((__m128i *)(Elocalk + 8));

                    sumx4 = _mm_add_epi32 (sumx4, _mm_madd_epi16 (localL, localK));
                    sumx4 = _mm_hadd_epi32 (sumx4, sumx4);
                    sumx4 = _mm_hadd_epi32 (sumx4, sumx4);

                    alfCov[classIdx].E[0][0][k][l] += _mm_cvtsi128_si32 (sumx4);

                }

                localY = _mm_loadu_si128 ((__m128i *)(&yLocal[0][0]));
                localK = _mm_loadu_si128 ((__m128i *)(Elocalk));

                sumx4 = _mm_madd_epi16 (localY, localK);

                localY = _mm_loadu_si128 ((__m128i *)(&yLocal[2][0]));
                localK = _mm_loadu_si128 ((__m128i *)(Elocalk + 8));

                sumx4 = _mm_add_epi32 (sumx4, _mm_madd_epi16 (localY, localK));
                sumx4 = _mm_hadd_epi32 (sumx4, sumx4);
                sumx4 = _mm_hadd_epi32 (sumx4, sumx4);

                alfCov[classIdx].y[0][k] += _mm_cvtsi128_si32 (sumx4);

            }

            localY = _mm_loadu_si128 ((__m128i *)(&yLocal[0][0]));

            sumx4 = _mm_madd_epi16 (localY, localY);

            localY = _mm_loadu_si128 ((__m128i *)(&yLocal[2][0]));

            sumx4 = _mm_add_epi32 (sumx4, _mm_madd_epi16 (localY, localY));
            sumx4 = _mm_hadd_epi32 (sumx4, sumx4);
            sumx4 = _mm_hadd_epi32 (sumx4, sumx4);

            alfCov[classIdx].pixAcc += _mm_cvtsi128_si32 (sumx4);

        }

        input += inputStride << 2;
        recon += reconStride << 2;

    }

    int numClasses = alfClass ? XIN_ALF_MAX_CLS_NUM : 1;

    for( classIdx = 0; classIdx < numClasses; classIdx++ )
    {
        for( int k = 1; k < alfFilter->numCoeff; k++ )
        {
            for( int l = 0; l < k; l++ )
            {
                alfCov[classIdx].E[0][0][k][l] = alfCov[classIdx].E[0][0][l][k];
            }
        }
    }

}

FLOAT32 Xin266CalcCoeffErrorLin13_AVX2 (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth)
{
    (void)numCoeff;
    (void)clip;

    FLOAT32 error = 0;
    FLOAT32 factor = (FLOAT32)1.0 / (FLOAT32)(1 << (bitDepth - 1));

    const __m128 mzero = _mm_setzero_ps();
    const __m128 minvf = _mm_set1_ps( factor );
    const __m128 mtwo  = _mm_set1_ps( 2.0 );

    __m128 merror  = _mm_setzero_ps(), msum0, msum1, msum2, msum3;
    __m128 mcoef1  = _mm_cvtepi32_ps( _mm_loadu_si128( ( const __m128i* ) &coeff[1] ) );
    __m128 mcoef5  = _mm_cvtepi32_ps( _mm_loadu_si128( ( const __m128i* ) &coeff[5] ) );
    __m128 mcoef9  = _mm_cvtepi32_ps( _mm_loadu_si128( ( const __m128i* ) &coeff[9] ) );

    // i = 0

    __m128 mE1  = _mm_loadu_ps( &E[0][0][0][ 1] );
    __m128 mE5  = _mm_loadu_ps( &E[0][0][0][ 5] );
    __m128 mE9  = _mm_loadu_ps( &E[0][0][0][ 9] );

    mE1  = _mm_mul_ps( mcoef1, mE1 );
    mE5  = _mm_mul_ps( mcoef5, mE5 );
    mE9  = _mm_mul_ps( mcoef9, mE9 );

    msum0 = _mm_add_ps( _mm_add_ps( mE1, mE5 ), mE9 );

    // i = 1

    mcoef1 = _mm_blend_ps( mcoef1, mzero, 1 );

    mE1  = _mm_loadu_ps( &E[0][0][1][ 1] );
    mE5  = _mm_loadu_ps( &E[0][0][1][ 5] );
    mE9  = _mm_loadu_ps( &E[0][0][1][ 9] );

    mE1  = _mm_mul_ps( mcoef1,  mE1 );
    mE5  = _mm_mul_ps( mcoef5,  mE5 );
    mE9  = _mm_mul_ps( mcoef9,  mE9 );

    msum1 = _mm_add_ps( _mm_add_ps( mE1, mE5 ), mE9 );

    // i = 2

    mcoef1 = _mm_blend_ps( mcoef1, mzero, 2 );

    mE1 = _mm_loadu_ps( &E[0][0][2][1] );
    mE5 = _mm_loadu_ps( &E[0][0][2][5] );
    mE9 = _mm_loadu_ps( &E[0][0][2][9] );

    mE1 = _mm_mul_ps( mcoef1, mE1 );
    mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum2 = _mm_add_ps( _mm_add_ps( mE1, mE5 ), mE9 );

    // i = 3

    mcoef1 = _mm_blend_ps( mcoef1, mzero, 4 );

    mE1 = _mm_loadu_ps( &E[0][0][3][1] );
    mE5 = _mm_loadu_ps( &E[0][0][3][5] );
    mE9 = _mm_loadu_ps( &E[0][0][3][9] );

    mE1 = _mm_mul_ps( mcoef1, mE1 );
    mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum3 = _mm_add_ps( _mm_add_ps( mE1, mE5 ), mE9 );

    msum0 = _mm_hadd_ps( msum0, msum1 );
    msum2 = _mm_hadd_ps( msum2, msum3 );
    msum0 = _mm_hadd_ps( msum0, msum2 );

    // combine i = 0...3

    __m128 mcoef   = _mm_cvtepi32_ps( _mm_loadu_si128( ( const __m128i* ) &coeff[0] ) );
    __m128 my      =                  _mm_loadu_ps(                       &y[0][0] );
    __m128 mE      =                  _mm_setr_ps ( E[0][0][0][0], E[0][0][1][1], E[0][0][2][2], E[0][0][3][3] );

    mE1 = _mm_mul_ps( mE, mcoef );
    mE5 = _mm_mul_ps( msum0, mtwo );
    mE1 = _mm_add_ps( mE1, mE5 );
    mE1 = _mm_mul_ps( mE1, minvf );
    mE5 = _mm_mul_ps( mtwo, my );
    mE1 = _mm_sub_ps( mE1, mE5 );
    mE1 = _mm_mul_ps( mE1, mcoef );
    merror = _mm_add_ps(merror, mE1);

    //error += ( ( E[0][0][0][0] * coeff[0] + sum0 * 2 ) * invFactor - 2 * y[0][0] ) * coeff[0];
    //error += ( ( E[0][0][1][1] * coeff[1] + sum1 * 2 ) * invFactor - 2 * y[0][1] ) * coeff[1];
    //error += ( ( E[0][0][2][2] * coeff[2] + sum2 * 2 ) * invFactor - 2 * y[0][2] ) * coeff[2];
    //error += ( ( E[0][0][3][3] * coeff[3] + sum3 * 2 ) * invFactor - 2 * y[0][3] ) * coeff[3];

    // i = 4

    //__m128 mE1 = _mm_loadu_ps( &E[0][0][4][1] );
    mE5 = _mm_loadu_ps( &E[0][0][4][5] );
    mE9 = _mm_loadu_ps( &E[0][0][4][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum0 = _mm_add_ps( mE5, mE9 );

    // i = 5

    mcoef5 = _mm_blend_ps( mcoef5, mzero, 1 );

    //mE1 = _mm_loadu_ps( &E[0][0][5][1] );
    mE5 = _mm_loadu_ps( &E[0][0][5][5] );
    mE9 = _mm_loadu_ps( &E[0][0][5][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum1 = _mm_add_ps( mE5, mE9 );

    // i = 6

    mcoef5 = _mm_blend_ps( mcoef5, mzero, 2 );

    //mE1 = _mm_loadu_ps( &E[0][0][6][1] );
    mE5 = _mm_loadu_ps( &E[0][0][6][5] );
    mE9 = _mm_loadu_ps( &E[0][0][6][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum2 = _mm_add_ps( mE5, mE9 );

    // i = 7

    mcoef5 = _mm_blend_ps( mcoef5, mzero, 4 );

    //mE1 = _mm_loadu_ps( &E[0][0][7][1] );
    mE5 = _mm_loadu_ps( &E[0][0][7][5] );
    mE9 = _mm_loadu_ps( &E[0][0][7][9] );

//  mE1 = _mm_mul_ps( mcoef1, mE1 );
    mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum3 = _mm_add_ps( mE5, mE9 );

    msum0 = _mm_hadd_ps( msum0, msum1 );
    msum2 = _mm_hadd_ps( msum2, msum3 );
    msum0 = _mm_hadd_ps( msum0, msum2 );

    // combine i = 4...7

    mcoef = _mm_cvtepi32_ps( _mm_loadu_si128( ( const __m128i* ) &coeff[4] ) );
    my = _mm_loadu_ps( &y[0][4] );
    mE = _mm_setr_ps( E[0][0][4][4], E[0][0][5][5], E[0][0][6][6], E[0][0][7][7] );

    mE1 = _mm_mul_ps( mE, mcoef );
    mE5 = _mm_mul_ps( msum0, mtwo );
    mE1 = _mm_add_ps( mE1, mE5 );
    mE1 = _mm_mul_ps( mE1, minvf );
    mE5 = _mm_mul_ps( mtwo, my );
    mE1 = _mm_sub_ps( mE1, mE5 );
    mE1 = _mm_mul_ps( mE1, mcoef );
    merror = _mm_add_ps(merror, mE1);

    //error += ( ( E[0][0][4][4] * coeff[4] + sum0 * 2 ) * invFactor - 2 * y[0][4] ) * coeff[4];
    //error += ( ( E[0][0][5][5] * coeff[5] + sum1 * 2 ) * invFactor - 2 * y[0][5] ) * coeff[5];
    //error += ( ( E[0][0][6][6] * coeff[6] + sum2 * 2 ) * invFactor - 2 * y[0][6] ) * coeff[6];
    //error += ( ( E[0][0][7][7] * coeff[7] + sum3 * 2 ) * invFactor - 2 * y[0][7] ) * coeff[7];

    // i = 8

    //__m128 mE1 = _mm_loadu_ps( &E[0][0][8][1] );
    //__m128 mE5 = _mm_loadu_ps( &E[0][0][8][5] );
    mE9 = _mm_loadu_ps( &E[0][0][8][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    //mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum0 = mE9;

    // i = 9

    mcoef9 = _mm_blend_ps( mcoef9, mzero, 1 );

    //mE1 = _mm_loadu_ps( &E[0][0][9][1] );
    //mE5 = _mm_loadu_ps( &E[0][0][9][5] );
    mE9 = _mm_loadu_ps( &E[0][0][9][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    //mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum1 = mE9;

    // i = 10

    mcoef9 = _mm_blend_ps( mcoef9, mzero, 2 );

    //mE1 = _mm_loadu_ps( &E[0][0][10][1] );
    //mE5 = _mm_loadu_ps( &E[0][0][10][5] );
    mE9 = _mm_loadu_ps( &E[0][0][10][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    //mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum2 = mE9;

    // i = 11

    mcoef9 = _mm_blend_ps( mcoef9, mzero, 4 );

    //mE1 = _mm_loadu_ps( &E[0][0][11][1] );
    //mE5 = _mm_loadu_ps( &E[0][0][11][5] );
    mE9 = _mm_loadu_ps( &E[0][0][11][9] );

    //mE1 = _mm_mul_ps( mcoef1, mE1 );
    //mE5 = _mm_mul_ps( mcoef5, mE5 );
    mE9 = _mm_mul_ps( mcoef9, mE9 );

    msum3 = mE9;

    msum0 = _mm_hadd_ps( msum0, msum1 );
    msum2 = _mm_hadd_ps( msum2, msum3 );
    msum0 = _mm_hadd_ps( msum0, msum2 );

    // combine i = 8...11

    mcoef = _mm_cvtepi32_ps( _mm_loadu_si128( ( const __m128i* ) & coeff[8] ) );
    my = _mm_loadu_ps( &y[0][8] );
    mE = _mm_setr_ps( E[0][0][8][8], E[0][0][9][9], E[0][0][10][10], E[0][0][11][11] );

    mE1 = _mm_mul_ps( mE, mcoef );
    mE5 = _mm_mul_ps( msum0, mtwo );
    mE1 = _mm_add_ps( mE1, mE5 );
    mE1 = _mm_mul_ps( mE1, minvf );
    mE5 = _mm_mul_ps( mtwo, my );
    mE1 = _mm_sub_ps( mE1, mE5 );
    mE1 = _mm_mul_ps( mE1, mcoef );
    merror = _mm_add_ps(merror, mE1);

    //error += ( ( E[0][0][ 8][ 8] * coeff[ 8] + sum0 * 2 ) * invFactor - 2 * y[0][ 8] ) * coeff[ 8];
    //error += ( ( E[0][0][ 9][ 9] * coeff[ 9] + sum1 * 2 ) * invFactor - 2 * y[0][ 9] ) * coeff[ 9];
    //error += ( ( E[0][0][10][10] * coeff[10] + sum2 * 2 ) * invFactor - 2 * y[0][10] ) * coeff[10];
    //error += ( ( E[0][0][11][11] * coeff[11] + sum3 * 2 ) * invFactor - 2 * y[0][11] ) * coeff[11];

    merror = _mm_hadd_ps( merror, mzero );
    merror = _mm_hadd_ps( merror, mzero );
    error  = _mm_cvtss_f32( merror );
    error += ( ( E[0][0][12][12] * coeff[12] ) * factor - 2 * y[0][12] ) * coeff[12];

    return error * factor;

}

FLOAT32 Xin266CalcCoeffErrorLin_AVX2 (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth)
{
    FLOAT32 error;

    if (numCoeff != XIN_ALF_MAX_LUMA_COEF_NUM)
    {
        error = Xin266CalcCoeffError (
                    clip,
                    coeff,
                    E,
                    y,
                    numCoeff,
                    bitDepth);
    }
    else
    {
        error = Xin266CalcCoeffErrorLin13_AVX2 (
                    clip,
                    coeff,
                    E,
                    y,
                    numCoeff,
                    bitDepth);
        
    }

    return error;

}

