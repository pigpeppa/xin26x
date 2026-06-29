/***************************************************************************//**
 *
 * @file          h266_alf_rdo.c
 * @brief         Get ALF statistics.
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
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_ct_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h26x_common_data.h"
#include "h266_section_struct.h"
#include "basic_macro.h"
#include "h266_alf.h"
#include "memory.h"
#include "h26x_extend_picture.h"
#include "h266_alf_rdo.h"
#include "stdlib.h"
#include "h26x_cpu_detection.h"
#include "h266_func_struct.h"

#define ROUND(a)                (((a) < 0)? (int)((a) - 0.5) : (int)((a) + 0.5))
#define REG                     0.0001
#define REG_SQR                 0.0000001
#define XIN_FRAC_BITS_SCALE     (((FLOAT32)1.0) / (1 << 15))
#define CCALF_CANDS_COEFF_NR    8
static const FLOAT32 MAX_DOUBLE = (FLOAT32)(3.4e+38); ///< max. value of double-type value

extern const SINT32 classToFilterMapping[XIN_ALF_FIXED_FLT_SET_NUM][XIN_ALF_MAX_CLS_NUM];

static const int CCALF_SMALL_TAB[CCALF_CANDS_COEFF_NR] = { 0, 1, 2, 4, 8, 16, 32, 64 };

#if XIN_CC_ALF_MAX_FILTER_NUM > 1
typedef struct FilterIdxCount
{
    UINT64 count;
    UINT8  filterIdx;
} FilterIdxCount;

static int comparator(const void *v1, const void *v2)
{
    FilterIdxCount *p1 = (FilterIdxCount *)v1;
    FilterIdxCount *p2 = (FilterIdxCount *)v2;
    return (p1->count <= p2->count);
}
#endif

int lengthUvlc (
    int uiCode)
{
    int uiLength = 1;
    int uiTemp = ++uiCode;

    while( 1 != uiTemp )
    {
        uiTemp >>= 1;
        uiLength += 2;
    }

    // Take care of cases where uiLength > 32
    return ( uiLength >> 1 ) + ( ( uiLength + 1 ) >> 1 );
}

static void Xin266StateUpdate (
    xin_prob_model *context,
    UINT32         bin)
{
    SINT32 rate0;
    SINT32 rate1;

    rate0 = context->rate >> 4;
    rate1 = context->rate & 15;

    context->state[0] -= (context->state[0] >> rate0) & XIN_CABAC_STATE_MASK_0;
    context->state[1] -= (context->state[1] >> rate1) & XIN_CABAC_STATE_MASK_1;

    if (bin)
    {
        context->state[0] += (0x7fffu >> rate0) & XIN_CABAC_STATE_MASK_0;
        context->state[1] += (0x7fffu >> rate1) & XIN_CABAC_STATE_MASK_1;
    }
}

SINT32 gnsCholeskyDec (
    FLOAT32 inpMatr[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 outMatr[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numEq)
{
    SINT32  i;
    SINT32  j;
    SINT32  k;
    FLOAT32 invDiag[XIN_ALF_MAX_LUMA_COEF_NUM];  /* Vector of the inverse of diagonal entries of outMatr */
    FLOAT32 scale;

    for (i = 0; i < numEq; i++)
    {
        for (j = i; j < numEq; j++)
        {
            /* Compute the scaling factor */
            scale = inpMatr[i][j];

            if (i > 0)
            {
                for (k = i - 1; k >= 0; k--)
                {
                    scale -= outMatr[k][j] * outMatr[k][i];
                }
            }

            /* Compute i'th row of outMatr */
            if (i == j)
            {
                if (scale <= REG_SQR) // if(scale <= 0 )  /* If inpMatr is singular */
                {
                    return 0;
                }
                else              /* Normal operation */
                {
                    invDiag[i] = (FLOAT32)1.0 / (outMatr[i][i] = (FLOAT32)sqrt(scale));
                }
            }
            else
            {
                outMatr[i][j] = scale * invDiag[i]; /* Upper triangular part          */
                outMatr[j][i] = 0.0;              /* Lower triangular part set to 0 */
            }
        }
    }

    return 1; /* Signal that Cholesky factorization is successfully performed */

}

