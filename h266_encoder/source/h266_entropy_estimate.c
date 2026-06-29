/***************************************************************************//**
 *
 * @file          h266_entropy_estimate.c
 * @brief         h266 syntax and coefficient rate estimation.
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
#include "string.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
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
#include "video_macro.h"
#include "h266_bit_stream.h"
#include "h266_enc_init.h"
#include "h266_scan_order.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h266_get_neighbour_mv.h"
#include "assert.h"
#include "h266_func_struct.h"

extern const UINT32 cabacbinFracBits[256][2];
extern const SINT32 amvrPrecision[4];
extern const UINT8  tbMax[257];
extern const UINT32 lastSigXYGroupIdx[];
extern const UINT64 lftGrpSigMask[6];
extern const UINT64 rgtGrpSigMask[6];
extern const UINT32 goRiceParsCoeff[32];
extern const UINT32 bcwCodingOrder[XIN_BCW_NUM];

static const UINT32 prefixCtx[8] =
{
    0, 0, 0, 3, 6, 10, 15, 21
};

static const UINT32 cbfCtxOffset[3] =
{
    XIN_CO_QT_CBF_Y,
    XIN_CO_QT_CBF_U,
    XIN_CO_QT_CBF_V
};

#define COEF_REMAIN_BIN_REDUCTION       5 ///< indicates the level at which the VLC transitions from Golomb-Rice to TU+EG(k)
#define MAX_TU_LEVEL_CTX_CODED_BIN      28
#define ENTROPY_WRITE_UPDATE(x)         {int a = XIN_ABS(x); sumAbs += XIN_MIN(4+(a&1), a); numPos+=!!a;}

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

static inline void Xin266EstimateBin (
    UINT32         binValue,
    BOOL           updateConext,
    xin_prob_model *context,
    UINT32         *bitNum)
{
    UINT32  state;

    state   = (context->state[0] + context->state[1]) >> 8;
    *bitNum = cabacbinFracBits[state][binValue];

    if (updateConext)
    {
        Xin266StateUpdate (
            context,
            binValue);
    }
}

static inline void Xin266EstimateRemAbsEP (
    UINT32  binsValue,
    UINT32  goRicePar,
    UINT32  cutOff,
    SINT32  maxLgTrRange,
    UINT32  *bitNum)
{
    UINT32 threshold;
    UINT32 totalBits;
    UINT32 maxPreLength;
    UINT32 prefixLength;
    SINT32 codeValue;
    UINT32 suffixLength;

    threshold = cutOff << goRicePar;
    totalBits = 0;

    if (binsValue < threshold)
    {
        totalBits += ((binsValue >> goRicePar) + 1 + goRicePar) << 15;
    }
    else
    {
        maxPreLength = 32 - cutOff - maxLgTrRange;
        prefixLength = 0;
        codeValue    = (binsValue >> goRicePar) - cutOff;

        if (codeValue >= ((1 << maxPreLength) - 1))
        {
            prefixLength = maxPreLength;
            suffixLength = maxLgTrRange;
        }
        else
        {
            while (codeValue > ((2 << prefixLength) - 2))
            {
                prefixLength++;
            }

            suffixLength = prefixLength + goRicePar + 1; //+1 for the separator bit
        }

        totalBits += (cutOff + prefixLength + suffixLength) << 15;

    }

    *bitNum = totalBits;

}

static inline void Xin266EstimateEGKBin (
    UINT32           value,
    UINT32           k,
    UINT32           *bitNum)
{
    SINT32 numBins;

    numBins = 0;

    while (value >= (UINT32)(1<<k))
    {
        numBins++;
        value -= 1 << k;
        k++;
    }

    numBins++;

    numBins += k;

    *bitNum = numBins << 15;

}

static void Xin266EstimateTruncBinCode (
    SINT32  symbol,
    SINT32  maxSymbol,
    UINT32  *bitNum)
{
    SINT32 thresh;
    SINT32 threshVal;
    SINT32 val;
    SINT32 b;

    if (maxSymbol > 256)
    {
        threshVal = 1 << 8;
        thresh    = 8;

        while (threshVal <= maxSymbol)
        {
            thresh++;
            threshVal <<= 1;
        }

        thresh--;
    }
    else
    {
        thresh = tbMax[maxSymbol];
    }

    val = 1 << thresh;
    b   = maxSymbol - val;

    if (symbol < val - b)
    {
        *bitNum = thresh << 15;
    }
    else
    {
        *bitNum = (thresh + 1) << 15;
    }

}

static inline void Xin266EstimateMaxUvlc (
    UINT32  value,
    UINT32  *bitNum)
{
    *bitNum  = (value+1) << 15;
}

void Xin266EstimateSaoType (
    xin_prob_model *context,
    SINT32         saoType,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        saoType >= 0,
        FALSE,
        context + XIN_CO_SAO_TYPE_IDX,
        bitNum);

    *bitNum += (saoType >= 0) << 15;
}

void Xin266EstiamteSaoBoOffsets (
    SINT8   offset[4],
    UINT32  *bitNum)
{
    UINT32 numOfBit;

    Xin266EstimateMaxUvlc (
        XIN_ABS(offset[0]),
        &numOfBit);

    *bitNum = numOfBit;

    Xin266EstimateMaxUvlc (
        XIN_ABS(offset[1]),
        &numOfBit);

    *bitNum += numOfBit;

    Xin266EstimateMaxUvlc (
        XIN_ABS(offset[2]),
        &numOfBit);

    *bitNum += numOfBit;

    Xin266EstimateMaxUvlc (
        XIN_ABS(offset[3]),
        &numOfBit);

    *bitNum += numOfBit;
    *bitNum += (offset[0] != 0) << 15;
    *bitNum += (offset[1] != 0) << 15;
    *bitNum += (offset[2] != 0) << 15;
    *bitNum += (offset[3] != 0) << 15;

    // code band position
    *bitNum += 5 << 15;

}

void Xin266EstiamteSaoEoOffsets (
    SINT8   offset[5],
    UINT32  *bitNum)
{
    UINT32 numOfBit;

    // code the offset values
    Xin266EstimateMaxUvlc (
        offset[0],
        &numOfBit);

    *bitNum = numOfBit;

    Xin266EstimateMaxUvlc (
        offset[1],
        &numOfBit);

    *bitNum += numOfBit;

    Xin266EstimateMaxUvlc (
        (UINT32)-offset[3],
        &numOfBit);

    *bitNum += numOfBit;

    Xin266EstimateMaxUvlc (
        (UINT32)-offset[4],
        &numOfBit);

    *bitNum += numOfBit;

}

void Xin266EstimateSaoParam (
    xin_prob_model *context,
    xin_ctu_struct *ctu,
    UINT32         planeIdx,
    UINT32         *bitNum)
{
    UINT32 numOfBit;
    SINT32 saoType;

    saoType = (planeIdx == PLANE_LUMA) ? ctu->saoType[PLANE_LUMA] : ctu->saoType[PLANE_CHROMA];
    *bitNum = 0;

    if (planeIdx != PLANE_CHROMA_V)
    {
        Xin266EstimateSaoType (
            context,
            saoType,
            &numOfBit);

        *bitNum += numOfBit;
    }

    if (saoType >= 0)
    {
        if (saoType == XIN_SAO_BO)
        {
            Xin266EstiamteSaoBoOffsets (
                ctu->saoOffset[planeIdx],
                &numOfBit);

            *bitNum += numOfBit;
        }
        else
        {
            if (planeIdx != PLANE_CHROMA_V)
            {
                *bitNum += 2 << XIN_RATE_FRACTION;
            }

            Xin266EstiamteSaoEoOffsets (
                ctu->saoOffset[planeIdx],
                &numOfBit);

            *bitNum += numOfBit;
        }

    }

}

void Xin266EstimateSaoMerge (
    xin_prob_model *context,
    BOOL           mergeFlag,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        mergeFlag,
        FALSE,
        context + XIN_CO_SAO_MERGE_FLAG,
        bitNum);
}

static void Xin266EstimateSkipFlag (
    xin_prob_model *context,
    BOOL           updateContext,
    xin_cu_struct  *cu,
    BOOL           skipFlag,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        skipFlag,
        updateContext,
        context + XIN_CO_SKIP_FLAG + cu->skipContext,
        bitNum);
}

void Xin266EstimateMergeIndex (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mergeIdx,
    UINT32         maxMergeCand,
    UINT32         *bitNum)
{
    UINT32  numOfBit;

    numOfBit = 0;

    if (maxMergeCand > 1)
    {
        Xin266EstimateBin (
            mergeIdx != 0,
            updateContext,
            context + XIN_CO_MERGE_IDX,
            &numOfBit);

        if (mergeIdx != 0)
        {
            numOfBit += ((mergeIdx - ((mergeIdx + 1) == maxMergeCand))<<XIN_RATE_FRACTION);
        }
    }

    *bitNum = numOfBit;

}

void Xin266EstimateAffineMergeIndex (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mergeIdx,
    UINT32         maxMergeCand,
    UINT32         *bitNum)
{
    UINT32  numOfBit;

    numOfBit = 0;

    if (maxMergeCand > 1)
    {
        Xin266EstimateBin (
            mergeIdx != 0,
            updateContext,
            context + XIN_CO_AFF_MERGE_IDX,
            &numOfBit);

        if (mergeIdx != 0)
        {
            numOfBit += ((mergeIdx - ((mergeIdx + 1) == maxMergeCand))<<XIN_RATE_FRACTION);
        }
    }

    *bitNum = numOfBit;

}


static void Xin266EstimatePredMode (
    xin_prob_model *context,
    BOOL           updateContxt,
    xin_cu_struct  *cu,
    UINT32         predMode,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        predMode,
        updateContxt,
        context + XIN_CO_PRED_MODE + cu->predModeContext,
        bitNum);
}

void Xin266EstimateMergeFlag (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mergeFlag,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        mergeFlag,
        updateContext,
        context + XIN_CO_MERGE_FLAG,
        bitNum);
}

void Xin266EstimateSubblockMergeFlag (
    xin_prob_model *context,
    xin_cu_struct  *cu,
    BOOL           updateContext,
    UINT32         mergeFlag,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        mergeFlag,
        updateContext,
        context + cu->affineContext + XIN_CO_SUBBLOCK_MERGE_FLAG,
        bitNum);
}

void Xin266EstimateInterPredIdc (
    xin_prob_model *context,
    BOOL           updateContext,
    SINT32         interPredIdc,
    UINT32         lgWidth,
    UINT32         lgHeight,
    UINT32         *bitNum)
{
    UINT32  numOfBit;
    UINT32  totalBits;
    BOOL    noBiPred;
    UINT32  predIdcCtxInc;

    totalBits     = 0;
    noBiPred      = ((lgWidth == 2) && (lgHeight == 2)) || ((lgWidth + lgHeight) == 5);
    predIdcCtxInc = 7 - ((lgHeight + lgWidth + 1) >> 1);

    if (!noBiPred)
    {
        if (interPredIdc == 2)
        {
            Xin266EstimateBin (
                1,
                updateContext,
                context + XIN_CO_INTER_DIR + predIdcCtxInc,
                &numOfBit);

            totalBits += numOfBit;
        }
        else
        {
            Xin266EstimateBin (
                0,
                updateContext,
                context + XIN_CO_INTER_DIR + predIdcCtxInc,
                &numOfBit);

            totalBits += numOfBit;

            Xin266EstimateBin (
                interPredIdc == 1,
                updateContext,
                context + XIN_CO_INTER_DIR + 5,
                &numOfBit);

            totalBits += numOfBit;

        }

    }

    *bitNum = totalBits;

}

void Xin266EstimateMvd (
    xin_prob_model *context,
    BOOL           updateContext,
    xin_mv32_u     *mvd,
    SINT32         imvIdx,
    UINT32         *bitNum)
{
    SINT32     mvdX;
    SINT32     mvdY;

    UINT32     absMvdX;
    UINT32     absMvdY;

    UINT32     mvdXneq0;
    UINT32     mvdYneq0;

    UINT32     absmvdXgt1;
    UINT32     absmvdYgt1;

    UINT32     totalBits;
    UINT32     numOfBit;

    Xin266ChangeMv32Prec (
        mvd,
        XIN_MV_PREC_INTERNAL,
        amvrPrecision[imvIdx]);

    mvdX = mvd->mv.mv32X;
    mvdY = mvd->mv.mv32Y;

    absMvdX = XIN_ABS(mvdX);
    absMvdY = XIN_ABS(mvdY);

    mvdXneq0 = (mvdX != 0);
    mvdYneq0 = (mvdY != 0);

    absmvdXgt1 = (absMvdX > 1);
    absmvdYgt1 = (absMvdY > 1);

    totalBits = 0;

    Xin266EstimateBin (
        mvdXneq0,
        updateContext,
        context + XIN_CO_MVD,
        &numOfBit);

    totalBits += numOfBit;

    Xin266EstimateBin (
        mvdYneq0,
        updateContext,
        context + XIN_CO_MVD,
        &numOfBit);

    totalBits += numOfBit;

    if (mvdXneq0)
    {
        Xin266EstimateBin (
            absmvdXgt1,
            updateContext,
            context + XIN_CO_MVD + 1,
            &numOfBit);

        totalBits += numOfBit;
        totalBits += (1 << XIN_RATE_FRACTION);

        if (absmvdXgt1)
        {
            Xin266EstimateEGKBin (
                (SINT32)absMvdX - 2,
                1,
                &numOfBit);

            totalBits += numOfBit;
        }
    }

    if (mvdYneq0)
    {
        Xin266EstimateBin (
            absmvdYgt1,
            updateContext,
            context + XIN_CO_MVD + 1,
            &numOfBit);

        totalBits += numOfBit;
        totalBits += (1 << XIN_RATE_FRACTION);

        if (absmvdYgt1)
        {
            Xin266EstimateEGKBin (
                (SINT32)absMvdY - 2,
                1,
                &numOfBit);

            totalBits += numOfBit;
        }

    }

    *bitNum = totalBits;

}

void Xin266EstimateNormalMvd (
    xin_prob_model *context,
    BOOL           updateContext,
    xin_mv32_u     *mv,
    xin_mv32_u     *predMv,
    SINT32         imvIdx,
    UINT32         *bitNum)
{
    xin_mv32_u  mvd;

    mvd.mv.mv32X = mv->mv.mv32X - predMv->mv.mv32X;
    mvd.mv.mv32Y = mv->mv.mv32Y - predMv->mv.mv32Y;

    Xin266EstimateMvd (
        context,
        updateContext,
        &mvd,
        imvIdx,
        bitNum);

}

void Xin266EstimateAffineMvd (
    xin_prob_model *context,
    BOOL           updateContext,
    xin_mv32_u     *mv,
    xin_mv32_u     *predMv,
    SINT32         imvIdx,
    UINT32         affineType,
    UINT32         *bitNum)
{
    xin_mv32_u  mvd[3];
    UINT32      mvNum;
    UINT32      mvIdx;
    UINT32      numOfBit;

    mvNum   = affineType ? 3 : 2;
    *bitNum = 0;

    for (mvIdx = 0; mvIdx < mvNum; mvIdx++)
    {
        mvd[mvIdx].mv.mv32X = mv[mvIdx].mv.mv32X - predMv[mvIdx].mv.mv32X;
        mvd[mvIdx].mv.mv32Y = mv[mvIdx].mv.mv32Y - predMv[mvIdx].mv.mv32Y;

        if (mvIdx)
        {
            mvd[mvIdx].mv.mv32X = mvd[mvIdx].mv.mv32X - mvd[0].mv.mv32X;
            mvd[mvIdx].mv.mv32Y = mvd[mvIdx].mv.mv32Y - mvd[0].mv.mv32Y;
        }

        Xin266EstimateMvd (
            context,
            updateContext,
            mvd + mvIdx,
            imvIdx,
            &numOfBit);

        *bitNum += numOfBit;

    }

}


void Xin266EstimateRefIdx (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         refIdx,
    UINT32         refNum,
    UINT32         *bitNum)
{
    UINT32  totalBits;
    UINT32  numOfBit;

    totalBits = 0;

    if (refNum > 1)
    {
        Xin266EstimateBin (
            refIdx > 0,
            updateContext,
            context + XIN_CO_REF_PIC,
            &numOfBit);

        totalBits += numOfBit;

        if ((refIdx > 0) && (refNum > 2))
        {
            refIdx--;
            refNum -= 2;

            Xin266EstimateBin (
                refIdx > 0,
                updateContext,
                context + XIN_CO_REF_PIC + 1,
                &numOfBit);

            totalBits += numOfBit;

            if (refIdx > 0)
            {
                totalBits += (refIdx - (refIdx == refNum)) << XIN_RATE_FRACTION;
            }

        }
    }

    *bitNum = totalBits;

}

void Xin266EstimateMvpIdx (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mvpIdx,
    UINT32         *bitNum)
{
    mvpIdx = mvpIdx > 0;

    Xin266EstimateBin (
        mvpIdx,
        updateContext,
        context + XIN_CO_MVP_IDX,
        bitNum);
}

void Xin266EstimateChromaIntraPredMode (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         chromaMode,
    BOOL           enableCclm,
    UINT32         *bitNum)
{
    UINT32  totalBits;
    UINT32  numOfBit;

    totalBits = 0;

    if (enableCclm)
    {
        if ((chromaMode >= XIN_LM_CHROMA_IDX) && (chromaMode <= XIN_LM_CHROMA_T_IDX))
        {
            Xin266EstimateBin (
                TRUE,
                updateContext,
                context + XIN_CO_CCLM_MODE_FLAG,
                &numOfBit);

            totalBits += numOfBit;

            Xin266EstimateBin (
                chromaMode != XIN_LM_CHROMA_IDX,
                updateContext,
                context + XIN_CO_CCLM_MODE_IDX,
                &numOfBit);

            totalBits += numOfBit;

            if (chromaMode != XIN_LM_CHROMA_IDX)
            {
                totalBits += 1<<XIN_RATE_FRACTION;
            }

            *bitNum = totalBits;

            return;

        }
        else
        {
            Xin266EstimateBin (
                FALSE,
                updateContext,
                context + XIN_CO_CCLM_MODE_FLAG,
                &numOfBit);

            totalBits += numOfBit;
        }

    }

    if (chromaMode == XIN_DM_CHROMA_IDX)
    {
        Xin266EstimateBin (
            FALSE,
            updateContext,
            context + XIN_CO_INTRA_CHROMA_PRED_MODE,
            &numOfBit);

        totalBits += numOfBit;
    }
    else
    {
        Xin266EstimateBin (
            TRUE,
            updateContext,
            context + XIN_CO_INTRA_CHROMA_PRED_MODE,
            &numOfBit);

        totalBits += numOfBit;
        totalBits += 2<<XIN_RATE_FRACTION;
    }

    *bitNum = totalBits;

}

static void Xin266EstimateInterPu (
    xin_sec_struct  *secSet,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_fast_md_buf *mdBuf,
    xin_cu_struct   *cu,
    UINT32          refNum[2],
    BOOL            mvdL1Zero,
    UINT32          frameType,
    UINT32          *bitNum)
{
    UINT32         totalBits;
    UINT32         numOfBit;
    SINT32         interPredIdc;
    SINT32         listIdx;
    UINT32         lgWidth;
    UINT32         lgHeight;
    UINT32         maxMergeCand;
    UINT32         maxAffineMergeCand;
    xin_seq_struct *seqSet;

    lgWidth            = cu->lgWidth;
    lgHeight           = cu->lgHeight;
    totalBits          = 0;
    seqSet             = secSet->seqSet;
    maxMergeCand       = seqSet->config.maxMergeCand;
    maxAffineMergeCand = seqSet->config.maxAffineMergeCand;

    if (mdBuf->type == XIN_SKIP_MODE)
    {
        assert (mdBuf->mergeFlag);
    }
    else
    {
        Xin266EstimateMergeFlag (
            context,
            updateContext,
            mdBuf->mergeFlag,
            &numOfBit);

        totalBits += numOfBit;
    }

    if (mdBuf->mergeFlag)
    {
        if (maxAffineMergeCand)
        {
            Xin266EstimateSubblockMergeFlag (
                context,
                cu,
                updateContext,
                mdBuf->affine,
                &numOfBit);

            totalBits += numOfBit;
        }

        if (mdBuf->affine)
        {
            Xin266EstimateAffineMergeIndex (
                context,
                updateContext,
                mdBuf->mergeIndex,
                maxAffineMergeCand,
                &numOfBit);

            totalBits += numOfBit;
        }
        else
        {
            Xin266EstimateMergeIndex (
                context,
                updateContext,
                mdBuf->mergeIndex,
                maxMergeCand,
                &numOfBit);

            totalBits += numOfBit;
        }

    }
    else
    {
        if (frameType == XIN_B_FRAME)
        {
            if ((mdBuf->refIdx[XIN_LIST_0] >= 0) && (mdBuf->refIdx[XIN_LIST_1] >= 0))
            {
                interPredIdc = 2;
            }
            else if (mdBuf->refIdx[XIN_LIST_0] >= 0)
            {
                interPredIdc = 0;
            }
            else
            {
                interPredIdc = 1;
            }

            Xin266EstimateInterPredIdc (
                context,
                updateContext,
                interPredIdc,
                lgWidth,
                lgHeight,
                &numOfBit);

            totalBits += numOfBit;

        }


        for (listIdx = XIN_LIST_0; listIdx < XIN_LIST_NUM; listIdx++)
        {

            if (mdBuf->refIdx[listIdx] < 0)
            {
                continue;
            }

            // Reference Index
            Xin266EstimateRefIdx (
                context,
                updateContext,
                mdBuf->refIdx[listIdx],
                refNum[listIdx],
                &numOfBit);

            totalBits += numOfBit;

            // Motion Vector Difference
            if ((listIdx == XIN_LIST_1) && (mvdL1Zero) && ((mdBuf->refIdx[XIN_LIST_0] >= 0) && (mdBuf->refIdx[XIN_LIST_1] >= 0)))
            {
                numOfBit = 0;
            }
            else
            {
                if (mdBuf->affine)
                {
                    Xin266EstimateAffineMvd (
                        context,
                        updateContext,
                        mdBuf->affineMv[listIdx],
                        mdBuf->affinePredMv[listIdx],
                        mdBuf->imvIdx,
                        mdBuf->affineType,
                        &numOfBit);
                }
                else
                {
                    Xin266EstimateNormalMvd (
                        context,
                        updateContext,
                        &(mdBuf->mv[listIdx]),
                        &(mdBuf->predMv[listIdx]),
                        mdBuf->imvIdx,
                        &numOfBit);
                }
            }

            totalBits += numOfBit;

            // Motion Vector Prediction Index
            Xin266EstimateMvpIdx (
                context,
                updateContext,
                mdBuf->mvpIndex[listIdx],
                &numOfBit);

            totalBits += numOfBit;

        }

    }

    *bitNum = totalBits;

}

void Xin266EstimateIntraPredMode (
    xin_prob_model *context,
    BOOL           updateContext,
    SINT32         lumaMode,
    xin_pu_struct  *pu,
    UINT32         *bitNum)
{
    SINT8   *intraMPM;
    UINT32  mpmIndex;
    UINT32  totalBits;
    UINT32  numOfBit;
    SINT32  idx;

    totalBits = 0;
    mpmIndex  = XIN_INTRA_MPM_NUM;
    intraMPM = pu->sortedIntraMPM;

    for (idx = 0; idx < XIN_INTRA_MPM_NUM; idx++)
    {
        if (lumaMode == pu->intraMPM[idx])
        {
            mpmIndex = idx;

            break;
        }
    }

    Xin266EstimateBin (
        (mpmIndex < XIN_INTRA_MPM_NUM),
        updateContext,
        context + XIN_CO_INTRA_LUMA_MPM_FLAG,
        &numOfBit);

    totalBits += numOfBit;

    if (mpmIndex != XIN_INTRA_MPM_NUM)
    {
        Xin266EstimateBin (
            (mpmIndex > 0),
            updateContext,
            context + XIN_CO_INTRA_LUMA_PLANAR_FLAG + 1,
            &numOfBit);

        totalBits += numOfBit;
        totalBits += XIN_MIN (XIN_INTRA_MPM_NUM - 2, mpmIndex) << XIN_RATE_FRACTION;

    }
    else
    {
        for (idx = XIN_INTRA_MPM_NUM - 1; idx >= 0; idx--)
        {
            if (lumaMode > intraMPM[idx])
            {
                lumaMode--;
            }
        }

        Xin266EstimateTruncBinCode (
            lumaMode,
            XIN_INTRA_NUM - XIN_INTRA_MPM_NUM,
            &numOfBit);

        totalBits += numOfBit;

    }

    *bitNum = totalBits;

}

static void Xin266EstimateTransSkipFlag (
    xin_prob_model *context,
    BOOL           updateContext,
    BOOL           transSkipFlag,
    UINT32         planeIdx,
    UINT32         *bitNum)
{
    UINT32  transSkipFlagCtx;

    transSkipFlagCtx = (planeIdx == PLANE_LUMA) ? 0 : 1;

    Xin266EstimateBin (
        transSkipFlag,
        updateContext,
        context + XIN_CO_TRANSFORM_SKIP_FLAG + transSkipFlagCtx,
        bitNum);
}

static void Xin266EstimateCbf (
    xin_prob_model *context,
    BOOL           updateContext,
    BOOL           cbf,
    BOOL           prevCbf,
    UINT32         planeIdx,
    UINT32         *bitNum)
{
    UINT32 cbfCtxInc;
    UINT32 cbfCtxOff;

    cbfCtxInc = ((planeIdx == PLANE_CHROMA_V) & (prevCbf != 0)) ? 1 : 0;
    cbfCtxOff = cbfCtxOffset[planeIdx];

    Xin266EstimateBin (
        cbf,
        updateContext,
        context + cbfCtxOff + cbfCtxInc,
        bitNum);

}

static void Xin266EstiamteRootCbf (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         rootCbf,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        rootCbf,
        updateContext,
        context + XIN_CO_QT_ROOT_CBF,
        bitNum);
}

void Xin266EstimateCuCbf (
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          *cbfBits)
{
    UINT32          rootCbfBits;
    UINT32          numOfBit;
    xin_full_md_buf *fullBuf;
    UINT32          tuIdx;
    UINT32          tuNum;
    BOOL            yCbf;
    BOOL            uCbf;
    BOOL            vCbf;

    fullBuf     = fastBuf->fullBuf;
    rootCbfBits = 0;
    *cbfBits    = 0;
    tuNum       = fullBuf->tuNum;
    uCbf        = 0;
    vCbf        = 0;

    if (fastBuf->type != XIN_INTRA_MODE)
    {
        if (!fastBuf->mergeFlag)
        {
            Xin266EstiamteRootCbf (
                context,
                updateContext,
                fullBuf->rootCbf,
                &rootCbfBits);
        }
    }

    if ((fullBuf->rootCbf) || (fastBuf->type == XIN_INTRA_MODE))
    {
        for (tuIdx = 0; tuIdx < tuNum; tuIdx++)
        {
            if (cu->treeMask & XIN_CU_TREE_C_MASK)
            {
                uCbf = ((fullBuf->yuvCbf[fullBuf->mtsIdx[1]][1] >> tuIdx) & 1) != 0;
                vCbf = ((fullBuf->yuvCbf[fullBuf->mtsIdx[2]][2] >> tuIdx) & 1) != 0;

                Xin266EstimateCbf (
                    context,
                    updateContext,
                    uCbf,
                    FALSE,
                    PLANE_CHROMA_U,
                    &numOfBit);

                *cbfBits += numOfBit;

                Xin266EstimateCbf (
                    context,
                    updateContext,
                    vCbf,
                    uCbf,
                    PLANE_CHROMA_V,
                    &numOfBit);

                *cbfBits += numOfBit;

            }

            if (((fastBuf->type == XIN_INTRA_MODE) || (fullBuf->tuNum > 1) || (uCbf || vCbf)) && (cu->treeMask & XIN_CU_TREE_L_MASK))
            {
                yCbf = ((fullBuf->yuvCbf[fullBuf->mtsIdx[0]][0] >> tuIdx) & 1) != 0;

                Xin266EstimateCbf (
                    context,
                    updateContext,
                    yCbf,
                    FALSE,
                    PLANE_LUMA,
                    &numOfBit);

                *cbfBits += numOfBit;

            }

        }

    }

    *cbfBits += rootCbfBits;

}

BOOL Xin266NonZeroMvd (
    xin_fast_md_buf *mdBuf,
    BOOL            mvdL1Zero)
{

    if (mdBuf->refIdx[XIN_LIST_0] >= 0)
    {
        if ((mdBuf->mv[XIN_LIST_0].mv.mv32X != mdBuf->predMv[XIN_LIST_0].mv.mv32X) || (mdBuf->mv[XIN_LIST_0].mv.mv32Y != mdBuf->predMv[XIN_LIST_0].mv.mv32Y))
        {
            return TRUE;
        }
    }

    if ((mdBuf->refIdx[XIN_LIST_1] >= 0) && (!mvdL1Zero))
    {
        if ((mdBuf->mv[XIN_LIST_1].mv.mv32X != mdBuf->predMv[XIN_LIST_1].mv.mv32X) || (mdBuf->mv[XIN_LIST_1].mv.mv32Y != mdBuf->predMv[XIN_LIST_1].mv.mv32Y))
        {
            return TRUE;
        }
    }

    return FALSE;

}

void Xin266EstimateImv (
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_fast_md_buf *mdBuf,
    BOOL            mvdL1Zero,
    UINT32          *bitNum)
{
    UINT32 numOfBit;

    *bitNum = 0;

    if (mdBuf->affine)
    {
        return;
    }

    assert(mdBuf->imvIdx >= XIN_IMV_OFF);
    assert(mdBuf->imvIdx <= XIN_IMV_HPEL);

    if (Xin266NonZeroMvd (mdBuf, mvdL1Zero))
    {
        assert(!mdBuf->mergeFlag);

        // amvr_flag
        Xin266EstimateBin (
            mdBuf->imvIdx > 0,
            updateContext,
            context + XIN_CO_IMV_FLAG,
            &numOfBit);

        *bitNum += numOfBit;

        if (mdBuf->imvIdx)
        {
            // amvr_precision_idx
            Xin266EstimateBin (
                mdBuf->imvIdx < XIN_IMV_HPEL,
                updateContext,
                context + XIN_CO_IMV_FLAG + 4,
                &numOfBit);

            *bitNum += numOfBit;

            if (mdBuf->imvIdx < XIN_IMV_HPEL)
            {
                Xin266EstimateBin (
                    mdBuf->imvIdx > XIN_IMV_FPEL,
                    updateContext,
                    context + XIN_CO_IMV_FLAG + 1,
                    &numOfBit);

                *bitNum += numOfBit;

            }

        }

    }

}

void Xin266EstimateBcw (
    xin_prob_model  *context,
    BOOL            updateContext,
    UINT32          bcwIdx,
    BOOL            checkLdc,
    UINT32          *bitNum)
{
    UINT32  numOfBit;
    SINT32  numBcw;
    SINT32  bcwCodingIdx;
    UINT32  prefixBitNum;
    UINT32  step;
    UINT32  idx;
    SINT32  codingIdx;

    *bitNum      = 0;
    numBcw       = checkLdc ? 5 : 3;
    bcwCodingIdx = bcwCodingOrder[bcwIdx];
    prefixBitNum = numBcw - 2;

    Xin266EstimateBin (
        (bcwCodingIdx == 0 ? 0 : 1),
        updateContext,
        context + XIN_CO_BCW_IDX,
        &numOfBit);

    *bitNum += numOfBit;

    if ((numBcw > 2) && (bcwCodingIdx != 0))
    {
        step      = 1;
        codingIdx = 1;

        for (idx = 0; idx < prefixBitNum; ++idx)
        {
            if (bcwCodingIdx == codingIdx)
            {
                *bitNum += 1 << XIN_RATE_FRACTION;

                break;
            }
            else
            {
                *bitNum += 1 << XIN_RATE_FRACTION;

                codingIdx += step;
            }
        }

    }

}

void Xin266EstimateCuSynatax (
    xin_sec_struct  *secSet,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          *bitNum)
{
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    BOOL            skipFlag;
    BOOL            intraFlag;
    UINT32          frameType;
    UINT32          numOfBit;
    UINT32          totalBits;
    BOOL            isCu4x4;

    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    skipFlag     = fastBuf->type == XIN_SKIP_MODE;
    intraFlag    = fastBuf->type == XIN_INTRA_MODE;
    totalBits    = 0;
    pictureWrite = picSet->pictureWrite;
    frameType    = pictureWrite->frameType;
    isCu4x4      = (cu->height == 4) && (cu->width == 4);

    if ((frameType < XIN_I_FRAME) && (!isCu4x4) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        Xin266EstimateSkipFlag (
            context,
            updateContext,
            cu,
            skipFlag,
            &numOfBit);

        totalBits += numOfBit;
    }

    if (skipFlag == TRUE)
    {
        Xin266EstimateInterPu (
            secSet,
            context,
            updateContext,
            fastBuf,
            cu,
            pictureWrite->numOfRefs,
            pictureWrite->mvdL1Zero,
            frameType,
            &numOfBit);

        totalBits += numOfBit;

        *bitNum = totalBits;

        return;

    }

    if ((frameType < XIN_I_FRAME) && (!isCu4x4) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        Xin266EstimatePredMode (
            context,
            updateContext,
            cu,
            intraFlag,
            &numOfBit);

        totalBits += numOfBit;
    }

    if (intraFlag)
    {
        if (cu->treeMask & XIN_CU_TREE_L_MASK)
        {
            Xin266EstimateIntraPredMode (
                context,
                updateContext,
                fastBuf->intraLumaMode,
                &cu->pu,
                &numOfBit);

            totalBits += numOfBit;
        }
    }
    else
    {
        Xin266EstimateInterPu (
            secSet,
            context,
            updateContext,
            fastBuf,
            cu,
            pictureWrite->numOfRefs,
            pictureWrite->mvdL1Zero,
            frameType,
            &numOfBit);

        totalBits += numOfBit;

        if ((seqSet->config.enableAmvr) && (!fastBuf->mergeFlag))
        {
            Xin266EstimateImv (
                context,
                updateContext,
                fastBuf,
                pictureWrite->mvdL1Zero,
                &numOfBit);

            totalBits += numOfBit;
        }

        if ((seqSet->config.enableBcw) && (!fastBuf->mergeFlag) && (cu->width * cu->height >= 256)
                && (fastBuf->refIdx[0] >= 0) && (fastBuf->refIdx[1] >= 0))
        {
            Xin266EstimateBcw (
                context,
                updateContext,
                fastBuf->bcwIdx,
                pictureWrite->checkLDC,
                &numOfBit);

            totalBits += numOfBit;
        }

    }

    *bitNum = totalBits;

}

static inline void Xin266FindLastSigPos (
    UINT16       *sigCoefMap,
    UINT32       lgCgWidth,
    UINT32       lgCgHeight,
    UINT64       sigGrpMap,
    xin_scan_pos *scanOrder,
    xin_scan_pos *scanOrderCg,
    SINT32       *lastSigPosX,
    SINT32       *lastSigPosY,
    SINT32       *scanPosLast)
{
    UINT32  blockIdx;
    UINT32  coeffIdx;
    UINT32  blockX;
    UINT32  blockY;
    UINT16  sigMap;

    BIT_SCAN_REVERSE_64(sigGrpMap, blockIdx);

    blockX   = scanOrderCg[blockIdx].posX;
    blockY   = scanOrderCg[blockIdx].posY;
    sigMap   = *(sigCoefMap + blockIdx);

    BIT_SCAN_REVERSE_32(sigMap, coeffIdx);

    *lastSigPosX = scanOrder[coeffIdx].posX;
    *lastSigPosY = scanOrder[coeffIdx].posY;

    *lastSigPosX += blockX << lgCgWidth;
    *lastSigPosY += blockY << lgCgHeight;
    *scanPosLast  = coeffIdx + (blockIdx << (lgCgWidth + lgCgHeight));

}

void Xin266EstimateLastSigXY (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         lastSigXPos,
    UINT32         lastSigYPos,
    const UINT32   lgWidth,
    const UINT32   lgHeight,
    UINT32         compType,
    UINT32         *bitNum)
{
    UINT32 xGroupIdx;
    UINT32 yGroupIdx;
    UINT32 maxGrpIdxX;
    UINT32 maxGrpIdxY;
    SINT32 shiftX;
    SINT32 shiftY;
    UINT32 cxtOffX;
    UINT32 cxtOffY;
    UINT32 cxtIdx;
    SINT32 groupCount;
    UINT32 numOfBit;
    UINT32 totalBits;

    if (compType == PLANE_LUMA)
    {
        shiftX = (lgWidth + 1) >> 2;
        shiftY = (lgHeight + 1) >> 2;

        cxtOffX = prefixCtx[lgWidth] + XIN_CO_LAST_X_LUMA;
        cxtOffY = prefixCtx[lgHeight] + XIN_CO_LAST_Y_LUMA;
    }
    else
    {
        shiftX = XIN_CLIP ((1 << lgWidth)  >> 3, 0, 2);
        shiftY = XIN_CLIP ((1 << lgHeight) >> 3, 0, 2);

        cxtOffX = XIN_CO_LAST_X_CHROMA;
        cxtOffY = XIN_CO_LAST_Y_CHROMA;
    }

    totalBits  = 0;
    maxGrpIdxX = lastSigXYGroupIdx[XIN_MIN(1 << lgWidth, 32) - 1];
    maxGrpIdxY = lastSigXYGroupIdx[XIN_MIN(1 << lgHeight, 32) - 1];
    xGroupIdx  = lastSigXYGroupIdx[lastSigXPos];
    yGroupIdx  = lastSigXYGroupIdx[lastSigYPos];

    // X position
    for (cxtIdx = 0; cxtIdx < xGroupIdx; cxtIdx++)
    {
        Xin266EstimateBin (
            1,
            updateContext,
            context + cxtOffX + (cxtIdx>>shiftX),
            &numOfBit);

        totalBits += numOfBit;
    }

    if (xGroupIdx < maxGrpIdxX)
    {
        Xin266EstimateBin (
            0,
            updateContext,
            context + cxtOffX + (cxtIdx >> shiftX),
            &numOfBit);

        totalBits += numOfBit;
    }

    // Y position
    for (cxtIdx = 0; cxtIdx < yGroupIdx; cxtIdx++)
    {
        Xin266EstimateBin (
            1,
            updateContext,
            context + cxtOffY + (cxtIdx >> shiftY),
            &numOfBit);

        totalBits += numOfBit;
    }

    if (yGroupIdx < maxGrpIdxY)
    {
        Xin266EstimateBin (
            0,
            updateContext,
            context + cxtOffY + (cxtIdx >> shiftY),
            &numOfBit);

        totalBits += numOfBit;
    }

    if (xGroupIdx > 3)
    {
        groupCount  = (xGroupIdx - 2 ) >> 1;
        totalBits  += groupCount << XIN_RATE_FRACTION;
    }

    if (yGroupIdx > 3)
    {
        groupCount  = (yGroupIdx - 2) >> 1;
        totalBits  += groupCount << XIN_RATE_FRACTION;
    }

    *bitNum = totalBits;

}

static void Xin266EstimateFullGrpCoeff (
    xin_coeff_context *coeffCtx,
    xin_scan_pos      *scanOrder,
    xin_prob_model    *context,
    BOOL              updateContext,
    COEFF             *coeff,
    intptr_t          coeffStride,
    UINT16            gt0CoefMap,
    SINT32            stateTransTable,
    SINT32            *state,
    UINT32            *bitNum)
{
    SINT32       minSubIdx;
    SINT32       firstSigIdx;
    SINT32       scanIdx;
    SINT32       firstPosMode2;
    SINT32       nextSigIdx;
    SINT32       remRegBins;
    COEFF        coeffVal;
    BOOL         isLast;
    BOOL         gt0Flag;
    BOOL         gt1Flag;
    BOOL         gt2Flag;
    SINT32       posX, posY;
    SINT32       innerIdx;
    SINT32       subSetMask;
    SINT32       numNonZero;
    SINT32       inferSigIdx;
    UINT32       sigCtxIdx;
    SINT32       sumAbs1;
    UINT32       compType;
    UINT32       gtxCtxIdx;
    SINT32       remAbsLevel;
    UINT32       ricePar;
    UINT32       sumAll;
    SINT32       absLevel;
    SINT32       remPar;
    SINT32       pos0Par;
    UINT32       numOfBit;
    UINT32       totalBits;
    SINT32       firstNZPosInCG;
    SINT32       lastNZPosInCG;
    BOOL         sbhFlag;

    minSubIdx   = coeffCtx->minSubIdx;
    isLast      = (coeffCtx->lastScanIdx >> coeffCtx->lgCGSize) == coeffCtx->subCGIdx;
    firstSigIdx = isLast ? coeffCtx->lastScanIdx : coeffCtx->maxSubIdx;
    nextSigIdx  = firstSigIdx;
    remRegBins  = coeffCtx->regBinLimit;
    subSetMask  = (1 << coeffCtx->lgCGSize) - 1;
    numNonZero  = 0;
    inferSigIdx = !isLast ? (coeffCtx->subCGIdx ? minSubIdx : -1 ) : nextSigIdx;
    compType    = coeffCtx->planeIdx != PLANE_LUMA;
    ricePar     = 0;
    totalBits   = 0;
    sumAbs1     = coeffCtx->sumAbs1;
    sbhFlag     = FALSE;

    if ((!isLast) && (coeffCtx->subCGIdx != 0))
    {
        if (gt0CoefMap)
        {
            Xin266EstimateBin (
                1,
                updateContext,
                context + coeffCtx->sigGrpCtx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                &numOfBit);

            totalBits += numOfBit;
        }
        else
        {
            Xin266EstimateBin (
                0,
                updateContext,
                context + coeffCtx->sigGrpCtx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                &numOfBit);

            *bitNum = numOfBit;

            return;
        }

    }

    for ( ; nextSigIdx >= minSubIdx && remRegBins >= 4; nextSigIdx--)
    {
        innerIdx = nextSigIdx & subSetMask;
        gt0Flag  = (gt0CoefMap >> innerIdx) & 1;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;
        coeffVal = coeff[posY*coeffStride + posX];

        if (numNonZero || nextSigIdx != inferSigIdx)
        {
            Xin266GetSigCxtIdx (
                coeff,
                coeffStride,
                compType,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                &sumAbs1,
                &sigCtxIdx,
                *state);

            Xin266EstimateBin (
                gt0Flag,
                updateContext,
                context + XIN_CO_SIG_FLAG_LUMA + XIN_NUM_SIG_FLAG_LUMA_CTX*compType + sigCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

            remRegBins--;

        }
        else if (nextSigIdx != coeffCtx->lastScanIdx)
        {
            Xin266GetSigCxtIdx (
                coeff,
                coeffStride,
                compType,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                &sumAbs1,
                &sigCtxIdx,
                *state);
        }

        if (gt0Flag)
        {
            Xin266GetGtxCxtIdx (
                compType,
                (nextSigIdx == coeffCtx->lastScanIdx) ? -1 : posX + posY,
                sumAbs1,
                &gtxCtxIdx);

            numNonZero++;

            remAbsLevel = XIN_ABS (coeffVal) - 1;

            gt1Flag = !!remAbsLevel;

            Xin266EstimateBin (
                gt1Flag,
                updateContext,
                context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

            remRegBins--;

            if (gt1Flag)
            {
                remAbsLevel -= 1;

                Xin266EstimateBin (
                    remAbsLevel&1,
                    updateContext,
                    context + XIN_CO_PAR_FLAG_LUMA + XIN_NUM_PAR_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                    &numOfBit);

                totalBits += numOfBit;

                remAbsLevel >>= 1;

                remRegBins--;

                gt2Flag = !!remAbsLevel;

                Xin266EstimateBin (
                    gt2Flag,
                    updateContext,
                    context + XIN_CO_GT2_FLAG_LUMA + XIN_NUM_GT2_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                    &numOfBit);

                totalBits += numOfBit;

                remRegBins--;

            }

        }

        *state = (stateTransTable >> ((*state<<2)+((coeffVal&1)<<1))) & 3;

    }

    firstPosMode2 = nextSigIdx;

    coeffCtx->regBinLimit = remRegBins;

    //===== 2nd PASS: Go-rice codes =====
    for (scanIdx = firstSigIdx; scanIdx > firstPosMode2; scanIdx--)
    {
        innerIdx = scanIdx & subSetMask;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;
        absLevel = XIN_ABS (coeff[posY*coeffStride+posX]);

        if (absLevel >= 4)
        {
            Xin266GetAbsSum (
                coeff,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                coeffStride,
                4,
                &sumAll);

            ricePar = goRiceParsCoeff[sumAll];
            remPar  = (absLevel - 4) >> 1;

            Xin266EstimateRemAbsEP (
                remPar,
                ricePar,
                COEF_REMAIN_BIN_REDUCTION,
                15,
                &numOfBit);

            totalBits += numOfBit;

        }

    }

    //===== coeff bypass ====
    for (scanIdx = firstPosMode2; scanIdx >= minSubIdx; scanIdx--)
    {
        innerIdx = scanIdx & subSetMask;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;

        Xin266GetAbsSum (
            coeff,
            posX,
            posY,
            coeffCtx->width,
            coeffCtx->height,
            coeffStride,
            0,
            &sumAll);

        coeffVal = coeff[posY*coeffStride+posX];
        absLevel = XIN_ABS (coeffVal);
        ricePar  = goRiceParsCoeff[sumAll];
        pos0Par  = (*state < 2 ? 1 : 2) << ricePar;
        remPar   = (absLevel == 0 ? pos0Par : absLevel <= pos0Par ? absLevel - 1 : absLevel);

        Xin266EstimateRemAbsEP (
            remPar,
            ricePar,
            COEF_REMAIN_BIN_REDUCTION,
            15,
            &numOfBit);

        totalBits += numOfBit;

        *state = (stateTransTable >> ((*state<<2)+((absLevel&1)<<1))) & 3;

        if (absLevel)
        {
            numNonZero++;
        }

    }

    //===== encode sign's =====
    if (coeffCtx->sbhOn && numNonZero)
    {
        BIT_SCAN_FORWARD_32(gt0CoefMap, firstNZPosInCG);
        BIT_SCAN_REVERSE_32(gt0CoefMap, lastNZPosInCG);

        sbhFlag = (lastNZPosInCG - firstNZPosInCG >= XIN_SBH_THRESHOLD);
    }

    totalBits += (numNonZero - sbhFlag) << XIN_RATE_FRACTION;
    *bitNum    = totalBits;

}

void Xin266EstSigCxtIdx (
    COEFF    *qCoeff,
    intptr_t stride,
    UINT32   compType,
    SINT32   posX,
    SINT32   posY,
    SINT32   width,
    SINT32   height,
    SINT32   *sumAbs1,
    UINT32   *sigCxtIdx)
{
    SINT32   numPos;
    SINT32   sumAbs;
    COEFF    *coeff;
    SINT32   diag;
    SINT32   ctxOffset;

    numPos = 0;
    sumAbs = 0;
    diag   = posX + posY;
    coeff  = qCoeff + posX + posY*stride;

    if (posX < width - 1)
    {
        ENTROPY_WRITE_UPDATE (coeff[1]);
    }

    if (posY < height - 1)
    {
        ENTROPY_WRITE_UPDATE (coeff[stride]);
    }

    ctxOffset = XIN_MIN ((sumAbs+1) >> 1, 3) + (diag < 2 ? 4 : 0);

    if (compType == PLANE_LUMA)
    {
        ctxOffset += diag < 5 ? 4 : 0;
    }

    *sumAbs1   = sumAbs - numPos;
    *sigCxtIdx = ctxOffset;

}

static void Xin266EstimateFastGrpCoeff (
    xin_coeff_context *coeffCtx,
    xin_scan_pos      *scanOrder,
    xin_prob_model    *context,
    BOOL              updateContext,
    COEFF             *coeff,
    intptr_t          coeffStride,
    UINT16            gt0CoefMap,
    UINT32            *bitNum)
{
    SINT32       minSubIdx;
    SINT32       firstSigIdx;
    SINT32       nextSigIdx;
    COEFF        coeffVal;
    BOOL         isLast;
    BOOL         gt0Flag;
    BOOL         gt1Flag;
    SINT32       posX, posY;
    SINT32       innerIdx;
    SINT32       subSetMask;
    SINT32       numNonZero;
    SINT32       inferSigIdx;
    UINT32       sigCtxIdx;
    SINT32       sumAbs1;
    UINT32       compType;
    UINT32       gtxCtxIdx;
    SINT32       remAbsLevel;
    UINT32       numOfBit;
    UINT32       totalBits;
    SINT32       firstNZPosInCG;
    SINT32       lastNZPosInCG;
    BOOL         sbhFlag;

    minSubIdx   = coeffCtx->minSubIdx;
    isLast      = (coeffCtx->lastScanIdx >> coeffCtx->lgCGSize) == coeffCtx->subCGIdx;
    firstSigIdx = isLast ? coeffCtx->lastScanIdx : coeffCtx->maxSubIdx;
    nextSigIdx  = firstSigIdx;
    subSetMask  = (1 << coeffCtx->lgCGSize) - 1;
    numNonZero  = 0;
    inferSigIdx = !isLast ? (coeffCtx->subCGIdx ? minSubIdx : -1 ) : nextSigIdx;
    compType    = coeffCtx->planeIdx != PLANE_LUMA;
    totalBits   = 0;
    sumAbs1     = coeffCtx->sumAbs1;
    sbhFlag     = FALSE;

    if ((!isLast) && (coeffCtx->subCGIdx != 0))
    {
        if (gt0CoefMap)
        {
            Xin266EstimateBin (
                1,
                updateContext,
                context + coeffCtx->sigGrpCtx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                &numOfBit);

            totalBits += numOfBit;
        }
        else
        {
            Xin266EstimateBin (
                0,
                updateContext,
                context + coeffCtx->sigGrpCtx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                &numOfBit);

            *bitNum = numOfBit;

            return;
        }

    }

    for ( ; nextSigIdx >= minSubIdx; nextSigIdx--)
    {
        innerIdx = nextSigIdx & subSetMask;
        gt0Flag  = (gt0CoefMap >> innerIdx) & 1;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;

        if (numNonZero || nextSigIdx != inferSigIdx)
        {
            Xin266EstSigCxtIdx (
                coeff,
                coeffStride,
                compType,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                &sumAbs1,
                &sigCtxIdx);

            Xin266EstimateBin (
                gt0Flag,
                updateContext,
                context + XIN_CO_SIG_FLAG_LUMA + XIN_NUM_SIG_FLAG_LUMA_CTX*compType + sigCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

        }
        else if (nextSigIdx != coeffCtx->lastScanIdx)
        {
            Xin266EstSigCxtIdx (
                coeff,
                coeffStride,
                compType,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                &sumAbs1,
                &sigCtxIdx);
        }

        if (gt0Flag)
        {
            Xin266GetGtxCxtIdx (
                compType,
                (nextSigIdx == coeffCtx->lastScanIdx) ? -1 : posX + posY,
                sumAbs1,
                &gtxCtxIdx);

            numNonZero++;

            coeffVal    = coeff[posY*coeffStride + posX];
            remAbsLevel = XIN_ABS (coeffVal) - 1;

            gt1Flag = !!remAbsLevel;

            Xin266EstimateBin (
                gt1Flag,
                updateContext,
                context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

            if (gt1Flag)
            {
                totalBits += numOfBit;
                totalBits += (remAbsLevel - 1) << XIN_RATE_FRACTION;
            }

        }

    }

    //===== encode sign's =====
    if (coeffCtx->sbhOn && numNonZero)
    {
        BIT_SCAN_FORWARD_32(gt0CoefMap, firstNZPosInCG);
        BIT_SCAN_REVERSE_32(gt0CoefMap, lastNZPosInCG);

        sbhFlag = (lastNZPosInCG - firstNZPosInCG >= XIN_SBH_THRESHOLD);
    }

    totalBits += (numNonZero - sbhFlag) << XIN_RATE_FRACTION;
    *bitNum    = totalBits;

}

void Xin266EstimateFullTuCoeff (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    BOOL            depQuant,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum)
{
    xin_scan_pos      *scanOrder;
    xin_scan_pos      *scanOrderCG;
    SINT32            lastSigPosX;
    SINT32            lastSigPosY;
    SINT32            lastBlockIdx;
    SINT32            blockIdx;
    SINT32            sigGrpCtx;
    UINT32            sigRgt;
    UINT32            sigDwn;
    SINT32            scanPosLast;
    UINT32            width, height;
    UINT32            clipWidth, clipHeight;
    UINT32            lgWidth, lgHeight;
    UINT32            clipLgWidth;
    UINT32            cgWidth, cgHeight;
    UINT32            lgCGWidth, lgCGHeight;
    UINT32            widthInCG;
    UINT32            lgCGSize;
    UINT32            compType;
    UINT32            blockPos;
    UINT32            blockX;
    UINT32            blockY;
    UINT64            sigGrpMapEs;
    UINT64            sigGrpMapRs;
    UINT64            sigGrpMapRgt;
    UINT64            sigGrpMapDwn;
    UINT16            *gt0BitMap;
    UINT16            gt0CoefMap;
    SINT32            remRegBins;
    COEFF             *qCoeff;
    intptr_t          stride;
    UINT32            tuIdx;
    UINT32            totalBits;
    UINT32            numOfBit;
    xin_coeff_context coeffCtx;
    SINT32            state;
    SINT32            stateTab;

    if (!(fullBuf->yuvCbf[mtsIdx][planeIdx] & (1 << tu->tuIdx)))
    {
        *bitNum = 0;

        return;
    }

    tuIdx     = tu->tuIdx;
    compType  = (planeIdx == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA;
    totalBits = 0;
    numOfBit  = 0;
    state     = 0;
    stateTab  = depQuant ? 32040 : 0;

    if ((tu->lgWidth[compType] <= maxTrSkipLgSize) && (tu->lgHeight[compType] <= maxTrSkipLgSize))
    {
        Xin266EstimateTransSkipFlag (
            context,
            updateContext,
            0,
            planeIdx,
            &numOfBit);

        totalBits += numOfBit;
    }

    scanOrderCG = tu->scanOrderCG[compType];
    scanOrder   = tu->scanOrder[compType];
    qCoeff      = fullBuf->qCoefBuf[mtsIdx][planeIdx] + coefPos;
    stride      = fullBuf->coeffStride[compType];
    lgWidth     = tu->lgWidth[compType];
    lgHeight    = tu->lgHeight[compType];
    lgCGWidth   = tu->lgCGWidth[compType];
    lgCGHeight  = tu->lgCGHeight[compType];
    cgWidth     = 1 << lgCGWidth;
    cgHeight    = 1 << lgCGHeight;
    lgCGSize    = lgCGWidth + lgCGHeight;
    width       = 1 << lgWidth;
    height      = 1 << lgHeight;
    gt0BitMap   = fullBuf->gt0BitMap[mtsIdx][planeIdx] + partIdx;
    sigGrpMapEs = fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx];
    sigGrpMapRs = fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx];
    clipHeight  = XIN_MIN (height, 32);
    clipWidth   = XIN_MIN (width, 32);
    remRegBins  = (clipHeight*clipWidth*MAX_TU_LEVEL_CTX_CODED_BIN) >> 4;
    clipLgWidth = XIN_MIN (lgWidth, 5);
    widthInCG   = clipWidth >> lgCGWidth;

    Xin266FindLastSigPos (
        gt0BitMap,
        lgCGWidth,
        lgCGHeight,
        sigGrpMapEs,
        scanOrder,
        scanOrderCG,
        &lastSigPosX,
        &lastSigPosY,
        &scanPosLast);

    sigGrpMapRgt = (sigGrpMapRs >> 1) & rgtGrpSigMask[clipLgWidth];
    sigGrpMapDwn = (sigGrpMapRs >> widthInCG);

    // Encode the position of last significant coefficient
    Xin266EstimateLastSigXY (
        context,
        updateContext,
        lastSigPosX,
        lastSigPosY,
        lgWidth,
        lgHeight,
        compType,
        &numOfBit);

    totalBits += numOfBit;

    coeffCtx.planeIdx    = planeIdx;
    coeffCtx.regBinLimit = remRegBins;
    coeffCtx.width       = clipWidth;
    coeffCtx.height      = clipHeight;
    coeffCtx.cgWidth     = cgWidth;
    coeffCtx.cgHeight    = cgHeight;
    coeffCtx.lgCGSize    = lgCGSize;
    coeffCtx.lastScanIdx = scanPosLast;
    coeffCtx.sumAbs1     = -1;
    coeffCtx.sbhOn       = sbhOn;

    lastBlockIdx = scanPosLast >> lgCGSize;

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        sigRgt     = (sigGrpMapRgt >> blockPos) & 1;
        sigDwn     = (sigGrpMapDwn >> blockPos) & 1;
        sigGrpCtx  = sigRgt | sigDwn;
        gt0CoefMap = gt0BitMap[blockIdx];

        coeffCtx.cgPosX    = blockX*cgWidth;
        coeffCtx.cgPosY    = blockY*cgHeight;
        coeffCtx.sigGrpCtx = sigGrpCtx;
        coeffCtx.minSubIdx = blockIdx << lgCGSize;
        coeffCtx.maxSubIdx = ((blockIdx + 1) << lgCGSize) - 1;
        coeffCtx.subCGIdx  = blockIdx;

        // encode significant_coeffgroup_flag
        Xin266EstimateFullGrpCoeff (
            &coeffCtx,
            scanOrder,
            context,
            updateContext,
            qCoeff,
            stride,
            gt0CoefMap,
            stateTab,
            &state,
            &numOfBit);

        totalBits += numOfBit;

    }

    *bitNum = totalBits;

}

void Xin266EstimateFastTuCoeff (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    BOOL            depQuant,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum)
{
    xin_scan_pos      *scanOrder;
    xin_scan_pos      *scanOrderCG;
    SINT32            lastSigPosX;
    SINT32            lastSigPosY;
    SINT32            lastBlockIdx;
    SINT32            blockIdx;
    SINT32            sigGrpCtx;
    UINT32            sigRgt;
    UINT32            sigDwn;
    SINT32            scanPosLast;
    UINT32            width, height;
    UINT32            clipWidth, clipHeight;
    UINT32            lgWidth, lgHeight;
    UINT32            clipLgWidth;
    UINT32            cgWidth, cgHeight;
    UINT32            lgCGWidth, lgCGHeight;
    UINT32            widthInCG;
    UINT32            lgCGSize;
    UINT32            compType;
    UINT32            blockPos;
    UINT32            blockX;
    UINT32            blockY;
    UINT64            sigGrpMapEs;
    UINT64            sigGrpMapRs;
    UINT64            sigGrpMapRgt;
    UINT64            sigGrpMapDwn;
    UINT16            *gt0BitMap;
    UINT16            gt0CoefMap;
    COEFF             *qCoeff;
    intptr_t          stride;
    UINT32            tuIdx;
    UINT32            totalBits;
    UINT32            numOfBit;
    xin_coeff_context coeffCtx;

    (void)depQuant;

    if (!(fullBuf->yuvCbf[mtsIdx][planeIdx] & (1 << tu->tuIdx)))
    {
        *bitNum = 0;

        return;
    }

    tuIdx     = tu->tuIdx;
    compType  = (planeIdx == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA;
    totalBits = 0;
    numOfBit  = 0;

    if ((tu->lgWidth[compType] <= maxTrSkipLgSize) && (tu->lgHeight[compType] <= maxTrSkipLgSize))
    {
        Xin266EstimateTransSkipFlag (
            context,
            updateContext,
            0,
            planeIdx,
            &numOfBit);

        totalBits += numOfBit;
    }

    scanOrderCG = tu->scanOrderCG[compType];
    scanOrder   = tu->scanOrder[compType];
    qCoeff      = fullBuf->qCoefBuf[mtsIdx][planeIdx] + coefPos;
    stride      = fullBuf->coeffStride[compType];
    lgWidth     = tu->lgWidth[compType];
    lgHeight    = tu->lgHeight[compType];
    lgCGWidth   = tu->lgCGWidth[compType];
    lgCGHeight  = tu->lgCGHeight[compType];
    cgWidth     = 1 << lgCGWidth;
    cgHeight    = 1 << lgCGHeight;
    lgCGSize    = lgCGWidth + lgCGHeight;
    width       = 1 << lgWidth;
    height      = 1 << lgHeight;
    gt0BitMap   = fullBuf->gt0BitMap[mtsIdx][planeIdx] + partIdx;
    sigGrpMapEs = fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx];
    sigGrpMapRs = fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx];
    clipHeight  = XIN_MIN (height, 32);
    clipWidth   = XIN_MIN (width, 32);
    clipLgWidth = XIN_MIN (lgWidth, 5);
    widthInCG   = clipWidth >> lgCGWidth;

    Xin266FindLastSigPos (
        gt0BitMap,
        lgCGWidth,
        lgCGHeight,
        sigGrpMapEs,
        scanOrder,
        scanOrderCG,
        &lastSigPosX,
        &lastSigPosY,
        &scanPosLast);

    sigGrpMapRgt = (sigGrpMapRs >> 1) & rgtGrpSigMask[clipLgWidth];
    sigGrpMapDwn = (sigGrpMapRs >> widthInCG);

    // Encode the position of last significant coefficient
    Xin266EstimateLastSigXY (
        context,
        updateContext,
        lastSigPosX,
        lastSigPosY,
        lgWidth,
        lgHeight,
        compType,
        &numOfBit);

    totalBits += numOfBit;

    coeffCtx.planeIdx    = planeIdx;
    coeffCtx.width       = clipWidth;
    coeffCtx.height      = clipHeight;
    coeffCtx.cgWidth     = cgWidth;
    coeffCtx.cgHeight    = cgHeight;
    coeffCtx.lgCGSize    = lgCGSize;
    coeffCtx.lastScanIdx = scanPosLast;
    coeffCtx.sumAbs1     = -1;
    coeffCtx.sbhOn       = sbhOn;

    lastBlockIdx = scanPosLast >> lgCGSize;

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        sigRgt     = (sigGrpMapRgt >> blockPos) & 1;
        sigDwn     = (sigGrpMapDwn >> blockPos) & 1;
        sigGrpCtx  = sigRgt | sigDwn;
        gt0CoefMap = gt0BitMap[blockIdx];

        coeffCtx.cgPosX    = blockX*cgWidth;
        coeffCtx.cgPosY    = blockY*cgHeight;
        coeffCtx.sigGrpCtx = sigGrpCtx;
        coeffCtx.minSubIdx = blockIdx << lgCGSize;
        coeffCtx.maxSubIdx = ((blockIdx + 1) << lgCGSize) - 1;
        coeffCtx.subCGIdx  = blockIdx;

        // encode significant_coeffgroup_flag
        Xin266EstimateFastGrpCoeff (
            &coeffCtx,
            scanOrder,
            context,
            updateContext,
            qCoeff,
            stride,
            gt0CoefMap,
            &numOfBit);

        totalBits += numOfBit;

    }

    *bitNum = totalBits;

}

static void Xin266EstimateGrpCoeffTs (
    xin_coeff_context *coeffCtx,
    xin_scan_pos      *scanOrder,
    xin_prob_model    *context,
    BOOL              updateContext,
    COEFF             *coeff,
    intptr_t          coeffStride,
    UINT16            gt0CoefMap,
    UINT32            *bitNum)
{
    SINT32       minSubIdx;
    SINT32       firstSigIdx;
    SINT32       nextSigIdx;
    SINT32       remRegBins;
    COEFF        coeffVal;
    BOOL         isLast;
    BOOL         gt0Flag;
    BOOL         gt1Flag;
    BOOL         gt2Flag;
    SINT32       posX, posY;
    SINT32       innerIdx;
    SINT32       subSetMask;
    SINT32       numNonZero;
    SINT32       inferSigIdx;
    UINT32       sigCtxIdx;
    UINT32       signCtxIdx;
    UINT32       gtxCtxIdx;
    SINT32       remAbsLevel;
    SINT32       absLevel;
    SINT32       remPar;
    COEFF        rgtCoeff;
    COEFF        dwnCoeff;
    SINT32       modAbsCoeff;
    SINT32       lastScanIdx1;
    SINT32       lastScanIdx2;
    SINT32       cutOffVal;
    SINT32       binIdx;
    UINT32       numOfBit;
    UINT32       totalBits;

    minSubIdx    = coeffCtx->maxSubIdx;
    isLast       = (coeffCtx->lastScanIdx >> coeffCtx->lgCGSize) == coeffCtx->subCGIdx;
    firstSigIdx  = coeffCtx->minSubIdx;
    nextSigIdx   = firstSigIdx;
    remRegBins   = coeffCtx->regBinLimit;
    subSetMask   = (1 << coeffCtx->lgCGSize) - 1;
    numNonZero   = 0;
    inferSigIdx  = minSubIdx;
    remAbsLevel  = -1;
    lastScanIdx1 = -1;
    lastScanIdx2 = -1;
    totalBits    = 0;

    if ((!isLast) || (!coeffCtx->onlyLastSigGrp))
    {
        if (gt0CoefMap)
        {
            Xin266EstimateBin (
                1,
                updateContext,
                context + coeffCtx->sigGrpCtx + XIN_CO_TS_SIG_COEFF_GROUP,
                &numOfBit);

            totalBits += numOfBit;
        }
        else
        {
            Xin266EstimateBin (
                0,
                updateContext,
                context + coeffCtx->sigGrpCtx + XIN_CO_TS_SIG_COEFF_GROUP,
                &numOfBit);

            totalBits += numOfBit;

            return;
        }

    }

    for ( ; nextSigIdx <= minSubIdx && remRegBins >= 4; nextSigIdx++)
    {
        innerIdx = nextSigIdx & subSetMask;
        gt0Flag  = (gt0CoefMap >> innerIdx) & 1;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;

        if (numNonZero || nextSigIdx != inferSigIdx)
        {
            Xin266GetSigCxtIdxTs (
                coeff,
                coeffStride,
                posX,
                posY,
                &sigCtxIdx);

            Xin266EstimateBin (
                gt0Flag,
                updateContext,
                context + XIN_CO_TS_SIG_FLAG + sigCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

            remRegBins--;
        }

        if (gt0Flag)
        {
            coeffVal = coeff[posY*coeffStride + posX];

            Xin266SignCtxIdTs (
                coeff,
                coeffStride,
                posX,
                posY,
                &signCtxIdx);

            Xin266EstimateBin (
                coeffVal < 0,
                updateContext,
                context + XIN_CO_TS_RESIDUAL_SIGN + signCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

            remRegBins--;
            numNonZero++;

            Xin266GetNeighCoeff (
                coeff,
                coeffStride,
                posX,
                posY,
                &rgtCoeff,
                &dwnCoeff);

            Xin266DeriveModCoeff (
                rgtCoeff,
                dwnCoeff,
                XIN_ABS(coeffVal),
                FALSE,
                &modAbsCoeff);

            Xin266GetSigCxtIdxTs (
                coeff,
                coeffStride,
                posX,
                posY,
                &gtxCtxIdx);

            remAbsLevel = modAbsCoeff - 1;
            gt1Flag     = !!remAbsLevel;

            Xin266EstimateBin (
                gt1Flag,
                updateContext,
                context + XIN_CO_TS_GT1_FLAG + gtxCtxIdx,
                &numOfBit);

            totalBits += numOfBit;

            remRegBins--;

            if (gt1Flag)
            {
                remAbsLevel -= 1;

                Xin266EstimateBin (
                    remAbsLevel&1,
                    updateContext,
                    context + XIN_CO_TS_PAR_FLAG,
                    &numOfBit);

                totalBits += numOfBit;

                remRegBins--;
            }

        }

        lastScanIdx1 = nextSigIdx;

    }

    //===== 2nd PASS: Go-rice codes =====
    for (nextSigIdx = firstSigIdx; nextSigIdx <= minSubIdx && remRegBins >= 4; nextSigIdx++)
    {
        innerIdx  = nextSigIdx & subSetMask;
        posX      = scanOrder[innerIdx].posX;
        posY      = scanOrder[innerIdx].posY;
        posX     += coeffCtx->cgPosX;
        posY     += coeffCtx->cgPosY;
        coeffVal  = coeff[posY*coeffStride + posX];
        cutOffVal = 2;

        Xin266GetNeighCoeff (
            coeff,
            coeffStride,
            posX,
            posY,
            &rgtCoeff,
            &dwnCoeff);

        Xin266DeriveModCoeff (
            rgtCoeff,
            dwnCoeff,
            XIN_ABS(coeffVal),
            FALSE,
            &absLevel);

        for (binIdx = 0; binIdx < 4; binIdx++)
        {
            if (absLevel >= cutOffVal)
            {
                gt2Flag = (absLevel >= (cutOffVal + 2));

                Xin266EstimateBin (
                    gt2Flag,
                    updateContext,
                    context + XIN_CO_TS_GTX_FLAG + (cutOffVal >> 1),
                    &numOfBit);

                totalBits += numOfBit;

                remRegBins--;
            }

            cutOffVal += 2;
        }

        lastScanIdx2 = nextSigIdx;

    }

    coeffCtx->regBinLimit = remRegBins;

    //===== coeff bypass ====
    for (nextSigIdx = firstSigIdx; nextSigIdx <= minSubIdx; nextSigIdx++ )
    {
        innerIdx  = nextSigIdx & subSetMask;
        posX      = scanOrder[innerIdx].posX;
        posY      = scanOrder[innerIdx].posY;
        posX     += coeffCtx->cgPosX;
        posY     += coeffCtx->cgPosY;
        coeffVal  = coeff[posY*coeffStride + posX];
        cutOffVal = (nextSigIdx <= lastScanIdx2 ? 10 : (nextSigIdx <= lastScanIdx1 ? 2 : 0));

        Xin266GetNeighCoeff (
            coeff,
            coeffStride,
            posX,
            posY,
            &rgtCoeff,
            &dwnCoeff);

        Xin266DeriveModCoeff (
            rgtCoeff,
            dwnCoeff,
            XIN_ABS(coeffVal),
            !cutOffVal,
            &absLevel);

        if (absLevel >= cutOffVal)
        {
            remPar = nextSigIdx <= lastScanIdx1 ? (absLevel - cutOffVal) >> 1 : absLevel;

            Xin266EstimateRemAbsEP (
                remPar,
                1,
                COEF_REMAIN_BIN_REDUCTION,
                15,
                &numOfBit);

            totalBits += numOfBit;

            if (absLevel && nextSigIdx > lastScanIdx1)
            {
                totalBits += 1 << 15;
            }
        }

    }

    *bitNum = totalBits;

}

void Xin266EstimateFullTuCoeffTs (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum)
{
    xin_scan_pos      *scanOrder;
    xin_scan_pos      *scanOrderCG;
    SINT32            lastBlockIdx;
    SINT32            blockIdx;
    SINT32            sigGrpCtx;
    UINT32            sigLft;
    UINT32            sigTop;
    SINT32            scanPosLast;
    UINT32            width, height;
    UINT32            lgWidth, lgHeight;
    UINT32            cgWidth, cgHeight;
    UINT32            lgCGWidth, lgCGHeight;
    UINT32            widthInCG;
    UINT32            lgCGSize;
    UINT32            compType;
    UINT32            blockPos;
    UINT32            blockX;
    UINT32            blockY;
    UINT64            sigGrpMapEs;
    UINT64            sigGrpMapRs;
    UINT64            sigGrpMapLft;
    UINT64            sigGrpMapTop;
    UINT16            *gt0BitMap;
    UINT16            gt0CoefMap;
    SINT32            remRegBins;
    COEFF             *qCoeff;
    intptr_t          stride;
    UINT32            tuIdx;
    UINT32            totalBits;
    UINT32            numOfBit;
    xin_coeff_context coeffCtx;

    if (!(fullBuf->yuvCbf[mtsIdx][planeIdx] & (1 << tu->tuIdx)))
    {
        *bitNum = 0;

        return;
    }

    tuIdx     = tu->tuIdx;
    compType  = (planeIdx == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA;
    totalBits = 0;
    numOfBit  = 0;

    if ((tu->lgWidth[compType] <= maxTrSkipLgSize) && (tu->lgHeight[compType] <= maxTrSkipLgSize))
    {
        Xin266EstimateTransSkipFlag (
            context,
            updateContext,
            1,
            planeIdx,
            &numOfBit);

        totalBits += numOfBit;
    }

    scanOrderCG = tu->scanOrderCG[compType];
    scanOrder   = tu->scanOrder[compType];
    qCoeff      = fullBuf->qCoefBuf[mtsIdx][planeIdx] + coefPos;
    stride      = fullBuf->coeffStride[compType];
    lgWidth     = tu->lgWidth[compType];
    lgHeight    = tu->lgHeight[compType];
    lgCGWidth   = tu->lgCGWidth[compType];
    lgCGHeight  = tu->lgCGHeight[compType];
    cgWidth     = 1 << lgCGWidth;
    cgHeight    = 1 << lgCGHeight;
    lgCGSize    = lgCGWidth + lgCGHeight;
    width       = 1 << lgWidth;
    height      = 1 << lgHeight;
    gt0BitMap   = fullBuf->gt0BitMap[mtsIdx][planeIdx] + partIdx;
    sigGrpMapEs = fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx];
    sigGrpMapRs = fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx];
    remRegBins  = (height*width*7) >> 2;
    widthInCG   = width >> lgCGWidth;
    sigGrpMapLft = (sigGrpMapRs << 1) & lftGrpSigMask[lgWidth];
    sigGrpMapTop = (sigGrpMapRs << widthInCG);
    scanPosLast  = width*height - 1;
    lastBlockIdx = scanPosLast >> lgCGSize;

    coeffCtx.planeIdx       = planeIdx;
    coeffCtx.regBinLimit    = remRegBins;
    coeffCtx.width          = width;
    coeffCtx.height         = height;
    coeffCtx.cgWidth        = cgWidth;
    coeffCtx.cgHeight       = cgHeight;
    coeffCtx.lgCGSize       = lgCGSize;
    coeffCtx.lastScanIdx    = scanPosLast;
    coeffCtx.sumAbs1        = -1;
    coeffCtx.sbhOn          = sbhOn;
    coeffCtx.onlyLastSigGrp = (sigGrpMapEs & (~(1<<lastBlockIdx))) == 0;

    lastBlockIdx = scanPosLast >> lgCGSize;

    for (blockIdx = 0; blockIdx <= lastBlockIdx; blockIdx++)
    {
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        sigLft     = (sigGrpMapLft >> blockPos) & 1;
        sigTop     = (sigGrpMapTop >> blockPos) & 1;
        sigGrpCtx  = sigLft + sigTop;
        gt0CoefMap = gt0BitMap[blockIdx];

        coeffCtx.cgPosX    = blockX*cgWidth;
        coeffCtx.cgPosY    = blockY*cgHeight;
        coeffCtx.sigGrpCtx = sigGrpCtx;
        coeffCtx.minSubIdx = blockIdx << lgCGSize;
        coeffCtx.maxSubIdx = ((blockIdx + 1) << lgCGSize) - 1;
        coeffCtx.subCGIdx  = blockIdx;

        // encode significant_coeffgroup_flag
        Xin266EstimateGrpCoeffTs (
            &coeffCtx,
            scanOrder,
            context,
            updateContext,
            qCoeff,
            stride,
            gt0CoefMap,
            &numOfBit);

        totalBits += numOfBit;

    }

    *bitNum = totalBits;

}

void Xin266EstimateCoeff (
    xin_sec_struct  *secSet,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    UINT32          *coeffBits,
    UINT32          planeIdx)
{
    UINT32          tuIdx;
    UINT32          compType;
    xin_tu_struct   *tu;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_full_md_buf *fullBuf;
    UINT32          coefBits;
    intptr_t        coefPos;
    UINT32          partIdx;

    seqSet     = secSet->seqSet;
    *coeffBits = 0;
    compType   = (planeIdx == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA;
    funcSet    = secSet->funcSet;
    fullBuf    = fastBuf->fullBuf;

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        tu       = cu->tu[tuIdx];
        coefBits = 0;

        if ((fullBuf->yuvCbf[mtsIdx][planeIdx] >> tuIdx) & 0x01)
        {
            coefPos = ((cu->offX + tu->offsetX) >> compType) + ((cu->offY + tu->offsetY) >> compType)*fullBuf->coeffStride[compType];
            partIdx = (cu->partIdx + tu->partIdx) >> (compType*2);

            Xin266ReorderCoeff (
                secSet,
                fullBuf,
                mtsIdx,
                coefPos,
                partIdx,
                tu,
                planeIdx);

            if ((seqSet->config.enableRdoq) && (mtsIdx != XIN_MTS_SKIP) && (!seqSet->config.enableDepQuant))
            {
                Xin266RdoQuant (
                    secSet,
                    fastBuf,
                    mtsIdx,
                    coefPos,
                    partIdx,
                    tu,
                    planeIdx);
            }

            Xin266SignBitHidingHdq (
                secSet,
                fullBuf,
                mtsIdx,
                coefPos,
                partIdx,
                tu,
                planeIdx);

            if (mtsIdx == XIN_MTS_SKIP)
            {
                Xin266EstimateFullTuCoeffTs (
                    fullBuf,
                    mtsIdx,
                    partIdx,
                    coefPos,
                    context,
                    updateContext,
                    tu,
                    seqSet->config.enableSignDataHiding,
                    seqSet->config.maxTrSkipLgSize,
                    planeIdx,
                    &coefBits);
            }
            else
            {
                funcSet->pfXinEstimateTuCoeff (
                    fullBuf,
                    mtsIdx,
                    partIdx,
                    coefPos,
                    context,
                    updateContext,
                    tu,
                    seqSet->config.enableSignDataHiding,
                    seqSet->config.enableDepQuant,
                    seqSet->config.maxTrSkipLgSize,
                    planeIdx,
                    &coefBits);
            }

        }

        *coeffBits += coefBits;

    }

}

void Xin266EstimateSplitType (
    xin_prob_model *context,
    BOOL           updateCabacState,
    xin_cu_struct  *cu,
    UINT32         splitType,
    UINT32         *bitNum)
{
    UINT32  numOfBit;
    UINT32  totalBits;
    UINT32  canQt;
    UINT32  canBtt;
    UINT32  canSplit;
    UINT32  canNoSplit;
    UINT32  canHor;
    UINT32  canVer;
    BOOL    isVer;
    BOOL    is12;
    UINT32  can14;
    UINT32  can12;

    totalBits  = 0;
    canSplit   = cu->canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT | XIN_CAN_QUAD_SPLIT);
    canNoSplit = cu->canSplit & XIN_CAN_NO_SPLIT;

    if (canNoSplit && canSplit)
    {
        Xin266EstimateBin (
            splitType != XIN_CU_NO_SPLIT,
            updateCabacState,
            context + XIN_CO_SPLIT_FLAG + cu->splitCtx,
            &numOfBit);

        totalBits += numOfBit;
    }

    if (splitType == XIN_CU_NO_SPLIT)
    {
        *bitNum = totalBits;

        return;
    }

    canQt  = canSplit & XIN_CAN_QUAD_SPLIT;
    canBtt = canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT);

    if (canQt && canBtt)
    {
        Xin266EstimateBin (
            splitType == XIN_CU_QUAD_SPLIT,
            updateCabacState,
            context + XIN_CO_SPLIT_QT_FLAG + cu->qtCtx,
            &numOfBit);

        totalBits += numOfBit;
    }

    if (splitType == XIN_CU_QUAD_SPLIT)
    {
        *bitNum = totalBits;

        return;
    }

    canHor = canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_TRIH_SPLIT);
    canVer = canSplit & (XIN_CAN_VERT_SPLIT | XIN_CAN_TRIV_SPLIT);
    isVer  = (splitType == XIN_CU_VERT_SPLIT) || (splitType == XIN_CU_TRIV_SPLIT);

    if (canVer && canHor)
    {
        Xin266EstimateBin (
            isVer,
            updateCabacState,
            context + XIN_CO_SPLIT_HV_FLAG + cu->hvCtx,
            &numOfBit);

        totalBits += numOfBit;
    }

    can14 = isVer ? canSplit & XIN_CAN_TRIV_SPLIT : canSplit & XIN_CAN_TRIH_SPLIT;
    can12 = isVer ? canSplit & XIN_CAN_VERT_SPLIT : canSplit & XIN_CAN_HORZ_SPLIT;
    is12  = isVer ? (splitType == XIN_CU_VERT_SPLIT) : (splitType == XIN_CU_HORZ_SPLIT);

    if (can12 && can14)
    {
        Xin266EstimateBin (
            is12,
            updateCabacState,
            context + XIN_CO_SPLIT_12_FLAG + (isVer ? cu->verBtCtx : cu->horBtCtx),
            &numOfBit);

        totalBits += numOfBit;
    }

    *bitNum = totalBits;

}

static void Xin266InitRdCbfBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         compType)
{
    UINT32  ctxIndex;

    if (compType == PLANE_LUMA)
    {
        for (ctxIndex = 0; ctxIndex < XIN_NUM_QT_CBF_Y_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_QT_CBF_Y + ctxIndex,
                &cabacEst->blockCbpBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_QT_CBF_Y + ctxIndex,
                &cabacEst->blockCbpBits[ctxIndex][1]);
        }

        Xin266EstimateBin (
            0,
            FALSE,
            context + XIN_CO_QT_ROOT_CBF,
            &cabacEst->blockRootCbpBits[0][0]);

        Xin266EstimateBin (
            1,
            FALSE,
            context + XIN_CO_QT_ROOT_CBF,
            &cabacEst->blockRootCbpBits[0][1]);

    }
    else
    {
        for (ctxIndex = XIN_NUM_QT_CBF_Y_CTX; ctxIndex < XIN_NUM_QT_CBF_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_QT_CBF_Y + ctxIndex,
                &cabacEst->blockCbpBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_QT_CBF_Y + ctxIndex,
                &cabacEst->blockCbpBits[ctxIndex][1]);
        }

    }

}

static void Xin266InitRdLastPosXBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         lgWidth,
    UINT32         compType)
{
    UINT32  ctxIndex;
    UINT32  width;
    UINT32  bitNum0;
    UINT32  bitNum1;
    UINT32  bitsX;
    SINT32  shiftX;
    SINT32  ctxOffset;

    width = XIN_MIN (32, (1 << lgWidth));
    bitsX = 0;

    if (compType == PLANE_LUMA)
    {
        shiftX    = (lgWidth + 1) >> 2;
        ctxOffset = prefixCtx[lgWidth];

        for (ctxIndex = 0; ctxIndex < lastSigXYGroupIdx[width - 1]; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_LAST_X_LUMA + ctxOffset + (ctxIndex>>shiftX),
                &bitNum0);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_LAST_X_LUMA + ctxOffset + (ctxIndex>>shiftX),
                &bitNum1);

            cabacEst->lastXBits[0][lgWidth][ctxIndex] = bitsX + bitNum0;

            bitsX += bitNum1;

        }

        cabacEst->lastXBits[0][lgWidth][ctxIndex] = bitsX;

    }
    else
    {
        shiftX = XIN_CLIP ((1 << lgWidth) >> 3, 0, 2);

        for (ctxIndex = 0; ctxIndex < lastSigXYGroupIdx[width - 1]; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_LAST_X_CHROMA + (ctxIndex>>shiftX),
                &bitNum0);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_LAST_X_CHROMA + (ctxIndex>>shiftX),
                &bitNum1);

            cabacEst->lastXBits[1][lgWidth][ctxIndex] = bitsX + bitNum0;

            bitsX += bitNum1;

        }

        cabacEst->lastXBits[1][lgWidth][ctxIndex] = bitsX;

    }

}

static void Xin266InitRdLastPosYBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         lgHeight,
    UINT32         compType)
{
    UINT32  ctxIndex;
    UINT32  height;
    UINT32  bitNum0;
    UINT32  bitNum1;
    UINT32  bitsY;
    SINT32  shiftY;
    SINT32  ctxOffset;

    height = XIN_MIN (32, (1 << lgHeight));
    bitsY  = 0;

    if (compType == PLANE_LUMA)
    {
        shiftY    = (lgHeight + 1) >> 2;
        ctxOffset = prefixCtx[lgHeight];

        for (ctxIndex = 0; ctxIndex < lastSigXYGroupIdx[height - 1]; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_LAST_Y_LUMA + ctxOffset + (ctxIndex>>shiftY),
                &bitNum0);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_LAST_Y_LUMA + ctxOffset + (ctxIndex>>shiftY),
                &bitNum1);

            cabacEst->lastYBits[0][lgHeight][ctxIndex] = bitsY + bitNum0;

            bitsY += bitNum1;

        }

        cabacEst->lastYBits[0][lgHeight][ctxIndex] = bitsY;

    }
    else
    {
        shiftY = XIN_CLIP ((1 << lgHeight) >> 3, 0, 2);

        for (ctxIndex = 0; ctxIndex < lastSigXYGroupIdx[height - 1]; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_LAST_Y_CHROMA + (ctxIndex>>shiftY),
                &bitNum0);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_LAST_Y_CHROMA + (ctxIndex>>shiftY),
                &bitNum1);

            cabacEst->lastYBits[1][lgHeight][ctxIndex] = bitsY + bitNum0;

            bitsY += bitNum1;

        }

        cabacEst->lastYBits[1][lgHeight][ctxIndex] = bitsY;

    }

}

static void Xin266InitRdGt1Bit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         compType)
{
    UINT32  ctxIndex;

    if (compType == PLANE_LUMA)
    {
        for (ctxIndex = 0; ctxIndex < XIN_NUM_GT1_FLAG_LUMA_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_GT1_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterOneBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_GT1_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterOneBits[ctxIndex][1]);
        }
    }
    else
    {
        for (ctxIndex = XIN_NUM_GT1_FLAG_LUMA_CTX; ctxIndex < XIN_NUM_GT1_FLAG_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_GT1_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterOneBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_GT1_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterOneBits[ctxIndex][1]);
        }
    }

}

static void Xin266InitRdParFlagBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         compType)
{
    UINT32  ctxIndex;

    if (compType == PLANE_LUMA)
    {
        for (ctxIndex = 0; ctxIndex < XIN_NUM_PAR_FLAG_LUMA_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_PAR_FLAG_LUMA + ctxIndex,
                &cabacEst->parFlagBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_PAR_FLAG_LUMA + ctxIndex,
                &cabacEst->parFlagBits[ctxIndex][1]);
        }
    }
    else
    {
        for (ctxIndex = XIN_NUM_PAR_FLAG_LUMA_CTX; ctxIndex < XIN_NUM_PAR_FLAG_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_PAR_FLAG_LUMA + ctxIndex,
                &cabacEst->parFlagBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_PAR_FLAG_LUMA + ctxIndex,
                &cabacEst->parFlagBits[ctxIndex][1]);
        }
    }

}

static void Xin266InitRdGt2Bit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         compType)
{

    UINT32  ctxIndex;

    if (compType == PLANE_LUMA)
    {
        for (ctxIndex = 0; ctxIndex < XIN_NUM_GT2_FLAG_LUMA_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_GT2_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterTwoBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_GT2_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterTwoBits[ctxIndex][1]);
        }
    }
    else
    {
        for (ctxIndex = XIN_NUM_GT2_FLAG_LUMA_CTX; ctxIndex < XIN_NUM_GT2_FLAG_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_GT2_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterTwoBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_GT2_FLAG_LUMA + ctxIndex,
                &cabacEst->greaterTwoBits[ctxIndex][1]);
        }
    }

}

static void Xin266InitRdSigCoeffGrpBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         compType)
{
    UINT32  ctxIndex;

    if (compType == PLANE_LUMA)
    {
        for (ctxIndex = 0; ctxIndex < XIN_NUM_SIG_COEFF_GROUP_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_SIG_COEFF_GROUP_LUMA + ctxIndex,
                &cabacEst->sigCoeffGrpBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_SIG_COEFF_GROUP_LUMA + ctxIndex,
                &cabacEst->sigCoeffGrpBits[ctxIndex][1]);
        }
    }
    else
    {
        for (ctxIndex = XIN_NUM_SIG_COEFF_GROUP_CTX; ctxIndex < XIN_NUM_SIG_COEFF_GROUP_CTX*2; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_SIG_COEFF_GROUP_LUMA + ctxIndex,
                &cabacEst->sigCoeffGrpBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_SIG_COEFF_GROUP_LUMA + ctxIndex,
                &cabacEst->sigCoeffGrpBits[ctxIndex][1]);
        }
    }

}

static void Xin266InitRdSigCoeffBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         compType)
{
    UINT32  ctxIndex;

    if (compType == PLANE_LUMA)
    {
        for (ctxIndex = 0; ctxIndex < XIN_NUM_SIG_FLAG_LUMA_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_SIG_FLAG_LUMA + ctxIndex,
                &cabacEst->sigCoeffBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_SIG_FLAG_LUMA + ctxIndex,
                &cabacEst->sigCoeffBits[ctxIndex][1]);
        }
    }
    else
    {
        for (ctxIndex = XIN_NUM_SIG_FLAG_LUMA_CTX; ctxIndex < XIN_NUM_SIG_FLAG_CTX; ctxIndex++)
        {
            Xin266EstimateBin (
                0,
                FALSE,
                context + XIN_CO_SIG_FLAG_LUMA + ctxIndex,
                &cabacEst->sigCoeffBits[ctxIndex][0]);

            Xin266EstimateBin (
                1,
                FALSE,
                context + XIN_CO_SIG_FLAG_LUMA + ctxIndex,
                &cabacEst->sigCoeffBits[ctxIndex][1]);
        }
    }

}

void Xin266InitRdEstBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         lgWidth,
    UINT32         lgHeight,
    UINT32         compType)
{
    if (!cabacEst->estValid[compType])
    {
        Xin266InitRdSigCoeffGrpBit (
            context,
            cabacEst,
            compType);

        Xin266InitRdCbfBit (
            context,
            cabacEst,
            compType);

        Xin266InitRdGt1Bit (
            context,
            cabacEst,
            compType);

        Xin266InitRdGt2Bit (
            context,
            cabacEst,
            compType);

        Xin266InitRdParFlagBit (
            context,
            cabacEst,
            compType);

        Xin266InitRdSigCoeffBit (
            context,
            cabacEst,
            compType);

        cabacEst->estValid[compType] = TRUE;

    }

    if (!cabacEst->lastXValid[compType][lgWidth])
    {
        Xin266InitRdLastPosXBit (
            context,
            cabacEst,
            lgWidth,
            compType);

        cabacEst->lastXValid[compType][lgWidth] = TRUE;
    }

    if (!cabacEst->lastYValid[compType][lgHeight])
    {
        Xin266InitRdLastPosYBit (
            context,
            cabacEst,
            lgHeight,
            compType);

        cabacEst->lastYValid[compType][lgHeight] = TRUE;
    }

}

void Xin266EstimateAlfCtuFlag (
    xin_prob_model *context,
    UINT32         ctxInc,
    UINT32         planeIdx,
    BOOL           ctuAlfEnabled,
    BOOL           updateContext,
    UINT32         *bitNum)
{
    Xin266EstimateBin (
        ctuAlfEnabled,
        updateContext,
        context + planeIdx*3 + XIN_CO_CTB_ALF_FLAG + ctxInc,
        bitNum);
}

void Xin266EstimateAlfCtuFilterIdx (
    xin_prob_model *context,
    xin_alf_aps    *alfAps,
    UINT32          apsNum,
    UINT32         filterIdx,
    BOOL           updateContext,
    UINT32         *bitNum)
{
    UINT32  filterSetNum;
    BOOL    useTemporalFlt;
    UINT32  numOfBit;

    (void)alfAps;

    filterSetNum = apsNum + XIN_ALF_FIXED_FLT_SET_NUM;
    *bitNum      = 0;

    if (filterSetNum > XIN_ALF_FIXED_FLT_SET_NUM)
    {
        useTemporalFlt = (filterIdx >= XIN_ALF_FIXED_FLT_SET_NUM);

        Xin266EstimateBin (
            useTemporalFlt,
            updateContext,
            context + XIN_CO_USE_TEMPO_FLT,
            &numOfBit);

        *bitNum += numOfBit;

        if (useTemporalFlt)
        {
            if (apsNum > 1)
            {
                Xin266EstimateTruncBinCode (
                    filterIdx - XIN_ALF_FIXED_FLT_SET_NUM,
                    filterSetNum - XIN_ALF_FIXED_FLT_SET_NUM,
                    &numOfBit);

                *bitNum += numOfBit;
            }
        }
        else
        {
            Xin266EstimateTruncBinCode (
                filterIdx,
                XIN_ALF_FIXED_FLT_SET_NUM,
                &numOfBit);

            *bitNum += numOfBit;
        }

    }
    else
    {
        Xin266EstimateTruncBinCode (
            filterIdx,
            XIN_ALF_FIXED_FLT_SET_NUM,
            &numOfBit);

        *bitNum += numOfBit;
    }

}

void Xin266EstimateAlfCtuAlt (
    xin_prob_model *context,
    UINT32         ctbAlfAlt,
    UINT32         altNum,
    UINT32         planeIdx,
    BOOL           updateContext,
    UINT32         *bitNum)
{
    UINT32  idx;
    UINT32  numOfBit;

    *bitNum = 0;

    for (idx = 0; idx < ctbAlfAlt; ++idx)
    {
        Xin266EstimateBin (
            TRUE,
            updateContext,
            context + XIN_CO_CTB_ALF_ALT + planeIdx - 1,
            &numOfBit);

        *bitNum += numOfBit;
    }

    if (ctbAlfAlt < altNum - 1)
    {
        Xin266EstimateBin (
            FALSE,
            updateContext,
            context + XIN_CO_CTB_ALF_ALT + planeIdx - 1,
            &numOfBit);

        *bitNum += numOfBit;
    }

}

void Xin266EstimateCcAlfFltCtrlIdc (
    xin_prob_model *context,
    SINT32         ctxInc,
    UINT32         idcVal,
    UINT32         filterCount,
    BOOL           updateContext,
    UINT32         *bitNum)
{
    UINT32  numOfBit;

    *bitNum = 0;

    Xin266EstimateBin (
        idcVal != 0,
        updateContext,
        context + XIN_CO_CC_ALF_FLT_CTRL_FLAG + ctxInc,
        &numOfBit); // ON/OFF flag is context coded

    *bitNum += numOfBit;

    if (idcVal > 0)
    {
        int val = (idcVal - 1);

        while ( val )
        {
            *bitNum += 1 << XIN_RATE_FRACTION;
            val--;
        }

        if ( idcVal < filterCount )
        {
            *bitNum += 1 << XIN_RATE_FRACTION;
        }
    }

}


