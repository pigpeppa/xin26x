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

static inline SINT32 XinAlfHsumMaddEpi32_256 (
    __m256i prod)
{
    __m128i lo = _mm256_castsi256_si128 (prod);
    __m128i hi = _mm256_extracti128_si256 (prod, 1);
    __m128i s  = _mm_add_epi32 (lo, hi);

    s = _mm_add_epi32 (s, _mm_shuffle_epi32 (s, 0x4E));
    s = _mm_add_epi32 (s, _mm_shuffle_epi32 (s, 0xB1));

    return _mm_cvtsi128_si32 (s);
}

static inline __m128i XinAlfLoadPix4 (
    const PIXEL *p)
{
    return _mm_cvtepu8_epi16 (_mm_cvtsi32_si128 (*((const UINT32 *)p)));
}

static inline __m128i XinAlfTap4 (
    const PIXEL *p0,
    const PIXEL *p1,
    __m128i     xcur)
{
    __m128i v0 = XinAlfLoadPix4 (p0);
    __m128i v1 = XinAlfLoadPix4 (p1);

    return _mm_sub_epi16 (_mm_add_epi16 (v0, v1), xcur);
}

static inline void XinAlfStoreTapRow (
    SINT16  *eLocal,
    SINT32  tap,
    SINT32  row,
    __m128i val)
{
    _mm_storel_epi64 ((__m128i *)(eLocal + (tap << 4) + (row << 2)), val);
}

static void XinAlfAccumulateBlock (
    xin_alf_cov    *alfCov,
    SINT32         classIdx,
    const SINT16   *ELocal,
    const SINT16   (*yLocal)[4],
    SINT32         numCoeff)
{
    xin_alf_cov *cov = &alfCov[classIdx];
    __m256i     yv;
    __m256i     taps[XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32      k;
    SINT32      l;
    SINT32      Etmp[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32      ytmp[XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32      pixAcc;

    yv = _mm256_loadu_si256 ((const __m256i *)yLocal);

    for (k = 0; k < numCoeff; k++)
    {
        taps[k] = _mm256_loadu_si256 ((const __m256i *)(ELocal + (k << 4)));
    }

    /* Integer outer products first; one float RMW pass at the end. */
    for (k = 0; k < numCoeff; k++)
    {
        for (l = k; l < numCoeff; l++)
        {
            Etmp[k][l] = XinAlfHsumMaddEpi32_256 (
                _mm256_madd_epi16 (taps[k], taps[l]));
        }

        ytmp[k] = XinAlfHsumMaddEpi32_256 (_mm256_madd_epi16 (yv, taps[k]));
    }

    pixAcc = XinAlfHsumMaddEpi32_256 (_mm256_madd_epi16 (yv, yv));

    for (k = 0; k < numCoeff; k++)
    {
        for (l = k; l < numCoeff; l++)
        {
            cov->E[0][0][k][l] += (FLOAT32)Etmp[k][l];
        }

        cov->y[0][k] += (FLOAT32)ytmp[k];
    }

    cov->pixAcc += (FLOAT32)pixAcc;
}

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

/*
 * 4x4 block, transpose 0, no VB clip.
 * Fills all four rows of ELocal (tap layout: tap*16 + row*4).
 * For T0 the first half of the diamond uses tap == sample order (0..numCoeff-1).
 */
static void XinCalcCovariance4x4_T0_NoClip (
    SINT16   *ELocal,
    PIXEL    *rec,
    intptr_t stride,
    BOOL     isLuma)
{
    const int half = isLuma ? 3 : 2;
    SINT32    row;

    for (row = 0; row < 4; row++)
    {
        PIXEL   *p = rec + row * stride;
        __m128i xrec = XinAlfLoadPix4 (p);
        __m128i xcur = _mm_slli_epi16 (xrec, 1);
        int     k = 0;
        int     i;
        int     j;

        for (i = -half; i < 0; i++)
        {
            PIXEL *rec0 = p + i * stride;
            PIXEL *rec1 = p - i * stride;

            for (j = -half - i; j <= half + i; j++, k++)
            {
                XinAlfStoreTapRow (
                    ELocal,
                    k,
                    row,
                    XinAlfTap4 (rec0 + j, rec1 - j, xcur));
            }
        }

        for (j = -half; j < 0; j++, k++)
        {
            XinAlfStoreTapRow (
                ELocal,
                k,
                row,
                XinAlfTap4 (p + j, p - j, xcur));
        }

        XinAlfStoreTapRow (ELocal, k, row, xrec);
    }
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

    const BOOL isLuma = (planeIdx == PLANE_LUMA);

    for (int i = 0; i < height; i += 4)
    {

        int clipTopRow[4] = { -maxFilterSamples, -maxFilterSamples, -maxFilterSamples, -maxFilterSamples };
        int clipBotRow[4] = {  maxFilterSamples,  maxFilterSamples,  maxFilterSamples,  maxFilterSamples };
        BOOL noVbClip = TRUE;

        for (int ii = 0; ii < 4; ii++)
        {
            int vbDistance = ((i + ii) % vbCtuHeight) - vbPos;

            if (vbDistance >= -3 && vbDistance < 0)
            {
                clipBotRow[ii] = -vbDistance - 1;
                clipTopRow[ii] = -clipBotRow[ii]; // symmetric
                noVbClip = FALSE;
            }
            else if( vbDistance >= 0 && vbDistance < 3 )
            {
                clipTopRow[ii] = -vbDistance;
                clipBotRow[ii] = -clipTopRow[ii]; // symmetric
                noVbClip = FALSE;
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

            for (int ii = 0; ii < 4; ii++)
            {
                inputx4 = XinAlfLoadPix4 (input + j + ii * inputStride);
                reconx4 = XinAlfLoadPix4 (recon + j + ii * reconStride);
                diffx4  = _mm_sub_epi16 (inputx4, reconx4);
                _mm_storel_epi64 ((__m128i *)(&yLocal[ii][0]), diffx4);
            }

            if (noVbClip && transposeIdx == 0)
            {
                XinCalcCovariance4x4_T0_NoClip (
                    ELocal,
                    recon + j,
                    reconStride,
                    isLuma);
            }
            else
            {
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
            }

            XinAlfAccumulateBlock (
                alfCov,
                classIdx,
                ELocal,
                yLocal,
                alfFilter->numCoeff);

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