void gnsTransposeBacksubstitution (
    FLOAT32 U[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 *rhs,
    FLOAT32 *x,
    SINT32  order)
{
    FLOAT32 sum;
    SINT32  i;
    SINT32  j;

    /* Backsubstitution starts */
    x[0] = rhs[0] / U[0][0];               /* First row of U'                   */

    for (i = 1; i < order; i++)
    {
        /* For the rows 1..order-1           */

        sum = 0; //Holds backsubstitution from already handled rows

        for (j = 0; j < i; j++ ) /* Backsubst already solved unknowns */
        {
            sum += x[j] * U[j][i];
        }

        x[i] = ( rhs[i] - sum ) / U[i][i];       /* i'th component of solution vect.  */
    }

}

void gnsBacksubstitution (
    FLOAT32 R[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 *z,
    SINT32  size,
    FLOAT32 *A)
{
    SINT32  i;
    SINT32  j;
    FLOAT32 sum;

    size--;
    A[size] = z[size] / R[size][size];

    for (i = size - 1; i >= 0; i--)
    {
        sum = 0;

        for (j = i + 1; j <= size; j++)
        {
            sum += R[i][j] * A[j];
        }

        A[i] = ( z[i] - sum ) / R[i][i];
    }

}

SINT32 gnsSolveByChol (
    FLOAT32 LHS[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 *rhs,
    FLOAT32 *x,
    SINT32  numEq)
{
    FLOAT32 aux[XIN_ALF_MAX_LUMA_COEF_NUM];     /* Auxiliary vector */
    FLOAT32 U[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];    /* Upper triangular Cholesky factor of LHS */

    SINT32 res = 1;  // Signal that Cholesky factorization is successfully performed
    SINT32 i;

    /* The equation to be solved is LHSx = rhs */

    /* Compute upper triangular U such that U'*U = LHS */
    if( gnsCholeskyDec (LHS, U, numEq)) /* If Cholesky decomposition has been successful */
    {
        /* Now, the equation is  U'*U*x = rhs, where U is upper triangular
        * Solve U'*aux = rhs for aux
        */
        gnsTransposeBacksubstitution (U, rhs, aux, numEq);

        /* The equation is now U*x = aux, solve it for x (new motion coefficients) */
        gnsBacksubstitution (U, aux, numEq, x);

    }
    else /* LHS was singular */
    {
        res = 0;

        /* Regularize LHS */
        for (i = 0; i < numEq; i++ )
        {
            LHS[i][i] += (FLOAT32)REG;
        }

        /* Compute upper triangular U such that U'*U = regularized LHS */
        res = gnsCholeskyDec( LHS, U, numEq);

        if( !res )
        {
            memset(x, 0, sizeof(FLOAT32)*numEq);

            return 0;
        }

        /* Solve  U'*aux = rhs for aux */
        gnsTransposeBacksubstitution (U, rhs, aux, numEq);

        /* Solve U*x = aux for x */
        gnsBacksubstitution (U, aux, numEq, x);

    }

    return res;

}

void XinAlfCovAdd (
    xin_alf_cov *lhs,
    xin_alf_cov *rhs,
    xin_alf_cov *alfCov)
{
    alfCov->numCoeff = lhs->numCoeff;
    alfCov->numBins  = lhs->numBins;

    for (int b0 = 0; b0 < alfCov->numBins; b0++)
    {
        for (int b1 = 0; b1 < alfCov->numBins; b1++)
        {
            for (int j = 0; j < alfCov->numCoeff; j++)
            {
                for (int i = 0; i < alfCov->numCoeff; i++)
                {
                    alfCov->E[b0][b1][j][i] = lhs->E[b0][b1][j][i] + rhs->E[b0][b1][j][i];
                }
            }
        }
    }

    for (int b = 0; b < alfCov->numBins; b++)
    {
        for (int j = 0; j < alfCov->numCoeff; j++)
        {
            alfCov->y[b][j] = lhs->y[b][j] + rhs->y[b][j];
        }
    }

    alfCov->pixAcc = lhs->pixAcc + rhs->pixAcc;

}

void XinAlfCovReset (
    xin_alf_cov *alfCov,
    int         numBins)
{
    if (numBins > 0)
    {
        alfCov->numBins = numBins;
    }

    alfCov->pixAcc = 0;

    memset (alfCov->y, 0, sizeof(alfCov->y));
    memset (alfCov->E, 0, sizeof(alfCov->E));
}

void XinReduceClipCost (
    xin_alf_cov    *alfCov,
    SINT32         *clip)
{
    SINT32 k;
    BOOL   dec;
    SINT32 l;

    for (k = 0; k < alfCov->numCoeff-1; ++k)
    {
        dec = TRUE;

        while (dec && clip[k] > 0 && alfCov->y[clip[k] - 1][k] == alfCov->y[clip[k]][k])
        {
            for (l = 0; dec && l < alfCov->numCoeff; ++l)
            {
                if (alfCov->E[clip[k]][clip[l]][k][l] != alfCov->E[clip[k] - 1][clip[l]][k][l])
                {
                    dec = FALSE;
                }
            }

            if( dec )
            {
                --clip[k];
            }
        }
    }

}

void XinGetFrameStat (
    xin_alf_struct *alfSet,
    xin_alf_cov    *frameCov,
    xin_alf_cov    *ctbCov,
    UINT8          *ctbEnableFlags,
    UINT8          *ctbAltIdx,
    int            numClasses,
    int            altIdx)
{
    if (!ctbAltIdx)
    {
        for (int ctuIdx = 0; ctuIdx < (SINT32)alfSet->frameSizeInCtu; ctuIdx++)
        {
            if (ctbEnableFlags[ctuIdx])
            {
                for (int classIdx = 0; classIdx < numClasses; classIdx++)
                {
                    XinAlfCovAdd (
                        frameCov + classIdx,
                        ctbCov + ctuIdx*numClasses + classIdx,
                        frameCov + classIdx);
                }
            }
        }
    }
    else
    {
        for (int ctuIdx = 0; ctuIdx < (SINT32)alfSet->frameSizeInCtu; ctuIdx++)
        {
            if (ctbEnableFlags[ctuIdx]  && (altIdx == ctbAltIdx[ctuIdx]))
            {
                for (int classIdx = 0; classIdx < numClasses; classIdx++)
                {
                    XinAlfCovAdd (
                        frameCov + altIdx,
                        ctbCov + ctuIdx*numClasses + classIdx,
                        frameCov + altIdx);
                }
            }
        }
    }
}

void XinGetFrameStats (
    xin_alf_struct *alfSet,
    UINT32         channel)
{
    int numClasses = (channel == PLANE_LUMA) ? XIN_ALF_MAX_CLS_NUM : 1;
    int numAlternatives = (channel == PLANE_LUMA) ? 1 : alfSet->alfParamTemp.numAltChroma;
    int alfClipNum;

    if (channel == PLANE_LUMA)
    {
        alfClipNum = alfSet->useNonLinearAlfLuma ? XIN_ALF_CLIP_NUM : 1;
    }
    else
    {
        alfClipNum = alfSet->useNonLinearAlfChroma ? XIN_ALF_CLIP_NUM : 1;
    }

    // When calling this function m_ctuEnableFlag shall be set to 0 for CTUs using alternative APS
    // Here we compute frame stats for building new alternative filters
    for (int altIdx = 0; altIdx < numAlternatives; ++altIdx)
    {
        for (int i = 0; i < numClasses; i++)
        {
            XinAlfCovReset (
                channel == PLANE_LUMA ? alfSet->alfCovarianceFrameLuma + i : alfSet->alfCovarianceFrameChroma + altIdx,
                alfClipNum);

        }

        if (channel == PLANE_LUMA)
        {
            XinGetFrameStat (alfSet, alfSet->alfCovarianceFrameLuma, alfSet->alfCovarianceY, alfSet->ctuEnableFlag[PLANE_LUMA], NULL, numClasses, altIdx);
        }
        else
        {
            XinGetFrameStat (alfSet, alfSet->alfCovarianceFrameChroma, alfSet->alfCovarianceU, alfSet->ctuEnableFlag[PLANE_CHROMA_U], alfSet->ctuAlternative[PLANE_CHROMA_U], numClasses, altIdx);
            XinGetFrameStat (alfSet, alfSet->alfCovarianceFrameChroma, alfSet->alfCovarianceV, alfSet->ctuEnableFlag[PLANE_CHROMA_V], alfSet->ctuAlternative[PLANE_CHROMA_V], numClasses, altIdx);
        }
    }

}

static void XinGetAlfCtuFlagCtxIdx (
    UINT8   *ctuEnableFlag,
    UINT32  ctuAvialField,
    SINT32  frameWidthInCtu,
    UINT32  *ctxInc)
{

    *ctxInc = 0;

    if (ctuAvialField & XIN_LFT_CTU_AVAIL)
    {
        *ctxInc += ctuEnableFlag[-1];
    }

    if (ctuAvialField & XIN_TOP_CTU_AVAIL)
    {
        *ctxInc += ctuEnableFlag[-frameWidthInCtu];
    }

}

static void XinGetCcAlfFltCtrlIdcCtxIdx (
    UINT8   *filterControlIdc,
    UINT32  ctuAvialField,
    SINT32  frameWidthInCtu,
    SINT32  compId,
    SINT32  *ctxInc)
{

    *ctxInc = 0;

    if (ctuAvialField & XIN_LFT_CTU_AVAIL)
    {
        *ctxInc += ( filterControlIdc[- 1]) ? 1 : 0;
    }

    if (ctuAvialField & XIN_TOP_CTU_AVAIL)
    {
        *ctxInc += (filterControlIdc[-frameWidthInCtu]) ? 1 : 0;
    }

    *ctxInc += ( compId == PLANE_CHROMA_V ) ? 3 : 0;

}

void XinEstimateAlfCtuFlagFrame (
    xin_alf_struct *alfSet,
    UINT32         channel,
    BOOL           updateContext,
    UINT32         *bitNum)
{
    UINT32 numOfBit;
    UINT32 ctxInc;

    *bitNum = 0;

    if (channel == PLANE_LUMA)
    {
        if (alfSet->alfParamTemp.alfEnabled[PLANE_LUMA])
        {
            for (int i = 0; i < alfSet->frameSizeInCtu; i++)
            {
                XinGetAlfCtuFlagCtxIdx (
                    alfSet->ctuEnableFlag[PLANE_LUMA] + i,
                    alfSet->ctu[i].availField,
                    alfSet->frameWidthInCtu,
                    &ctxInc);

                Xin266EstimateAlfCtuFlag (
                    alfSet->context,
                    ctxInc,
                    PLANE_LUMA,
                    alfSet->ctuEnableFlag[PLANE_LUMA][i],
                    updateContext,
                    &numOfBit);

                *bitNum += numOfBit;
            }
        }

    }
    else
    {
        if (alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_U])
        {
            for (int i = 0; i < alfSet->frameSizeInCtu; i++)
            {
                XinGetAlfCtuFlagCtxIdx (
                    alfSet->ctuEnableFlag[PLANE_CHROMA_U] + i,
                    alfSet->ctu[i].availField,
                    alfSet->frameWidthInCtu,
                    &ctxInc);

                Xin266EstimateAlfCtuFlag (
                    alfSet->context,
                    ctxInc,
                    PLANE_CHROMA_U,
                    updateContext,
                    alfSet->ctuEnableFlag[PLANE_CHROMA_U][i],
                    &numOfBit);

                *bitNum += numOfBit;
            }
        }

        if (alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_V])
        {
            for (int i = 0; i < (SINT32)alfSet->frameSizeInCtu; i++)
            {
                XinGetAlfCtuFlagCtxIdx (
                    alfSet->ctuEnableFlag[PLANE_CHROMA_V] + i,
                    alfSet->ctu[i].availField,
                    alfSet->frameWidthInCtu,
                    &ctxInc);

                Xin266EstimateAlfCtuFlag (
                    alfSet->context,
                    ctxInc,
                    PLANE_CHROMA_V,
                    alfSet->ctuEnableFlag[PLANE_CHROMA_V][i],
                    updateContext,
                    &numOfBit);

                *bitNum += numOfBit;
            }
        }

    }

}

int XinGetNonFilterCoeffRate (
    xin_alf_param *alfParam)
{
    int filterNum = alfParam->numLumaFilters;
    int len = 2 + lengthUvlc(filterNum - 1);

    if (filterNum > 1)
    {
        const int coeffLength = calcLog2[filterNum] + ((filterNum & (filterNum - 1)) ? 1 : 0);

        for (int i = 0; i < XIN_ALF_MAX_CLS_NUM; i++)
        {
            len += coeffLength;                              // alf_luma_coeff_delta_idx   u(v)
        }
    }

    return len;
}

int XinGetCostFilterClipp (
    xin_alf_struct *alfSet,
    xin_alf_filter *alfFilter,
    int            pDiffQFilterCoeffIntPP[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    int            numFilters)
{
    for (int filterIdx = 0; filterIdx < numFilters; ++filterIdx)
    {
        for (int i = 0; i < alfFilter->numCoeff - 1; i++)
        {
            if (!abs(pDiffQFilterCoeffIntPP[filterIdx][i]))
            {
                alfSet->filterClippSet[filterIdx][i] = 0;
            }
        }
    }

    return (numFilters * (alfFilter->numCoeff - 1)) << 1;
}

static int XinLengthFilterCoeffs (
    xin_alf_filter *alfFilter,
    int            numFilters,
    int            FilterCoeff[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM])
{
    int bitCnt = 0;

    for (int ind = 0; ind < numFilters; ++ind)
    {
        for (int i = 0; i < alfFilter->numCoeff - 1; i++ )
        {
            bitCnt += lengthUvlc (abs (FilterCoeff[ind][i]));

            if (abs (FilterCoeff[ind][i]) != 0)
            {
                bitCnt += 1;
            }
        }
    }

    return bitCnt;

}

int XinGetCostFilterCoeff (
    xin_alf_filter *alfFilter,
    int            pDiffQFilterCoeffIntPP[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    int            numFilters)
{
    return XinLengthFilterCoeffs (alfFilter, numFilters, pDiffQFilterCoeffIntPP);  // alf_coeff_luma_delta[i][j];
}

static int XinDeriveFilterCoefficientsPredictionMode (
    xin_alf_struct *alfSet,
    xin_alf_filter *alfFilter,
    int            filterSet[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    int            **filterCoeffDiff,
    int            numFilters)
{
    (void)filterCoeffDiff;

    return (alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] ? XinGetCostFilterClipp (alfSet, alfFilter, filterSet, numFilters) : 0) + XinGetCostFilterCoeff (alfFilter, filterSet, numFilters);
}

int XinGetCostFilterCoeffForce0 (
    xin_alf_struct *alfSet,
    xin_alf_filter *alfFilter,
    int            pDiffQFilterCoeffIntPP[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    int            numFilters,
    UINT8          *codedVarBins)
{
    int len = 0;

    // Filter coefficients
    for( int ind = 0; ind < numFilters; ++ind )
    {
        if( codedVarBins[ind] )
        {
            for( int i = 0; i < alfFilter->numCoeff - 1; i++ )
            {
                len += lengthUvlc (abs(pDiffQFilterCoeffIntPP[ind][i])); // alf_coeff_luma_delta[i][j]
                if ((abs (pDiffQFilterCoeffIntPP[ind][i]) != 0))
                    len += 1;
            }
        }
        else
        {
            for (int i = 0; i < alfFilter->numCoeff - 1; i++)
            {
                len += lengthUvlc(0); // alf_coeff_luma_delta[i][j]
            }
        }
    }

    if (alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA])
    {
        for (int ind = 0; ind < numFilters; ++ind)
        {
            for (int i = 0; i < alfFilter->numCoeff - 1; i++)
            {
                if (!abs(pDiffQFilterCoeffIntPP[ind][i]))
                {
                    alfSet->filterClippSet[ind][i] = 0;
                }

                len += 2;
            }
        }
    }

    return len;

}

void XinGetClipMax (
    xin_alf_cov    *alfCov,
    SINT32         *clipMax)
{
    SINT32 k;
    SINT32 l;
    BOOL   inc;

    for (k = 0; k < alfCov->numCoeff-1; ++k)
    {
        clipMax[k] = 0;

        inc = TRUE;

        while (inc && clipMax[k]+1 < alfCov->numBins && alfCov->y[clipMax[k] + 1][k] == alfCov->y[clipMax[k]][k])
        {
            for (l = 0; inc && l < alfCov->numCoeff; ++l)
            {
                if( alfCov->E[clipMax[k]][0][k][l] != alfCov->E[clipMax[k] + 1][0][k][l] )
                {
                    inc = FALSE;
                }
            }

            if( inc )
            {
                ++clipMax[k];
            }
        }

    }

    clipMax[alfCov->numCoeff-1] = 0;

}

void XinSetEyFromClip(
    xin_alf_cov *alfCov,
    SINT32      *clip,
    FLOAT32     _E[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32     *_y,
    SINT32      size)
{
    SINT32 k;
    SINT32 l;

    for (k = 0; k < size; k++)
    {
        _y[k] = alfCov->y[clip[k]][k];

        for (l = 0; l < size; l++)
        {
            _E[k][l] = alfCov->E[clip[k]][clip[l]][k][l];
        }
    }

}

FLOAT32 XinCalculateError(
    xin_alf_cov *alfCov,
    SINT32      *clip,
    FLOAT32     *coeff,
    SINT32      numCoeff)
{

    FLOAT32 sum = 0;

    for( int i = 0; i < numCoeff; i++ )
    {
        sum += coeff[i] * alfCov->y[clip[i]][i];
    }

    return alfCov->pixAcc - sum;
}

FLOAT32 XinOptimizeFilter (
    xin_alf_cov    *alfCov,
    xin_alf_filter *alfFilter,
    SINT32         *clip,
    FLOAT32        *f,
    BOOL            optimizeClip)
{
    SINT32  size;
    SINT32  clipMax[XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 errBest, errLast;
    FLOAT32 kE[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 ky[XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32  k;
    SINT32  l;

    size = alfFilter->numCoeff;

    if (optimizeClip)
    {
        // Start by looking for min clipping that has no impact => max_clipping
        XinGetClipMax (alfCov, clipMax);

        for (k = 0; k < size; ++k)
        {
            clip[k] = XIN_MAX (clipMax[k], clip[k]);
            clip[k] = XIN_MIN (clip[k], alfCov->numBins-1);
        }
    }

    XinSetEyFromClip (alfCov, clip, kE, ky, size);

    gnsSolveByChol ( kE, ky, f, size );
    errBest = XinCalculateError (alfCov, clip, f, size );

    int step = optimizeClip ? (alfCov->numBins+1)/2 : 0;

    while( step > 0 )
    {
        FLOAT32 errMin = errBest;
        int idxMin = -1;
        int incMin = 0;

        for(k = 0; k < size-1; ++k )
        {
            if( clip[k] - step >= clipMax[k] )
            {
                clip[k] -= step;
                ky[k] = alfCov->y[clip[k]][k];
                for (l = 0; l < size; l++)
                {
                    kE[k][l] = alfCov->E[clip[k]][clip[l]][k][l];
                    kE[l][k] = alfCov->E[clip[l]][clip[k]][l][k];
                }

                gnsSolveByChol( kE, ky, f, size );
                errLast = XinCalculateError (alfCov, clip, f, size );

                if( errLast < errMin )
                {
                    errMin = errLast;
                    idxMin = k;
                    incMin = -step;
                }

                clip[k] += step;

            }

            if( clip[k] + step < alfCov->numBins )
            {
                clip[k] += step;
                ky[k] = alfCov->y[clip[k]][k];
                for ( l = 0; l < size; l++)
                {
                    kE[k][l] = alfCov->E[clip[k]][clip[l]][k][l];
                    kE[l][k] = alfCov->E[clip[l]][clip[k]][l][k];
                }

                gnsSolveByChol (kE, ky, f, size);
                errLast = XinCalculateError (alfCov, clip, f, size);

                if (errLast < errMin)
                {
                    errMin = errLast;
                    idxMin = k;
                    incMin = step;
                }

                clip[k] -= step;

            }

            ky[k] = alfCov->y[clip[k]][k];

            for (l = 0; l < size; l++)
            {
                kE[k][l] = alfCov->E[clip[k]][clip[l]][k][l];
                kE[l][k] = alfCov->E[clip[l]][clip[k]][l][k];
            }

        }

        if (idxMin >= 0)
        {
            errBest = errMin;

            clip[idxMin] += incMin;

            ky[idxMin] = alfCov->y[clip[idxMin]][idxMin];

            for (l = 0; l < size; l++ )
            {
                kE[idxMin][l] = alfCov->E[clip[idxMin]][clip[l]][idxMin][l];
                kE[l][idxMin] = alfCov->E[clip[l]][clip[idxMin]][l][idxMin];
            }
        }
        else
        {
            --step;
        }

    }

    if (optimizeClip)
    {
        // test all max
        for (k = 0; k < size-1; ++k)
        {
            clipMax[k] = 0;
        }

        FLOAT32 kEMax[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
        FLOAT32 kyMax[XIN_ALF_MAX_LUMA_COEF_NUM];

        XinSetEyFromClip (alfCov, clipMax, kEMax, kyMax, size);

        gnsSolveByChol (kEMax, kyMax, f, size);

        errLast = XinCalculateError (alfCov, clipMax, f, size);

        if (errLast < errBest)
        {
            errBest = errLast;

            for (k = 0; k < size; ++k)
            {
                clip[k] = clipMax[k];
            }
        }
        else
        {
            // update clip to reduce coding cost
            XinReduceClipCost (alfCov, clip);

            // update f with best solution
            gnsSolveByChol (kE, ky, f, size);
        }

    }

    return errBest;

}

FLOAT32 XinOptimizeFilterClip(
    xin_alf_cov    *alfCov,
    xin_alf_filter *alfFilter,
    SINT32         *clip)
{
    FLOAT32 f[XIN_ALF_MAX_LUMA_COEF_NUM];

    return XinOptimizeFilter (alfCov, alfFilter, clip, f, TRUE);
}

FLOAT32 XinOptimizeFilterNoClip(
    xin_alf_cov *alfCov,
    int         *clip,
    int         size)
{
    FLOAT32 c[XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 LHS[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 rhs[XIN_ALF_MAX_LUMA_COEF_NUM];

    XinSetEyFromClip (
        alfCov,
        clip,
        LHS,
        rhs,
        size);

    gnsSolveByChol (LHS, rhs, c, size);

    return XinCalculateError (alfCov, clip, c, alfCov->numCoeff);

}

void XinMergeClasses (
    xin_alf_struct *alfSet,
    xin_alf_filter *alfFilter,
    xin_alf_cov    *cov,
    xin_alf_cov    *covMerged,
    SINT32         clipMerged[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32         numClasses,
    SINT16         filterIndices[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_CLS_NUM])
{
    int       tmpClip          [XIN_ALF_MAX_LUMA_COEF_NUM];
    int       bestMergeClip    [XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32   err              [XIN_ALF_MAX_CLS_NUM];
    FLOAT32   bestMergeErr = MAX_DOUBLE;
    BOOL      availableClass   [XIN_ALF_MAX_CLS_NUM];
    uint8_t   indexList        [XIN_ALF_MAX_CLS_NUM];
    uint8_t   indexListTemp    [XIN_ALF_MAX_CLS_NUM];
    int numRemaining = numClasses;
    xin_alf_cov tmpCov;

    memset (filterIndices, 0, sizeof(SINT16) * XIN_ALF_MAX_CLS_NUM * XIN_ALF_MAX_CLS_NUM);

    for (int i = 0; i < numClasses; i++)
    {
        filterIndices[numRemaining - 1][i] = (SINT16)i;
        indexList[i] = (UINT8)i;
        availableClass[i] = TRUE;
        covMerged[i] = cov[i];
        covMerged[i].numBins = alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] ? XIN_ALF_CLIP_NUM : 1;
    }

    // Try merging different covariance matrices

    // temporal AlfCovariance structure is allocated as the last element in covMerged array, the size of covMerged is MAX_NUM_ALF_CLASSES + 1
    tmpCov = covMerged[XIN_ALF_MAX_CLS_NUM];
    tmpCov.numBins = alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] ? XIN_ALF_CLIP_NUM : 1;

    // init Clip
    for (int i = 0; i < numClasses; i++)
    {
        for (int k = 0; k < XIN_ALF_MAX_LUMA_COEF_NUM; k++)
        {
            clipMerged[numRemaining-1][i][k] = (alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] ? XIN_ALF_CLIP_NUM / 2 : 0);
        }

        if (alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA])
        {
            err[i] = XinOptimizeFilterClip (covMerged + i, alfFilter, clipMerged[numRemaining - 1][i]);
        }
        else
        {
            err[i] = XinOptimizeFilterNoClip (covMerged + i, clipMerged[numRemaining-1][i], covMerged[i].numCoeff);
        }
    }

    while( numRemaining >= 2 )
    {
        FLOAT32 errorMin = MAX_DOUBLE;
        int bestToMergeIdx1 = 0, bestToMergeIdx2 = 1;

        for( int i = 0; i < numClasses - 1; i++ )
        {
            if( availableClass[i] )
            {
                for( int j = i + 1; j < numClasses; j++ )
                {
                    if( availableClass[j] )
                    {
                        FLOAT32 error1 = err[i];
                        FLOAT32 error2 = err[j];

                        XinAlfCovAdd (
                            covMerged + i,
                            covMerged + j,
                            &tmpCov);

                        for( int l = 0; l < XIN_ALF_MAX_LUMA_COEF_NUM; ++l )
                        {
                            tmpClip[l] = (clipMerged[numRemaining-1][i][l] + clipMerged[numRemaining-1][j][l] + 1 ) >> 1;
                        }

                        FLOAT32 errorMerged = alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] ? XinOptimizeFilterClip(&tmpCov, alfFilter, tmpClip) : XinOptimizeFilterNoClip(&tmpCov, tmpClip, tmpCov.numCoeff);
                        FLOAT32 error = errorMerged - error1 - error2;

                        if( error < errorMin )
                        {
                            bestMergeErr = errorMerged;

                            memcpy (bestMergeClip, tmpClip, sizeof(bestMergeClip));

                            errorMin        = error;
                            bestToMergeIdx1 = i;
                            bestToMergeIdx2 = j;
                        }

                    }
                }
            }
        }

        XinAlfCovAdd (
            covMerged + bestToMergeIdx2,
            covMerged + bestToMergeIdx1,
            covMerged + bestToMergeIdx1);

        memcpy (clipMerged[numRemaining-2], clipMerged[numRemaining-1], sizeof(int[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM]));
        memcpy (clipMerged[numRemaining-2][bestToMergeIdx1], bestMergeClip, sizeof(bestMergeClip));

        err[bestToMergeIdx1] = bestMergeErr;
        availableClass[bestToMergeIdx2] = FALSE;

        for( int i = 0; i < numClasses; i++ )
        {
            if( indexList[i] == bestToMergeIdx2 )
            {
                indexList[i] = (UINT8)bestToMergeIdx1;
            }
        }

        numRemaining--;
        if (numRemaining <= numClasses)
        {
            memcpy (indexListTemp, indexList, sizeof(UINT8) * numClasses);

            BOOL exist = FALSE;
            int ind = 0;

            for (int j = 0; j < numClasses; j++)
            {
                exist = FALSE;
                for (int i = 0; i < numClasses; i++)
                {
                    if (indexListTemp[i] == j)
                    {
                        exist = TRUE;
                        break;
                    }
                }

                if (exist)
                {
                    for (int i = 0; i < numClasses; i++)
                    {
                        if (indexListTemp[i] == j)
                        {
                            filterIndices[numRemaining - 1][i] = (short)ind;
                            indexListTemp[i] = 0xff;
                        }
                    }

                    ind++;
                }

            }

        }

    }

}

void XinRoundFiltCoeff (
    int     *filterCoeffQuant,
    FLOAT32 *filterCoeff,
    int     numCoeff,
    int     factor)
{
    for( int i = 0; i < numCoeff; i++ )
    {
        int sign = filterCoeff[i] > 0 ? 1 : -1;
        filterCoeffQuant[i] = (int)( filterCoeff[i] * sign * factor + 0.5 ) * sign;
    }
}

FLOAT32 Xin266CalcCoeffError (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth)
{
    SINT32  i;
    SINT32  j;
    FLOAT32 sum;
    FLOAT32 error;

    FLOAT32 factor = (FLOAT32)1.0 / (FLOAT32)(1 << (bitDepth - 1));

    error = 0;

    for (i = 0; i < numCoeff; i++ )   //diagonal
    {
        sum = 0;
        for (j = i + 1; j < numCoeff; j++ )
        {
            // E[j][i] = E[i][j], sum will be multiplied by 2 later
            sum += E[clip[i]][clip[j]][i][j] * coeff[j];
        }

        error += ((E[clip[i]][clip[i]][i][i] * coeff[i] + sum * 2) * factor - 2 * y[clip[i]][i]) * coeff[i];
    }

    return error * factor;

}

FLOAT32 Xin266CalcErrorForCoeffsLin_13 (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth)
{
    // calculate in same order as SIMD to avoid rounding errors
    FLOAT32 error = 0;
    FLOAT32 vE1[4],vE5[4],vE9[4];
    FLOAT32 vSum[4][4];
    FLOAT32 verror[4];
    FLOAT32 factor = (FLOAT32)1.0 / (FLOAT32)(1 << (bitDepth - 1));
    int i,n;

    (void)clip;
    (void)numCoeff;

    // i= 0,1,2,3
    for (i=0; i<4; i++)
    {
        memset(vE1,0,sizeof(vE1));
        
        for (n=i; n<4; n++)
        {
            vE1[n]=E[0][0][i][n+1]*coeff[n+1];
        }
        for (n=0; n<4; n++)
        {
            vE5[n]=E[0][0][i][n+5]*coeff[n+5];
            vE9[n]=E[0][0][i][n+9]*coeff[n+9];
            vSum[i][n]=vE1[n]+vE5[n]+vE9[n];
        }
    }
    for (n=0; n<4; n++)
    {
        vSum[0][n]=vSum[n>>1][(n&1)<<1]+vSum[n>>1][((n&1)<<1)+1];
        vSum[2][n]=vSum[(n>>1)+2][(n&1)<<1]+vSum[(n>>1)+2][((n&1)<<1)+1];;
    }
    for (n=0; n<4; n++)
    {
        vSum[0][n]=vSum[(n>>1)<<1][(n&1)<<1]+vSum[(n>>1)<<1][((n&1)<<1)+1];
        vE1[n]=(((E[0][0][n][n] * coeff[n])+(vSum[0][n]*2)) *factor) - (y[0][n]*2);
        verror[n]=vE1[n]* coeff[n];
    }

    // i= 4,5,6,7
    for (i=0; i<4; i++)
    {
        memset(vE5,0,sizeof(vE5));
        for (n=i; n<4; n++)
        {
            vE5[n]=E[0][0][i+4][n+5]*coeff[n+5];
        }
        for (n=0; n<4; n++)
        {
            vE9[n]=E[0][0][i+4][n+9]*coeff[n+9];
            vSum[i][n]=vE5[n]+vE9[n];
        }
    }
    for (n=0; n<4; n++)
    {
        vSum[0][n]=vSum[n>>1][(n&1)<<1]+vSum[n>>1][((n&1)<<1)+1];
        vSum[2][n]=vSum[(n>>1)+2][(n&1)<<1]+vSum[(n>>1)+2][((n&1)<<1)+1];
    }
    for (n=0; n<4; n++)
    {
        vSum[0][n]=vSum[(n>>1)<<1][(n&1)<<1]+vSum[(n>>1)<<1][((n&1)<<1)+1];
        vE1[n]=(((E[0][0][n+4][n+4] * coeff[n+4])+(vSum[0][n]*2)) *factor) - (y[0][n+4]*2);
        verror[n]=verror[n] + vE1[n]* coeff[n+4];
    }

    // i= 8,9,10,11
    for (i=0; i<4; i++)
    {
        memset(vE9,0,sizeof(vE5));
        for (n=i; n<4; n++)
        {
            vE9[n]=E[0][0][i+8][n+9]*coeff[n+9];
        }
        for (n=0; n<4; n++)
        {
            vSum[i][n]=vE9[n];
        }
    }
    for (n=0; n<4; n++)
    {
        vSum[0][n]=vSum[n>>1][(n&1)<<1]+vSum[n>>1][((n&1)<<1)+1];
        vSum[2][n]=vSum[(n>>1)+2][(n&1)<<1]+vSum[(n>>1)+2][((n&1)<<1)+1];
    }
    for (n=0; n<4; n++)
    {
        vSum[0][n]=vSum[(n>>1)<<1][(n&1)<<1]+vSum[(n>>1)<<1][((n&1)<<1)+1];
        vE1[n]=(((E[0][0][n+8][n+8] * coeff[n+8])+(vSum[0][n]*2)) *factor) - (y[0][n+8]*2);
        verror[n]=verror[n] + vE1[n]* coeff[n+8];
    }
    verror[0]=verror[0]+verror[1];
    verror[2]=verror[2]+verror[3];
    error=verror[0]+verror[2];
    error += ( ( E[0][0][12][12] * coeff[12] ) * factor - 2 * y[0][12] ) * coeff[12];
    return error * factor;
}

FLOAT32 Xin266CalcCoeffError_Lin (
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
        error = Xin266CalcErrorForCoeffsLin_13 (
                    clip,
                    coeff,
                    E,
                    y,
                    numCoeff,
                    bitDepth);
        
    }

    return error;
    
}

FLOAT32 XinDeriveCoeffQuant(
    xin_alf_struct *alfSet,
    int            *filterClipp,
    int            *filterCoeffQuant,
    xin_alf_cov    *cov,
    xin_alf_filter *alfFilter,
    int            bitDepth,
    BOOL           optimizeClip)
{
    const int factor = 1 << ( bitDepth - 1 );
    const int max_value = factor - 1;
    const int min_value = -factor + 1;

    const int numCoeff = alfFilter->numCoeff;
    FLOAT32 filterCoeff[XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 errRef;
    FLOAT32 error;
    xin_alf_func *funcSet;

    funcSet = alfSet->funcSet;

    XinOptimizeFilter (cov, alfFilter, filterClipp, filterCoeff, optimizeClip);
    XinRoundFiltCoeff (filterCoeffQuant, filterCoeff, numCoeff, factor);

    for ( int i = 0; i < numCoeff - 1; i++ )
    {
        filterCoeffQuant[i] = XIN_MIN ( max_value, XIN_MAX ( min_value, filterCoeffQuant[i] ) );
    }

    filterCoeffQuant[numCoeff - 1] = 0;

    int modified=1;

    errRef = funcSet->pfXinCalcCoeffError (filterClipp, filterCoeffQuant, cov->E, cov->y, numCoeff, bitDepth);

    while (modified)
    {
        modified = 0;

        for (int sign = 1; sign >= -1; sign -= 2)
        {
            FLOAT32 errMin = MAX_DOUBLE;
            int minInd = -1;

            for( int k = 0; k < numCoeff-1; k++ )
            {
                if (filterCoeffQuant[k] - sign > max_value || filterCoeffQuant[k] - sign < min_value)
                {
                    continue;
                }

                filterCoeffQuant[k] -= sign;

                error = funcSet->pfXinCalcCoeffError (filterClipp, filterCoeffQuant, cov->E, cov->y, numCoeff, bitDepth);

                if( error < errMin )
                {
                    errMin = error;
                    minInd = k;
                }

                filterCoeffQuant[k] += sign;
            }

            if (errMin < errRef)
            {
                filterCoeffQuant[minInd] -= sign;
                modified++;
                errRef = errMin;
            }
        }
    }

    return errRef;

}

FLOAT32 XinDeriveFilterCoeffs(
    xin_alf_struct *alfSet,
    xin_alf_cov    *cov,
    xin_alf_cov    *covMerged,
    int            clipMerged[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    xin_alf_filter *alfFilter,
    short          *filterIndices,
    int            numFilters,
    FLOAT32        errorTabForce0Coeff[XIN_ALF_MAX_CLS_NUM][2],
    xin_alf_param  *alfParam)
{
    FLOAT32 error = 0.0;
    xin_alf_cov tmpCov = covMerged[XIN_ALF_MAX_CLS_NUM];

    (void)alfParam;

    for( int filtIdx = 0; filtIdx < numFilters; filtIdx++ )
    {
        XinAlfCovReset (
            &tmpCov,
            -1);

        BOOL found_clip = FALSE;

        for (int classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
        {
            if (filterIndices[classIdx] == filtIdx)
            {
                XinAlfCovAdd (
                    &cov[classIdx],
                    &tmpCov,
                    &tmpCov);

                if (!found_clip)
                {
                    found_clip = TRUE; // clip should be at the adress of shortest one
                    memcpy (alfSet->filterClippSet[filtIdx], clipMerged[numFilters-1][classIdx], sizeof(int[XIN_ALF_MAX_LUMA_COEF_NUM]));
                }
            }
        }

        // Find coeffcients
        //assert(alfFilter->numCoeff == tmpCov.numCoeff);
        errorTabForce0Coeff[filtIdx][1] = tmpCov.pixAcc + XinDeriveCoeffQuant (alfSet, alfSet->filterClippSet[filtIdx], alfSet->filterCoeffSet[filtIdx], &tmpCov, alfFilter, XIN_8_BIT_DEPTH, FALSE);
        errorTabForce0Coeff[filtIdx][0] = tmpCov.pixAcc;
        error += errorTabForce0Coeff[filtIdx][1];

    }

    return error;

}

FLOAT32 XinGetDistCoeffForce0(
    xin_alf_struct *alfSet,
    UINT8          *codedVarBins,
    FLOAT32        errorForce0CoeffTab[XIN_ALF_MAX_CLS_NUM][2],
    int            *bitsVarBin,
    int            zeroBitsVarBin,
    int            numFilters)
{
    FLOAT32 distForce0 = 0;

    memset (codedVarBins, 0, sizeof(UINT8)*XIN_ALF_MAX_CLS_NUM);

    for (int filtIdx = 0; filtIdx < numFilters; filtIdx++)
    {
        FLOAT32 costDiff = (errorForce0CoeffTab[filtIdx][0] + alfSet->lambda[PLANE_LUMA] * zeroBitsVarBin) - (errorForce0CoeffTab[filtIdx][1] + alfSet->lambda[PLANE_LUMA] * bitsVarBin[filtIdx]);
        codedVarBins[filtIdx] = costDiff > 0 ? TRUE : FALSE;
        distForce0 += errorForce0CoeffTab[filtIdx][codedVarBins[filtIdx] ? 1 : 0];
    }

    return distForce0;

}

FLOAT32 XinGetDistForce0 (
    xin_alf_struct *alfSet,
    xin_alf_filter *alfFilter,
    int            numFilters,
    FLOAT32        errorTabForce0Coeff[XIN_ALF_MAX_CLS_NUM][2],
    UINT8          *codedVarBins)
{
    int bitsVarBin[XIN_ALF_MAX_CLS_NUM];

    for (int ind = 0; ind < numFilters; ++ind)
    {
        bitsVarBin[ind] = 0;

        for (int i = 0; i < alfFilter->numCoeff - 1; i++)
        {
            bitsVarBin[ ind ] += lengthUvlc (abs(alfSet->filterCoeffSet[ind][i]));

            if( abs(alfSet->filterCoeffSet[ind][i]) != 0)
            {
                bitsVarBin[ ind ] += 1;
            }
        }
    }

    int zeroBitsVarBin = 0;

    for (int i = 0; i < alfFilter->numCoeff - 1; i++)
    {
        zeroBitsVarBin += lengthUvlc (0);
    }

    if (alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA])
    {
        for (int ind = 0; ind < numFilters; ++ind)
        {
            for (int i = 0; i < alfFilter->numCoeff - 1; i++)
            {
                if (!abs(alfSet->filterCoeffSet[ind][i]))
                {
                    alfSet->filterClippSet[ind][i] = 0;
                }
            }
        }
    }

    FLOAT32 distForce0 = XinGetDistCoeffForce0(alfSet, codedVarBins, errorTabForce0Coeff, bitsVarBin, zeroBitsVarBin, numFilters);

    return distForce0;

}

FLOAT32 XinMergeFiltersAndCost(
    xin_alf_struct *alfSet,
    xin_alf_param  *alfParam,
    xin_alf_filter *alfFilter,
    xin_alf_cov    *covFrame,
    xin_alf_cov    *covMerged,
    int            clipMerged[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    int            *uiCoeffBits)
{
    int numFiltersBest = 0;
    int numFilters = XIN_ALF_MAX_CLS_NUM;
    UINT8 codedVarBins[XIN_ALF_MAX_CLS_NUM];
    FLOAT32 errorForce0CoeffTab[XIN_ALF_MAX_CLS_NUM][2];

    FLOAT32 cost, cost0, dist, distForce0, costMin = MAX_DOUBLE;
    int coeffBits, coeffBitsForce0;

    XinMergeClasses (alfSet, alfFilter, covFrame, covMerged, clipMerged, XIN_ALF_MAX_CLS_NUM, alfSet->filterIndices);

    while( numFilters >= 1 )
    {
        // filter coeffs are stored in m_filterCoeffSet
        dist      = XinDeriveFilterCoeffs (alfSet, covFrame, covMerged, clipMerged, alfFilter, alfSet->filterIndices[numFilters - 1], numFilters, errorForce0CoeffTab, alfParam);
        coeffBits = XinDeriveFilterCoefficientsPredictionMode (alfSet, alfFilter, alfSet->filterCoeffSet, alfSet->diffFilterCoeff, numFilters);
        distForce0      = XinGetDistForce0 (alfSet, alfFilter, numFilters, errorForce0CoeffTab, codedVarBins);
        coeffBitsForce0 = XinGetCostFilterCoeffForce0 (alfSet, alfFilter, alfSet->filterCoeffSet, numFilters, codedVarBins);

        cost = dist + alfSet->lambda[PLANE_LUMA] * coeffBits;
        cost0 = distForce0 + alfSet->lambda[PLANE_LUMA] * coeffBitsForce0;

        if( cost0 < cost )
        {
            cost = cost0;
        }

        if( cost <= costMin )
        {
            costMin = cost;
            numFiltersBest = numFilters;
        }

        numFilters--;

    }

    dist      = XinDeriveFilterCoeffs (alfSet, covFrame, covMerged, clipMerged, alfFilter, alfSet->filterIndices[numFiltersBest - 1], numFiltersBest, errorForce0CoeffTab, alfParam);
    coeffBits = XinDeriveFilterCoefficientsPredictionMode (alfSet, alfFilter, alfSet->filterCoeffSet, alfSet->diffFilterCoeff, numFiltersBest);

    distForce0      = XinGetDistForce0 (alfSet, alfFilter, numFiltersBest, errorForce0CoeffTab, codedVarBins);
    coeffBitsForce0 = XinGetCostFilterCoeffForce0 (alfSet, alfFilter, alfSet->filterCoeffSet, numFiltersBest, codedVarBins);

    cost = dist + alfSet->lambda[PLANE_LUMA] * coeffBits;
    cost0 = distForce0 + alfSet->lambda[PLANE_LUMA] * coeffBitsForce0;

    alfParam->numLumaFilters = numFiltersBest;

    FLOAT32 distReturn;

    if (cost <= cost0)
    {
        distReturn = dist;
        alfParam->alfLumaCoeffDeltaFlag = 0;
        *uiCoeffBits = coeffBits;
    }
    else
    {
        distReturn = distForce0;
        alfParam->alfLumaCoeffDeltaFlag = 1;
        *uiCoeffBits = coeffBitsForce0;

        memcpy(alfParam->alfLumaCoeffFlag, codedVarBins, XIN_ALF_MAX_CLS_NUM*sizeof(UINT8));

        for (int varInd = 0; varInd < numFiltersBest; varInd++)
        {
            if (codedVarBins[varInd] == 0)
            {
                memset (alfSet->filterCoeffSet[varInd], 0, sizeof(int)*XIN_ALF_MAX_LUMA_COEF_NUM);
                memset (alfSet->filterClippSet[varInd], 0, sizeof(int)*XIN_ALF_MAX_LUMA_COEF_NUM);
            }
        }
    }

    for (int ind = 0; ind < alfParam->numLumaFilters; ++ind)
    {
        for (int i = 0; i < alfFilter->numCoeff; i++)
        {
            alfParam->lumaCoeff[ind * XIN_ALF_MAX_LUMA_COEF_NUM + i] = (SINT16)alfSet->filterCoeffSet[ind][i];
            alfParam->lumaClipp[ind * XIN_ALF_MAX_LUMA_COEF_NUM + i] = (SINT16)alfSet->filterClippSet[ind][i];
        }
    }

    memcpy (alfParam->filterCoeffDeltaIdx, alfSet->filterIndices[numFiltersBest - 1], sizeof(short)*XIN_ALF_MAX_CLS_NUM);

    *uiCoeffBits += XinGetNonFilterCoeffRate (alfParam);

    return distReturn;

}

int XinGetChromaCoeffRate (
    xin_alf_struct *alfSet,
    xin_alf_param  *alfParam,
    int            altIdx)
{
    int iBits = 0;

    xin_alf_filter alfFilter;

    alfFilter.numCoeff = 7;
    alfFilter.filterType = XIN_ALF_FILTER_5;

    // Filter coefficients
    for( int i = 0; i < alfFilter.numCoeff - 1; i++ )
    {
        iBits += lengthUvlc( abs( alfParam->chromaCoeff[ altIdx ][ i ] ) );  // alf_coeff_chroma[altIdx][i]
        if( ( alfParam->chromaCoeff[ altIdx ][ i ] ) != 0 )
            iBits += 1;
    }

    if (alfSet->alfParamTemp.nonLinearFlag[PLANE_CHROMA])
    {
        for (int i = 0; i < alfFilter.numCoeff - 1; i++)
        {
            if( !abs( alfParam->chromaCoeff[altIdx][i] ) )
            {
                alfParam->chromaClipp[altIdx][i] = 0;
            }
        }
        iBits += ((alfFilter.numCoeff - 1) << 1);
    }

    return iBits;

}

FLOAT32 XinGetFilterCoeffAndCost (
    xin_alf_struct *alfSet,
    FLOAT32        distUnfilter,
    UINT32         channel,
    BOOL           bReCollectStat,
    int            *uiCoeffBits,
    BOOL           onlyFilterCost)
{
    UINT32 numOfBit;

    //collect stat based on CTU decision
    if( bReCollectStat )
    {
        XinGetFrameStats (
            alfSet,
            channel);
    }

    FLOAT32 dist = distUnfilter;
    *uiCoeffBits = 0;
    xin_alf_filter *alfFilter = alfSet->alfParamTemp.alfFilter[channel];

    //get filter coeff
    if (channel == PLANE_LUMA)
    {
        if (alfSet->alfParamTemp.nonLinearFlag[channel])
        {
            for (int j = 0; j < XIN_ALF_MAX_CLS_NUM; j++)
            {
                for (int k = 0; k < XIN_ALF_MAX_CLS_NUM; k++)
                {
                    for (int i = 0; i < XIN_ALF_MAX_LUMA_COEF_NUM; i++)
                    {
                        alfSet->alfClipMerged[j][k][i] = XIN_ALF_CLIP_NUM / 2;
                    }
                }
            }
        }
        else
        {
            for (int j = 0; j < XIN_ALF_MAX_CLS_NUM; j++)
            {
                for (int k = 0; k < XIN_ALF_MAX_CLS_NUM; k++)
                {
                    for (int i = 0; i < XIN_ALF_MAX_LUMA_COEF_NUM; i++)
                    {
                        alfSet->alfClipMerged[j][k][i] = 0;
                    }
                }
            }
        }

        // Reset Merge Tmp Cov
        XinAlfCovReset (
            &alfSet->alfCovarianceMerged[XIN_ALF_MAX_CLS_NUM],
            alfSet->useNonLinearAlfLuma ? XIN_ALF_CLIP_NUM : 1);

        XinAlfCovReset (
            &alfSet->alfCovarianceMerged[XIN_ALF_MAX_CLS_NUM+1],
            alfSet->useNonLinearAlfChroma ? XIN_ALF_CLIP_NUM : 1);

        //distortion
        dist += XinMergeFiltersAndCost (alfSet, &alfSet->alfParamTemp, alfFilter, alfSet->alfCovarianceFrameLuma, alfSet->alfCovarianceMerged, alfSet->alfClipMerged, uiCoeffBits);

    }
    else
    {
        //distortion
        for( int altIdx = 0; altIdx < alfSet->alfParamTemp.numAltChroma; ++altIdx )
        {
            xin_alf_param bestSliceParam;
            FLOAT32 bestCost = MAX_DOUBLE;
            FLOAT32 bestDist = MAX_DOUBLE;
            int bestCoeffBits = 0;
            const int nonLinearFlagMax = alfSet->useNonLinearAlfChroma ? 2 : 1;

            memset(&bestSliceParam, 0, sizeof(xin_alf_param));

            for( int nonLinearFlag = 0; nonLinearFlag < nonLinearFlagMax; nonLinearFlag++ )
            {
                int currentNonLinearFlag = alfSet->alfParamTemp.nonLinearFlag[channel] ? 1 : 0;

                if (nonLinearFlag != currentNonLinearFlag)
                {
                    continue;
                }

                if (nonLinearFlag)
                {
                    for ( int i = 0; i < XIN_ALF_MAX_CHROMA_COEF_NUM; i++ )
                    {
                        alfSet->filterClippSet[altIdx][i] = XIN_ALF_CLIP_NUM/2;
                    }
                }
                else
                {
                    for ( int i = 0; i < XIN_ALF_MAX_CHROMA_COEF_NUM; i++ )
                    {
                        alfSet->filterClippSet[altIdx][i] = 0;
                    }
                }

                FLOAT32 chromaDist = alfSet->alfCovarianceFrameChroma[altIdx].pixAcc + XinDeriveCoeffQuant(alfSet, alfSet->filterClippSet[altIdx], alfSet->filterCoeffSet[altIdx], &alfSet->alfCovarianceFrameChroma[altIdx], alfFilter, XIN_8_BIT_DEPTH, nonLinearFlag);
                for( int i = 0; i < XIN_ALF_MAX_CHROMA_COEF_NUM; i++ )
                {
                    alfSet->alfParamTemp.chromaCoeff[altIdx][i] = (SINT16)alfSet->filterCoeffSet[altIdx][i];
                    alfSet->alfParamTemp.chromaClipp[altIdx][i] = (SINT16)alfSet->filterClippSet[altIdx][i];
                }
                int coeffBits = XinGetChromaCoeffRate (alfSet, &alfSet->alfParamTemp, altIdx );
                FLOAT32 cost = chromaDist + alfSet->lambda[channel] * coeffBits;

                if (cost < bestCost)
                {
                    bestCost = cost;
                    bestDist = chromaDist;
                    bestCoeffBits = coeffBits;
                    bestSliceParam = alfSet->alfParamTemp;
                }

            }

            *uiCoeffBits += bestCoeffBits;
            dist += bestDist;
            alfSet->alfParamTemp = bestSliceParam;

        }

        *uiCoeffBits += lengthUvlc (alfSet->alfParamTemp.numAltChroma-1);
        *uiCoeffBits += 1;

    }

    if (onlyFilterCost)
    {
        return dist + alfSet->lambda[channel] * *uiCoeffBits;
    }

    FLOAT32 rate = (FLOAT32)(*uiCoeffBits);

    XinEstimateAlfCtuFlagFrame (
        alfSet,
        channel,
        TRUE,
        &numOfBit);

    rate += XIN_FRAC_BITS_SCALE * numOfBit;

    if (channel == PLANE_LUMA)
    {
        for (int ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
        {
            if (alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx])
            {
                Xin266EstimateAlfCtuFilterIdx (
                    alfSet->context,
                    alfSet->alfAps,
                    alfSet->apsNum,
                    alfSet->alfCtbFilterSetIndex[ctuIdx],
                    TRUE,
                    &numOfBit);

                rate += XIN_FRAC_BITS_SCALE * numOfBit;
            }
        }
    }
    else
    {
        for (int ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
        {
            if (alfSet->ctuEnableFlag[PLANE_CHROMA_U][ctuIdx])
            {
                Xin266EstimateAlfCtuAlt (
                    alfSet->context,
                    alfSet->ctuAlternative[PLANE_CHROMA_U][ctuIdx],
                    alfSet->alfParamTemp.numAltChroma,
                    PLANE_CHROMA_U,
                    TRUE,
                    &numOfBit);

                rate += XIN_FRAC_BITS_SCALE * numOfBit;
            }
        }

        for (int ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
        {
            if (alfSet->ctuEnableFlag[PLANE_CHROMA_V][ctuIdx])
            {
                Xin266EstimateAlfCtuAlt (
                    alfSet->context,
                    alfSet->ctuAlternative[PLANE_CHROMA_V][ctuIdx],
                    alfSet->alfParamTemp.numAltChroma,
                    PLANE_CHROMA_V,
                    TRUE,
                    &numOfBit);

                rate += XIN_FRAC_BITS_SCALE * numOfBit;
            }

        }

    }


    return dist + alfSet->lambda[channel] * rate;

}

FLOAT32 XinGetUnfilteredDist (
    xin_alf_cov *cov,
    SINT32      numClasses)
{
    FLOAT32 dist = 0;

    for (int classIdx = 0; classIdx < numClasses; classIdx++)
    {
        dist += cov[classIdx].pixAcc;
    }

    return dist;
}

void XinCopyAlfParam (
    xin_alf_param *alfParamDst,
    xin_alf_param *alfParamSrc,
    UINT32        channel)
{
    if (channel == PLANE_LUMA)
    {
        memcpy (alfParamDst, alfParamSrc, sizeof(xin_alf_param));
    }
    else
    {
        alfParamDst->alfEnabled[PLANE_CHROMA_U] = alfParamSrc->alfEnabled[PLANE_CHROMA_U];
        alfParamDst->alfEnabled[PLANE_CHROMA_V] = alfParamSrc->alfEnabled[PLANE_CHROMA_V];
        alfParamDst->numAltChroma = alfParamSrc->numAltChroma;
        alfParamDst->nonLinearFlag[PLANE_CHROMA] = alfParamSrc->nonLinearFlag[PLANE_CHROMA];
        memcpy( alfParamDst->chromaCoeff, alfParamSrc->chromaCoeff, sizeof( alfParamDst->chromaCoeff ) );
        memcpy( alfParamDst->chromaClipp, alfParamSrc->chromaClipp, sizeof( alfParamDst->chromaClipp ) );
    }
}

void XinCopyCtuAlternativeChroma (
    UINT8  *ctuAltsDst[PLANE_NUM],
    UINT8  *ctuAltsSrc[PLANE_NUM],
    UINT32 numCTUsInPic)
{
    memcpy (ctuAltsDst[PLANE_CHROMA_U], ctuAltsSrc[PLANE_CHROMA_U], numCTUsInPic*sizeof(UINT8));
    memcpy (ctuAltsDst[PLANE_CHROMA_V], ctuAltsSrc[PLANE_CHROMA_V], numCTUsInPic*sizeof(UINT8));
}

void XinSetCtuEnableFlag (
    UINT8  **ctuFlags,
    UINT32 channel,
    UINT8  val,
    UINT32 numCTUsInPic)
{
    if (channel == PLANE_LUMA)
    {
        memset( ctuFlags[PLANE_LUMA],     val, sizeof( UINT8 ) * numCTUsInPic );
    }
    else
    {
        memset( ctuFlags[PLANE_CHROMA_U], val, sizeof( UINT8 ) * numCTUsInPic );
        memset( ctuFlags[PLANE_CHROMA_V], val, sizeof( UINT8 ) * numCTUsInPic );
    }
}

void XinCopyCtuEnableFlag (
    UINT8   **ctuFlagsDst,
    UINT8   **ctuFlagsSrc,
    UINT32  channel,
    UINT32  numCTUsInPic)
{
    if (channel == PLANE_LUMA)
    {
        memcpy (ctuFlagsDst[PLANE_LUMA], ctuFlagsSrc[PLANE_LUMA], sizeof(UINT8) * numCTUsInPic);
    }
    else
    {
        memcpy (ctuFlagsDst[PLANE_CHROMA_U], ctuFlagsSrc[PLANE_CHROMA_U], sizeof(UINT8) * numCTUsInPic);
        memcpy (ctuFlagsDst[PLANE_CHROMA_V], ctuFlagsSrc[PLANE_CHROMA_V], sizeof(UINT8) * numCTUsInPic);
    }
}

void XinSetCtuAlternativeChroma (
    UINT8  *ctuAlts[PLANE_NUM],
    UINT8  val,
    UINT32 numCTUsInPic)
{
    memset (ctuAlts[PLANE_CHROMA_U], numCTUsInPic, val);
    memset (ctuAlts[PLANE_CHROMA_V], numCTUsInPic, val);
}

int XinGetMaxNumAlternativesChroma (
    xin_alf_struct *alfSet)
{
    return XIN_MIN(alfSet->frameSizeInCtu * 2, alfSet->maxNumAlfAltChroma);
}

FLOAT32 XinGetFilteredDist(
    xin_alf_struct *alfSet,
    xin_alf_cov    *cov,
    SINT32         fltClipSet[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32         fltCoeffSet[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32         numClasses,
    SINT32         numFiltersMinus1,
    SINT32         numCoeff)
{
    FLOAT32      dist;
    SINT32       classIdx;
    xin_alf_func *funcSet;

    (void)numFiltersMinus1;

    funcSet = alfSet->funcSet;
    dist    = 0;

    for (classIdx = 0; classIdx < numClasses; classIdx++)
    {
        dist += funcSet->pfXinCalcCoeffError (fltClipSet[classIdx], fltCoeffSet[classIdx], cov[classIdx].E, cov[classIdx].y, numCoeff, XIN_8_BIT_DEPTH);
    }

    return dist;

}

void XinReconstructCoeff (
    xin_alf_struct *alfSet,
    xin_alf_param  *alfParam,
    UINT32         channel,
    BOOL           isRdo)
{
    int factor               = isRdo ? 0 : (1 << (XIN_8_BIT_DEPTH - 1));
    xin_alf_shape filterShape = (channel == PLANE_LUMA) ? XIN_ALF_FILTER_7 : XIN_ALF_FILTER_5;
    int numClasses           = (channel == PLANE_LUMA) ? XIN_ALF_MAX_CLS_NUM : 1;
    int numCoeff             = filterShape == XIN_ALF_FILTER_5 ? 7 : 13;
    int numCoeffMinus1       = numCoeff - 1;
    int numAlts = (channel == PLANE_LUMA) ? 1 : alfParam->numAltChroma;

    for( int altIdx = 0; altIdx < numAlts; ++ altIdx )
    {
        int numFilters = (channel == PLANE_LUMA) ? alfParam->numLumaFilters : 1;
        short* coeff = (channel == PLANE_LUMA) ? alfParam->lumaCoeff : alfParam->chromaCoeff[altIdx];
        short* clipp = (channel == PLANE_LUMA) ? alfParam->lumaClipp : alfParam->chromaClipp[altIdx];

        for( int filterIdx = 0; filterIdx < numFilters; filterIdx++ )
        {
            coeff[filterIdx* XIN_ALF_MAX_LUMA_COEF_NUM + numCoeffMinus1] = (short)factor;
        }

        if (channel != PLANE_LUMA)
        {
            for( int coeffIdx = 0; coeffIdx < numCoeffMinus1; ++coeffIdx )
            {
                alfSet->chromaCoeffFinal[altIdx][coeffIdx] = coeff[coeffIdx];
                int clipIdx = alfParam->nonLinearFlag[channel] ? clipp[coeffIdx] : 0;
                alfSet->chromaClippFinal[altIdx][coeffIdx] = isRdo ? (SINT16)clipIdx : alfSet->alfClippingValues[channel][clipIdx];
            }

            alfSet->chromaCoeffFinal[altIdx][numCoeffMinus1] = (SINT16)factor;
            alfSet->chromaClippFinal[altIdx][numCoeffMinus1] = isRdo ? 0 : alfSet->alfClippingValues[channel][0];

            continue;
        }

        for (int classIdx = 0; classIdx < numClasses; classIdx++ )
        {
            int filterIdx = alfParam->filterCoeffDeltaIdx[classIdx];

            for (int coeffIdx = 0; coeffIdx < numCoeffMinus1; ++coeffIdx)
            {
                alfSet->coeffFinal[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx] = coeff[filterIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx];
            }

            alfSet->coeffFinal[classIdx* XIN_ALF_MAX_LUMA_COEF_NUM + numCoeffMinus1] = (SINT16)factor;
            alfSet->clippFinal[classIdx* XIN_ALF_MAX_LUMA_COEF_NUM + numCoeffMinus1] = isRdo ? 0 : alfSet->alfClippingValues[channel][0];

            for (int coeffIdx = 0; coeffIdx < numCoeffMinus1; ++coeffIdx )
            {
                int clipIdx = alfParam->nonLinearFlag[channel] ? clipp[filterIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx] : 0;

                alfSet->clippFinal[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx] = isRdo ? (SINT16)clipIdx : alfSet->alfClippingValues[channel][clipIdx];
            }

            alfSet->clippFinal[classIdx* XIN_ALF_MAX_LUMA_COEF_NUM + numCoeffMinus1] =
                isRdo ? 0 :
                alfSet->alfClippingValues[channel][0];

        }

    }

}

void XinSetEnableFlag (
    xin_alf_struct *alfSet,
    xin_alf_param  *alfParam,
    UINT32         channel,
    UINT8          **ctuFlags)
{
    SINT32 compIDFirst = (channel == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA_U;
    SINT32 compIDLast = (channel == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA_V;

    for( int compId = compIDFirst; compId <= compIDLast; compId++ )
    {
        alfParam->alfEnabled[compId] = FALSE;

        for (int i = 0; i < (SINT32)alfSet->frameSizeInCtu; i++)
        {
            if (ctuFlags[compId][i])
            {
                alfParam->alfEnabled[compId] = TRUE;
                break;
            }
        }
    }

}

FLOAT32 XinDeriveCtbAlfEnableFlags(
    xin_alf_struct *alfSet,
    int            iShapeIdx,
    UINT32         channel,
    int            numClasses,
    int            numCoeff,
    FLOAT32        *distUnfilter)
{
    SINT32 compIDFirst = (channel == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA_U;
    SINT32 compIDLast = (channel == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA_V;
    const int numAlts = (channel == PLANE_LUMA) ? 1 : alfSet->alfParamTemp.numAltChroma;
    FLOAT32 cost = 0;
    *distUnfilter = 0;
    UINT32  numOfBit;
    UINT32  totalBits;
    xin_alf_cov *alfCov;
    UINT32  ctxInc;
    xin_alf_func *funcSet;

    (void)iShapeIdx;

    funcSet = alfSet->funcSet;

    if (channel == PLANE_LUMA)
    {
        alfSet->alfParamTemp.alfEnabled[PLANE_LUMA] = TRUE;
    }
    else
    {
        alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_U] = alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_V] = TRUE;
    }

    XinReconstructCoeff (
        alfSet,
        &alfSet->alfParamTemp,
        channel,
        TRUE);

    for( int altIdx = 0; altIdx < ((channel == PLANE_LUMA) ? 1 : XIN_ALF_MAX_CHROMA_ALT_NUM); altIdx++)
    {
        for (int classIdx = 0; classIdx < ((channel == PLANE_LUMA) ? XIN_ALF_MAX_CLS_NUM : 1); classIdx++)
        {
            for (int i = 0; i < ((channel == PLANE_LUMA) ? XIN_ALF_MAX_LUMA_COEF_NUM : XIN_ALF_MAX_CHROMA_COEF_NUM); i++)
            {
                alfSet->filterCoeffSet[(channel == PLANE_LUMA) ? classIdx : altIdx][i] = (channel == PLANE_LUMA) ? alfSet->coeffFinal[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + i] : alfSet->chromaCoeffFinal[altIdx][i];
                alfSet->filterClippSet[(channel == PLANE_LUMA) ? classIdx : altIdx][i] = (channel == PLANE_LUMA) ? alfSet->clippFinal[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + i] : alfSet->chromaClippFinal[altIdx][i];
            }
        }
    }

    for (int ctuIdx = 0; ctuIdx < (SINT32)alfSet->frameSizeInCtu; ctuIdx++)
    {
        for (int compID = compIDFirst; compID <= compIDLast; compID++)
        {
            alfCov = (compID == PLANE_LUMA ? alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM :
                      (compID == PLANE_CHROMA_U ? alfSet->alfCovarianceU + ctuIdx : alfSet->alfCovarianceV + ctuIdx));

            const FLOAT32 ctuLambda = alfSet->lambda[channel];
            FLOAT32 distUnfilterCtu = XinGetUnfilteredDist(alfCov, numClasses);

            alfSet->ctuEnableFlag[compID][ctuIdx] = 1;

            XinGetAlfCtuFlagCtxIdx (
                alfSet->ctuEnableFlag[compID] + ctuIdx,
                alfSet->ctu[ctuIdx].availField,
                alfSet->frameWidthInCtu,
                &ctxInc);

            Xin266EstimateAlfCtuFlag (
                alfSet->context,
                ctxInc,
                compID,
                TRUE,
                FALSE,
                &numOfBit);

            totalBits = numOfBit;

            if (channel == PLANE_LUMA)
            {
                Xin266EstimateAlfCtuFilterIdx (
                    alfSet->context,
                    alfSet->alfAps,
                    alfSet->apsNum,
                    XIN_ALF_FIXED_FLT_SET_NUM,
                    FALSE,
                    &numOfBit);

                totalBits += numOfBit;
            }

            FLOAT32 costOn = distUnfilterCtu + ctuLambda * XIN_FRAC_BITS_SCALE * totalBits;

            if (channel == PLANE_LUMA)
            {
                costOn += XinGetFilteredDist (
                              alfSet,
                              alfCov,
                              alfSet->filterClippSet,
                              alfSet->filterCoeffSet,
                              numClasses,
                              alfSet->alfParamTemp.numLumaFilters - 1,
                              numCoeff);
            }
            else
            {
                FLOAT32 bestAltCost = MAX_DOUBLE;
                int bestAltIdx = -1;
                xin_prob_model stateBackup;

                stateBackup = alfSet->context[XIN_CO_CTB_ALF_ALT + compID - 1];

                for( int altIdx = 0; altIdx < numAlts; ++altIdx )
                {
                    Xin266EstimateAlfCtuAlt (
                        alfSet->context,
                        altIdx,
                        alfSet->alfParamTemp.numAltChroma,
                        compID,
                        TRUE,
                        &numOfBit);

                    // restore cabac state
                    alfSet->context[XIN_CO_CTB_ALF_ALT + compID - 1] = stateBackup;

                    FLOAT32 r_altCost = ctuLambda * XIN_FRAC_BITS_SCALE * numOfBit;

                    FLOAT32 altDist = 0.;

                    altDist += funcSet->pfXinCalcCoeffError (
                                   alfSet->filterClippSet[altIdx],
                                   alfSet->filterCoeffSet[altIdx],
                                   alfCov->E,
                                   alfCov->y,
                                   numCoeff,
                                   XIN_8_BIT_DEPTH);

                    FLOAT32 altCost = altDist + r_altCost;

                    if( altCost < bestAltCost )
                    {
                        bestAltCost = altCost;
                        bestAltIdx = altIdx;
                    }

                }

                alfSet->ctuAlternative[compID][ctuIdx] = (UINT8)bestAltIdx;
                costOn += bestAltCost;

            }

            alfSet->ctuEnableFlag[compID][ctuIdx] = 0;

            Xin266EstimateAlfCtuFlag (
                alfSet->context,
                ctxInc,
                compID,
                FALSE,
                FALSE,
                &numOfBit);

            FLOAT32 costOff = distUnfilterCtu + ctuLambda * XIN_FRAC_BITS_SCALE * numOfBit;

            if (costOn < costOff)
            {
                Xin266StateUpdate (
                    alfSet->context + compID * 3 + XIN_CO_CTB_ALF_FLAG + ctxInc,
                    TRUE);

                if (channel == PLANE_LUMA)
                {
                    Xin266StateUpdate (
                        alfSet->context + XIN_CO_USE_TEMPO_FLT,
                        TRUE);
                }
                else
                {
                    Xin266EstimateAlfCtuAlt (
                        alfSet->context,
                        alfSet->ctuAlternative[compID][ctuIdx],
                        alfSet->alfParamTemp.numAltChroma,
                        compID,
                        TRUE,
                        &numOfBit);
                }

                cost += costOn;
                alfSet->ctuEnableFlag[compID][ctuIdx] = 1;

            }
            else
            {
                Xin266StateUpdate (
                    alfSet->context + compID * 3 + XIN_CO_CTB_ALF_FLAG + ctxInc,
                    FALSE);

                cost += costOff;
                alfSet->ctuEnableFlag[compID][ctuIdx] = 0;
                *distUnfilter += distUnfilterCtu;
            }

        }

    }

    if (channel != PLANE_LUMA)
    {
        XinSetEnableFlag (
            alfSet,
            &alfSet->alfParamTemp,
            channel,
            alfSet->ctuEnableFlag);
    }

    return cost;

}

void XinInitCtuAlternativeChroma (
    xin_alf_struct *alfSet,
    UINT8          *ctuAlts[PLANE_NUM],
    SINT32         numCTUsInPic)
{
    UINT8 altIdx = 0;

    for( int ctuIdx = 0; ctuIdx < numCTUsInPic; ++ctuIdx )
    {
        ctuAlts[PLANE_CHROMA_U][ctuIdx] = altIdx;
        if( (ctuIdx+1) * alfSet->alfParamTemp.numAltChroma >= (altIdx+1)*numCTUsInPic )
            ++altIdx;
    }

    altIdx = 0;
    for( int ctuIdx = 0; ctuIdx < numCTUsInPic; ++ctuIdx )
    {
        ctuAlts[PLANE_CHROMA_V][ctuIdx] = altIdx;
        if( (ctuIdx+1) * alfSet->alfParamTemp.numAltChroma >= (altIdx+1)*numCTUsInPic )
            ++altIdx;
    }

}

void Xin266AlfEncoder (
    xin_alf_struct *alfSet,
    xin_alf_param  *alfParam,
    UINT32         channel)
{

    FLOAT32 costMin = MAX_DOUBLE;

    xin_alf_filter *alfFilter = alfParam->alfFilter[channel];
    alfSet->bitsNewFilter[channel] = 0;
    const int numClasses = (channel == PLANE_LUMA) ? XIN_ALF_MAX_CLS_NUM : 1;
    int uiCoeffBits = 0;

    for (int iShapeIdx = 0; iShapeIdx < 1; iShapeIdx++)
    {
        alfSet->alfParamTemp = *alfParam;

        //1. get unfiltered distortion
        if (channel != PLANE_LUMA)
        {
            alfSet->alfParamTemp.numAltChroma = 1;
        }

        FLOAT32 cost = XinGetUnfilteredDist((channel == PLANE_LUMA) ? alfSet->alfCovarianceFrameLuma : alfSet->alfCovarianceFrameChroma, numClasses);
        cost /= (FLOAT32)1.001; // slight preference for unfiltered choice

        if (cost < costMin)
        {
            costMin = cost;

            if (channel == PLANE_LUMA)
            {
                alfParam->alfEnabled[PLANE_LUMA] = FALSE;
            }
            else
            {
                alfParam->alfEnabled[PLANE_CHROMA_U] = alfParam->alfEnabled[PLANE_CHROMA_V] = FALSE;
            }

            // no CABAC signalling
            XinSetCtuEnableFlag (
                alfSet->ctuEnableFlagTmp,
                channel,
                0,
                alfSet->frameSizeInCtu);

            if (channel != PLANE_LUMA)
            {
                XinSetCtuAlternativeChroma (
                    alfSet->ctuAlternativeTmp,
                    0,
                    alfSet->frameSizeInCtu);
            }

        }

        const int nonLinearFlagMax =
            ( (channel == PLANE_LUMA) ? alfSet->useNonLinearAlfLuma : alfSet->useNonLinearAlfChroma ) // For Chroma non linear flag is check for each alternative filter
            ? 2 : 1;

        for( int nonLinearFlag = 0; nonLinearFlag < nonLinearFlagMax; nonLinearFlag++ )
        {
            for (int numAlternatives = (channel == PLANE_LUMA) ? 1 : XinGetMaxNumAlternativesChroma(alfSet); numAlternatives > 0; numAlternatives--)
            {
                memcpy (
                    alfSet->context,
                    alfSet->contextOrg,
                    XIN_NUM_OF_CTX*sizeof(xin_prob_model));

                if(channel != PLANE_LUMA)
                {
                    alfSet->alfParamTemp.numAltChroma = numAlternatives;
                }

                //2. all CTUs are on
                if (channel == PLANE_LUMA)
                {
                    alfSet->alfParamTemp.alfEnabled[PLANE_LUMA] = TRUE;
                }
                else
                {
                    alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_U] = alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_V] = TRUE;
                }

                alfSet->alfParamTemp.nonLinearFlag[channel] = nonLinearFlag;

                XinSetCtuEnableFlag (
                    alfSet->ctuEnableFlag,
                    channel,
                    1,
                    alfSet->frameSizeInCtu);

                // all alternatives are on
                if (channel != PLANE_LUMA)
                {
                    XinInitCtuAlternativeChroma (
                        alfSet,
                        alfSet->ctuAlternative,
                        alfSet->frameSizeInCtu);
                }

                cost = XinGetFilterCoeffAndCost (alfSet, 0, channel, TRUE, &uiCoeffBits, FALSE);

                if (cost < costMin)
                {
                    alfSet->bitsNewFilter[channel] = uiCoeffBits;
                    costMin = cost;

                    XinCopyAlfParam (
                        alfParam,
                        &alfSet->alfParamTemp,
                        channel);

                    XinSetCtuEnableFlag (
                        alfSet->ctuEnableFlagTmp,
                        channel,
                        1,
                        alfSet->frameSizeInCtu);

                    if (channel != PLANE_LUMA)
                    {
                        XinCopyCtuAlternativeChroma (
                            alfSet->ctuAlternativeTmp,
                            alfSet->ctuAlternative,
                            alfSet->frameSizeInCtu);
                    }

                }

                //3. CTU decision
                FLOAT32 distUnfilter = 0;
                FLOAT32 prevItCost = MAX_DOUBLE;
                const int iterNum = (channel == PLANE_LUMA) ? (2 * 4 + 1) : (2 * (2 + alfSet->alfParamTemp.numAltChroma - 1) + 1);

                for( int iter = 0; iter < iterNum; iter++ )
                {

                    if ((iter & 0x01) == 0)
                    {
                        memcpy (
                            alfSet->context,
                            alfSet->contextOrg,
                            XIN_NUM_OF_CTX*sizeof(xin_prob_model));

                        cost = alfSet->lambda[channel] * uiCoeffBits;
                        cost += XinDeriveCtbAlfEnableFlags (alfSet, iShapeIdx, channel,
                                                            numClasses, alfFilter->numCoeff, &distUnfilter);
                        if (cost < costMin)
                        {
                            alfSet->bitsNewFilter[channel] = uiCoeffBits;
                            costMin = cost;

                            XinCopyCtuEnableFlag (
                                alfSet->ctuEnableFlagTmp,
                                alfSet->ctuEnableFlag,
                                channel,
                                alfSet->frameSizeInCtu);

                            if(channel != PLANE_LUMA)
                            {
                                XinCopyCtuAlternativeChroma (
                                    alfSet->ctuAlternativeTmp,
                                    alfSet->ctuAlternative,
                                    alfSet->frameSizeInCtu);
                            }

                            XinCopyAlfParam (
                                alfParam,
                                &alfSet->alfParamTemp,
                                channel);
                        }
                        else if (cost >= prevItCost)
                        {
                            // High probability that we have converged or we are diverging
                            break;
                        }

                        prevItCost = cost;

                    }
                    else
                    {
                        // unfiltered distortion is added due to some CTBs may not use filter
                        // no need to reset CABAC here, since uiCoeffBits is not affected
                        /*cost = */XinGetFilterCoeffAndCost (alfSet, distUnfilter, channel, TRUE, &uiCoeffBits, FALSE);
                    }

                }//for iter

                // Decrease number of alternatives and reset ctu params and filters

            }
        }// for nonLineaFlag
    }//for shapeIdx

    if (channel != PLANE_LUMA)
    {
        XinCopyCtuAlternativeChroma (
            alfSet->ctuAlternative,
            alfSet->ctuAlternativeTmp,
            alfSet->frameSizeInCtu);
    }

    XinCopyCtuEnableFlag (
        alfSet->ctuEnableFlag,
        alfSet->ctuEnableFlagTmp,
        channel,
        alfSet->frameSizeInCtu);

}

static inline int clipIdx(int i, int clip, BOOL clipToBdry)
{
    if( clipToBdry )
    {
        return XIN_MAX ( i, clip );
    }
    else
    {
        return i;
    }
}

static int16_t clip_alf(const int16_t clip, const int16_t ref, const int16_t val0, const int16_t val1)
{
    return XIN_CLIP(val0 - ref, -clip, +clip) + XIN_CLIP(val1 - ref, -clip, +clip);
}

static void XinCalcCovariance (
    int      eLocal[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_CLIP_NUM],
    PIXEL    *rec,
    intptr_t stride,
    UINT32   channel,
    int      transpose_idx,
    int      vb_distance,
    short    alf_clipping_values[PLANE_TYPE][XIN_ALF_CLIP_NUM],
    BOOL     useNonLinearAlf)
{
    static const int alf_pattern_5[13] =
    {
        0,
        1,  2,  3,
        4,  5,  6,  5,  4,
        3,  2,  1,
        0
    };

    static const int alf_pattern_7[25] =
    {
        0,
        1,  2,  3,
        4,  5,  6,  7,  8,
        9, 10, 11, 12, 11, 10, 9,
        8,  7,  6,  5,  4,
        3,  2,  1,
        0
    };

    int clip_top_row = -4;
    int clip_bot_row = 4;
    if (vb_distance >= -3 && vb_distance < 0)
    {
        clip_bot_row = -vb_distance - 1;
        clip_top_row = -clip_bot_row; // symmetric
    }
    else if (vb_distance >= 0 && vb_distance < 3)
    {
        clip_top_row = -vb_distance;
        clip_bot_row = -clip_top_row; // symmetric
    }

    const BOOL is_luma = channel == PLANE_LUMA;
    const int* filter_pattern = is_luma ? alf_pattern_7 : alf_pattern_5;
    const int half_filter_length = (is_luma ? 7 : 5) >> 1;
    const short* clip = alf_clipping_values[channel];
    const int num_bins = useNonLinearAlf ? XIN_ALF_CLIP_NUM : 1;

    int k = 0;

    const int16_t curr = rec[0];

    if (transpose_idx == 0)
    {
        for (int i = -half_filter_length; i < 0; i++)
        {
            PIXEL *rec0 = rec + XIN_MAX(i, clip_top_row) * stride;
            PIXEL *rec1 = rec - XIN_MAX(i, -clip_bot_row) * stride;

            for (int j = -half_filter_length - i; j <= half_filter_length + i; j++, k++)
            {
                for (int b = 0; b < num_bins; b++)
                {
                    eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec0[j], rec1[-j]);
                }
            }
        }
        for (int j = -half_filter_length; j < 0; j++, k++)
        {
            for (int b = 0; b < num_bins; b++)
            {
                eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec[j], rec[-j]);
            }
        }
    }
    else if (transpose_idx == 1)
    {
        for (int j = -half_filter_length; j < 0; j++)
        {
            PIXEL *rec0 = rec + j;
            PIXEL *rec1 = rec - j;

            for (int i = -half_filter_length - j; i <= half_filter_length + j; i++, k++)
            {
                for (int b = 0; b < num_bins; b++)
                {
                    eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec0[XIN_MAX(i, clip_top_row) * stride], rec1[-XIN_MAX(i, -clip_bot_row) * stride]);
                }
            }
        }
        for (int i = -half_filter_length; i < 0; i++, k++)
        {
            for (int b = 0; b < num_bins; b++)
            {
                eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec[XIN_MAX(i, clip_top_row) * stride], rec[-XIN_MAX(i, -clip_bot_row) * stride]);
            }
        }
    }
    else if (transpose_idx == 2)
    {
        for (int i = -half_filter_length; i < 0; i++)
        {
            PIXEL *rec0 = rec + XIN_MAX(i, clip_top_row) * stride;
            PIXEL *rec1 = rec - XIN_MAX(i, -clip_bot_row) * stride;

            for (int j = half_filter_length + i; j >= -half_filter_length - i; j--, k++)
            {
                for (int b = 0; b < num_bins; b++)
                {
                    eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec0[j], rec1[-j]);
                }
            }
        }

        for (int j = -half_filter_length; j < 0; j++, k++)
        {
            for (int b = 0; b < num_bins; b++)
            {
                eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec[j], rec[-j]);
            }
        }
    }
    else
    {
        for (int j = -half_filter_length; j < 0; j++)
        {
            PIXEL *rec0 = rec + j;
            PIXEL *rec1 = rec - j;

            for (int i = half_filter_length + j; i >= -half_filter_length - j; i--, k++)
            {
                for (int b = 0; b < num_bins; b++)
                {
                    eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec0[XIN_MAX(i, clip_top_row) * stride], rec1[-XIN_MAX(i, -clip_bot_row) * stride]);
                }
            }
        }
        for (int i = -half_filter_length; i < 0; i++, k++)
        {
            for (int b = 0; b < num_bins; b++)
            {
                eLocal[filter_pattern[k]][b] += clip_alf(clip[b], curr, rec[XIN_MAX(i, clip_top_row) * stride], rec[-XIN_MAX(i, -clip_bot_row) * stride]);
            }
        }
    }

    for (int b = 0; b < num_bins; b++)
    {
        eLocal[filter_pattern[k]][b] += curr;
    }

}

void XinSetCtuAltChroma (
    UINT8   *ctuAlts[PLANE_NUM],
    UINT8   val,
    UINT32  numCTUsInPic)
{
    memset (ctuAlts[PLANE_CHROMA_U], val, numCTUsInPic);
    memset (ctuAlts[PLANE_CHROMA_V], val, numCTUsInPic);
}

void Xin266GetPreBlkStats (
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
    SINT32 numBins = useNonLinearAlf ? XIN_ALF_CLIP_NUM : 1;

    for( int i = 0; i < height; i++ )
    {
        int vbDistance = (i % vbCtuHeight) - vbPos;

        for( int j = 0; j < width; j++ )
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

            int ELocal[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_CLIP_NUM];

            memset (&(ELocal[0][0]), 0, sizeof(int)*XIN_ALF_MAX_LUMA_COEF_NUM*XIN_ALF_CLIP_NUM);

            int yLocal = input[j] - recon[j];

            XinCalcCovariance (
                ELocal,
                recon + j,
                reconStride,
                planeIdx != PLANE_LUMA,
                transposeIdx,
                vbDistance,
                alfClippingValues,
                useNonLinearAlf);

            for( int k = 0; k < alfFilter->numCoeff; k++ )
            {
                for( int l = k; l < alfFilter->numCoeff; l++ )
                {
                    for( int b0 = 0; b0 < numBins; b0++ )
                    {
                        for( int b1 = 0; b1 < numBins; b1++ )
                        {
                            alfCov[classIdx].E[b0][b1][k][l] += ELocal[k][b0] * ELocal[l][b1];
                        }
                    }
                }

                for( int b = 0; b < numBins; b++ )
                {
                    alfCov[classIdx].y[b][k] += ELocal[k][b] * yLocal;
                }
            }

            alfCov[classIdx].pixAcc += yLocal * yLocal;

        }

        input += inputStride;
        recon += reconStride;

    }

    int numClasses = alfClass ? XIN_ALF_MAX_CLS_NUM : 1;

    for( classIdx = 0; classIdx < numClasses; classIdx++ )
    {
        for( int k = 1; k < alfFilter->numCoeff; k++ )
        {
            for( int l = 0; l < k; l++ )
            {
                for( int b0 = 0; b0 < numBins; b0++ )
                {
                    for( int b1 = 0; b1 < numBins; b1++ )
                    {
                        alfCov[classIdx].E[b0][b1][k][l] = alfCov[classIdx].E[b1][b0][l][k];
                    }
                }
            }
        }
    }

}

void Xin266AlfDeriveClassCtu (
    xin_alf_struct *alfSet,
    xin_alf_class  *alfClass,
    intptr_t       classStride,
    PIXEL          *src,
    intptr_t       srcStride,
    SINT32         ctuWidth,
    SINT32         ctuHeight,
    SINT32         vbCtuHeight,
    SINT32         vbPos,
    SINT32         shift)
{
    SINT32       rowIdx;
    SINT32       colIdx;
    SINT32       blockWidth;
    SINT32       blockHeight;
    xin_alf_func *funcSet;

    funcSet = alfSet->funcSet;

    for (rowIdx = 0; rowIdx < ctuHeight; rowIdx += XIN_ALF_CLS_BLK_SIZE)
    {
        blockHeight = XIN_MIN (XIN_ALF_CLS_BLK_SIZE, ctuHeight - rowIdx);

        for (colIdx = 0; colIdx < ctuWidth; colIdx += XIN_ALF_CLS_BLK_SIZE)
        {
            blockWidth = XIN_MIN (XIN_ALF_CLS_BLK_SIZE, ctuWidth - colIdx);

            funcSet->pfXinAlfDeriveClass (
                alfClass + (rowIdx>>2)*classStride + (colIdx>>2),
                classStride,
                src + rowIdx*srcStride + colIdx,
                srcStride,
                blockWidth,
                blockHeight,
                rowIdx,
                vbCtuHeight,
                vbPos,
                shift);            
        }
        
    }

}

static void Xin266CopyPicture (
    xin_seq_struct    *seqSet,
    xin_ref_picture   *srcBuf,
    xin_frame_struct  *dstBuf,
    UINT32            planeMask)
{
    PIXEL    *srcY;
    PIXEL    *dstY;
    PIXEL    *srcU;
    PIXEL    *dstU;
    PIXEL    *srcV;
    PIXEL    *dstV;
    intptr_t srcLumaStride;
    intptr_t srcChromaStride;
    intptr_t dstLumaStride;
    intptr_t dstChromaStride;

    xin_func_struct *funcSet;

    funcSet = seqSet->funcSet;

    srcLumaStride   = srcBuf->refStride[0];
    srcChromaStride = srcBuf->refStride[1];
    dstLumaStride   = dstBuf->lumaStride;
    dstChromaStride = dstBuf->chromaStride;

    srcY = srcBuf->refBuf[PLANE_LUMA];
    srcU = srcBuf->refBuf[PLANE_CHROMA_U];
    srcV = srcBuf->refBuf[PLANE_CHROMA_V];

    dstY = dstBuf->yuvBuf[PLANE_LUMA];
    dstU = dstBuf->yuvBuf[PLANE_CHROMA_U];
    dstV = dstBuf->yuvBuf[PLANE_CHROMA_V];

    srcY -= XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*srcLumaStride;
    srcU -= XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*srcChromaStride;
    srcV -= XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*srcChromaStride;

    dstY -= XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*dstLumaStride;
    dstU -= XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*dstChromaStride;
    dstV -= XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*dstChromaStride;

    if (planeMask & XIN_LUMA_MASK)
    {
        funcSet->pfXinPictureCopy (
            srcY,
            srcLumaStride,
            dstY,
            dstLumaStride,
            srcBuf->lumaWidth + XIN_ALF_FRAME_PADDING * 2,
            srcBuf->lumaHeight + XIN_ALF_FRAME_PADDING * 2);
    }

    if (planeMask & XIN_CHROMA_U_MASK)
    {
        funcSet->pfXinPictureCopy (
            srcU,
            srcChromaStride,
            dstU,
            dstChromaStride,
            srcBuf->lumaWidth / 2 + XIN_ALF_FRAME_PADDING * 2,
            srcBuf->lumaHeight / 2 + XIN_ALF_FRAME_PADDING * 2);
    }

    if (planeMask & XIN_CHROMA_V_MASK)
    {
        funcSet->pfXinPictureCopy (
            srcV,
            srcChromaStride,
            dstV,
            dstChromaStride,
            srcBuf->lumaWidth / 2 + XIN_ALF_FRAME_PADDING * 2,
            srcBuf->lumaHeight / 2 + XIN_ALF_FRAME_PADDING * 2);
    }

}

void Xin266AlfStatCtu (
    xin_ctu_struct *ctu)
{
    xin_seq_struct    *seqSet;
    xin_pic_struct    *picSet;
    xin_input_picture *inputPicture;
    xin_ref_picture   *pictureWrite;
    xin_frame_struct  *filterFrame;
    xin_alf_struct    *alfSet;
    xin_alf_cov       *alfCov;
    xin_alf_class     *alfClass;
    PIXEL             *input;
    PIXEL             *recon;
    intptr_t          inputStride;
    intptr_t          reconStride;
    SINT32            ctuPelX;
    SINT32            ctuPelY;
    UINT32            compIdx;
    UINT32            isChroma;
    SINT32            ctuSize;
    UINT32            ctuIdx;
    BOOL              nonLinearAlf;
    xin_alf_func      *funcSet;

    picSet = ctu->picSet;
    seqSet = picSet->seqSet;

    if (!picSet->enableAlf)
    {
        return;
    }

    ctuPelX      = ctu->ctuPelX;
    ctuPelY      = ctu->ctuPelY;
    alfSet       = picSet->alfSet;
    inputPicture = picSet->inputPicture;
    pictureWrite = picSet->pictureWrite;
    filterFrame  = alfSet->filterFrame;
    ctuSize      = seqSet->ctuSize;
    ctuIdx       = ctu->ctuAddr;
    alfClass     = alfSet->alfClass + (ctuPelX>>2) + (ctuPelY>>2)*alfSet->alfClassStride;
    funcSet      = alfSet->funcSet;

    for (compIdx = 0; compIdx < PLANE_NUM; compIdx++)
    {
        isChroma     = compIdx != 0;
        inputStride  = inputPicture->inputStride[isChroma];
        reconStride  = pictureWrite->refStride[isChroma];
        input        = inputPicture->inputBuf[compIdx] + ((ctuPelX + ctuPelY*inputStride) >> isChroma);
        recon        = pictureWrite->refBuf[compIdx] + ((ctuPelX + ctuPelY*reconStride) >> isChroma);
        alfCov       = (compIdx == PLANE_LUMA ? alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM : (compIdx == PLANE_CHROMA_U ? alfSet->alfCovarianceU + ctuIdx : alfSet->alfCovarianceV + ctuIdx));
        nonLinearAlf = compIdx == PLANE_LUMA ? alfSet->useNonLinearAlfLuma : alfSet->useNonLinearAlfChroma;

        if (compIdx == 0)
        {
            Xin266AlfDeriveClassCtu (
                alfSet,
                alfClass,
                alfSet->alfClassStride,
                recon,
                reconStride,
                ctu->width,
                ctu->height,
                seqSet->ctuSize,
                seqSet->ctuSize - 4,
                XIN_INTERNAL_BIT_DEPTH + 4);
        }

        if (nonLinearAlf)
        {
            Xin266GetPreBlkStats (
                alfCov,
                TRUE,
                alfSet->alfFilter + isChroma,
                isChroma ? NULL : alfClass,
                alfSet->alfClassStride,
                input,
                inputStride,
                recon,
                reconStride,
                compIdx,
                ctu->width>>isChroma,
                ctu->height>>isChroma,
                alfSet->alfClippingValues,
                ctuSize>>isChroma,
                (ctuSize - 4)>>isChroma);
        }
        else
        {
            funcSet->pfXinGetPreBlkStats (
                alfCov,
                FALSE,
                alfSet->alfFilter + isChroma,
                isChroma ? NULL : alfClass,
                alfSet->alfClassStride,
                input,
                inputStride,
                recon,
                reconStride,
                compIdx,
                ctu->width>>isChroma,
                ctu->height>>isChroma,
                alfSet->alfClippingValues,
                ctuSize>>isChroma,
                (ctuSize - 4)>>isChroma);
        }

    }

    if (ctuIdx == (seqSet->frameSizeInCtu - 1))
    {
        Xin266CopyPicture (
            picSet->seqSet,
            pictureWrite,
            filterFrame,
            XIN_ALL_PLANE_MASK);

        Xin26xExtendPicture (
            filterFrame->yuvBuf[0],
            filterFrame->lumaStride,
            filterFrame->lumaWidth,
            filterFrame->lumaHeight,
            4,
            4);

        Xin26xExtendPicture (
            filterFrame->yuvBuf[1],
            filterFrame->chromaStride,
            filterFrame->lumaWidth/2,
            filterFrame->lumaHeight/2,
            4,
            4);

        Xin26xExtendPicture (
            filterFrame->yuvBuf[2],
            filterFrame->chromaStride,
            filterFrame->lumaWidth/2,
            filterFrame->lumaHeight/2,
            4,
            4);

    }

}

void  XinInitCtbDist (
    xin_alf_struct *alfSet)
{
    SINT32 ctuIdx;

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        alfSet->ctuUnfilterDist[PLANE_LUMA][ctuIdx] = XinGetUnfilteredDist (alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM, XIN_ALF_MAX_CLS_NUM);
    }

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        alfSet->ctuUnfilterDist[PLANE_CHROMA_U][ctuIdx] = XinGetUnfilteredDist(alfSet->alfCovarianceU + ctuIdx, 1);
    }

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        alfSet->ctuUnfilterDist[PLANE_CHROMA_V][ctuIdx] = XinGetUnfilteredDist(alfSet->alfCovarianceV + ctuIdx, 1);
    }

}

void Xin266AlfEncoderCtu (
    xin_alf_struct *alfSet,
    xin_alf_param  *alfParam)
{
    SINT32        ctuIdx;
    FLOAT32       costOff;
    BOOL          hasNewFilters[2];
    SINT32        useNewFilter;
    SINT32        numLoops;
    SINT32        bitsNewFilter;
    SINT32        numIter;
    SINT32        numFilterSet;
    SINT32        iter;
    FLOAT32       curCost;
    FLOAT32       distOrgNewFilter;
    SINT32        blocksUsingNewFilter;
    SINT32        classIdx;
    xin_alf_cov   *alfCov;
    xin_alf_param alfParamNewFiltersBest;
    FLOAT32       distUnfilterCtb;
    SINT32        filterSetIdx;
    FLOAT32       costMin;
    UINT32        bestApsSizeIds;
    UINT32        numOfBit;
    UINT32        ctxInc;
    xin_alf_func  *funcSet;

    hasNewFilters[0] = alfParam->alfEnabled[PLANE_LUMA];
    hasNewFilters[1] = alfParam->alfEnabled[PLANE_CHROMA_U] || alfParam->alfEnabled[PLANE_CHROMA_V];
    numLoops         = hasNewFilters[PLANE_LUMA] ? 2 : 1;
    costMin          = MAX_DOUBLE;
    bestApsSizeIds   = 0;
    funcSet          = alfSet->funcSet;

    XinInitCtbDist (
        alfSet);

    XinSetCtuEnableFlag (
        alfSet->ctuEnableFlag,
        PLANE_LUMA,
        1,
        alfSet->frameSizeInCtu);

    XinGetFrameStats (
        alfSet,
        PLANE_LUMA);

    XinSetCtuEnableFlag (
        alfSet->ctuEnableFlag,
        PLANE_LUMA,
        0,
        alfSet->frameSizeInCtu);

    costOff = XinGetUnfilteredDist (alfSet->alfCovarianceFrameLuma, XIN_ALF_MAX_CLS_NUM);

    for (useNewFilter = 0; useNewFilter < numLoops; useNewFilter++)
    {
        bitsNewFilter = 0;

        alfSet->apsNum = useNewFilter ? 1 : 0;

        if (useNewFilter == 1)
        {
            if (!hasNewFilters[PLANE_LUMA])
            {
                continue;
            }
            else
            {
                bitsNewFilter = alfSet->bitsNewFilter[PLANE_LUMA];

                XinReconstructCoeff (
                    alfSet,
                    alfParam,
                    PLANE_LUMA,
                    TRUE);
            }
        }

        numIter      = useNewFilter ? 2 : 1;
        numFilterSet = XIN_ALF_FIXED_FLT_SET_NUM + useNewFilter;

        for (iter = 0; iter < numIter; iter++)
        {
            alfSet->alfParamTemp = *alfParam;
            alfSet->alfParamTemp.alfEnabled[0] = TRUE;

            curCost = 3 * alfSet->lambda[PLANE_LUMA];

            if (iter > 0)  //re-derive new filter-set
            {
                distOrgNewFilter     = 0;
                blocksUsingNewFilter = 0;

                for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
                {
                    alfCov = alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM;

                    if (alfSet->ctuEnableFlag[0][ctuIdx] && alfSet->alfCtbFilterSetIndex[ctuIdx] != XIN_ALF_FIXED_FLT_SET_NUM)
                    {
                        alfSet->ctuEnableFlag[0][ctuIdx] = 0;
                    }
                    else if (alfSet->ctuEnableFlag[0][ctuIdx] && alfSet->alfCtbFilterSetIndex[ctuIdx] == XIN_ALF_FIXED_FLT_SET_NUM)
                    {
                        blocksUsingNewFilter++;
                        distOrgNewFilter += alfSet->ctuUnfilterDist[PLANE_LUMA][ctuIdx];

                        for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
                        {
                            short* pCoeff = alfSet->coeffFinal;
                            short* pClipp = alfSet->clippFinal;

                            for (int i = 0; i < XIN_ALF_MAX_LUMA_COEF_NUM; i++)
                            {
                                alfSet->filterTmp[i] = pCoeff[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + i];
                                alfSet->clipTmp[i]   = pClipp[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + i];
                            }

                            distOrgNewFilter += funcSet->pfXinCalcCoeffError (
                                                    alfSet->clipTmp,
                                                    alfSet->filterTmp,
                                                    alfCov[classIdx].E,
                                                    alfCov[classIdx].y,
                                                    XIN_ALF_MAX_LUMA_COEF_NUM,
                                                    XIN_8_BIT_DEPTH);

                        }
                    }
                } //for(ctbIdx)

                if (blocksUsingNewFilter > 0 && blocksUsingNewFilter < alfSet->frameSizeInCtu)
                {
                    int bitNL[2] = { 0, 0 };
                    FLOAT32 errNL[2] = { 0.0, 0.0 };

                    alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] = 1;

                    if (alfSet->useNonLinearAlfLuma)
                    {
                        memcpy (
                            alfSet->context,
                            alfSet->contextOrg,
                            XIN_NUM_OF_CTX*sizeof(xin_prob_model));

                        errNL[1] = XinGetFilterCoeffAndCost (
                                       alfSet,
                                       0,
                                       PLANE_LUMA,
                                       TRUE,
                                       bitNL + 1,
                                       TRUE);

                        alfSet->alfParamTempNL = alfSet->alfParamTemp;
                    }
                    else
                    {
                        errNL[1] = MAX_DOUBLE;
                    }

                    alfSet->alfParamTemp.nonLinearFlag[PLANE_LUMA] = 0;

                    memcpy (
                        alfSet->context,
                        alfSet->contextOrg,
                        XIN_NUM_OF_CTX*sizeof(xin_prob_model));

                    errNL[0] = XinGetFilterCoeffAndCost (
                                   alfSet,
                                   0,
                                   PLANE_LUMA,
                                   TRUE,
                                   bitNL + 0,
                                   TRUE);

                    int bitsNewFilterTempLuma = bitNL[0];
                    FLOAT32 err = errNL[0];

                    if (errNL[1]  < errNL[0])
                    {
                        err = errNL[1];
                        bitsNewFilterTempLuma = bitNL[1];
                        alfSet->alfParamTemp = alfSet->alfParamTempNL;
                    }


                    if (distOrgNewFilter + alfSet->lambda[PLANE_LUMA] * alfSet->bitsNewFilter[PLANE_LUMA] < err) //re-derived filter is not good, skip
                    {
                        continue;
                    }

                    XinReconstructCoeff (
                        alfSet,
                        &alfSet->alfParamTemp,
                        PLANE_LUMA,
                        TRUE);

                    bitsNewFilter = bitsNewFilterTempLuma;

                }
                else //no blocks using new filter, skip
                {
                    continue;
                }

            }

            memcpy (
                alfSet->context,
                alfSet->contextOrg,
                XIN_NUM_OF_CTX*sizeof(xin_prob_model));

            for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
            {
                distUnfilterCtb = alfSet->ctuUnfilterDist[PLANE_LUMA][ctuIdx];

                alfCov = alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM;

                //ctb on
                alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx] = 1;
                FLOAT32         costOn = MAX_DOUBLE;
                int iBestFilterSetIdx = 0;

                XinGetAlfCtuFlagCtxIdx (
                    alfSet->ctuEnableFlag[PLANE_LUMA] + ctuIdx,
                    alfSet->ctu[ctuIdx].availField,
                    alfSet->frameWidthInCtu,
                    &ctxInc);

                for (filterSetIdx = 0; filterSetIdx < numFilterSet; filterSetIdx++)
                {
                    Xin266EstimateAlfCtuFlag (
                        alfSet->context,
                        ctxInc,
                        PLANE_LUMA,
                        alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx],
                        FALSE,
                        &numOfBit);

                    FLOAT32 rateOn = numOfBit*XIN_FRAC_BITS_SCALE;

                    Xin266EstimateAlfCtuFilterIdx (
                        alfSet->context,
                        alfSet->alfAps,
                        alfSet->apsNum,
                        filterSetIdx,
                        FALSE,
                        &numOfBit);

                    rateOn += numOfBit*XIN_FRAC_BITS_SCALE;

                    //distortion
                    FLOAT32 dist = distUnfilterCtb;

                    for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
                    {
                        if (filterSetIdx < XIN_ALF_FIXED_FLT_SET_NUM)
                        {
                            // fixed filter set
                            int filterIdx = classToFilterMapping[filterSetIdx][classIdx];

                            dist += funcSet->pfXinCalcCoeffError (
                                        alfSet->clipDefaultEnc,
                                        alfSet->fixedFilterSetCoeff[filterIdx],
                                        alfCov[classIdx].E,
                                        alfCov[classIdx].y,
                                        XIN_ALF_MAX_LUMA_COEF_NUM,
                                        XIN_8_BIT_DEPTH);
                        }
                        else
                        {
                            short *pCoeff;
                            short *pClipp;

                            if (useNewFilter && filterSetIdx == XIN_ALF_FIXED_FLT_SET_NUM)
                            {
                                // New filter, no APS
                                pCoeff = alfSet->coeffFinal;
                                pClipp = alfSet->clippFinal;
                            }
                            else
                            {
                                // New filter after APS
                                pCoeff = alfSet->coeffFinal;
                                pClipp = alfSet->clippFinal;
                            }

                            alfCov = alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM;

                            for (int i = 0; i < XIN_ALF_MAX_LUMA_COEF_NUM; i++)
                            {
                                alfSet->filterTmp[i] = pCoeff[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + i];
                                alfSet->clipTmp[i] = pClipp[classIdx * XIN_ALF_MAX_LUMA_COEF_NUM + i];
                            }

                            dist += funcSet->pfXinCalcCoeffError (
                                        alfSet->clipTmp,
                                        alfSet->filterTmp,
                                        alfCov[classIdx].E,
                                        alfCov[classIdx].y,
                                        XIN_ALF_MAX_LUMA_COEF_NUM,
                                        XIN_8_BIT_DEPTH);

                        }

                    } //for(classIdx)

                    //cost
                    const FLOAT32 costOnTmp = dist + alfSet->lambda[0] * rateOn;

                    if (costOnTmp < costOn)
                    {
                        costOn = costOnTmp;
                        iBestFilterSetIdx = filterSetIdx;
                    }

                } //for(filterSetIdx)

                //ctb off
                alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx] = 0;

                //rate
                Xin266EstimateAlfCtuFlag (
                    alfSet->context,
                    ctxInc,
                    PLANE_LUMA,
                    FALSE,
                    FALSE,
                    &numOfBit);

                FLOAT32 rateOff = numOfBit*XIN_FRAC_BITS_SCALE;

                //cost
                const FLOAT32 lumaCostOff = distUnfilterCtb + alfSet->lambda[PLANE_LUMA] * rateOff;

                if (costOn < lumaCostOff)
                {
                    alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx] = 1;
                    alfSet->alfCtbFilterSetIndex[ctuIdx] = (UINT8)iBestFilterSetIdx;
                    curCost += costOn;

                    Xin266StateUpdate (
                        alfSet->context + XIN_CO_CTB_ALF_FLAG + ctxInc,
                        TRUE);

                    if (useNewFilter)
                    {
                        Xin266StateUpdate (
                            alfSet->context + XIN_CO_USE_TEMPO_FLT,
                            iBestFilterSetIdx >= XIN_ALF_FIXED_FLT_SET_NUM);
                    }
                }
                else
                {
                    alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx] = 0;
                    curCost += lumaCostOff;

                    Xin266StateUpdate (
                        alfSet->context + XIN_CO_CTB_ALF_FLAG + ctxInc,
                        FALSE);
                }

            } //for(ctbIdx)

            int tmpBits = bitsNewFilter + 3 * (numFilterSet - XIN_ALF_FIXED_FLT_SET_NUM);
            curCost += tmpBits * alfSet->lambda[PLANE_LUMA];

            if (curCost < costMin)
            {
                costMin = curCost;
                bestApsSizeIds = numFilterSet - XIN_ALF_FIXED_FLT_SET_NUM;
                alfParamNewFiltersBest = alfSet->alfParamTemp;

                XinCopyCtuEnableFlag (
                    alfSet->ctuEnableFlagTmp,
                    alfSet->ctuEnableFlag,
                    PLANE_LUMA,
                    alfSet->frameSizeInCtu);

                for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
                {
                    alfSet->alfCtbFilterSetIndexTmp[ctuIdx] = alfSet->alfCtbFilterSetIndex[ctuIdx];
                }

                alfParamNewFiltersBest.newFilterFlag[PLANE_LUMA] = useNewFilter;

            }

        }

    }

    if (costOff <= costMin)
    {
        memset (
            alfSet->alfEnabled,
            0,
            sizeof(alfSet->alfEnabled));

        alfSet->apsNum = 0;

        XinSetCtuEnableFlag (
            alfSet->ctuEnableFlag,
            PLANE_LUMA,
            0,
            alfSet->frameSizeInCtu);

        XinSetCtuEnableFlag (
            alfSet->ctuEnableFlag,
            PLANE_CHROMA,
            0,
            alfSet->frameSizeInCtu);

        return;

    }
    else
    {
        alfSet->alfEnabled[PLANE_LUMA] = TRUE;

        alfSet->apsNum = bestApsSizeIds;

        XinCopyCtuEnableFlag (
            alfSet->ctuEnableFlag,
            alfSet->ctuEnableFlagTmp,
            PLANE_LUMA,
            alfSet->frameSizeInCtu);

        for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
        {
            alfSet->alfCtbFilterSetIndex[ctuIdx] = alfSet->alfCtbFilterSetIndexTmp[ctuIdx];
        }

        if (alfParamNewFiltersBest.newFilterFlag[PLANE_LUMA])
        {

            alfSet->alfAps[0].apsId      = 0;
            alfSet->alfAps[0].apsType    = 0;
            alfSet->alfAps[0].alfParam   = alfParamNewFiltersBest;
            alfSet->alfAps[0].temporalId = alfSet->temporalId;
            alfSet->alfAps[0].alfParam.newFilterFlag[1] = FALSE;
            alfSet->alfAps[0].changed = TRUE;
        }

    }

    // chroma
    alfSet->alfParamTemp = alfParamNewFiltersBest;

    if (alfSet->alfParamTemp.numAltChroma < 1)
    {
        alfSet->alfParamTemp.numAltChroma = 1;
    }

    XinSetCtuAltChroma (
        alfSet->ctuAlternative,
        0,
        alfSet->frameSizeInCtu);

    XinSetCtuEnableFlag (
        alfSet->ctuEnableFlag,
        PLANE_CHROMA,
        1,
        alfSet->frameSizeInCtu);

    XinGetFrameStats (
        alfSet,
        PLANE_CHROMA);

    costOff = XinGetUnfilteredDist (alfSet->alfCovarianceFrameChroma, 1);
    costMin = MAX_DOUBLE;

    curCost = alfSet->lambda[PLANE_CHROMA] * 3;

    alfSet->alfParamTemp = *alfParam;

    curCost += alfSet->lambda[PLANE_CHROMA] * alfSet->bitsNewFilter[PLANE_CHROMA];

    XinReconstructCoeff (
        alfSet,
        &alfSet->alfParamTemp,
        PLANE_CHROMA,
        TRUE);

    for (int compId = 1; compId < PLANE_NUM; compId++)
    {
        alfSet->alfParamTemp.alfEnabled[compId] = TRUE;

        for (int ctbIdx = 0; ctbIdx < alfSet->frameSizeInCtu; ctbIdx++)
        {
            FLOAT32 distUnfilterCtu = alfSet->ctuUnfilterDist[compId][ctbIdx];
            alfCov = (compId == PLANE_CHROMA_U ? alfSet->alfCovarianceU + ctbIdx : alfSet->alfCovarianceV + ctbIdx);

            // cost on
            alfSet->ctuEnableFlag[compId][ctbIdx] = 1;

            // rate
            // ctb flag

            XinGetAlfCtuFlagCtxIdx (
                alfSet->ctuEnableFlag[compId] + ctbIdx,
                alfSet->ctu[ctbIdx].availField,
                alfSet->frameWidthInCtu,
                &ctxInc);

            Xin266EstimateAlfCtuFlag (
                alfSet->context,
                ctxInc,
                compId,
                alfSet->ctuEnableFlag[compId][ctbIdx],
                FALSE,
                &numOfBit);

            FLOAT32 rateOn = XIN_FRAC_BITS_SCALE * numOfBit;

            const FLOAT32 ctuLambda = alfSet->lambda[PLANE_CHROMA];

            FLOAT32 dist = MAX_DOUBLE;
            int    numAlts      = alfSet->alfParamTemp.numAltChroma;
            FLOAT32 bestAltRate = 0;
            FLOAT32 bestAltCost = MAX_DOUBLE;
            int    bestAltIdx   = -1;

            xin_prob_model stateBackup;

            stateBackup = alfSet->context[XIN_CO_CTB_ALF_ALT + compId - 1];

            for (int altIdx = 0; altIdx < numAlts; ++altIdx)
            {
                alfSet->ctuAlternative[compId][ctbIdx] = (UINT8)altIdx;

                Xin266EstimateAlfCtuAlt (
                    alfSet->context,
                    altIdx,
                    alfSet->alfParamTemp.numAltChroma,
                    compId,
                    TRUE,
                    &numOfBit);

                // restore cabac state
                alfSet->context[XIN_CO_CTB_ALF_ALT + compId - 1] = stateBackup;

                FLOAT32 altRate = XIN_FRAC_BITS_SCALE * numOfBit;
                FLOAT32 r_altCost = ctuLambda * altRate;

                // distortion
                for (int i = 0; i < XIN_ALF_MAX_CHROMA_COEF_NUM; i++)
                {
                    alfSet->filterTmp[i] = alfSet->chromaCoeffFinal[altIdx][i];
                    alfSet->clipTmp[i]   = alfSet->chromaClippFinal[altIdx][i];
                }

                FLOAT32 altDist;

                altDist = funcSet->pfXinCalcCoeffError (
                              alfSet->clipTmp,
                              alfSet->filterTmp,
                              alfCov->E,
                              alfCov->y,
                              XIN_ALF_MAX_CHROMA_COEF_NUM,
                              XIN_8_BIT_DEPTH);


                FLOAT32 altCost = altDist + r_altCost;

                if (altCost < bestAltCost)
                {
                    bestAltCost = altCost;
                    bestAltIdx  = altIdx;
                    bestAltRate = altRate;
                    dist        = altDist;
                }

            }

            alfSet->ctuAlternative[compId][ctbIdx] = (UINT8)bestAltIdx;
            rateOn += bestAltRate;
            dist += distUnfilterCtu;
            // cost
            FLOAT32 costOn = dist + ctuLambda * rateOn;

            // cost off
            alfSet->ctuEnableFlag[compId][ctbIdx] = 0;

            // rate
            Xin266EstimateAlfCtuFlag (
                alfSet->context,
                ctxInc,
                compId,
                alfSet->ctuEnableFlag[compId][ctbIdx],
                FALSE,
                &numOfBit);

            // cost
            FLOAT32 chromaCostOff = distUnfilterCtu + alfSet->lambda[PLANE_CHROMA] * XIN_FRAC_BITS_SCALE * numOfBit;

            if (costOn < chromaCostOff)
            {
                alfSet->ctuEnableFlag[compId][ctbIdx] = 1;
                curCost += costOn;

                Xin266EstimateAlfCtuAlt (
                    alfSet->context,
                    alfSet->ctuAlternative[compId][ctbIdx],
                    alfSet->alfParamTemp.numAltChroma,
                    compId,
                    TRUE,
                    &numOfBit);

                Xin266EstimateAlfCtuFlag (
                    alfSet->context,
                    ctxInc,
                    compId,
                    alfSet->ctuEnableFlag[compId][ctbIdx],
                    TRUE,
                    &numOfBit);

            }
            else
            {
                alfSet->ctuEnableFlag[compId][ctbIdx] = 0;
                curCost += chromaCostOff;

                Xin266EstimateAlfCtuFlag (
                    alfSet->context,
                    ctxInc,
                    compId,
                    alfSet->ctuEnableFlag[compId][ctbIdx],
                    TRUE,
                    &numOfBit);
            }

        }

    }

    // chroma idc
    XinSetEnableFlag (
        alfSet,
        &alfSet->alfParamTemp,
        PLANE_CHROMA,
        alfSet->ctuEnableFlag);

    if (curCost < costMin)
    {
        costMin = curCost;

        alfSet->alfEnabled[PLANE_CHROMA_U] = alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_U];
        alfSet->alfEnabled[PLANE_CHROMA_V] = alfSet->alfParamTemp.alfEnabled[PLANE_CHROMA_V];

        XinCopyCtuEnableFlag (
            alfSet->ctuEnableFlagTmp,
            alfSet->ctuEnableFlag,
            PLANE_CHROMA,
            alfSet->frameSizeInCtu);

        XinCopyCtuAlternativeChroma (
            alfSet->ctuAlternativeTmp,
            alfSet->ctuAlternative,
            alfSet->frameSizeInCtu);

    }

    if (costOff < costMin)
    {
        alfSet->alfEnabled[PLANE_CHROMA_U] = FALSE;
        alfSet->alfEnabled[PLANE_CHROMA_V] = FALSE;

        XinSetCtuEnableFlag (
            alfSet->ctuEnableFlag,
            PLANE_CHROMA,
            0,
            alfSet->frameSizeInCtu);
    }
    else
    {
        XinCopyCtuEnableFlag (
            alfSet->ctuEnableFlag,
            alfSet->ctuEnableFlagTmp,
            PLANE_CHROMA,
            alfSet->frameSizeInCtu);

        XinCopyCtuAlternativeChroma (
            alfSet->ctuAlternative,
            alfSet->ctuAlternativeTmp,
            alfSet->frameSizeInCtu);

        alfSet->alfAps[0].alfParam.newFilterFlag[PLANE_CHROMA] = TRUE;


        alfSet->alfAps[0].alfParam.numAltChroma = alfParam->numAltChroma;
        alfSet->alfAps[0].alfParam.nonLinearFlag[PLANE_CHROMA] = alfParam->nonLinearFlag[PLANE_CHROMA];

        for (int altIdx = 0; altIdx < XIN_ALF_MAX_CHROMA_ALT_NUM; ++altIdx)
        {
            for (int i = 0; i < XIN_ALF_MAX_CHROMA_COEF_NUM; i++)
            {
                alfSet->alfAps[0].alfParam.chromaCoeff[altIdx][i] = alfParam->chromaCoeff[altIdx][i];
                alfSet->alfAps[0].alfParam.chromaClipp[altIdx][i] = alfParam->chromaClipp[altIdx][i];
            }
        }

    }

}

