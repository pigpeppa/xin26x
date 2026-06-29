/***************************************************************************//**
 *
 * @file          h266_dep_quant.c
 * @brief         h266 dependent quantization.
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
#include "stdlib.h"
#include "assert.h"
#include "basic_macro.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
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
#include "h266_coding_unit_struct.h"
#include "h266_sub_dct.h"
#include "h266_quant_inv_quant.h"
#include "h266_dep_quant_struct.h"
#include "h26x_common_data.h"
#include "video_macro.h"
#include "h266_func_struct.h"

#define XIN_INNER_UPDATE(k)             {COEFF t=absLevels[scanInfo->nextNbInfoSbb.inPos[k]]; sumAbs+=t; sumAbs1+=XIN_MIN(4+(t&1),t); sumNum+=!!t;}
#define XIN_UPDATE(k)                   {COEFF t=absLevels[scanInfo->nextNbInfoSbb.inPos[k]]; sumAbs+=t;}
#define XIN_UPDATE_DEPS(k)              {ctx_acc *ctx; ctx = &state[idx].absAndCtx.absCtx.ctx[scanInfo->currNbInfoSbb.invInPos[k]]; ctx->sumAbs = (UINT8)XIN_MIN (ctx->sumAbs + absLevel, 255); ctx->tplAcc += 32 + (UINT8)min4Or5;}

extern const bin_frac_bits cabacbinFracBits[256];

static const UINT32 cbfCtxOffset[3] =
{
    XIN_CO_QT_CBF_Y,
    XIN_CO_QT_CBF_U,
    XIN_CO_QT_CBF_V
};

static const UINT32 prefixCtx[] =
{
    0, 0, 0, 3, 6, 10, 15, 21
};

extern const UINT32 lastSigXYGroupIdx[];

static const UINT32 qMult[2][6]  =
{
    {26214, 23302, 20560, 18396, 16384, 14564},

    {18396, 16384, 14564, 13107, 11651, 10280}
};

static const UINT32 iqMult[2][6] =
{
    {40,   45,   51,   57,   64,   72},
    {57,   64,   72,   80,   90,  102}
};

const SINT32 goRiceBits[4][32] =
{
    { 32768,  65536,  98304, 131072, 163840, 196608, 262144, 262144, 327680, 327680, 327680, 327680, 393216, 393216, 393216, 393216, 393216, 393216, 393216, 393216, 458752, 458752, 458752, 458752, 458752, 458752, 458752, 458752, 458752, 458752, 458752, 458752},
    { 65536,  65536,  98304,  98304, 131072, 131072, 163840, 163840, 196608, 196608, 229376, 229376, 294912, 294912, 294912, 294912, 360448, 360448, 360448, 360448, 360448, 360448, 360448, 360448, 425984, 425984, 425984, 425984, 425984, 425984, 425984, 425984},
    { 98304,  98304,  98304,  98304, 131072, 131072, 131072, 131072, 163840, 163840, 163840, 163840, 196608, 196608, 196608, 196608, 229376, 229376, 229376, 229376, 262144, 262144, 262144, 262144, 327680, 327680, 327680, 327680, 327680, 327680, 327680, 327680},
    {131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 163840, 163840, 163840, 163840, 163840, 163840, 163840, 163840, 196608, 196608, 196608, 196608, 196608, 196608, 196608, 196608, 229376, 229376, 229376, 229376, 229376, 229376, 229376, 229376}
};

const UINT32 auiGoRiceParsCoeff[32] =
{
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3
};

const xin_decision startDec[2] =
{
    {
        {XIN_MAX_DEP_COST>>1, XIN_MAX_DEP_COST>>1, XIN_MAX_DEP_COST>>1, XIN_MAX_DEP_COST>>1},
        {                 -1,                  -1,                  -1,                  -1},
        {                 -2,                  -2,                  -2,                  -2},
    },

    {
        {XIN_MAX_DEP_COST>>1, XIN_MAX_DEP_COST>>1, XIN_MAX_DEP_COST>>1, XIN_MAX_DEP_COST>>1},
        {                  0,                   0,                   0,                   0},
        {                  4,                   5,                   6,                   7},
    }
};

static inline UINT32 auiGoRicePosCoeff0 (
    SINT32 st,
    UINT32 ricePar)
{
    return (st < 2 ? 1 : 2) << ricePar;
}

void Xin266SetSigSbbFracBits (
    xin_dep_quant   *depQuant,
    xin_prob_model  *context,
    UINT32          chType)
{
    UINT32  ctxId;
    UINT32  state;

    context = (chType == 0) ? context + XIN_CO_SIG_COEFF_GROUP_LUMA : context + XIN_CO_SIG_COEFF_GROUP_CHROMA;

    for (ctxId = 0; ctxId < XIN_NUM_SIG_COEFF_GROUP_CTX; ctxId++)
    {
        state = (context[ctxId].state[0] + context[ctxId].state[1]) >> 8;

        depQuant->sigSbbFracBits[ctxId] = cabacbinFracBits[state];
    }
}

void Xin266SetSigFlagBits (
    xin_dep_quant   *depQuant,
    xin_prob_model  *context,
    UINT32          chType)
{
    UINT32  ctxId;
    UINT32  state;
    UINT32  ctxNum;
    UINT32  ctxSetId;

    context = (chType == 0) ? context + XIN_CO_SIG_FLAG_LUMA : context + XIN_CO_SIG_FLAG_CHROMA;
    ctxNum  = (chType == 0) ? XIN_NUM_SIG_FLAG_STATE_LUMA_CTX : XIN_NUM_SIG_FLAG_STATE_CHROMA_CTX;

    for (ctxSetId = 0; ctxSetId < 3; ctxSetId++)
    {
        for (ctxId = 0; ctxId < ctxNum; ctxId++)
        {
            state = (context[ctxId].state[0] + context[ctxId].state[1]) >> 8;

            depQuant->sigFracBits[ctxSetId][ctxId] = cabacbinFracBits[state];
        }

        context += ctxNum;
    }

}

void Xin266SetGtxFlagBits (
    xin_dep_quant   *depQuant,
    xin_prob_model  *context,
    UINT32          chType)
{
    UINT32          ctxId;
    UINT32          state;
    UINT32          ctxNum;
    SINT32          par0;
    SINT32          par1;
    bin_frac_bits   fbPar;
    bin_frac_bits   fbGt1;
    bin_frac_bits   fbGt2;
    xin_prob_model  *ctxSetPar;
    xin_prob_model  *ctxSetGt1;
    xin_prob_model  *ctxSetGt2;

    ctxSetPar   = chType == 0 ? context + XIN_CO_PAR_FLAG_LUMA : context + XIN_CO_PAR_FLAG_CHROMA;
    ctxSetGt1   = chType == 0 ? context + XIN_CO_GT1_FLAG_LUMA : context + XIN_CO_GT1_FLAG_LUMA;
    ctxSetGt2   = chType == 0 ? context + XIN_CO_GT2_FLAG_LUMA : context + XIN_CO_GT2_FLAG_LUMA;
    ctxNum      = chType == 0 ? XIN_NUM_GT1_FLAG_LUMA_CTX : XIN_NUM_GT1_FLAG_CHROMA_CTX;

    for (ctxId = 0; ctxId < ctxNum; ctxId++)
    {
        state = (ctxSetPar[ctxId].state[0] + ctxSetPar[ctxId].state[1]) >> 8;
        fbPar = cabacbinFracBits[state];

        state = (ctxSetGt1[ctxId].state[0] + ctxSetGt1[ctxId].state[1]) >> 8;
        fbGt1 = cabacbinFracBits[state];

        state = (ctxSetGt2[ctxId].state[0] + ctxSetGt2[ctxId].state[1]) >> 8;
        fbGt2 = cabacbinFracBits[state];

        par0  = (1<<15) + fbPar.intBits[0];
        par1  = (1<<15) + fbPar.intBits[1];

        depQuant->gtxFracBits[ctxId].intBits[0] = 0;
        depQuant->gtxFracBits[ctxId].intBits[1] = fbGt1.intBits[0] + (1 << 15);
        depQuant->gtxFracBits[ctxId].intBits[2] = fbGt1.intBits[1] + par0 + fbGt2.intBits[0];
        depQuant->gtxFracBits[ctxId].intBits[3] = fbGt1.intBits[1] + par1 + fbGt2.intBits[0];
        depQuant->gtxFracBits[ctxId].intBits[4] = fbGt1.intBits[1] + par0 + fbGt2.intBits[1];
        depQuant->gtxFracBits[ctxId].intBits[5] = fbGt1.intBits[1] + par1 + fbGt2.intBits[1];

    }

}

void Xin266SetLastCoeffOffset (
    xin_dep_quant   *depQuant,
    xin_tu_param    *tuParam,
    xin_fast_md_buf *fastBuf,
    xin_prob_model  *context,
    UINT32          compId)
{
    SINT32          cbfDeltaBits;
    xin_prob_model  *ctxCbf;
    xin_full_md_buf *fullBuf;
    bin_frac_bits   bits;
    UINT32          state;
    UINT32          ctxBits[14];
    UINT32          xy;
    SINT32          bitOffset;
    SINT32          *lastBits;
    SINT32          size;
    SINT32          log2Size;
    UINT32          lastShift;
    UINT32          lastOffset;
    UINT32          sumFBits;
    UINT32          maxCtxId;
    UINT32          ctxId;
    SINT32          posIdx;

    fullBuf      = fastBuf->fullBuf;
    cbfDeltaBits = 0;

    if ((compId == PLANE_LUMA) && (fastBuf->type != XIN_INTRA_MODE) && (fullBuf->tuNum == 1))
    {
        ctxCbf       = context + XIN_CO_QT_ROOT_CBF;
        state        = (ctxCbf[0].state[0] + ctxCbf[0].state[1]) >> 8;
        bits         = cabacbinFracBits[state];
        cbfDeltaBits = (SINT32)(bits.intBits[1] - bits.intBits[0]);
    }
    else
    {
        ctxCbf       = context + cbfCtxOffset[compId];
        state        = (ctxCbf[0].state[0] + ctxCbf[0].state[1]) >> 8;
        bits         = cabacbinFracBits[state];
        cbfDeltaBits = (SINT32)(bits.intBits[1] - bits.intBits[0]);
    }

    for (xy = 0; xy < 2; xy++)
    {
        bitOffset  = xy ? cbfDeltaBits : 0;
        lastBits   = xy ? depQuant->lastBitsY : depQuant->lastBitsX;
        size       = xy ? tuParam->height : tuParam->width;
        log2Size   = calcLog2[size];
        lastShift  = compId == PLANE_LUMA ? (log2Size+1)>>2 : XIN_CLIP ((1 << log2Size) >> 3, 0, 2);
        lastOffset = compId == PLANE_LUMA ? prefixCtx[log2Size] + XIN_CO_LAST_X_LUMA : 0 + XIN_CO_LAST_X_CHROMA;
        sumFBits   = 0;
        maxCtxId   = lastSigXYGroupIdx[XIN_MIN (size, 32) - 1];

        if (xy)
        {
            lastOffset = (compId == PLANE_LUMA) ? lastOffset + (XIN_CO_LAST_Y_LUMA - XIN_CO_LAST_X_LUMA) : lastOffset + (XIN_CO_LAST_Y_CHROMA - XIN_CO_LAST_X_CHROMA);
        }

        for (ctxId = 0; ctxId < maxCtxId; ctxId++)
        {
            state          = (context[(ctxId >> lastShift) + lastOffset].state[0] + context[(ctxId >> lastShift) + lastOffset].state[1]) >> 8;
            bits           = cabacbinFracBits[state];
            ctxBits[ctxId] = sumFBits + bits.intBits[0] + (ctxId > 3 ? ((ctxId-2)>>1)<<15 : 0) + bitOffset;
            sumFBits      += bits.intBits[1];
        }
        ctxBits[maxCtxId] = sumFBits + (maxCtxId > 3 ? ((maxCtxId-2)>>1)<<15 : 0) + bitOffset;

        for (posIdx = 0; posIdx < XIN_MIN(size, 32); posIdx++)
        {
            lastBits[posIdx] = ctxBits[lastSigXYGroupIdx[posIdx]];
        }

    }

}

void Xin266InitContext (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_param    *tuParam,
    xin_prob_model  *context,
    UINT32          compId)
{
    UINT32  chType;

    chType = compId != PLANE_LUMA;

    if (compId != PLANE_CHROMA_V)
    {
        Xin266SetSigSbbFracBits (
            depQuant,
            context,
            chType);

        Xin266SetSigFlagBits (
            depQuant,
            context,
            chType);

        Xin266SetGtxFlagBits (
            depQuant,
            context,
            chType);
    }

    Xin266SetLastCoeffOffset (
        depQuant,
        tuParam,
        fastBuf,
        context,
        compId);

}

SINT32 Xin266CreateDepQuant (
    xin_sec_struct *secSet)
{
    xin_seq_struct *seqSet;
    xin_dep_quant  *depQuant;
    xin_dp_state   *state;
    SINT32         hd, vd;
    SINT32         idx;
    SINT32         stateId;

    seqSet = secSet->seqSet;

    XIN_MALLOC_CHECK (depQuant, sizeof(xin_dep_quant));

    for (hd = 0; hd < XIN_MAX_LG_TU_SIZE + 1; hd++)
    {
        for (vd = 0; vd < XIN_MAX_LG_TU_SIZE + 1; vd++)
        {
            depQuant->tuParam[hd][vd][0] = seqSet->tuParam[hd][vd][0];
            depQuant->tuParam[hd][vd][1] = seqSet->tuParam[hd][vd][1];
        }
    }

    for (idx = 0; idx <= XIN_MAX_LG_TU_SIZE + XIN_MAX_LG_TU_SIZE; idx++)
    {
        depQuant->dpParam[idx] = seqSet->dpParam[idx];
    }

    depQuant->currStates = depQuant->allStates;
    depQuant->prevStates = depQuant->allStates + 4;
    depQuant->skipStates = depQuant->allStates + 8;

    for (idx = 0; idx < 13; idx++)
    {
        state   = idx < 12 ? depQuant->allStates + idx : &depQuant->startState;
        stateId = idx & 0x03;

        state->sbbFracBits.intBits[0] = 0;
        state->sbbFracBits.intBits[1] = 0;
        state->stateId                = (SINT8)stateId;
        state->sigFracBitsArray       = depQuant->sigFracBits[XIN_MAX(stateId - 1, 0)];
        state->gtxFracBitsArray       = depQuant->gtxFracBits;

    }

    depQuant->currSbbCtx = depQuant->allSbbCtx;
    depQuant->prevSbbCtx = depQuant->allSbbCtx + 4;

    for (idx = 0; idx < (XIN_MAX_TU_SIZE * XIN_MAX_TU_SIZE); idx++ )
    {
        memcpy (
            &depQuant->trellis[idx][1],
            startDec + 1,
            sizeof(xin_decision));
    }

    secSet->depQuant  = depQuant;
    depQuant->funcSet = seqSet->funcSet;

    return XIN_SUCCESS;

}

void Xin266DepQuantReset (
    xin_dep_quant *depQuant,
    xin_tu_param  *tuParam)
{
    SINT32 numSbb;
    SINT32 chunkSize;
    UINT8  *nextMem;
    SINT32 idx;

    numSbb    = tuParam->numSbb;
    chunkSize = numSbb + tuParam->numCoeff;
    nextMem   = depQuant->memory;

    memcpy (
        depQuant->sbbFlagBits,
        depQuant->sigSbbFracBits,
        2*sizeof(bin_frac_bits));

    for (idx = 0; idx < 8; idx++, nextMem += chunkSize)
    {
        depQuant->allSbbCtx[idx].sbbFlags = nextMem;
        depQuant->allSbbCtx[idx].levels   = nextMem + numSbb;
    }

    depQuant->nbInfo = tuParam->scanId2NbInfoOut;

}

void Xin266SetScanInfo (
    xin_tu_param  *tuParam,
    xin_scan_info *scanInfo,
    SINT32        scanIdx)
{
    SINT32       nextScanIdx;
    SINT32       diag;
    xin_scan_pos *scanId2BlkPos;
    xin_scan_pos *scanSbbId2SbbPos;
    SINT32       innerX;
    SINT32       innerY;
    SINT32       blockX;
    SINT32       blockY;
    SINT32       blockIdx;
    SINT32       blockPos;
    SINT32       innerIdx;

    scanId2BlkPos        = tuParam->scanId2BlkPos;
    scanSbbId2SbbPos     = tuParam->scanSbbId2SbbPos;
    scanInfo->sbbSize    = tuParam->sbbSize;
    scanInfo->numSbb     = tuParam->numSbb;
    scanInfo->scanIdx    = scanIdx;
    blockIdx             = scanIdx >> tuParam->log2SbbSize;
    innerIdx             = scanIdx & tuParam->sbbMask;
    scanInfo->sbbPos     = scanSbbId2SbbPos[blockIdx].posIdx;
    scanInfo->insidePos  = innerIdx;
    innerX               = scanId2BlkPos[innerIdx].posX;
    innerY               = scanId2BlkPos[innerIdx].posY;
    blockX               = scanSbbId2SbbPos[blockIdx].posX << tuParam->log2SbbWidth;
    blockY               = scanSbbId2SbbPos[blockIdx].posY << tuParam->log2SbbHeight;
    scanInfo->spt        = XIN_SCAN_ISCSBB;

    if (scanInfo->insidePos == tuParam->sbbMask && scanIdx > scanInfo->sbbSize && scanIdx < tuParam->numCoeff - 1)
    {
        scanInfo->spt = XIN_SCAN_SOCSBB;
    }
    else if (scanInfo->insidePos == 0 && scanIdx > 0 && scanIdx < tuParam->numCoeff - tuParam->sbbSize)
    {
        scanInfo->spt = XIN_SCAN_EOCSBB;
    }

    scanInfo->posX = blockX + innerX;
    scanInfo->posY = blockY + innerY;

    if (scanIdx)
    {
        nextScanIdx = scanIdx - 1;
        blockIdx    = nextScanIdx >> tuParam->log2SbbSize;
        blockX      = scanSbbId2SbbPos[blockIdx].posX;
        blockY      = scanSbbId2SbbPos[blockIdx].posY;
        blockPos    = scanSbbId2SbbPos[blockIdx].posIdx;
        innerIdx    = nextScanIdx & tuParam->sbbMask;
        innerX      = scanId2BlkPos[innerIdx].posX;
        innerY      = scanId2BlkPos[innerIdx].posY;
        diag        = (blockX << tuParam->log2SbbWidth) + innerX + (blockY << tuParam->log2SbbHeight) + innerY;

        if (tuParam->chType == PLANE_LUMA)
        {
            scanInfo->sigCtxOffsetNext = (diag < 2 ? 8 : diag < 5 ?  4 : 0);
            scanInfo->gtxCtxOffsetNext = (diag < 1 ? 16 : diag < 3 ? 11 : diag < 10 ? 6 : 1);
        }
        else
        {
            scanInfo->sigCtxOffsetNext = (diag < 2 ? 4 : 0);
            scanInfo->gtxCtxOffsetNext = (diag < 1 ? 6 : 1);
        }

        scanInfo->nextInsidePos = innerIdx;
        scanInfo->currNbInfoSbb = tuParam->scanId2NbInfoSbb[scanIdx];

        if (scanInfo->insidePos == 0)
        {
            scanInfo->nextSbbRight = (blockX < tuParam->widthInSbb - 1 ? blockPos + 1 : 0);
            scanInfo->nextSbbBelow = (blockY < tuParam->heightInSbb - 1 ? blockPos + tuParam->widthInSbb : 0);
        }

    }

}

SINT32 Xin266CreateDepQuantRom (
    xin_seq_struct *seqSet)
{
    UINT32          raster2id[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    UINT32          id2raster[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT32          hd, vd;
    SINT32          scanId;
    UINT32          blockX;
    UINT32          blockY;
    UINT32          coeffX;
    UINT32          coeffY;
    UINT32          coeffPos;
    UINT32          blockNum;
    UINT32          blockIdx;
    UINT32          coeffIdx;
    UINT32          blockWidth;
    UINT32          blockHeight;
    UINT32          blockWidthNZ;
    UINT32          blockHeightNZ;
    UINT32          cgWidth;
    UINT32          cgHeight;
    UINT32          groupSize;
    xin_nb_info_sbb *nbInn;
    xin_nb_info_sbb *sId2NbSbb;
    xin_nb_info_out *sId2NbOut;
    xin_nb_info_out *nbOut;
    xin_scan_info   *scanInfo;
    SINT32          begSbb;
    SINT32          cPos[5];
    SINT32          nk;
    SINT32          idx;
    SINT32          totalCoeff;
    SINT32          compType;
    xin_scan_pos    *scanOrderCG;
    xin_scan_pos    *scanOrder;
    xin_tu_param    *tuParam;
    xin_dp_param    *dpParam;
    SINT32          qpIdx;
    SINT32          qpDQ;
    SINT32          qpPer;
    SINT32          qpRem;
    SINT32          iqShift;
    UINT32          qIdxBd;
    SINT32          qScale;
    SINT32          nomDShift;
    double          qScale2;
    double          nomDistFactor;
    SINT64          pow2dfShift;
    SINT32          dfShift;
    BOOL            isAdjust;
    SINT32          transformShift;
    SINT32          nomTransShift;
    double          lambda;

    if (!seqSet->config.enableDepQuant)
    {
        return XIN_SUCCESS;
    }

    memset (
        seqSet->scanId2NbInfoSbb,
        0,
        sizeof(seqSet->scanId2NbInfoSbb));

    memset (
        seqSet->scanId2NbInfoOut,
        0,
        sizeof(seqSet->scanId2NbInfoOut));

    memset (
        raster2id,
        0,
        sizeof(raster2id));

    for (hd = 0; hd < XIN_MAX_LG_TU_SIZE + 1; hd++)
    {
        for (vd = 0; vd < XIN_MAX_LG_TU_SIZE + 1; vd++)
        {
            if ((hd <= 1 || vd <= 1))
            {
                continue;
            }

            cgWidth       = 4;
            cgHeight      = 4;
            scanOrderCG   = seqSet->scanOrderCG[vd][hd];
            scanOrder     = seqSet->scanOrder[2][2];
            blockWidth    = 1 << hd;
            blockHeight   = 1 << vd;
            blockWidthNZ  = XIN_MIN (blockWidth, 32);
            blockHeightNZ = XIN_MIN (blockHeight, 32);
            totalCoeff    = blockWidthNZ * blockHeightNZ;
            groupSize     = cgWidth * cgHeight;
            blockNum      = totalCoeff / groupSize;

            XIN_MALLOC_CHECK (sId2NbSbb, totalCoeff*sizeof(xin_nb_info_sbb));
            XIN_MALLOC_CHECK (sId2NbOut, totalCoeff*sizeof(xin_nb_info_out));

            memset (sId2NbSbb, 0, sizeof(xin_nb_info_sbb)*totalCoeff);
            memset (sId2NbOut, 0, sizeof(xin_nb_info_out)*totalCoeff);

            seqSet->scanId2NbInfoSbb[hd][vd] = sId2NbSbb;
            seqSet->scanId2NbInfoOut[hd][vd] = sId2NbOut;

            for (blockIdx = 0; blockIdx < blockNum; blockIdx++)
            {
                blockX = scanOrderCG[blockIdx].posX;
                blockY = scanOrderCG[blockIdx].posY;
                blockX = blockX*cgWidth;
                blockY = blockY*cgHeight;

                for (coeffIdx = 0; coeffIdx < groupSize; coeffIdx++)
                {
                    coeffX = scanOrder[coeffIdx].posX + blockX;
                    coeffY = scanOrder[coeffIdx].posY + blockY;

                    id2raster[blockIdx*16+coeffIdx] = coeffX + coeffY*blockWidth;
                }
            }

            for (scanId = 0; scanId < totalCoeff; scanId++)
            {
                raster2id[id2raster[scanId]] = scanId;
            }

            for (blockIdx = 0; blockIdx < blockNum; blockIdx++)
            {
                blockX = scanOrderCG[blockIdx].posX;
                blockY = scanOrderCG[blockIdx].posY;
                blockX = blockX*cgWidth;
                blockY = blockY*cgHeight;

                for (coeffIdx = 0; coeffIdx < groupSize; coeffIdx++)
                {
                    coeffX   = scanOrder[coeffIdx].posX + blockX;
                    coeffY   = scanOrder[coeffIdx].posY + blockY;
                    coeffPos = coeffX + coeffY*blockWidth;
                    scanId   = blockIdx*groupSize + coeffIdx;

                    //===== inside subband neighbours =====
                    nbInn  = sId2NbSbb + scanId;
                    begSbb = scanId - (scanId & (groupSize-1)); // first pos in current subblock

                    cPos[0] = (coeffX + 1 < blockWidthNZ                               ? (raster2id[coeffPos + 1] < groupSize + begSbb              ? raster2id[coeffPos + 1] - begSbb : 0) : 0);
                    cPos[1] = (coeffX + 2 < blockWidthNZ                               ? (raster2id[coeffPos + 2] < groupSize + begSbb              ? raster2id[coeffPos + 2] - begSbb : 0) : 0);
                    cPos[2] = (coeffX + 1 < blockWidthNZ && coeffY + 1 < blockHeightNZ ? (raster2id[coeffPos + 1 + blockWidth] < groupSize + begSbb ? raster2id[coeffPos + 1 + blockWidth] - begSbb : 0) : 0);
                    cPos[3] = (coeffY + 1 < blockHeightNZ                              ? (raster2id[coeffPos + blockWidth] < groupSize + begSbb     ? raster2id[coeffPos + blockWidth] - begSbb : 0) : 0);
                    cPos[4] = (coeffY + 2 < blockHeightNZ                              ? (raster2id[coeffPos + 2 * blockWidth] < groupSize + begSbb ? raster2id[coeffPos + 2 * blockWidth] - begSbb : 0) : 0);

                    SINT32 num;
                    SINT32 inPos[5];

                    inPos[0] = 0;
                    inPos[1] = 0;
                    inPos[2] = 0;
                    inPos[3] = 0;
                    inPos[4] = 0;
                    num      = 0;

                    while (1)
                    {
                        nk = -1;

                        for (idx = 0; idx < 5; idx++)
                        {
                            if (cPos[idx] != 0 && (nk < 0 || cPos[idx] < cPos[nk]))
                            {
                                nk = idx;
                            }
                        }

                        if (nk < 0)
                        {
                            break;
                        }

                        inPos[num++] = (UINT8)(cPos[nk]);

                        cPos[nk] = 0;
                    }

                    for (idx = num; idx < 5; idx++)
                    {
                        inPos[idx] = 0;
                    }

                    for( int k = 0; k < num; k++ )
                    {
                        sId2NbSbb[begSbb + inPos[k]].invInPos[sId2NbSbb[begSbb + inPos[k]].numInv++] = (UINT8)(scanId & ( groupSize - 1 ));
                    }

                    //===== outside subband neighbours =====
                    nbOut  = sId2NbOut + scanId;
                    begSbb = scanId - (scanId & (groupSize-1)); // first pos in current subblock

                    cPos[0] = (coeffX + 1 < blockWidthNZ                               ? ( raster2id[coeffPos+1           ] >= groupSize + begSbb ? raster2id[coeffPos+1           ] : 0 ) : 0 );
                    cPos[1] = (coeffX + 2 < blockWidthNZ                               ? ( raster2id[coeffPos+2           ] >= groupSize + begSbb ? raster2id[coeffPos+2           ] : 0 ) : 0 );
                    cPos[2] = (coeffX + 1 < blockWidthNZ && coeffY + 1 < blockHeightNZ ? ( raster2id[coeffPos+1+blockWidth] >= groupSize + begSbb ? raster2id[coeffPos+1+blockWidth] : 0 ) : 0 );
                    cPos[3] = (coeffY + 1 < blockHeightNZ                              ? ( raster2id[coeffPos+  blockWidth] >= groupSize + begSbb ? raster2id[coeffPos+  blockWidth] : 0 ) : 0 );
                    cPos[4] = (coeffY + 2 < blockHeightNZ                              ? ( raster2id[coeffPos+2*blockWidth] >= groupSize + begSbb ? raster2id[coeffPos+2*blockWidth] : 0 ) : 0 );

                    for (nbOut->num = 0; ; )
                    {
                        nk = -1;

                        for (idx = 0; idx < 5; idx++)
                        {
                            if (cPos[idx] != 0 && ( nk < 0 || cPos[idx] < cPos[nk]))
                            {
                                nk = idx;
                            }
                        }

                        if (nk < 0)
                        {
                            break;
                        }

                        nbOut->outPos[nbOut->num++] = (UINT16)(cPos[nk]);

                        cPos[nk] = 0;

                    }

                    for (idx = nbOut->num; idx < 5; idx++)
                    {
                        nbOut->outPos[idx] = 0;
                    }

                    nbOut->maxDist = (scanId == 0 ? 0 : sId2NbOut[scanId-1].maxDist);

                    for (idx = 0; idx < nbOut->num; idx++)
                    {
                        if (nbOut->outPos[idx] > nbOut->maxDist)
                        {
                            nbOut->maxDist = nbOut->outPos[idx];
                        }
                    }

                }

            }

            // make it relative
            for (scanId = 0; scanId < totalCoeff; scanId++)
            {
                nbOut  = sId2NbOut + scanId;
                begSbb = scanId - (scanId & (groupSize-1)); // first pos in current subblock

                for (idx = 0; idx < nbOut->num; idx++)
                {
                    assert(begSbb <= nbOut->outPos[idx]);
                    nbOut->outPos[idx] -= (SINT16)begSbb;
                }

                nbOut->maxDist -= (UINT16)scanId;
            }

            for (compType = 0; compType < 2; compType++)
            {
                XIN_MALLOC_CHECK (scanInfo, totalCoeff*sizeof(xin_scan_info));
                XIN_MALLOC_CHECK (tuParam,  sizeof(xin_tu_param));

                tuParam->chType             = compType;
                tuParam->width              = blockWidth;
                tuParam->height             = blockHeight;
                tuParam->numCoeff           = blockWidthNZ*blockHeightNZ;
                tuParam->log2SbbWidth       = calcLog2[cgWidth];
                tuParam->log2SbbHeight      = calcLog2[cgHeight];
                tuParam->log2SbbSize        = tuParam->log2SbbWidth + tuParam->log2SbbHeight;
                tuParam->sbbSize            = 1 << tuParam->log2SbbSize;
                tuParam->sbbMask            = tuParam->sbbSize - 1;
                tuParam->widthInSbb         = blockWidthNZ >> tuParam->log2SbbWidth;
                tuParam->heightInSbb        = blockHeightNZ >> tuParam->log2SbbHeight;
                tuParam->numSbb             = tuParam->widthInSbb * tuParam->heightInSbb;
                tuParam->scanId2NbInfoSbb   = seqSet->scanId2NbInfoSbb[hd][vd];
                tuParam->scanId2NbInfoOut   = seqSet->scanId2NbInfoOut[hd][vd];
                tuParam->scanSbbId2SbbPos   = scanOrderCG;
                tuParam->scanId2BlkPos      = scanOrder;
                tuParam->scanInfo           = scanInfo;

                for (scanId = 0; scanId < totalCoeff; scanId++)
                {

                    Xin266SetScanInfo (
                        tuParam,
                        scanInfo + scanId,
                        scanId);
                }

                seqSet->tuParam[hd][vd][compType] = tuParam;

            }

        }

    }

    for (idx = 0; idx < XIN_MAX_LG_TU_SIZE + XIN_MAX_LG_TU_SIZE + 1; idx++)
    {
        XIN_MALLOC_CHECK (dpParam, sizeof(xin_dp_param)*XIN_QP_NUM);

        seqSet->dpParam[idx] = dpParam;

        for (qpIdx = 0; qpIdx <= XIN_MAX_QP; qpIdx++)
        {
            isAdjust       = idx & 1;
            qpDQ           = qpIdx + 1;
            qpPer          = qpDQ / 6;
            qpRem          = qpDQ - 6 * qpPer;
            nomTransShift  = 15 - XIN_INTERNAL_BIT_DEPTH - (idx >> 1);
            transformShift = nomTransShift - isAdjust;
            lambda         = 0.57 * pow(2.0, (qpIdx - 12) / 3.0);

            // quant parameters
            dpParam->qShift  = XIN_QUANT_SHIFT  - 1 + qpPer + transformShift;
            dpParam->qAdd    = -((3 << dpParam->qShift) >> 1);
            iqShift          = XIN_IQUANT_SHIFT + 1 - qpPer - transformShift;
            dpParam->qScale  = qMult[isAdjust][qpRem];
            qIdxBd           = XIN_MIN (15 + 1, 8*sizeof(SINT32) + iqShift - XIN_IQUANT_SHIFT - 1);
            dpParam->maxQIdx = (1 << (qIdxBd-1)) - 4;
            dpParam->iqShift = XIN_IQUANT_SHIFT + 1 - transformShift;
            dpParam->iqScale = iqMult[isAdjust][qpRem] << qpPer;

            if (dpParam->qShift)
            {
                dpParam->thresLast = 8 << (dpParam->qShift-1);
            }
            else
            {
                dpParam->thresLast = (8>>1) << dpParam->qShift;
            }

            dpParam->thresSSbb = 3 << dpParam->qShift;

            // distortion calculation parameters
            qScale        = dpParam->qScale;
            nomDShift     = 15 - 2 * nomTransShift + dpParam->qShift + isAdjust;
            qScale2       = (double)(qScale*qScale);
            nomDistFactor = (nomDShift < 0 ? 1.0/((double)((SINT64)1<<(-nomDShift))*qScale2*lambda) : (double)((SINT64)1<<nomDShift)/(qScale2*lambda));
            pow2dfShift   = (SINT64)(nomDistFactor * qScale2) + 1;

            if (pow2dfShift == 0)
            {
                dfShift = -1;
            }
            else
            {
                BIT_SCAN_REVERSE_32 (pow2dfShift - 1, dfShift);

                dfShift += 1;
            }

            dpParam->distShift   = 62 + dpParam->qShift - 2*15 - dfShift;
            dpParam->distAdd     = ((SINT64)1 << dpParam->distShift) >> 1;
            dpParam->distStepAdd = ((dpParam->distShift + dpParam->qShift) >= 64 ? (SINT64)(nomDistFactor * pow (2, dpParam->distShift + dpParam->qShift) + .5) : (SINT64)(nomDistFactor * (double)((SINT64)1<<(dpParam->distShift+dpParam->qShift)) + .5 ));
            dpParam->distOrgFact = (SINT64)(nomDistFactor*(double)((SINT64)1<<(dpParam->distShift+1)) + .5);

            dpParam++;

        }

    }

    return XIN_SUCCESS;

}

void Xin266DeleteDepQuantRom (
    xin_seq_struct *seqSet)
{
    SINT32       hd, vd;
    SINT32       compType;
    xin_tu_param *tuParam;
    UINT32       idx;

    if (!seqSet->config.enableDepQuant)
    {
        return;
    }

    for (hd = 0; hd < XIN_MAX_LG_TU_SIZE + 1; hd++)
    {
        for (vd = 0; vd < XIN_MAX_LG_TU_SIZE + 1; vd++)
        {
            if ((hd <= 1 || vd <= 1))
            {
                continue;
            }

            free (seqSet->scanId2NbInfoSbb[hd][vd]);
            free (seqSet->scanId2NbInfoOut[hd][vd]);

            for (compType = 0; compType < 2; compType++)
            {
                tuParam = seqSet->tuParam[hd][vd][compType];

                free (tuParam->scanInfo);
                free (tuParam);

            }
        }

    }

    for (idx = 0; idx < XIN_MAX_LG_TU_SIZE + XIN_MAX_LG_TU_SIZE + 1; idx++)
    {
        free (seqSet->dpParam[idx]);
    }

}

static void Xin266CheckRdCostSkipSbbZeroOut (
    xin_dp_state *state,
    xin_decision *decision)
{
    SINT32 idx;

    for (idx = 0; idx < 4; idx++)
    {
        decision->rdCost[idx]   = state[idx].rdCost + state[idx].sbbFracBits.intBits[0];
        decision->absLevel[idx] = 0;
        decision->prevId[idx]   = 4 | state[idx].stateId;
    }
}

void Xin266PreQuantCoeff (
    SINT32       absCoeff,
    xin_pq_data  *pqData,
    xin_dp_param *dpParam)
{
    xin_pq_data *pqA, *pqB, *pqC, *pqD;
    SINT64      scaledOrg;
    SINT32      qIdx;
    SINT64      scaledAdd;
    SINT64      quanCoeff;

    quanCoeff = dpParam->qScale;
    scaledOrg = (SINT64)absCoeff * quanCoeff;
    qIdx      = (SINT32)XIN_CLIP ((scaledOrg + dpParam->qAdd) >> dpParam->qShift, 1, dpParam->maxQIdx);
    scaledAdd = qIdx * dpParam->distStepAdd - scaledOrg * dpParam->distOrgFact;
    pqA       = &pqData[(qIdx + 0) & 3];
    pqB       = &pqData[(qIdx + 1) & 3];
    pqC       = &pqData[(qIdx + 2) & 3];
    pqD       = &pqData[(qIdx + 3) & 3];

    pqA->deltaDist    = (((scaledAdd + 0 * dpParam->distStepAdd) * (qIdx + 0) + dpParam->distAdd) >> dpParam->distShift);
    pqA->absLevel     = (qIdx + 1) >> 1;

    pqB->deltaDist    = (((scaledAdd + 1 * dpParam->distStepAdd) * (qIdx + 1) + dpParam->distAdd) >> dpParam->distShift);
    pqB->absLevel     = (qIdx + 2) >> 1;

    pqC->deltaDist    = (((scaledAdd + 2 * dpParam->distStepAdd) * (qIdx + 2) + dpParam->distAdd) >> dpParam->distShift);
    pqC->absLevel     = (qIdx + 3) >> 1;

    pqD->deltaDist    = (((scaledAdd + 3 * dpParam->distStepAdd) * (qIdx + 3) + dpParam->distAdd) >> dpParam->distShift);
    pqD->absLevel     = (qIdx + 4) >> 1;

}

void Xin266CheckRdCosts (
    xin_dp_state *state,
    UINT32       spt,
    xin_pq_data  *pqDataA,
    xin_pq_data  *pqDataB,
    xin_decision *decision,
    SINT32       indexA,
    SINT32       indexB)
{
    const SINT32  *goRiceTab;
    SINT64        rdCostA;
    SINT64        rdCostB;
    SINT64        rdCostZ;
    UINT32        value;

    goRiceTab = goRiceBits[state->goRicePar];
    rdCostA   = state->rdCost + pqDataA->deltaDist;
    rdCostB   = state->rdCost + pqDataB->deltaDist;
    rdCostZ   = state->rdCost;

    if (state->remRegBins >= 4)
    {
        if (pqDataA->absLevel < 4)
        {
            rdCostA += state->coeffFracBits.intBits[pqDataA->absLevel];
        }
        else
        {
            value    = (pqDataA->absLevel - 4) >> 1;
            rdCostA += state->coeffFracBits.intBits[pqDataA->absLevel - (value << 1)] + goRiceTab[XIN_MIN (value, 32 - 1)];
        }

        if (pqDataB->absLevel < 4)
        {
            rdCostB += state->coeffFracBits.intBits[pqDataB->absLevel];
        }
        else
        {
            value = (pqDataB->absLevel - 4) >> 1;
            rdCostB += state->coeffFracBits.intBits[pqDataB->absLevel - (value << 1)] + goRiceTab[XIN_MIN(value, 32 - 1)];
        }

        if (spt == XIN_SCAN_ISCSBB)
        {
            rdCostA += state->sigFracBits.intBits[1];
            rdCostB += state->sigFracBits.intBits[1];
            rdCostZ += state->sigFracBits.intBits[0];
        }
        else if (spt == XIN_SCAN_SOCSBB)
        {
            rdCostA += state->sbbFracBits.intBits[1] + state->sigFracBits.intBits[1];
            rdCostB += state->sbbFracBits.intBits[1] + state->sigFracBits.intBits[1];
            rdCostZ += state->sbbFracBits.intBits[1] + state->sigFracBits.intBits[0];
        }
        else if (state->numSigSbb)
        {
            rdCostA += state->sigFracBits.intBits[1];
            rdCostB += state->sigFracBits.intBits[1];
            rdCostZ += state->sigFracBits.intBits[0];
        }
        else
        {
            rdCostZ = decision->rdCost[indexA];
        }
    }
    else
    {
        rdCostA += (1 << 15) + goRiceTab[pqDataA->absLevel <= state->goRiceZero ? pqDataA->absLevel - 1 : XIN_MIN(pqDataA->absLevel, 32 - 1)];
        rdCostB += (1 << 15) + goRiceTab[pqDataB->absLevel <= state->goRiceZero ? pqDataB->absLevel - 1 : XIN_MIN(pqDataB->absLevel, 32 - 1)];
        rdCostZ += goRiceTab[state->goRiceZero];
    }

    if (rdCostA < rdCostZ && rdCostA < decision->rdCost[indexA])
    {
        decision->rdCost[indexA]    = rdCostA;
        decision->absLevel[indexA]  = (SINT16)pqDataA->absLevel;
        decision->prevId[indexA]    = state->stateId;
    }
    else if (rdCostZ < decision->rdCost[indexA])
    {
        decision->rdCost[indexA]    = rdCostZ;
        decision->absLevel[indexA]  = 0;
        decision->prevId[indexA]    = state->stateId;
    }

    if (rdCostB < decision->rdCost[indexB])
    {
        decision->rdCost[indexB]    = rdCostB;
        decision->absLevel[indexB]  = (SINT16)pqDataB->absLevel;
        decision->prevId[indexB]    = state->stateId;
    }

}

static void Xin266CheckRdCostSkipSbb (
    xin_dp_state *state,
    xin_decision *decision)
{
    SINT64 rdCost;
    SINT32 idx;

    for (idx = 0; idx < 4; idx++)
    {
        rdCost = state[idx].rdCost + state[idx].sbbFracBits.intBits[0];

        if (rdCost < decision->rdCost[idx])
        {
            decision->rdCost[idx]   = rdCost;
            decision->absLevel[idx] = 0;
            decision->prevId[idx]   = 4 | state[idx].stateId;
        }
    }
}

void Xin266CheckRdCostStart (
    xin_dp_state *state,
    SINT32       lastOffset,
    xin_pq_data  *pqData,
    xin_decision *decision)
{
    SINT64 rdCost;
    UINT32 value;
    SINT32 idx;

    for (idx = 0; idx < 4; idx += 2)
    {
        rdCost = pqData[idx].deltaDist + lastOffset;

        if (pqData[idx].absLevel < 4)
        {
            rdCost += state->coeffFracBits.intBits[pqData[idx].absLevel];
        }
        else
        {
            value   = (pqData[idx].absLevel - 4) >> 1;
            rdCost += state->coeffFracBits.intBits[pqData[idx].absLevel - (value << 1)] + goRiceBits[state->goRicePar][value < 32 ? value : 32 - 1];
        }

        if (rdCost < decision->rdCost[idx])
        {
            decision->rdCost[idx]   = rdCost;
            decision->absLevel[idx] = (SINT16)pqData[idx].absLevel;
            decision->prevId[idx]   = -1;
        }
    }

}

static void Xin266SetRiceParam (
    xin_dp_state  *state,
    xin_scan_info *scanInfo)
{
    SINT32  sumAbs;
    SINT32  sumAll;

    if (state->remRegBins >= 4)
    {
        sumAbs           = state->absAndCtx.absCtx.ctx[scanInfo->insidePos].sumAbs;
        sumAll           = XIN_MAX (XIN_MIN (31, sumAbs - 4 * 5), 0);
        state->goRicePar = (SINT8)auiGoRiceParsCoeff[sumAll];
    }
}

void Xin266Decide (
    xin_dep_quant *depQuant,
    xin_dp_param  *dpParam,
    xin_scan_info *scanInfo,
    SINT32        absCoeff,
    SINT32        lastOffset,
    xin_decision  *decisions,
    BOOL          zeroOut)
{
    xin_pq_data pqData[4];

    memcpy (
        decisions,
        startDec,
        sizeof(xin_decision));

    if (zeroOut)
    {
        if (scanInfo->spt == XIN_SCAN_EOCSBB)
        {
            Xin266CheckRdCostSkipSbbZeroOut (
                depQuant->skipStates,
                decisions);
        }

        return;

    }

    Xin266PreQuantCoeff (
        absCoeff,
        pqData,
        dpParam);

    if (pqData[0].absLevel >= 4 || pqData[2].absLevel >= 4 )
    {
        Xin266SetRiceParam (
            &depQuant->prevStates[0],
            scanInfo);

        Xin266SetRiceParam (
            &depQuant->prevStates[1],
            scanInfo);
    }

    if (pqData[1].absLevel >= 4 || pqData[3].absLevel >= 4)
    {
        Xin266SetRiceParam (
            &depQuant->prevStates[2],
            scanInfo);

        Xin266SetRiceParam (
            &depQuant->prevStates[3],
            scanInfo);
    }

    Xin266CheckRdCosts (
        depQuant->prevStates + 0,
        scanInfo->spt,
        pqData + 0,
        pqData + 2,
        decisions,
        0,
        2);

    Xin266CheckRdCosts (
        depQuant->prevStates + 1,
        scanInfo->spt,
        pqData + 0,
        pqData + 2,
        decisions,
        2,
        0);

    Xin266CheckRdCosts (
        depQuant->prevStates + 2,
        scanInfo->spt,
        pqData + 3,
        pqData + 1,
        decisions,
        1,
        3);

    Xin266CheckRdCosts (
        depQuant->prevStates + 3,
        scanInfo->spt,
        pqData + 3,
        pqData + 1,
        decisions,
        3,
        1);

    if (scanInfo->spt == XIN_SCAN_EOCSBB)
    {
        Xin266CheckRdCostSkipSbb (
            depQuant->skipStates,
            decisions);
    }

    Xin266CheckRdCostStart (
        &depQuant->startState,
        lastOffset,
        pqData,
        decisions);

}

static void Xin266Update (
    xin_dep_quant *depQuant,
    xin_scan_info *scanInfo,
    xin_dp_state  *prevState,
    xin_dp_state  *currState)
{
    UINT8           *sbbFlags;
    UINT8           *levels;
    UINT32          setCpSize;
    SINT32          sigNSbb;
    SINT32          scanBegin;
    xin_nb_info_out *nbOut;
    UINT8           *absLevels;
    SINT32          idx;
    COEFF           sumAbs, sumAbs1, sumNum;

    sbbFlags  = depQuant->currSbbCtx[currState->stateId].sbbFlags;
    levels    = depQuant->currSbbCtx[currState->stateId].levels;
    setCpSize = depQuant->nbInfo[scanInfo->scanIdx - 1].maxDist*sizeof(UINT8);

    if (prevState && prevState->refSbbCtxId >= 0)
    {
        memcpy (
            sbbFlags,
            depQuant->prevSbbCtx[prevState->refSbbCtxId].sbbFlags,
            scanInfo->numSbb*sizeof(UINT8));

        memcpy (
            levels + scanInfo->scanIdx,
            depQuant->prevSbbCtx[prevState->refSbbCtxId].levels + scanInfo->scanIdx,
            setCpSize);
    }
    else
    {
        memset (
            sbbFlags,
            0,
            scanInfo->numSbb*sizeof(UINT8));

        memset (
            levels + scanInfo->scanIdx,
            0,
            setCpSize);
    }

    sbbFlags[scanInfo->sbbPos] = !!currState->numSigSbb;

    memcpy (
        levels + scanInfo->scanIdx,
        currState->absAndCtx.absCtx.absLevels,
        scanInfo->sbbSize*sizeof(UINT8));

    sigNSbb = ((scanInfo->nextSbbRight ? sbbFlags[scanInfo->nextSbbRight] : FALSE) || (scanInfo->nextSbbBelow ? sbbFlags[scanInfo->nextSbbBelow] : FALSE) ? 1 : 0);

    currState->goRicePar   = 0;
    currState->refSbbCtxId = currState->stateId;
    currState->sbbFracBits = depQuant->sbbFlagBits[sigNSbb];
    currState->numSigSbb   = 0;

    memset (&currState->absAndCtx, 0, sizeof(xin_abs_ctx));

    if (sigNSbb || ((scanInfo->nextSbbRight && scanInfo->nextSbbBelow) ? sbbFlags[scanInfo->nextSbbBelow + 1] : FALSE))
    {
        scanBegin = scanInfo->scanIdx - scanInfo->sbbSize;
        nbOut     = depQuant->nbInfo + scanBegin;
        absLevels = levels + scanBegin;

        for (idx = 0; idx < scanInfo->sbbSize; idx++, nbOut++)
        {
            if (nbOut->num)
            {
                sumAbs  = 0;
                sumAbs1 = 0;
                sumNum  = 0;

                switch (nbOut->num)
                {
                default:
                case 5:
                    XIN_OUT_UPDATE (4);
                case 4:
                    XIN_OUT_UPDATE (3);
                case 3:
                    XIN_OUT_UPDATE (2);
                case 2:
                    XIN_OUT_UPDATE (1);
                case 1:
                    XIN_OUT_UPDATE (0);
                }

                currState->absAndCtx.absCtx.ctx[idx].tplAcc = (UINT8)((sumNum << 5) | sumAbs1);
                currState->absAndCtx.absCtx.ctx[idx].sumAbs = (UINT8)XIN_MIN (127, sumAbs);

            }

        }

    }

}

static void Xin266UpdateStateEOS (
    xin_dep_quant *depQuant,
    xin_dp_state  *state,
    xin_scan_info *scanInfo,
    xin_dp_state  *prevStates,
    xin_dp_state  *skipStates,
    xin_decision  *decision)
{
    xin_dp_state *prevState;
    UINT8        *absLevels;
    COEFF        sumNum;
    COEFF        sumAbs1;
    COEFF        sumAbs;
    SINT32       sumGt1;
    SINT32       idx;

    for (idx = 0; idx < 4; idx++)
    {
        prevState         = NULL;
        absLevels         = state[idx].absAndCtx.absCtx.absLevels;
        state[idx].rdCost = decision->rdCost[idx];

        if (decision->prevId[idx] > -2)
        {
            if (decision->prevId[idx]  >= 4)
            {
                assert(decision->absLevel[idx] == 0);

                prevState             = skipStates + (decision->prevId[idx] - 4);
                state[idx].numSigSbb  = 0;
                state[idx].remRegBins = prevState->remRegBins;

                memset (
                    state[idx].absAndCtx.absCtx.absLevels,
                    0,
                    16*sizeof(UINT8));
            }
            else if (decision->prevId[idx] >= 0)
            {
                prevState             = prevStates + decision->prevId[idx];
                state[idx].numSigSbb  = prevState->numSigSbb + !!decision->absLevel[idx];
                state[idx].remRegBins = prevState->remRegBins - 1;

                if (state[idx].remRegBins >= 4)
                {
                    state[idx].remRegBins -= (decision->absLevel[idx] < 2 ? decision->absLevel[idx] : 3);
                }

                memcpy (
                    state[idx].absAndCtx.absCtx.absLevels,
                    prevState->absAndCtx.absCtx.absLevels,
                    16*sizeof(UINT8));
            }
            else
            {
                state[idx].numSigSbb  = 1;
                state[idx].remRegBins = (state[idx].effWidth*state[idx].effHeight*MAX_TU_LEVEL_CTX_CODED_BIN) / 16;

                if (state[idx].remRegBins >= 4)
                {
                    state[idx].remRegBins -= (decision->absLevel[idx] < 2 ? decision->absLevel[idx] : 3);
                }

                memset (
                    state[idx].absAndCtx.absCtx.absLevels,
                    0,
                    16*sizeof(UINT8));
            }

            absLevels[scanInfo->insidePos] = (UINT8)XIN_MIN (254 + (decision->absLevel[idx] & 1), decision->absLevel[idx]);

            Xin266Update (
                depQuant,
                scanInfo,
                prevState,
                state + idx);

            if (state[idx].remRegBins >= 4)
            {
                sumAbs  = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].sumAbs;
                sumNum  = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].tplAcc >> 5;
                sumAbs1 = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].tplAcc & 31;
                sumGt1  = sumAbs1 - sumNum;

                state[idx].sigFracBits   = state[idx].sigFracBitsArray[scanInfo->sigCtxOffsetNext + XIN_MIN ((sumAbs1+1)>>1, 3)];
                state[idx].coeffFracBits = state[idx].gtxFracBitsArray[scanInfo->gtxCtxOffsetNext + XIN_MIN (sumGt1, 4)];
            }
            else
            {
                sumAbs = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].sumAbs;
                sumAbs = XIN_MIN (31, sumAbs);

                state[idx].goRicePar  = (UINT8)auiGoRiceParsCoeff[sumAbs];
                state[idx].goRiceZero = (UINT8)auiGoRicePosCoeff0(state[idx].stateId, state[idx].goRicePar);
            }

        }
    }

}

static void Xin266UpdateState (
    xin_dp_state  *state,
    xin_scan_info *scanInfo,
    xin_dp_state  *prevStates,
    xin_decision  *decision)
{
    xin_dp_state *prevState;
    SINT32       ctxBinSampleRatio;
    UINT8        *absLevels;
    SINT32       absLevel;
    COEFF        sumAbs;
    COEFF        sumAbs1;
    COEFF        sumNum;
    SINT32       sumGt1;
    SINT32       sumAll;
    UINT8        min4Or5;
    SINT32       idx;

    for (idx = 0; idx < 4; idx++)
    {
        state[idx].rdCost = decision->rdCost[idx];
        absLevels         = state[idx].absAndCtx.absCtx.absLevels;
        absLevel          = decision->absLevel[idx];

        if (decision->prevId[idx] > -2)
        {
            if (decision->prevId[idx] >= 0)
            {
                prevState              = prevStates + decision->prevId[idx];
                state[idx].numSigSbb   = prevState->numSigSbb + !!absLevel;
                state[idx].refSbbCtxId = prevState->refSbbCtxId;
                state[idx].sbbFracBits = prevState->sbbFracBits;
                state[idx].remRegBins  = prevState->remRegBins - 1;

                if (state[idx].remRegBins >= 4)
                {
                    state[idx].remRegBins -= (absLevel < 2 ? absLevel : 3);
                }

                memcpy (
                    &state[idx].absAndCtx,
                    &prevState->absAndCtx,
                    sizeof(xin_abs_ctx));
            }
            else
            {
                state[idx].numSigSbb   =  1;
                state[idx].refSbbCtxId = -1;

                ctxBinSampleRatio     = MAX_TU_LEVEL_CTX_CODED_BIN;
                state[idx].remRegBins = (state[idx].effWidth*state[idx].effHeight*ctxBinSampleRatio) / 16 - (absLevel < 2 ? absLevel : 3);

                memset (
                    &state[idx].absAndCtx,
                    0,
                    sizeof(xin_abs_ctx));
            }

            if (absLevel)
            {
                absLevels[scanInfo->insidePos] = (UINT8)XIN_MIN (255, absLevel);

                if (scanInfo->currNbInfoSbb.numInv)
                {
                    min4Or5 = (UINT8)XIN_MIN (4 + (absLevel & 1), absLevel);

                    switch (scanInfo->currNbInfoSbb.numInv)
                    {
                    default:
                    case 5:
                        XIN_UPDATE_DEPS (4);
                    case 4:
                        XIN_UPDATE_DEPS (3);
                    case 3:
                        XIN_UPDATE_DEPS (2);
                    case 2:
                        XIN_UPDATE_DEPS (1);
                    case 1:
                        XIN_UPDATE_DEPS (0);
                    case 0:
                        ;
                    }
                }
            }

            if (state[idx].remRegBins >= 4)
            {
                sumAbs  = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].sumAbs;
                sumAbs1 = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].tplAcc & 31;
                sumNum  = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].tplAcc >> 5;
                sumGt1  = sumAbs1 - sumNum;
                sumAll  = XIN_MAX (XIN_MIN (31, (SINT32)sumAbs - 4 * 5 ), 0);

                state[idx].sigFracBits   = state[idx].sigFracBitsArray[scanInfo->sigCtxOffsetNext + XIN_MIN ((sumAbs1+1)>>1, 3)];
                state[idx].coeffFracBits = state[idx].gtxFracBitsArray[scanInfo->gtxCtxOffsetNext + XIN_MIN (sumGt1, 4)];
                state[idx].goRicePar     = (SINT8)auiGoRiceParsCoeff[sumAll];

            }
            else
            {
                sumAbs = state[idx].absAndCtx.absCtx.ctx[scanInfo->nextInsidePos].sumAbs;
                sumAbs = XIN_MIN (31, sumAbs);

                state[idx].goRicePar  = (SINT8)auiGoRiceParsCoeff[sumAbs];
                state[idx].goRiceZero = (SINT8)auiGoRicePosCoeff0(state[idx].stateId, state[idx].goRicePar);

            }
        }
    }

}

static void Xin266DecideUpdate (
    xin_dep_quant *depQuant,
    xin_dp_param  *dpParam,
    SINT32        absCoeff,
    xin_scan_info *scanInfo,
    BOOL          zeroOut)
{
    xin_decision *decisions;
    SINT32       lastOffset;

    decisions  = &depQuant->trellis[scanInfo->scanIdx][0];
    lastOffset = XIN_LASTOFFSET(scanInfo);

    XIN_SWAP (xin_dp_state *, depQuant->prevStates, depQuant->currStates);

    Xin266Decide (
        depQuant,
        dpParam,
        scanInfo,
        absCoeff,
        lastOffset,
        decisions,
        zeroOut);

    if (scanInfo->scanIdx)
    {
        if (scanInfo->insidePos == 0)
        {
            XIN_SWAP (xin_sbb_ctx *, depQuant->currSbbCtx, depQuant->prevSbbCtx);

            Xin266UpdateStateEOS (
                depQuant,
                depQuant->currStates,
                scanInfo,
                depQuant->prevStates,
                depQuant->skipStates,
                decisions);

            memcpy (
                decisions + 1,
                decisions,
                sizeof(xin_decision));

        }
        else if (!zeroOut)
        {
            Xin266UpdateState (
                depQuant->currStates,
                scanInfo,
                depQuant->prevStates,
                decisions);

        }

        if (scanInfo->spt == XIN_SCAN_SOCSBB)
        {
            XIN_SWAP (xin_dp_state *, depQuant->prevStates, depQuant->skipStates);
        }

    }

}

void Xin266InitQuantBlock (
    xin_tu_struct *tu,
    xin_dp_param  *dpParam,
    SINT32        qp,
    UINT32        compId)
{
    SINT32  qpDQ;
    SINT32  qpPer;
    SINT32  qpRem;
    SINT32  iqShift;
    UINT32  qIdxBd;
    SINT32  qScale;
    SINT32  nomDShift;
    double  qScale2;
    double  nomDistFactor;
    SINT64  pow2dfShift;
    SINT32  dfShift;
    BOOL    isAdjust;
    UINT32  lgWidth;
    UINT32  lgHeight;
    SINT32  transformShift;
    SINT32  nomTransShift;
    UINT32  compType;
    double  lambda;

    compType       = compId != PLANE_LUMA;
    lgWidth        = tu->lgWidth[compType];
    lgHeight       = tu->lgHeight[compType];
    isAdjust       = (lgWidth + lgHeight) & 1;
    qpDQ           = qp + 1;
    qpPer          = qpDQ / 6;
    qpRem          = qpDQ - 6 * qpPer;
    nomTransShift  = 15 - XIN_INTERNAL_BIT_DEPTH - ((lgWidth + lgHeight) >> 1);
    transformShift = nomTransShift - isAdjust;
    lambda         = 0.57 * pow(2.0, (qp - 12) / 3.0);

    // quant parameters
    dpParam->qShift  = XIN_QUANT_SHIFT  - 1 + qpPer + transformShift;
    dpParam->qAdd    = -((3 << dpParam->qShift) >> 1);
    iqShift          = XIN_IQUANT_SHIFT + 1 - qpPer - transformShift;
    dpParam->qScale  = qMult[isAdjust][qpRem];
    qIdxBd           = XIN_MIN (15 + 1, 8*sizeof(SINT32) + iqShift - XIN_IQUANT_SHIFT - 1);
    dpParam->maxQIdx = (1 << (qIdxBd-1)) - 4;
    dpParam->iqShift = XIN_IQUANT_SHIFT + 1 - transformShift;
    dpParam->iqScale = iqMult[isAdjust][qpRem] << qpPer;

    if (dpParam->qShift)
    {
        dpParam->thresLast = 8 << (dpParam->qShift-1);
    }
    else
    {
        dpParam->thresLast = (8>>1) << dpParam->qShift;
    }

    dpParam->thresSSbb = 3 << dpParam->qShift;

    // distortion calculation parameters
    qScale        = dpParam->qScale;
    nomDShift     = 15 - 2 * nomTransShift + dpParam->qShift + isAdjust;
    qScale2       = (double)(qScale*qScale);
    nomDistFactor = (nomDShift < 0 ? 1.0/((double)((SINT64)1<<(-nomDShift))*qScale2*lambda) : (double)((SINT64)1<<nomDShift)/(qScale2*lambda));
    pow2dfShift   = (SINT64)(nomDistFactor * qScale2) + 1;

    if (pow2dfShift == 0)
    {
        dfShift = -1;
    }
    else
    {
        BIT_SCAN_REVERSE_32 (pow2dfShift - 1, dfShift);

        dfShift += 1;
    }

    dpParam->distShift   = 62 + dpParam->qShift - 2*15 - dfShift;
    dpParam->distAdd     = ((SINT64)1 << dpParam->distShift) >> 1;
    dpParam->distStepAdd = ((dpParam->distShift + dpParam->qShift) >= 64 ? (SINT64)(nomDistFactor * pow (2, dpParam->distShift + dpParam->qShift) + .5) : (SINT64)(nomDistFactor * (double)((SINT64)1<<(dpParam->distShift+dpParam->qShift)) + .5 ));
    dpParam->distOrgFact = (SINT64)(nomDistFactor*(double)((SINT64)1<<(dpParam->distShift+1)) + .5);

}

void Xin266PreDepQuant (
    COEFF       *tCoeff,
    COEFF       *rCoeff,
    COEFF       *qCoeff,
    intptr_t    coeffStride,
    UINT32       width,
    UINT32       height,
    UINT32       cGWidth,
    UINT32       cGHeight,
    SINT32       rdoqThrVal,
    xin_scan_pos *scanOrderCG,
    xin_scan_pos *scanOrder,
    SINT32       *nzCoeffIdx)
{
    SINT32  blockIdx;
    SINT32  blockPos;
    SINT32  blockNum;
    SINT32  blockX;
    SINT32  blockY;
    SINT32  innerPos;
    SINT32  innerX;
    SINT32  innerY;
    SINT32  coeffIdx;
    SINT32  widthInCG;
    COEFF   *tCoeffBlock;
    SINT32  coeffNum;
    BOOL    isBigCoeff;
    UINT32  rowIdx;

    blockNum    = width*height / (cGWidth*cGHeight);
    widthInCG   = width / cGWidth;
    coeffNum    = cGWidth*cGHeight;
    *nzCoeffIdx = -1;
    blockY      = 0;
    blockX      = 0;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        memset (rCoeff + rowIdx*coeffStride, 0, sizeof(COEFF)*width);
        memset (qCoeff + rowIdx*coeffStride, 0, sizeof(COEFF)*width);
    }

    for (blockIdx = blockNum - 1; blockIdx >= 0; blockIdx--)
    {
        blockPos    = scanOrderCG[blockIdx].posIdx;
        blockY      = blockPos / widthInCG;
        blockX      = blockPos - blockY*widthInCG;
        tCoeffBlock = tCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;
        isBigCoeff  = FALSE;

        for (coeffIdx = 0; coeffIdx < coeffNum; coeffIdx += cGWidth)
        {
            isBigCoeff |= XIN_ABS (tCoeffBlock[0]) > rdoqThrVal;
            isBigCoeff |= XIN_ABS (tCoeffBlock[1]) > rdoqThrVal;
            isBigCoeff |= XIN_ABS (tCoeffBlock[2]) > rdoqThrVal;
            isBigCoeff |= XIN_ABS (tCoeffBlock[3]) > rdoqThrVal;

            tCoeffBlock += coeffStride;
        }

        if (isBigCoeff)
        {
            break;
        }

    }

    // Find last significant coefficient scan index
    if (blockIdx >= 0)
    {
        tCoeffBlock = tCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;

        for (coeffIdx = cGWidth*cGHeight - 1; coeffIdx >= 0 ; coeffIdx--)
        {
            innerPos    = scanOrder[coeffIdx].posIdx;
            innerY      = innerPos / cGWidth;
            innerX      = innerPos - innerY*cGWidth;

            if (XIN_ABS (tCoeffBlock[innerX + innerY*coeffStride]) > rdoqThrVal)
            {
                break;
            }
        }

        *nzCoeffIdx = blockIdx*(cGWidth*cGHeight) + coeffIdx;
    }

}

static void Xin266StateInit (
    xin_dp_state *state,
    SINT32       effWidth,
    SINT32       effHeight)
{
    state->rdCost        = XIN_MAX_DEP_COST;
    state->numSigSbb     = 0;
    state->remRegBins    = 4;  // just large enough for last scan pos
    state->refSbbCtxId   = -1;
    state->sigFracBits   = state->sigFracBitsArray[0];
    state->coeffFracBits = state->gtxFracBitsArray[0];
    state->goRicePar     = 0;
    state->goRiceZero    = 0;
    state->effWidth      = effWidth;
    state->effHeight     = effHeight;
}

void Xin266DepQuant (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_struct   *tu,
    COEFF           *tCoeff,
    COEFF           *rCoeff,
    COEFF           *qCoeff,
    intptr_t        coeffStride,
    UINT32          qp,
    SINT32          width,
    SINT32          height,
    UINT64          *nzCGBitMapRs,
    UINT32          compId)
{
    xin_scan_pos    *scanOrderCG;
    xin_func_struct *funcSet;
    SINT32          defaultTh;
    SINT32          blockIdx;
    SINT32          blockPos;
    SINT32          coeffIdx;
    xin_scan_pos    *scanOrder;
    SINT32          posX;
    SINT32          posY;
    SINT32          blockX;
    SINT32          blockY;
    SINT32          coeffVal;
    SINT32          level;
    SINT64          minPathCost;
    SINT64          pathCost;
    SINT32          prevId;
    SINT32          absLevel;
    SINT32          stateId;
    SINT32          qIdx;
    UINT32          compType;
    UINT32          lgWidth;
    UINT32          lgHeight;
    xin_tu_param    *tuParam;
    xin_scan_info   *scanInfo;
    SINT32          idx;
    SINT32          effWidth;
    SINT32          effHeight;
    UINT64          nzCGBitMap;
    xin_dp_param    *dpParam;
    SINT32          iqScale;
    SINT32          iqShift;
    SINT32          iqAdd;
    BOOL            isNZBlock;

    compType    = compId != PLANE_LUMA;
    lgWidth     = calcLog2[width];
    lgHeight    = calcLog2[height];
    dpParam     = &depQuant->dpParam[lgWidth + lgHeight][qp];
    scanOrderCG = tu->scanOrderCG[compType];
    scanOrder   = tu->scanOrder[compType];
    scanOrder   = tu->scanOrder[compType];
    tuParam     = depQuant->tuParam[lgWidth][lgHeight][compType];
    scanInfo    = tuParam->scanInfo;
    effWidth    = XIN_MIN (width, 32);
    effHeight   = XIN_MIN (height, 32);
    nzCGBitMap  = 0;
    defaultTh   = dpParam->thresLast / (dpParam->qScale << 2);
    funcSet     = depQuant->funcSet;

    Xin266InitContext (
        depQuant,
        fastBuf,
        tuParam,
        depQuant->context,
        compId);

    funcSet->pfXinPreDepQuant (
        tCoeff,
        rCoeff,
        qCoeff,
        coeffStride,
        effWidth,
        effHeight,
        4,
        4,
        defaultTh,
        scanOrderCG,
        scanOrder,
        &coeffIdx);

    if (coeffIdx < 0)
    {
        *nzCGBitMapRs = 0;

        return;
    }

    Xin266DepQuantReset (
        depQuant,
        tuParam);

    for (idx = 0; idx < 12; idx++)
    {
        Xin266StateInit (
            depQuant->allStates + idx,
            effWidth,
            effHeight);
    }

    Xin266StateInit (
        &depQuant->startState,
        effWidth,
        effHeight);

    blockIdx = coeffIdx >> 4;
    coeffIdx = coeffIdx & 15;

    for (; blockIdx >= 0; blockIdx--)
    {
        blockX = scanOrderCG[blockIdx].posX * 4;
        blockY = scanOrderCG[blockIdx].posY * 4;

        for (; coeffIdx >= 0; coeffIdx--)
        {
            posX     = scanOrder[coeffIdx].posX + blockX;
            posY     = scanOrder[coeffIdx].posY + blockY;
            coeffVal = tCoeff[posX + posY*coeffStride];

            Xin266DecideUpdate (
                depQuant,
                dpParam,
                XIN_ABS (coeffVal),
                scanInfo + blockIdx*16+coeffIdx,
                FALSE);

        }

        coeffIdx = 15;

    }

    //===== find best path =====
    minPathCost =  0;
    prevId      = -1;

    for (stateId = 0; stateId < 4; stateId++)
    {
        pathCost = depQuant->trellis[0][0].rdCost[stateId];

        if (pathCost < minPathCost)
        {
            prevId      = stateId;
            minPathCost = pathCost;
        }
    }

    //===== backward scanning =====
    for (coeffIdx = 0; prevId >= 0; coeffIdx++)
    {
        absLevel = depQuant->trellis[coeffIdx][prevId >> 2].absLevel[prevId & 3];
        posX     = scanInfo[coeffIdx].posX;
        posY     = scanInfo[coeffIdx].posY;
        coeffVal = tCoeff[posX + posY*coeffStride];
        prevId   = depQuant->trellis[coeffIdx][prevId >> 2].prevId[prevId & 3];

        qCoeff[posX + posY*coeffStride] = coeffVal < 0 ? (COEFF)(-absLevel) : (COEFF)(absLevel);
    }

    // inverse quantization
    coeffIdx = coeffIdx - 1;
    blockIdx = coeffIdx >> 4;
    coeffIdx = coeffIdx & 15;
    stateId  = 0;
    iqScale  = dpParam->iqScale;
    iqShift  = dpParam->iqShift;
    iqAdd    = 1 << (iqShift - 1);

    for (; blockIdx >= 0; blockIdx--)
    {
        blockX    = scanOrderCG[blockIdx].posX * 4;
        blockY    = scanOrderCG[blockIdx].posY * 4;
        blockPos  = scanOrderCG[blockIdx].posIdx;
        isNZBlock = FALSE;

        for (; coeffIdx >= 0; coeffIdx--)
        {
            posX  = scanOrder[coeffIdx].posX + blockX;
            posY  = scanOrder[coeffIdx].posY + blockY;
            level = qCoeff[posX + posY*coeffStride];

            if (level)
            {
                qIdx      = 2 * level + (level > 0 ? -(stateId >> 1) : (stateId >> 1));
                coeffVal  = (qIdx * iqScale + iqAdd) >> iqShift;
                isNZBlock = TRUE;

                rCoeff[posX + posY*coeffStride] = (COEFF)XIN_CLIP (coeffVal, XIN_MIN_S16, XIN_MAX_S16);
            }

            stateId = (32040 >> ((stateId<<2)+((level&1)<<1))) & 3;
        }

        nzCGBitMap |= (UINT64)isNZBlock << blockPos;
        coeffIdx    = 15;

    }

    *nzCGBitMapRs = nzCGBitMap;

}
