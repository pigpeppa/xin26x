/***************************************************************************//**
 *
 * @file          h266_sub_pel_refine.c
 * @brief         h266 sub-pixel motion estimation refinement.
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
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_section_struct.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h26x_compute_dist.h"
#include "h266_motion_est.h"
#include "h266_inter_prediction.h"
#include "h266_inter_pred_context.h"
#include "h26x_common_data.h"
#include "assert.h"
#include "h266_func_struct.h"

static const xin_mv_s halfOffset[POS_NUM+1] =
{
    {-2, 0}, {2, 0}, {0, -2}, {0, 2}, {-2, -2}, {2, -2}, {-2, 2}, {2, 2}, {0, 0}
};

static const xin_mv_s quarOffset[POS_NUM+1] =
{
    {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}, {0, 0}
};

static const xin_mv_s mvOffset[POS_NUM+1] =
{
    {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}, {0, 0}
};

static inline BOOL CheckMvInRange (
    xin_mv_u *minMv,
    xin_mv_u *maxMv,
    SINT32   mvX,
    SINT32   mvY)
{
    if ((mvX >= minMv->mv.mvX)
            && (mvY >= minMv->mv.mvY)
            && (mvX <= maxMv->mv.mvX)
            && (mvY <= maxMv->mv.mvY))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void Xin266FullSubPelMe (
    xin_sec_struct *secSet,
    SINT32         listIdx,
    SINT32         refIdx)
{
    SINT16          mvX, mvY;
    SINT16          mvpX, mvpY;
    UINT32          bestCost;
    intptr_t        refStride;
    UINT32          lambda;
    UINT32          posIdx;
    PIXEL           predBuf[XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE];
    xin_mv_u        bestMv;
    UINT32          sad;
    UINT32          sadCost;
    xin_mv_u        *minMv;
    xin_mv_u        *maxMv;
    SINT32          testMvX;
    SINT32          testMvY;
    SINT32          frac;
    xin_me_struct   *meSet;
    xin_func_struct *funcSet;
    UINT32          width, height;
    UINT32          lgWidth;

    meSet     = secSet->meSet;
    funcSet   = secSet->funcSet;
    lambda    = meSet->sadLambda;
    mvpX      = meSet->predMv.mv.mvX;
    mvpY      = meSet->predMv.mv.mvY;
    bestCost  = meSet->bestCost;
    bestMv    = meSet->bestMv;
    minMv     = &meSet->minMv;
    maxMv     = &meSet->maxMv;
    mvX       = bestMv.mv.mvX;
    mvY       = bestMv.mv.mvY;
    refStride = meSet->refStride;
    width     = meSet->width;
    height    = meSet->height;
    lgWidth   = calcLog2[width];

    (void)listIdx;
    (void)refIdx;

    if (!CheckMvInRange(minMv, maxMv, mvX>>2, mvY>>2))
    {
        return;
    }
    
    for (posIdx = 0; posIdx < POS_NUM; posIdx++)
    {
        testMvX = mvX + halfOffset[posIdx].mvX;
        testMvY = mvY + halfOffset[posIdx].mvY;
        frac    = (((testMvY << 2) & XIN_MV_FRAC_MASK) << XIN_MV_FRAC_BITS) + ((testMvX << 2) & XIN_MV_FRAC_MASK);

        Xin266LumaInterpolation (
            funcSet,
            meSet->ref + refStride*((testMvY << 2)>>XIN_MV_FRAC_BITS) + ((testMvX << 2)>>XIN_MV_FRAC_BITS),
            refStride,
            predBuf,
            XIN_MAX_CU_SIZE,
            frac,
            lgWidth,
            height);

        funcSet->pfXinComputeSad[lgWidth] (
            meSet->input,
            meSet->inputStride,
            predBuf,
            XIN_MAX_CU_SIZE,
            width,
            height,
            &sad);

        sadCost  = sad << XIN_COST_FRACTION;
        sadCost += CALC_MVD_BIT(testMvX - mvpX, testMvY - mvpY)*lambda;

        if (sadCost < bestCost)
        {
            bestCost = sadCost;

            bestMv.s16x2[0] = (SINT16)(testMvX);
            bestMv.s16x2[1] = (SINT16)(testMvY);
        }

    }

    mvX = bestMv.mv.mvX;
    mvY = bestMv.mv.mvY;

    for (posIdx = 0; posIdx < POS_NUM; posIdx++)
    {
        testMvX = mvX + quarOffset[posIdx].mvX;
        testMvY = mvY + quarOffset[posIdx].mvY;
        frac    = (((testMvY << 2) & XIN_MV_FRAC_MASK) << XIN_MV_FRAC_BITS) + ((testMvX << 2) & XIN_MV_FRAC_MASK);
        
        Xin266LumaInterpolation (
            funcSet,
            meSet->ref + refStride*((testMvY << 2)>>XIN_MV_FRAC_BITS) + ((testMvX << 2)>>XIN_MV_FRAC_BITS),
            refStride,
            predBuf,
            XIN_MAX_CU_SIZE,
            frac,
            lgWidth,
            height);

        funcSet->pfXinComputeSad[lgWidth] (
            meSet->input,
            meSet->inputStride,
            predBuf,
            XIN_MAX_CU_SIZE,
            width,
            height,
            &sad);

        sadCost  = sad << XIN_COST_FRACTION;
        sadCost += CALC_MVD_BIT(testMvX - mvpX, testMvY - mvpY)*lambda;

        if (sadCost < bestCost)
        {
            bestCost = sadCost;

            bestMv.s16x2[0] = (SINT16)(testMvX);
            bestMv.s16x2[1] = (SINT16)(testMvY);
        }

    }

    meSet->bestMv   = bestMv;
    meSet->bestCost = bestCost;

}

void Xin266FastSubPelMeVar (
    xin_me_struct *meSet,
    SINT32        listIdx,
    SINT32        refIdx)
{
    SINT32      mvX, mvY;
    SINT32      intMvX, intMvY;
    SINT32      mvpX, mvpY;
    SINT32      testMvX, testMvY;
    intptr_t    refStride;
    UINT64      lambda;
    UINT32      bestPos;
    xin_mv_u    *minMv;
    xin_mv_u    *maxMv;
    xin_me_func *funcSet;
    UINT32      width, height;
    UINT32      lgWidth;
    SINT32      mvdShift;
    UINT32      varDist;
    UINT64      bestCost;
    UINT64      testCost;
    UINT32      mvBitsFrac;
    UINT32      posIdx;
    UINT32      bufIdx;

    funcSet   = &meSet->funcSet;
    lambda    = meSet->sseLambda;
    mvX       = meSet->bestMv.mv.mvX;
    mvY       = meSet->bestMv.mv.mvY;
    intMvX    = mvX >> 2;
    intMvY    = mvY >> 2;
    mvpX      = meSet->predMv.mv.mvX;
    mvpY      = meSet->predMv.mv.mvY;
    minMv     = &meSet->minMv;
    maxMv     = &meSet->maxMv;
    refStride = meSet->refStride;
    bestCost  = meSet->bestCost;
    width     = meSet->width;
    height    = meSet->height;
    lgWidth   = calcLog2[width];
    mvdShift  = meSet->iMvMode == XIN_IMV_HPEL ? 1 : (meSet->iMvMode << 1);

    (void)listIdx;
    (void)refIdx;

    assert ((mvX & 0x03) == 0);
    assert ((mvY & 0x03) == 0);

    if (!CheckMvInRange(minMv, maxMv, intMvX, intMvY))
    {
        return;
    }

    funcSet->pfXinComputeVar[lgWidth] (
        meSet->input,
        meSet->inputStride,
        meSet->ref + refStride*intMvY + intMvX, 
        refStride, 
        width, 
        height, 
        &varDist);

    bestCost   = varDist << XIN_COST_FRACTION;
    mvBitsFrac = CALC_MVD_BIT_SHIFT(mvX - mvpX, mvY - mvpY, mvdShift) << XIN_RATE_FRACTION;
    bestCost  += CALC_SSE_COST (lambda, mvBitsFrac);
    bestPos    = CEN_POS;

    funcSet->pfXinInterpoHalfPel (
        meSet->ref + refStride*intMvY + intMvX,
        refStride,
        width,
        height,
        meSet->integPel,
        meSet->halfPelH,
        meSet->halfPelV,
        meSet->halfPelHv,
        meSet->interpStride,
        meSet->iMvMode == XIN_IMV_HPEL);

    for (posIdx = 0; posIdx < POS_NUM; posIdx++)
    {
        funcSet->pfXinComputeVar[lgWidth] (
            meSet->input,
            meSet->inputStride,
            meSet->halfPel[posIdx], 
            meSet->interpStride, 
            width, 
            height, 
            &varDist);
        
        testCost   = varDist << XIN_COST_FRACTION;
        testMvX    = mvX + (mvOffset[posIdx].mvX << 1);
        testMvY    = mvY + (mvOffset[posIdx].mvY << 1);
        mvBitsFrac = CALC_MVD_BIT_SHIFT (testMvX - mvpX, testMvY - mvpY, mvdShift) << XIN_RATE_FRACTION;
        testCost  += CALC_SSE_COST (lambda, mvBitsFrac);

        if (testCost < bestCost)
        {
            bestCost = testCost;
            bestPos  = posIdx;
        }
        
    }

    mvX += (mvOffset[bestPos].mvX << 1);
    mvY += (mvOffset[bestPos].mvY << 1);

    if (meSet->iMvMode == XIN_IMV_OFF)
    {
        bufIdx  = bestPos;
        bestPos = CEN_POS;

        for (posIdx = 0; posIdx < POS_NUM; posIdx++)
        {
            funcSet->pfXinComputeAvgVar[lgWidth] (
                meSet->input,
                meSet->inputStride,
                meSet->qBufA[bufIdx][posIdx],
                meSet->qBufB[bufIdx][posIdx], 
                meSet->interpStride, 
                width, 
                height, 
                &varDist);

            testCost   = varDist << XIN_COST_FRACTION;
            testMvX    = mvX + mvOffset[posIdx].mvX;
            testMvY    = mvY + mvOffset[posIdx].mvY;
            mvBitsFrac = CALC_MVD_BIT_SHIFT(testMvX - mvpX, testMvY - mvpY, mvdShift) << XIN_RATE_FRACTION;
            testCost  += CALC_SSE_COST (lambda, mvBitsFrac);

            if (testCost < bestCost)
            {
                bestCost = testCost;
                bestPos  = posIdx;
            }
            
        }
        
        mvX += mvOffset[bestPos].mvX;
        mvY += mvOffset[bestPos].mvY;

    }

    meSet->bestMv.mv.mvX = (SINT16)mvX;
    meSet->bestMv.mv.mvY = (SINT16)mvY;
    meSet->bestPos       = bestPos;

}