void Xin266DeriveAlfFilter (
    xin_pic_struct *picSet)
{
    xin_seq_struct  *seqSet;
    xin_alf_struct  *alfSet;
    xin_ref_picture *pictureWrite;
    xin_alf_param   alfParam;
    SINT32          qp;
    SINT32          qpUv;
    SINT32          ctuIdx;
    SINT32          classIdx;
    UINT32          frameType;

    seqSet       = picSet->seqSet;
    alfSet       = picSet->alfSet;
    pictureWrite = picSet->pictureWrite;
    frameType    = XIN_MIN (pictureWrite->frameType, XIN_I_FRAME);

    if (!picSet->enableAlf)
    {
        return;
    }

    memset (
        &alfParam,
        0,
        sizeof(xin_alf_param));

    alfParam.alfFilter[0] = alfSet->alfFilter + PLANE_LUMA;
    alfParam.alfFilter[1] = alfSet->alfFilter + PLANE_CHROMA;

    memset (
        alfSet->alfCovarianceFrameLuma,
        0,
        XIN_ALF_MAX_CLS_NUM*sizeof(xin_alf_cov));

    memset (
        alfSet->alfCovarianceFrameChroma,
        0,
        sizeof(xin_alf_cov));

    for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
    {
        alfSet->alfCovarianceFrameLuma[classIdx].numBins  = alfSet->useNonLinearAlfLuma ? XIN_ALF_CLIP_NUM : 1;
        alfSet->alfCovarianceFrameLuma[classIdx].numCoeff = XIN_ALF_MAX_LUMA_COEF_NUM;
    }

    for (classIdx = 0; classIdx < XIN_ALF_MAX_CHROMA_ALT_NUM; classIdx++)
    {
        alfSet->alfCovarianceFrameChroma[classIdx].numBins  = alfSet->useNonLinearAlfChroma ? XIN_ALF_CLIP_NUM : 1;
        alfSet->alfCovarianceFrameChroma[classIdx].numCoeff = XIN_ALF_MAX_CHROMA_COEF_NUM;
    }

    alfSet->apsNum = 1;

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++ )
    {
        alfSet->alfCtbFilterSetIndex[ctuIdx] = XIN_ALF_FIXED_FLT_SET_NUM;
    }

    // Accumulate ALF statistic
    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
        {
            XinAlfCovAdd (
                alfSet->alfCovarianceFrameLuma + classIdx,
                alfSet->alfCovarianceY + ctuIdx*XIN_ALF_MAX_CLS_NUM + classIdx,
                alfSet->alfCovarianceFrameLuma + classIdx);
        }
    }

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        XinAlfCovAdd (
            alfSet->alfCovarianceFrameChroma,
            alfSet->alfCovarianceU + ctuIdx,
            alfSet->alfCovarianceFrameChroma);

        XinAlfCovAdd (
            alfSet->alfCovarianceFrameChroma,
            alfSet->alfCovarianceV + ctuIdx,
            alfSet->alfCovarianceFrameChroma);

    }

    qp   = picSet->picQp;
    qpUv = seqSet->chromaQpMap[qp] + seqSet->config.chromaQpOffset;

    if (seqSet->config.internalBitDepth == XIN_10_BIT_DEPTH)
    {
        alfSet->lambda[PLANE_LUMA]   = (FLOAT32)(0.57 * pow(2.0, qp / 3.0));
        alfSet->lambda[PLANE_CHROMA] = (FLOAT32)(0.57 * pow(2.0, qpUv / 3.0));
    }
    else
    {
        alfSet->lambda[PLANE_LUMA]   = (FLOAT32)(0.57 * pow(2.0, (qp - 12) / 3.0));
        alfSet->lambda[PLANE_CHROMA] = (FLOAT32)(0.57 * pow(2.0, (qpUv - 12) / 3.0));
    }

    memcpy (
        alfSet->contextOrg,
        seqSet->cabacContext + (frameType*XIN_QP_NUM + picSet->picQp)*XIN_NUM_OF_CTX,
        XIN_NUM_OF_CTX*sizeof(xin_prob_model));

    Xin266AlfEncoder (
        alfSet,
        &alfParam,
        PLANE_LUMA);

    Xin266AlfEncoder (
        alfSet,
        &alfParam,
        PLANE_CHROMA);

    Xin266AlfEncoderCtu (
        alfSet,
        &alfParam);

    if (!alfSet->alfEnabled[0])
    {
        picSet->enableCcAlf = FALSE;
    }

}

void Xin266AlfCtu (
    xin_ctu_struct *ctu)
{
    xin_alf_struct   *alfSet;
    xin_pic_struct   *picSet;
    xin_seq_struct   *seqSet;
    SINT32           ctuIdx;
    xin_ref_picture  *reconFrame;
    xin_frame_struct *filterFrame;
    SINT16           *filterSet;
    SINT16           *fClipSet;
    intptr_t         reconStrideY;
    intptr_t         filterStrideY;
    intptr_t         reconStrideUv;
    intptr_t         filterStrideUv;
    SINT32           ctuPelX;
    SINT32           ctuPelY;
    SINT32           filterIndex;
    SINT32           altIdx;
    xin_alf_func     *funcSet;

    picSet = ctu->picSet;
    alfSet = picSet->alfSet;

    if (!picSet->enableAlf)
    {
        return;
    }

    reconFrame     = picSet->pictureWrite;
    filterFrame    = alfSet->filterFrame;
    reconStrideY   = reconFrame->refStride[PLANE_LUMA];
    filterStrideY  = filterFrame->lumaStride;
    reconStrideUv  = reconFrame->refStride[PLANE_CHROMA];
    filterStrideUv = filterFrame->chromaStride;
    seqSet         = picSet->seqSet;
    ctuPelX        = ctu->ctuPelX;
    ctuPelY        = ctu->ctuPelY;
    ctuIdx         = ctu->ctuAddr;
    funcSet        = alfSet->funcSet;

    if (ctuIdx == 0)
    {
        XinReconstructCoeff (
            alfSet,
            &alfSet->alfAps[0].alfParam,
            PLANE_LUMA,
            FALSE);

        XinReconstructCoeff (
            alfSet,
            &alfSet->alfAps[0].alfParam,
            PLANE_CHROMA,
            FALSE);
    }

    if (alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx])
    {
        filterIndex = alfSet->alfCtbFilterSetIndex[ctuIdx];

        if (filterIndex == XIN_ALF_FIXED_FLT_SET_NUM)
        {
            filterSet = alfSet->coeffFinal;
            fClipSet  = alfSet->clippFinal;
        }
        else
        {
            filterSet = alfSet->fixedFilterSetCoeffDec[filterIndex];
            fClipSet  = alfSet->clipDefault;
        }

        funcSet->pfXinAlfBlockLuma (
            filterFrame->yuvBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*filterStrideY),
            filterStrideY,
            reconFrame->refBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*reconStrideY),
            reconStrideY,
            filterSet,
            fClipSet,
            alfSet->alfClass + ((ctuPelX>>2) + (ctuPelY>>2)*alfSet->alfClassStride),
            alfSet->alfClassStride,
            ctu->width,
            ctu->height,
            seqSet->ctuSize - 4,
            seqSet->ctuSize);

    }

    if (alfSet->ctuEnableFlag[PLANE_CHROMA_U][ctuIdx])
    {
        altIdx = alfSet->ctuAlternative[PLANE_CHROMA_U][ctuIdx];

        funcSet->pfXinAlfBlockChroma (
            filterFrame->yuvBuf[PLANE_CHROMA_U] + ((ctuPelX + ctuPelY*filterStrideUv) >> 1),
            filterStrideUv,
            reconFrame->refBuf[PLANE_CHROMA_U] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
            reconStrideUv,
            alfSet->chromaCoeffFinal[altIdx],
            alfSet->chromaClippFinal[altIdx],
            ctu->width >> 1,
            ctu->height >> 1,
            (seqSet->ctuSize - 4) >> 1,
            seqSet->ctuSize >> 1);
    }

    if (alfSet->ctuEnableFlag[PLANE_CHROMA_V][ctuIdx])
    {
        altIdx = alfSet->ctuAlternative[PLANE_CHROMA_V][ctuIdx];

        funcSet->pfXinAlfBlockChroma (
            filterFrame->yuvBuf[PLANE_CHROMA_V] + ((ctuPelX + ctuPelY*filterStrideUv) >> 1),
            filterStrideUv,
            reconFrame->refBuf[PLANE_CHROMA_V] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
            reconStrideUv,
            alfSet->chromaCoeffFinal[altIdx],
            alfSet->chromaClippFinal[altIdx],
            ctu->width >> 1,
            ctu->height >> 1,
            (seqSet->ctuSize - 4) >> 1,
            seqSet->ctuSize >> 1);
    }

}

void Xin266AlfFrame (
    xin_pic_struct *picSet)
{
    xin_alf_struct   *alfSet;
    xin_seq_struct   *seqSet;
    SINT32           ctuIdx;
    xin_ref_picture  *reconFrame;
    xin_frame_struct *filterFrame;
    xin_ctu_struct   *ctu;
    SINT16           *filterSet;
    SINT16           *fClipSet;
    intptr_t         reconStrideY;
    intptr_t         filterStrideY;
    intptr_t         reconStrideUv;
    intptr_t         filterStrideUv;
    SINT32           ctuPelX;
    SINT32           ctuPelY;
    SINT32           filterIndex;
    SINT32           altIdx;
    xin_alf_func     *funcSet;

    alfSet         = picSet->alfSet;
    reconFrame     = picSet->pictureWrite;
    filterFrame    = alfSet->filterFrame;
    reconStrideY   = reconFrame->refStride[PLANE_LUMA];
    filterStrideY  = filterFrame->lumaStride;
    reconStrideUv  = reconFrame->refStride[PLANE_CHROMA];
    filterStrideUv = filterFrame->chromaStride;
    seqSet         = picSet->seqSet;
    funcSet        = alfSet->funcSet;

    XinReconstructCoeff (
        alfSet,
        &alfSet->alfAps[0].alfParam,
        PLANE_LUMA,
        FALSE);

    XinReconstructCoeff (
        alfSet,
        &alfSet->alfAps[0].alfParam,
        PLANE_CHROMA,
        FALSE);

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        ctu     = alfSet->ctu + ctuIdx;
        ctuPelX = ctu->ctuPelX;
        ctuPelY = ctu->ctuPelY;

        if (alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx])
        {
            filterIndex = alfSet->alfCtbFilterSetIndex[ctuIdx];

            if (filterIndex == XIN_ALF_FIXED_FLT_SET_NUM)
            {
                filterSet = alfSet->coeffFinal;
                fClipSet  = alfSet->clippFinal;
            }
            else
            {
                filterSet = alfSet->fixedFilterSetCoeffDec[filterIndex];
                fClipSet  = alfSet->clipDefault;
            }

            funcSet->pfXinAlfBlockLuma (
                filterFrame->yuvBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*filterStrideY),
                filterStrideY,
                reconFrame->refBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*reconStrideY),
                reconStrideY,
                filterSet,
                fClipSet,
                alfSet->alfClass + ((ctuPelX>>2) + (ctuPelY>>2)*alfSet->alfClassStride),
                alfSet->alfClassStride,
                ctu->width,
                ctu->height,
                seqSet->ctuSize - 4,
                seqSet->ctuSize);

        }

        if (alfSet->ctuEnableFlag[PLANE_CHROMA_U][ctuIdx])
        {
            altIdx = alfSet->ctuAlternative[PLANE_CHROMA_U][ctuIdx];

            funcSet->pfXinAlfBlockChroma (
                filterFrame->yuvBuf[PLANE_CHROMA_U] + ((ctuPelX + ctuPelY*filterStrideUv) >> 1),
                filterStrideUv,
                reconFrame->refBuf[PLANE_CHROMA_U] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
                reconStrideUv,
                alfSet->chromaCoeffFinal[altIdx],
                alfSet->chromaClippFinal[altIdx],
                ctu->width >> 1,
                ctu->height >> 1,
                (seqSet->ctuSize - 4) >> 1,
                seqSet->ctuSize >> 1);
        }

        if (alfSet->ctuEnableFlag[PLANE_CHROMA_V][ctuIdx])
        {
            altIdx = alfSet->ctuAlternative[PLANE_CHROMA_V][ctuIdx];

            funcSet->pfXinAlfBlockChroma (
                filterFrame->yuvBuf[PLANE_CHROMA_V] + ((ctuPelX + ctuPelY*filterStrideUv) >> 1),
                filterStrideUv,
                reconFrame->refBuf[PLANE_CHROMA_V] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
                reconStrideUv,
                alfSet->chromaCoeffFinal[altIdx],
                alfSet->chromaClippFinal[altIdx],
                ctu->width >> 1,
                ctu->height >> 1,
                (seqSet->ctuSize - 4) >> 1,
                seqSet->ctuSize >> 1);
        }

    }

}

void Xin266CalcCovarianceCcAlf (
    SINT32   ELocal[XIN_CC_ALF_MAX_COEFF_NUM],
    PIXEL    *recon,
    intptr_t reconStride,
    SINT32   vbDistance)
{
    PIXEL *recYM1;
    PIXEL *recY0;
    PIXEL *recYP1;
    PIXEL *recYP2;

    recYM1 = recon - 1 * reconStride;
    recY0  = recon;
    recYP1 = recon + 1 * reconStride;
    recYP2 = recon + 2 * reconStride;

    if (vbDistance == -2 || vbDistance == +1)
    {
        recYP2 = recYP1;
    }
    else if (vbDistance == -1 || vbDistance == 0)
    {
        recYM1 = recY0;
        recYP2 = recYP1 = recY0;
    }

    ELocal[0] += recYM1[+0] - recY0[+0];
    ELocal[1] += recY0[-1] - recY0[+0];
    ELocal[2] += recY0[+1] - recY0[+0];
    ELocal[3] += recYP1[-1] - recY0[+0];
    ELocal[4] += recYP1[+0] - recY0[+0];
    ELocal[5] += recYP1[+1] - recY0[+0];
    ELocal[6] += recYP2[+0] - recY0[+0];

}

void Xin266GetBlkStatsCcAlf (
    PIXEL         *input,
    intptr_t      inputStride,
    PIXEL         *reconY,
    intptr_t      reconYStride,
    PIXEL         *recon,
    intptr_t      reconStride,
    xin_alf_cov   *alfCovCcAlf,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  vbDistance;
    SINT32  eLocal[XIN_CC_ALF_MAX_COEFF_NUM];

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        vbDistance = ((rowIdx << 1) % vbCtuHeight) - vbPos;

        for (colIdx = 0; colIdx < width; colIdx++)
        {
            memset (eLocal, 0, sizeof(eLocal));

            int yLocal = input[colIdx] - recon[colIdx];

            Xin266CalcCovarianceCcAlf (
                eLocal,
                reconY + colIdx*2,
                reconYStride,
                vbDistance);

            for( int k = 0; k < (XIN_CC_ALF_MAX_COEFF_NUM - 1); k++ )
            {
                for( int l = k; l < (XIN_CC_ALF_MAX_COEFF_NUM - 1); l++ )
                {
                    alfCovCcAlf->E[0][0][k][l] += eLocal[k] * eLocal[l];
                }

                alfCovCcAlf->y[0][k] += eLocal[k] * yLocal;

            }

            alfCovCcAlf->pixAcc += yLocal * yLocal;

        }

        input  += inputStride;
        reconY += reconYStride*2;
        recon  += reconStride;

    }

    for (int k = 1; k < (XIN_CC_ALF_MAX_COEFF_NUM - 1); k++)
    {
        for (int l = 0; l < k; l++)
        {
            alfCovCcAlf->E[0][0][k][l] = alfCovCcAlf->E[0][0][l][k];
        }
    }

}

void Xin266CcAlfStatCtu (
    xin_ctu_struct *ctu)
{
    xin_seq_struct    *seqSet;
    xin_pic_struct    *picSet;
    xin_input_picture *inputPicture;
    xin_frame_struct  *filterFrame;
    xin_ref_picture   *pictureWrite;
    xin_alf_struct    *alfSet;
    xin_alf_cov       *alfCov;
    PIXEL             *input;
    PIXEL             *recon;
    PIXEL             *reconY;
    intptr_t          inputStride;
    intptr_t          reconStride;
    intptr_t          reconYStride;
    SINT32            ctuPelX;
    SINT32            ctuPelY;
    UINT32            compIdx;
    SINT32            ctuSize;
    SINT32            ctuAddr;

    picSet = ctu->picSet;
    seqSet = picSet->seqSet;

    if (!picSet->enableCcAlf)
    {
        return;
    }

    ctuPelX      = ctu->ctuPelX;
    ctuPelY      = ctu->ctuPelY;
    alfSet       = picSet->alfSet;
    inputPicture = picSet->inputPicture;
    pictureWrite = picSet->pictureWrite;
    filterFrame  = alfSet->filterFrame;
    ctuSize      = seqSet->ctuSize;
    ctuAddr      = ctu->ctuAddr;
    reconYStride = filterFrame->lumaStride;
    reconY       = filterFrame->yuvBuf[0] + (ctuPelX + ctuPelY*reconYStride);

    for (compIdx = PLANE_CHROMA_U; compIdx <= PLANE_CHROMA_V; compIdx++)
    {
        inputStride = inputPicture->inputStride[PLANE_CHROMA];
        reconStride = pictureWrite->refStride[PLANE_CHROMA];
        input       = inputPicture->inputBuf[compIdx] + ((ctuPelX + ctuPelY*inputStride) >> 1);
        recon       = pictureWrite->refBuf[compIdx] + ((ctuPelX + ctuPelY*reconStride) >> 1);
        alfCov      = alfSet->alfCovarianceCcAlf[compIdx - 1] + ctuAddr;

        Xin266GetBlkStatsCcAlf (
            input,
            inputStride,
            reconY,
            reconYStride,
            recon,
            reconStride,
            alfCov,
            ctu->width>>1,
            ctu->height>>1,
            (ctuSize - 4),
            ctuSize);

    }

}

void Xin266DeriveStatsCcAlfFFrame (
    xin_pic_struct *picSet)
{
    UINT32         ctuIdx;
    SINT32         compId;
    xin_seq_struct *seqSet;
    xin_alf_struct *alfSet;

    seqSet = picSet->seqSet;
    alfSet = picSet->alfSet;

    for (compId = PLANE_CHROMA_U; compId <= PLANE_CHROMA_V; compId++)
    {
        alfSet->alfCovarianceFrameCcAlf[compId - 1][0].numBins  = 1;
        alfSet->alfCovarianceFrameCcAlf[compId - 1][0].numCoeff = XIN_CC_ALF_MAX_COEFF_NUM;

        XinAlfCovReset (
            alfSet->alfCovarianceFrameCcAlf[compId - 1],
            1);
    }

    for (ctuIdx = 0; ctuIdx < seqSet->frameSizeInCtu; ctuIdx++)
    {
        Xin266CcAlfStatCtu (
            picSet->ctu + ctuIdx);

        for (compId = PLANE_CHROMA_U; compId <= PLANE_CHROMA_V; compId++)
        {
            XinAlfCovAdd (
                alfSet->alfCovarianceFrameCcAlf[compId - 1],
                alfSet->alfCovarianceCcAlf[compId - 1] + ctuIdx,
                alfSet->alfCovarianceFrameCcAlf[compId - 1]);
        }
    }

    for (compId = PLANE_CHROMA_U; compId <= PLANE_CHROMA_V; compId++)
    {
        for (ctuIdx = 0; ctuIdx < seqSet->frameSizeInCtu; ctuIdx++)
        {
            alfSet->ctuUnfilterDist[compId][ctuIdx] = alfSet->alfCovarianceCcAlf[compId - 1][ctuIdx].pixAcc;
        }
    }

}

void Xin266GetFrameStatsCcAlf (
    xin_alf_struct *alfSet,
    SINT32         compId,
    SINT32         filterIdc)
{
    int ctuRsAddr;
    int filterIdx = filterIdc - 1;

    // init Frame stats buffers
    XinAlfCovReset (
        alfSet->alfCovarianceFrameCcAlf[compId - 1] + filterIdx,
        1);

    for (ctuRsAddr = 0; ctuRsAddr < alfSet->frameSizeInCtu; ctuRsAddr++)
    {
        if (alfSet->trainingCovControl[ctuRsAddr] == filterIdc)
        {
            XinAlfCovAdd (
                alfSet->alfCovarianceFrameCcAlf[compId - 1] + filterIdx,
                alfSet->alfCovarianceCcAlf[compId - 1] + ctuRsAddr,
                alfSet->alfCovarianceFrameCcAlf[compId - 1] + filterIdx);
        }

    }

}

void Xin266RoundFiltCoeffCcAlf (
    SINT16  *filterCoeffQuant,
    FLOAT32 *filterCoeff,
    SINT32  numCoeff,
    SINT32  factor)
{
    for( int i = 0; i < numCoeff; i++ )
    {
        int sign = filterCoeff[i] > 0 ? 1 : -1;
        FLOAT32 best_err = 128.0*128.0;
        int best_index = 0;
        for(int k = 0; k < CCALF_CANDS_COEFF_NR; k++)
        {
            FLOAT32 err = (filterCoeff[i] * sign * factor - CCALF_SMALL_TAB[k]);
            err = err*err;
            if(err < best_err)
            {
                best_err = err;
                best_index = k;
            }
        }
        filterCoeffQuant[i] = (SINT16)(CCALF_SMALL_TAB[best_index] * sign);
    }

}

FLOAT32 Xin266CalcErrorForCcAlfCoeffs(
    SINT16  *coeff,
    SINT32  numCoeff,
    FLOAT32 E[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  bitDepth)
{
    FLOAT32 factor = (FLOAT32)(1 << (bitDepth - 1));
    FLOAT32 error = 0;

    for (int i = 0; i < numCoeff; i++)   // diagonal
    {
        FLOAT32 sum = 0;
        for (int j = i + 1; j < numCoeff; j++)
        {
            // E[j][i] = E[i][j], sum will be multiplied by 2 later
            sum += E[i][j] * coeff[j];
        }
        error += ((E[i][i] * coeff[i] + sum * 2) / factor - 2 * y[i]) * coeff[i];
    }

    return error / factor;
}

void Xin266DeriveCcAlfFilterCoeff (
    xin_alf_struct *alfSet,
    SINT32         compId,
    SINT16         filterCoeff[XIN_CC_ALF_MAX_FILTER_NUM][XIN_CC_ALF_MAX_COEFF_NUM],
    SINT32         filterIdx)
{
    int forward_tab[CCALF_CANDS_COEFF_NR * 2 - 1] = {0};

    for (int i = 0; i < CCALF_CANDS_COEFF_NR; i++)
    {
        forward_tab[CCALF_CANDS_COEFF_NR - 1 + i] = CCALF_SMALL_TAB[i];
        forward_tab[CCALF_CANDS_COEFF_NR - 1 - i] = (-1) * CCALF_SMALL_TAB[i];
    }


    FLOAT32 filterCoeffDbl[XIN_CC_ALF_MAX_COEFF_NUM];
    SINT16 filterCoeffInt[XIN_CC_ALF_MAX_COEFF_NUM];

    memset (filterCoeffInt, 0, sizeof(SINT16)*XIN_CC_ALF_MAX_COEFF_NUM);

    FLOAT32 kE[XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 ky[XIN_ALF_MAX_LUMA_COEF_NUM];

    const int size = XIN_CC_ALF_MAX_COEFF_NUM - 1;

    for (int k = 0; k < size; k++)
    {
        ky[k] = alfSet->alfCovarianceFrameCcAlf[compId - 1][filterIdx].y[0][k];

        for (int l = 0; l < size; l++)
        {
            kE[k][l] = alfSet->alfCovarianceFrameCcAlf[compId - 1][filterIdx].E[0][0][k][l];
        }
    }

    gnsSolveByChol (
        kE,
        ky,
        filterCoeffDbl,
        size);

    Xin266RoundFiltCoeffCcAlf (
        filterCoeffInt,
        filterCoeffDbl,
        size,
        (1 << 7));

    // Refine quanitzation
    int modified   = 1;
    FLOAT32 errRef = Xin266CalcErrorForCcAlfCoeffs(filterCoeffInt, size, kE, ky, 8);

    while (modified)
    {
        modified = 0;

        for (int delta = 1; delta >= -1; delta -= 2)
        {
            FLOAT32 errMin = MAX_DOUBLE;
            int     idxMin = -1;
            int   minIndex = -1;

            for (int k = 0; k < size; k++)
            {
                int org_idx = -1;
                for (int i = 0; i < CCALF_CANDS_COEFF_NR * 2 - 1; i++)
                {
                    if (forward_tab[i] == filterCoeffInt[k])
                    {
                        org_idx = i;
                        break;
                    }
                }

                if ( (org_idx - delta < 0) || (org_idx - delta >= CCALF_CANDS_COEFF_NR * 2 - 1) )
                {
                    continue;
                }

                filterCoeffInt[k] = (SINT16)forward_tab[org_idx - delta];
                FLOAT32 error = Xin266CalcErrorForCcAlfCoeffs(filterCoeffInt, size, kE, ky, 8);
                if( error < errMin )
                {
                    errMin = error;
                    idxMin = k;
                    minIndex = org_idx;
                }
                filterCoeffInt[k] = (SINT16)forward_tab[org_idx];
            }

            if (errMin < errRef)
            {
                minIndex -= delta;

                filterCoeffInt[idxMin] = (SINT16)forward_tab[minIndex];
                modified++;
                errRef = errMin;
            }
        }
    }

    for (int k = 0; k < (size + 1); k++)
    {
        filterCoeff[filterIdx][k] = filterCoeffInt[k];
    }

}

void Xin266DetermineControlIdcValues (
    xin_alf_struct *alfSet,
    SINT32         compId,
    FLOAT32        **unfilteredDistortion,
    uint64_t       *trainingDistortion[XIN_CC_ALF_MAX_FILTER_NUM],
    BOOL           reuseTemporalFilterCoeff,
    uint8_t        *trainingCovControl,
    uint8_t        *filterControl,
    uint64_t       *curTotalDistortion,
    FLOAT32        *curTotalRate,
    BOOL           filterEnabled[XIN_CC_ALF_MAX_FILTER_NUM],
    uint8_t        mapFilterIdxToFilterIdc[XIN_CC_ALF_MAX_FILTER_NUM + 1],
    uint8_t        *ccAlfFilterCount)
{
    BOOL      curFilterEnabled[XIN_CC_ALF_MAX_FILTER_NUM];
    UINT32    numOfBit;
    UINT32    fracRate;
    SINT32    ctxInc;
    SINT32    ctuIdx;

    memset (curFilterEnabled, 0, XIN_CC_ALF_MAX_FILTER_NUM*sizeof(BOOL));

#if XIN_CC_ALF_MAX_FILTER_NUM > 1
    FilterIdxCount filterIdxCount[XIN_CC_ALF_MAX_FILTER_NUM];
    for (int i = 0; i < XIN_CC_ALF_MAX_FILTER_NUM; i++)
    {
        filterIdxCount[i].count     = 0;
        filterIdxCount[i].filterIdx = (UINT8)i;
    }

    FLOAT32 prevRate = *curTotalRate;
#endif

    memcpy(
        alfSet->context,
        alfSet->contextOrg,
        XIN_NUM_OF_CTX*sizeof(xin_prob_model));

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        uint64_t ssd;
        FLOAT32  rate;
        FLOAT32  cost;

        uint64_t bestSSD       = XIN_MAX_U64_COST;
        FLOAT32   bestRate = MAX_DOUBLE;
        FLOAT32   bestCost = MAX_DOUBLE;
        uint8_t  bestFilterIdc = 0;
        uint8_t  bestFilterIdx = 0;

        XinGetCcAlfFltCtrlIdcCtxIdx(
            alfSet->filterControl + ctuIdx,
            alfSet->ctu[ctuIdx].availField,
            alfSet->frameWidthInCtu,
            compId,
            &ctxInc);

        for (int filterIdx = 0; filterIdx <= XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
        {
            uint8_t filterIdc = mapFilterIdxToFilterIdc[filterIdx];

            if (filterIdx < XIN_CC_ALF_MAX_FILTER_NUM && !filterEnabled[filterIdx])
            {
                continue;
            }

            if (filterIdx == XIN_CC_ALF_MAX_FILTER_NUM)
            {
                ssd = (uint64_t)unfilteredDistortion[compId][ctuIdx];   // restore saved distortion computation
            }
            else
            {
                ssd = trainingDistortion[filterIdx][ctuIdx];
            }

            Xin266EstimateCcAlfFltCtrlIdc (
                alfSet->context,
                ctxInc,
                filterIdc,
                *ccAlfFilterCount,
                FALSE,
                &numOfBit);

            rate = XIN_FRAC_BITS_SCALE * numOfBit;
            cost = rate * alfSet->lambda[PLANE_CHROMA] + ssd;

            BOOL limitationExceeded = FALSE;

            if (cost < bestCost && !limitationExceeded)
            {
                bestCost      = cost;
                bestRate      = rate;
                bestSSD       = ssd;
                bestFilterIdc = filterIdc;
                bestFilterIdx = (UINT8)filterIdx;

                trainingCovControl[ctuIdx] = (filterIdx == XIN_CC_ALF_MAX_FILTER_NUM) ? 0 : (UINT8)(filterIdx + 1);
                filterControl[ctuIdx]      = (filterIdx == XIN_CC_ALF_MAX_FILTER_NUM) ? 0 : (UINT8)(filterIdx + 1);
            }

        }

        if (bestFilterIdc != 0)
        {
            curFilterEnabled[bestFilterIdx] = TRUE;
#if XIN_CC_ALF_MAX_FILTER_NUM > 1
            filterIdxCount[bestFilterIdx].count++;
#endif
        }

        Xin266StateUpdate (
            alfSet->context + XIN_CO_CC_ALF_FLT_CTRL_FLAG + ctxInc,
            bestFilterIdc != 0);

        *curTotalRate += bestRate;
        *curTotalDistortion += bestSSD;

    }

#if XIN_CC_ALF_MAX_FILTER_NUM > 1
    if (!reuseTemporalFilterCoeff)
    {
        memcpy(filterEnabled, curFilterEnabled, sizeof(BOOL)*XIN_CC_ALF_MAX_FILTER_NUM);
        qsort(filterIdxCount, XIN_CC_ALF_MAX_FILTER_NUM, sizeof(*filterIdxCount), comparator);

        int filterIdc = 1;
        *ccAlfFilterCount = 0;

        for (int i = 0; i < XIN_CC_ALF_MAX_FILTER_NUM; i++)
        {
            const int filterIdx = filterIdxCount[i].filterIdx;

            if (filterEnabled[filterIdx])
            {
                mapFilterIdxToFilterIdc[filterIdx] = (UINT8)filterIdc;
                filterIdc++;
                (*ccAlfFilterCount)++;
            }
        }

        *curTotalRate = prevRate;

        memcpy (
            alfSet->context,
            alfSet->contextOrg,
            XIN_NUM_OF_CTX*sizeof(xin_prob_model));

        fracRate = 0;

        for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
        {

            const int filterIdxPlus1 = filterControl[ctuIdx];

            XinGetCcAlfFltCtrlIdcCtxIdx (
                alfSet->filterControl + ctuIdx,
                alfSet->ctu[ctuIdx].availField,
                alfSet->frameWidthInCtu,
                compId,
                &ctxInc);

            Xin266EstimateCcAlfFltCtrlIdc (
                alfSet->context,
                ctxInc,
                filterIdxPlus1 == 0 ? 0 : mapFilterIdxToFilterIdc[filterIdxPlus1 - 1],
                *ccAlfFilterCount,
                TRUE,
                &numOfBit);

            fracRate += numOfBit;

        }

        *curTotalRate += XIN_FRAC_BITS_SCALE* fracRate;

    }

#endif

}

SINT32 Xin266GetCoeffRateCcAlf (
    SINT16 chromaCoeff[XIN_CC_ALF_MAX_FILTER_NUM][XIN_CC_ALF_MAX_COEFF_NUM],
    BOOL   filterEnabled[XIN_CC_ALF_MAX_FILTER_NUM],
    UINT8  filterCount)
{
    int bits = 0;

    if ( filterCount > 0 )
    {
        bits += lengthUvlc(filterCount - 1);
        int signaledFilterCount = 0;
        for ( int filterIdx=0; filterIdx<XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++ )
        {
            if (filterEnabled[filterIdx])
            {
                // Filter coefficients
                for (int i = 0; i < XIN_CC_ALF_MAX_COEFF_NUM - 1; i++)
                {
                    bits += CCALF_BITS_PER_COEFF_LEVEL + (chromaCoeff[filterIdx][i] == 0 ? 0 : 1);
                }

                signaledFilterCount++;
            }
        }
    }

    return bits;

}

void Xin266DeriveCcAlfFilter (
    xin_alf_struct *alfSet,
    SINT32         compId)
{
    uint8_t bestMapFilterIdxToFilterIdc[XIN_CC_ALF_MAX_FILTER_NUM+1];
    const int maxTrainingIterCount = 15;
    int ctuIdx;

    memcpy(
        alfSet->context,
        alfSet->contextOrg,
        XIN_NUM_OF_CTX*sizeof(xin_prob_model));

    for (int filterIdx = 0; filterIdx <= XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
    {
        if ( filterIdx < XIN_CC_ALF_MAX_FILTER_NUM)
        {
            memset( alfSet->bestFilterCoeffSet[filterIdx], 0, sizeof(alfSet->bestFilterCoeffSet[filterIdx]) );
            bestMapFilterIdxToFilterIdc[filterIdx] = (UINT8)(filterIdx + 1);
        }
        else
        {
            bestMapFilterIdxToFilterIdc[filterIdx] = 0;
        }
    }

    memset(alfSet->bestFilterControl, 0, sizeof(UINT8) * alfSet->frameSizeInCtu);

    // compute cost of not filtering
    UINT64 unfilteredDistortion = 0;

    for (int ctbIdx = 0; ctbIdx < alfSet->frameSizeInCtu; ctbIdx++)
    {
        unfilteredDistortion += (uint64_t)alfSet->alfCovarianceCcAlf[compId - 1][ctbIdx].pixAcc;
    }

    FLOAT32 bestUnfilteredTotalCost = 1 * alfSet->lambda[1] + unfilteredDistortion;   // 1 bit is for gating flag

    BOOL             ccAlfFilterIdxEnabled[XIN_CC_ALF_MAX_FILTER_NUM];
    short            ccAlfFilterCoeff[XIN_CC_ALF_MAX_FILTER_NUM][XIN_CC_ALF_MAX_COEFF_NUM];
    uint8_t          ccAlfFilterCount             = XIN_CC_ALF_MAX_FILTER_NUM;
    FLOAT32 bestFilteredTotalCost = MAX_DOUBLE;
    BOOL   bestreuseTemporalFilterCoeff = FALSE;

    for (int testFilterIdx = 0; testFilterIdx < 1; testFilterIdx++)
    {
        BOOL referencingExistingAps   = FALSE;

        SINT32 maxNumberOfFiltersBeingTested = XIN_CC_ALF_MAX_FILTER_NUM - (testFilterIdx);

        if (maxNumberOfFiltersBeingTested < 0)
        {
            maxNumberOfFiltersBeingTested = 1;
        }

        // Instead of rewriting the control buffer for every training iteration just keep a mapping from filterIdx to filterIdc
        uint8_t mapFilterIdxToFilterIdc[XIN_CC_ALF_MAX_FILTER_NUM + 1];
        for (int filterIdx = 0; filterIdx <= XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
        {
            if (filterIdx == XIN_CC_ALF_MAX_FILTER_NUM)
            {
                mapFilterIdxToFilterIdc[filterIdx] = 0;
            }
            else
            {
                mapFilterIdxToFilterIdc[filterIdx] = (UINT8)(filterIdx + 1);
            }
        }

        // initialize filters
        for ( int filterIdx = 0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++ )
        {
            ccAlfFilterIdxEnabled[filterIdx] = FALSE;
            memset(ccAlfFilterCoeff[filterIdx], 0, sizeof(ccAlfFilterCoeff[filterIdx]));
        }

        for (int i = 0; i < maxNumberOfFiltersBeingTested; i++)
        {
            ccAlfFilterIdxEnabled[i] = TRUE;
        }

        ccAlfFilterCount = (UINT8)maxNumberOfFiltersBeingTested;

        // initialize
        const int columnSize = alfSet->filterFrame->lumaWidth / maxNumberOfFiltersBeingTested;
        int ctuX;

        for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
        {
            ctuX = ctuIdx  - (ctuIdx / alfSet->frameWidthInCtu)*alfSet->frameWidthInCtu;

            alfSet->trainingCovControl[ctuIdx] = (UINT8)((ctuX*alfSet->ctuSize / columnSize) + 1);
        }

        // compute cost of filtering
        int    trainingIterCount = 0;
        BOOL   keepTraining      = TRUE;
        BOOL   improvement       = FALSE;
        FLOAT32 prevTotalCost    = MAX_DOUBLE;

        while (keepTraining)
        {
            improvement = FALSE;
            for (int filterIdx = 0; filterIdx < maxNumberOfFiltersBeingTested; filterIdx++)
            {
                if (ccAlfFilterIdxEnabled[filterIdx])
                {
                    if (!referencingExistingAps)
                    {
                        Xin266GetFrameStatsCcAlf (
                            alfSet,
                            compId,
                            filterIdx + 1);

                        Xin266DeriveCcAlfFilterCoeff (
                            alfSet,
                            compId,
                            ccAlfFilterCoeff,
                            filterIdx);
                    }

                    const int numCoeff  = XIN_CC_ALF_MAX_COEFF_NUM - 1;

                    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
                    {
                        alfSet->trainingDistortion[filterIdx][ctuIdx] =
                            (SINT32)(alfSet->ctuUnfilterDist[compId][ctuIdx]
                                     + Xin266CalcErrorForCcAlfCoeffs(ccAlfFilterCoeff[filterIdx], numCoeff, alfSet->alfCovarianceCcAlf[compId - 1][ctuIdx].E[0][0], alfSet->alfCovarianceCcAlf[compId - 1][ctuIdx].y[0], 8));

                    }
                }

            }

            uint64_t curTotalDistortion = 0;
            FLOAT32 curTotalRate = 0;

            Xin266DetermineControlIdcValues (
                alfSet,
                compId,
                alfSet->ctuUnfilterDist,
                alfSet->trainingDistortion,
                (referencingExistingAps == TRUE),
                alfSet->trainingCovControl,
                alfSet->filterControl,
                &curTotalDistortion,
                &curTotalRate,
                ccAlfFilterIdxEnabled,
                mapFilterIdxToFilterIdc,
                &ccAlfFilterCount);

            // compute coefficient coding bit cost
            if (ccAlfFilterCount > 0)
            {
                if (referencingExistingAps)
                {
                    curTotalRate += 1 + 3; // +1 for enable flag, +3 APS ID in slice header
                }
                else
                {
                    curTotalRate += Xin266GetCoeffRateCcAlf (ccAlfFilterCoeff, ccAlfFilterIdxEnabled, ccAlfFilterCount) + 1
                                    + 9;   // +1 for the enable flag, +9 3-bit for APS ID in slice header, 5-bit for APS ID in APS, a 1-bit
                    // new filter flags (ignore shared cost such as other new-filter flags/NALU header/RBSP
                    // terminating bit/byte alignment bits)
                }

                FLOAT32 curTotalCost = curTotalRate * alfSet->lambda[PLANE_CHROMA] + curTotalDistortion;

                if (curTotalCost < prevTotalCost)
                {
                    prevTotalCost = curTotalCost;
                    improvement = TRUE;
                }

                if (curTotalCost < bestFilteredTotalCost)
                {
                    bestFilteredTotalCost = curTotalCost;

                    memcpy(alfSet->bestFilterIdxEnabled, ccAlfFilterIdxEnabled, sizeof(ccAlfFilterIdxEnabled));
                    memcpy(alfSet->bestFilterCoeffSet, ccAlfFilterCoeff, sizeof(ccAlfFilterCoeff));
                    memcpy(alfSet->bestFilterControl, alfSet->filterControl, sizeof(UINT8) * alfSet->frameSizeInCtu);

                    alfSet->bestFilterCount = ccAlfFilterCount;

                    memcpy(bestMapFilterIdxToFilterIdc, mapFilterIdxToFilterIdc, sizeof(mapFilterIdxToFilterIdc));
                }
            }

            trainingIterCount++;

            if (!improvement || trainingIterCount > maxTrainingIterCount || referencingExistingAps)
            {
                keepTraining = FALSE;
            }

        }

    }

    if (bestUnfilteredTotalCost < bestFilteredTotalCost)
    {
        memset (alfSet->bestFilterControl, 0, sizeof(UINT8) * alfSet->frameSizeInCtu);
    }

    // save best coeff and control
    BOOL atleastOneBlockUndergoesFitlering = FALSE;

    for (int controlIdx = 0; alfSet->bestFilterCount > 0 && controlIdx < alfSet->frameSizeInCtu; controlIdx++)
    {
        if (alfSet->bestFilterControl[controlIdx])
        {
            atleastOneBlockUndergoesFitlering = TRUE;
            break;
        }
    }

    alfSet->ccAlfParam.numberValidComponents          = 3;
    alfSet->ccAlfParam.ccAlfFilterEnabled[compId - 1] = atleastOneBlockUndergoesFitlering;
    alfSet->ccAlfParam.newCcAlfFilter[compId - 1]     = alfSet->ccAlfParam.ccAlfFilterEnabled[compId - 1];

    if (compId == PLANE_CHROMA_U)
    {
        alfSet->ccAlfCbEnabled = alfSet->ccAlfParam.ccAlfFilterEnabled[compId - 1];
    }
    else
    {
        alfSet->ccAlfCrEnabled = alfSet->ccAlfParam.ccAlfFilterEnabled[compId - 1];
    }

    if (atleastOneBlockUndergoesFitlering)
    {
        // update the filter control indicators
        if (bestreuseTemporalFilterCoeff != 1)
        {
            short storedBestFilterCoeffSet[XIN_CC_ALF_MAX_FILTER_NUM][XIN_CC_ALF_MAX_COEFF_NUM];

            for (int filterIdx=0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
            {
                memcpy (storedBestFilterCoeffSet[filterIdx], alfSet->bestFilterCoeffSet[filterIdx], sizeof(alfSet->bestFilterCoeffSet[filterIdx]));
            }

            memcpy(alfSet->filterControl, alfSet->bestFilterControl, sizeof(uint8_t) * alfSet->frameSizeInCtu);

            int filterCount = 0;

            for ( int filterIdx = 0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++ )
            {
                uint8_t curFilterIdc = bestMapFilterIdxToFilterIdc[filterIdx];

                if (alfSet->bestFilterIdxEnabled[filterIdx])
                {
                    for (int controlIdx = 0; controlIdx < alfSet->frameSizeInCtu; controlIdx++)
                    {
                        if (alfSet->filterControl[controlIdx] == (filterIdx+1) )
                        {
                            alfSet->bestFilterControl[controlIdx] = curFilterIdc;
                        }
                    }

                    memcpy (alfSet->bestFilterCoeffSet[curFilterIdc-1], storedBestFilterCoeffSet[filterIdx], sizeof(storedBestFilterCoeffSet[filterIdx]));

                    filterCount++;
                }

                alfSet->bestFilterIdxEnabled[filterIdx] = ( filterIdx < alfSet->bestFilterCount ) ? TRUE : FALSE;

            }

        }

        alfSet->ccAlfParam.ccAlfFilterCount[compId - 1] = (UINT8)alfSet->bestFilterCount;

        for (int filterIdx = 0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++ )
        {
            memset (alfSet->ccAlfParam.ccAlfCoeff[compId - 1][filterIdx], 0,
                    sizeof(alfSet->ccAlfParam.ccAlfCoeff[compId - 1][filterIdx]));
        }

        memset(alfSet->ccAlfParam.ccAlfFilterIdxEnabled[compId - 1], FALSE,
               sizeof(alfSet->ccAlfParam.ccAlfFilterIdxEnabled[compId - 1]));

        for ( int filterIdx = 0; filterIdx < alfSet->bestFilterCount; filterIdx++ )
        {
            alfSet->ccAlfParam.ccAlfFilterIdxEnabled[compId - 1][filterIdx] = alfSet->bestFilterIdxEnabled[filterIdx];

            memcpy(alfSet->ccAlfParam.ccAlfCoeff[compId - 1][filterIdx], alfSet->bestFilterCoeffSet[filterIdx],
                   sizeof(alfSet->bestFilterCoeffSet[filterIdx]));
        }

        memcpy(alfSet->ccAlfFilterControl[compId - 1], alfSet->bestFilterControl, sizeof(uint8_t) * alfSet->frameSizeInCtu);

    }

    if (alfSet->ccAlfParam.newCcAlfFilter[compId - 1])
    {
        alfSet->alfAps[0].ccAlfParam.newCcAlfFilter[compId - 1] = alfSet->ccAlfParam.newCcAlfFilter[compId - 1];
        alfSet->alfAps[0].ccAlfParam.ccAlfFilterEnabled[compId - 1] = alfSet->ccAlfParam.ccAlfFilterEnabled[compId - 1];
        alfSet->alfAps[0].ccAlfParam.ccAlfFilterCount[compId - 1] = alfSet->ccAlfParam.ccAlfFilterCount[compId - 1];

        memcpy (
            alfSet->ccAlfParam.ccAlfFilterIdxEnabled[compId - 1],
            alfSet->alfAps[0].ccAlfParam.ccAlfFilterIdxEnabled[compId - 1],
            sizeof(BOOL)*XIN_CC_ALF_MAX_FILTER_NUM);

        for (int filterIdx = 0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
        {
            for (int coeffIdx = 0; coeffIdx < XIN_CC_ALF_MAX_COEFF_NUM; coeffIdx++)
            {
                alfSet->alfAps[0].ccAlfParam.ccAlfCoeff[compId - 1][filterIdx][coeffIdx] = alfSet->ccAlfParam.ccAlfCoeff[compId - 1][filterIdx][coeffIdx];
            }
        }
    }

}

void Xin266CcAlfCtu (
    xin_ctu_struct *ctu)
{
    xin_alf_struct   *alfSet;
    xin_seq_struct   *seqSet;
    SINT32           ctuIdx;
    xin_ref_picture  *pictureWrite;
    xin_frame_struct *filterFrame;
    xin_pic_struct   *picSet;
    xin_cc_alf_param *ccAlfParam;
    UINT8            filterIndex;
    intptr_t         filterStrideY;
    intptr_t         reconStrideUv;
    SINT32           ctuPelX;
    SINT32           ctuPelY;

    picSet         = ctu->picSet;
    alfSet         = picSet->alfSet;

    if (!picSet->enableCcAlf)
    {
        return;
    }

    pictureWrite   = picSet->pictureWrite;
    filterFrame    = alfSet->filterFrame;
    filterStrideY  = filterFrame->lumaStride;
    reconStrideUv  = pictureWrite->refStride[PLANE_CHROMA];
    seqSet         = picSet->seqSet;
    ccAlfParam     = &alfSet->ccAlfParam;
    ctuPelX        = ctu->ctuPelX;
    ctuPelY        = ctu->ctuPelY;
    ctuIdx         = ctu->ctuAddr;

    if (ctuIdx == 0)
    {
        Xin266CopyPicture (
            picSet->seqSet,
            pictureWrite,
            filterFrame,
            XIN_CHROMA_MASK);

        Xin26xExtendPicture (
            filterFrame->yuvBuf[1],
            filterFrame->chromaStride,
            filterFrame->lumaWidth/2,
            filterFrame->lumaHeight/2,
            4,
            4);

        Xin26xExtendPicture (
            filterFrame->yuvBuf[2],
            filterFrame->chromaStride,
            filterFrame->lumaWidth/2,
            filterFrame->lumaHeight/2,
            4,
            4);

    }

    filterIndex = alfSet->ccAlfFilterControl[0][ctuIdx];

    if (filterIndex != 0)
    {
        Xin266FilterBlockCcAlf (
            filterFrame->yuvBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*filterStrideY),
            filterStrideY,
            pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
            reconStrideUv,
            ccAlfParam->ccAlfCoeff[0][filterIndex - 1],
            ctu->width >> 1,
            ctu->height >> 1,
            (seqSet->ctuSize - 4),
            seqSet->ctuSize);
    }

    filterIndex = alfSet->ccAlfFilterControl[1][ctuIdx];

    if (filterIndex != 0)
    {
        Xin266FilterBlockCcAlf (
            filterFrame->yuvBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*filterStrideY),
            filterStrideY,
            pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
            reconStrideUv,
            ccAlfParam->ccAlfCoeff[1][filterIndex - 1],
            ctu->width >> 1,
            ctu->height >> 1,
            (seqSet->ctuSize - 4),
            seqSet->ctuSize);
    }

}

void Xin266CcAlfFrame (
    xin_pic_struct *picSet)
{
    xin_alf_struct   *alfSet;
    xin_seq_struct   *seqSet;
    SINT32           ctuIdx;
    xin_ref_picture  *reconFrame;
    xin_frame_struct *filterFrame;
    xin_ctu_struct   *ctu;
    xin_cc_alf_param *ccAlfParam;
    UINT8            filterIndex;
    intptr_t         filterStrideY;
    intptr_t         reconStrideUv;
    SINT32           ctuPelX;
    SINT32           ctuPelY;

    alfSet         = picSet->alfSet;
    reconFrame     = picSet->pictureWrite;
    filterFrame    = alfSet->filterFrame;
    filterStrideY  = filterFrame->lumaStride;
    reconStrideUv  = reconFrame->refStride[PLANE_CHROMA];
    seqSet         = picSet->seqSet;
    ccAlfParam     = &alfSet->ccAlfParam;

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        ctu         = alfSet->ctu + ctuIdx;
        ctuPelX     = ctu->ctuPelX;
        ctuPelY     = ctu->ctuPelY;

        filterIndex = alfSet->ccAlfFilterControl[0][ctuIdx];

        if (filterIndex != 0)
        {
            Xin266FilterBlockCcAlf (
                filterFrame->yuvBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*filterStrideY),
                filterStrideY,
                reconFrame->refBuf[PLANE_CHROMA_U] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
                reconStrideUv,
                ccAlfParam->ccAlfCoeff[0][filterIndex - 1],
                ctu->width >> 1,
                ctu->height >> 1,
                (seqSet->ctuSize - 4),
                seqSet->ctuSize);
        }

        filterIndex = alfSet->ccAlfFilterControl[1][ctuIdx];

        if (filterIndex != 0)
        {
            Xin266FilterBlockCcAlf (
                filterFrame->yuvBuf[PLANE_LUMA] + (ctuPelX + ctuPelY*filterStrideY),
                filterStrideY,
                reconFrame->refBuf[PLANE_CHROMA_V] + ((ctuPelX + ctuPelY*reconStrideUv) >> 1),
                reconStrideUv,
                ccAlfParam->ccAlfCoeff[1][filterIndex - 1],
                ctu->width >> 1,
                ctu->height >> 1,
                (seqSet->ctuSize - 4),
                seqSet->ctuSize);
        }

    }

}

void Xin266ALfFuncInit (
    xin_alf_struct *alfSet,
    UINT32         cpuFeature)
{
    xin_alf_func *funcSet;

    funcSet = alfSet->funcSet;

    funcSet->pfXinGetPreBlkStats = Xin266GetPreBlkStats;
    funcSet->pfXinCalcCoeffError = !alfSet->useNonLinearAlfLuma && !alfSet->useNonLinearAlfChroma ? Xin266CalcCoeffError_Lin : Xin266CalcCoeffError;
    funcSet->pfXinAlfBlockLuma   = Xin266AlfBlockLuma;
    funcSet->pfXinAlfBlockChroma = Xin266AlfBlockChroma;
    funcSet->pfXinAlfDeriveClass = Xin266AlfDeriveClass;

#ifdef _X86_OPT_
#ifdef ENABLE_10BIT_ENCODER

#else
    if (cpuFeature & XIN_CPU_AVX2)
    {
        funcSet->pfXinGetPreBlkStats = Xin266GetPreBlkStats_AVX2;
        funcSet->pfXinCalcCoeffError = !alfSet->useNonLinearAlfLuma && !alfSet->useNonLinearAlfChroma ? Xin266CalcCoeffErrorLin_AVX2 : funcSet->pfXinCalcCoeffError;
        funcSet->pfXinAlfBlockLuma   = Xin266AlfBlockLuma_AVX2;
        funcSet->pfXinAlfBlockChroma = Xin266AlfBlockChroma_AVX2;
        funcSet->pfXinAlfDeriveClass = Xin266AlfDeriveClass_AVX2;
    }
#endif
#endif
}
