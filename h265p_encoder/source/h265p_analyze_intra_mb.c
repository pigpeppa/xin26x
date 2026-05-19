/***************************************************************************//**
*
* @file          h265p_analyze_intra_mb.c
* @brief         Analyze intra block.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "assert.h"
#include "string.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "xin26x_logger.h"
#include "h26x_block_utility.h"
#include "h265p_analyze_mb.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_intra_prediction.h"
#include "h265p_md_buf_manipulate.h"
#include "h26x_compute_dist.h"
#include "h265p_common_data.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_trans_recon.h"
#include "h265p_compute_dist.h"

static const UINT32 intraMode2TxType[XIN_INTRA_MODE_NUM] =
{
    XIN_DCT_DCT,    // DC_PRED
    XIN_ADST_DCT,   // V_PRED
    XIN_DCT_ADST,   // H_PRED
    XIN_DCT_DCT,    // D45_PRED
    XIN_ADST_ADST,  // D135_PRED
    XIN_ADST_DCT,   // D113_PRED
    XIN_DCT_ADST,   // D157_PRED
    XIN_DCT_ADST,   // D203_PRED
    XIN_ADST_DCT,   // D67_PRED
    XIN_ADST_ADST,  // SMOOTH_PRED
    XIN_ADST_DCT,   // SMOOTH_V_PRED
    XIN_DCT_ADST,   // SMOOTH_H_PRED
    XIN_ADST_ADST,  // PAETH_PRED
};

static UINT32 Xin265pGetIntraUvTxType (
    UINT32  intraUvMode,
    UINT32  txSize,
    BOOL    reducedTxSet)
{
    UINT32  txType;
    UINT32  txSetType;

    Xin265pGetExtTxSetType (
        txSize,
        FALSE,
        reducedTxSet,
        &txSetType);

    txType = intraMode2TxType[intraUvMode];

    if (!av1ExtTxUsed[txSetType][txType])
    {
        txType = XIN_DCT_DCT;
    }

    return txType;

}

static void Xin265pIntraFastSearch (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf **fastBuf,
    UINT32          bufNum,
    UINT32          interSad)
{
    xin_fast_md_buf *curBuf;
    xin_pic_struct  *picSet;
    xin_func_struct *funcSet;
    xin_ref_picture *pictureWrite;
    UINT32          bufIdx;
    SINT32          modeIdx;
    PIXEL           *pred;
    intptr_t        predStride;
    intptr_t        predOffset;
    PIXEL           *input;
    intptr_t        inputStride;
    SINT32          offsetX;
    SINT32          offsetY;
    UINT32          sad;
    SINT32          modeBits;
    SINT32          topCtx;
    SINT32          lftCtx;
    SINT32          interCtx;
    UINT64          sadCost;
    UINT64          lambda;
    UINT32          lgWidth;
    SINT32          *modeRate;

    (void)interSad;

    picSet       = secSet->picSet;
    funcSet      = secSet->funcSet;
    pictureWrite = picSet->pictureWrite;
    input        = secSet->inputMb[PLANE_LUMA];
    inputStride  = secSet->inputYStride;
    offsetX      = mb->offX[PLANE_LUMA];
    offsetY      = mb->offY[PLANE_LUMA];
    lambda       = secSet->sadLambda[PLANE_LUMA];
    lgWidth      = mb->lgWidth[PLANE_LUMA];

    if (pictureWrite->frameType >= XIN_I_FRAME)
    {
        topCtx   = mb->topMi->predMode;
        topCtx   = (topCtx == XIN_INVALID_MODE) ? 0 : av1IntraModeContext[topCtx];
        lftCtx   = mb->lftMi->predMode;
        lftCtx   = (lftCtx == XIN_INVALID_MODE) ? 0 : av1IntraModeContext[lftCtx];
        modeRate = secSet->cabacEst.yModeRate[topCtx][lftCtx];
    }
    else
    {
        interCtx = XIN_MIN (3, XIN_MIN (mb->lgWidth[PLANE_LUMA], mb->lgHeight[PLANE_LUMA]));
        modeRate = secSet->cabacEst.mbModeRate[interCtx];
    }

    for (bufIdx = 0; bufIdx < bufNum; bufIdx++)
    {
        fastBuf[bufIdx]->sadCost = XIN_MAX_U64_COST;
    }

    for (modeIdx = XIN_INTRA_MODE_END - 1; modeIdx >= 0; modeIdx--)
    {
        Xin265pFindHighestSadBuf (
            fastBuf,
            bufNum,
            &curBuf);

        predStride = curBuf->predStride[0];
        predOffset = offsetX + offsetY*predStride;
        pred       = curBuf->predBuf[PLANE_LUMA] + predOffset;

        Xin265pIntraPred (
            secSet,
            PLANE_LUMA,
            pred,
            predStride,
            modeIdx,
            0);

        funcSet->pfXinComputeSad[lgWidth] (
            input,
            inputStride,
            pred,
            predStride,
            mb->width[PLANE_LUMA],
            mb->height[PLANE_LUMA],
            &sad);

        modeBits = modeRate[modeIdx];
        sadCost  = sad << XIN_COST_FRACTION;
        sadCost += CALC_RD_COST (lambda, modeBits);

        curBuf->predMode      = modeIdx;
        curBuf->sad           = sad;
        curBuf->sadCost       = sadCost;
        curBuf->angleDelta[0] = 0;

    }

}

static void Xin265pAnalyzeIntraPu (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf **fastBuf,
    UINT32          bufNum,
    UINT32          interSad)
{
    xin_pic_struct *picSet;
    intptr_t       miStride;

    picSet   = secSet->picSet;
    miStride = picSet->pictureWrite->miStride;

    Xin265pGetIntraAvail (
        mb,
        XIN_LOG_MI_SIZE,
        miStride);

    Xin265pExtractIntraNBLuma (
        secSet,
        mb);

    Xin265pIntraFastSearch (
        secSet,
        mb,
        fastBuf,
        bufNum,
        interSad);

}

void Xin265pAnalyzeIntraMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf **fastBuf,
    UINT32          bufNum,
    xin_fast_md_buf *interMdBuf)
{
    UINT32          bitNum;
    UINT32          lambda;
    UINT32          interSad;
    UINT32          bufIdx;
    xin_fast_md_buf *curBuf;

    if (bufNum < 2)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "At least 2 buffers for mode decision.\n");
    }

    lambda   = secSet->sadLambda[PLANE_LUMA];
    interSad = (interMdBuf == NULL) ? XIN_MAX_U32_COST : interMdBuf->sad;

    Xin265pAnalyzeIntraPu (
        secSet,
        mb,
        fastBuf,
        bufNum,
        interSad);

    Xin265pSortMdBufSad (
        fastBuf,
        bufNum);

    for (bufIdx = 0; bufIdx < bufNum - 1; bufIdx++)
    {
        curBuf = fastBuf[bufIdx];

        Xin265pEstimateMbSynatax (
            secSet,
            mb,
            curBuf,
            &bitNum);

        curBuf->syntaxRate = bitNum;
        curBuf->sadCost    = curBuf->sad << XIN_COST_FRACTION;
        curBuf->sadCost   += CALC_RD_COST (lambda, curBuf->syntaxRate);
        curBuf->type       = XIN_INTRA_MODE;
    }

}

static void Xin265pAnalyzeIntraChroma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf *fastBuf,
    UINT32          *uvSad)
{
    xin_cabac_est   *cabacEst;
    xin_func_struct *funcSet;
    UINT32          modeIdx;
    PIXEL           tmpPredU[XIN_MAX_TX_SIZE*XIN_MAX_TX_SIZE];
    PIXEL           tmpPredV[XIN_MAX_TX_SIZE*XIN_MAX_TX_SIZE];
    PIXEL           *predU[2];
    PIXEL           *predV[2];
    UINT32          bufIdx;
    intptr_t        predStride;
    PIXEL           *inputU;
    PIXEL           *inputV;
    intptr_t        inputStride;
    UINT32          sadU;
    UINT32          sadV;
    UINT32          lumaMode;
    UINT32          sadCost;
    UINT32          bestCost;
    UINT32          bestSad;
    UINT32          bestMode;
    UINT32          modeBits;
    UINT32          lambda;
    UINT32          lgWidth;

    funcSet     = secSet->funcSet;
    cabacEst    = &secSet->cabacEst;
    lambda      = secSet->sadLambda[PLANE_CHROMA];
    predStride  = fastBuf->predStride[PLANE_CHROMA];
    predU[0]    = fastBuf->predBuf[PLANE_CHROMA_U] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA] * predStride;
    predU[1]    = tmpPredU;
    predV[0]    = fastBuf->predBuf[PLANE_CHROMA_V] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA] * predStride;
    predV[1]    = tmpPredV;
    bufIdx      = 0;
    inputU      = secSet->inputMb[PLANE_CHROMA_U];
    inputV      = secSet->inputMb[PLANE_CHROMA_V];
    inputStride = secSet->inputUvStride;
    lumaMode    = fastBuf->predMode;
    bestCost    = XIN_MAX_U32_COST;
    bestSad     = XIN_MAX_U32_COST;
    lgWidth     = mb->lgWidth[PLANE_CHROMA];

    Xin265pExtractIntraNBChroma (
        secSet,
        mb);

    Xin265pIntraPred (
        secSet,
        PLANE_CHROMA_U,
        predU[0],
        predStride,
        XIN_UV_DC_PRED,
        0);

    Xin265pIntraPred (
        secSet,
        PLANE_CHROMA_V,
        predV[0],
        predStride,
        XIN_UV_DC_PRED,
        0);

    funcSet->pfXinComputeSad[lgWidth] (
        inputU,
        inputStride,
        predU[0],
        predStride,
        mb->width[PLANE_CHROMA],
        mb->height[PLANE_CHROMA],
        &sadU);

    funcSet->pfXinComputeSad[lgWidth] (
        inputV,
        inputStride,
        predV[0],
        predStride,
        mb->width[PLANE_CHROMA],
        mb->height[PLANE_CHROMA],
        &sadV);

    modeBits  = cabacEst->intraUvModeRate[XIN_CFL_DISALLOWED][lumaMode][XIN_UV_DC_PRED];
    bestCost  = (sadU + sadV) << XIN_COST_FRACTION;
    bestSad   = sadU + sadV;
    bestCost += CALC_RD_COST (lambda, modeBits);
    bestMode  = XIN_UV_DC_PRED;
    bufIdx   ^= 1;

    for (modeIdx = XIN_UV_V_PRED; modeIdx < XIN_UV_SMOOTH_PRED; modeIdx++)
    {
        Xin265pIntraPred (
            secSet,
            PLANE_CHROMA_U,
            predU[bufIdx],
            predStride,
            modeIdx,
            0);

        funcSet->pfXinComputeSad[lgWidth] (
            inputU,
            inputStride,
            predU[bufIdx],
            predStride,
            mb->width[PLANE_CHROMA],
            mb->height[PLANE_CHROMA],
            &sadU);

        Xin265pIntraPred (
            secSet,
            PLANE_CHROMA_V,
            predV[bufIdx],
            predStride,
            modeIdx,
            0);

        funcSet->pfXinComputeSad[lgWidth] (
            inputV,
            inputStride,
            predV[bufIdx],
            predStride,
            mb->width[PLANE_CHROMA],
            mb->height[PLANE_CHROMA],
            &sadV);

        modeBits = cabacEst->intraUvModeRate[XIN_CFL_DISALLOWED][lumaMode][modeIdx];
        sadCost  = (sadU + sadV) << XIN_COST_FRACTION;
        sadCost += CALC_RD_COST (lambda, modeBits);

        if (sadCost < bestCost)
        {
            bestSad  = sadU + sadV;
            bestCost = sadCost;
            bestMode = modeIdx;
            bufIdx  ^= 1;
        }

    }

    fastBuf->intraUvMode   = bestMode;
    fastBuf->angleDelta[1] = 0;

    if (!bufIdx)
    {
        funcSet->pfXinBlockCopy[lgWidth] (
            predU[1],
            predStride,
            predU[0],
            predStride,
            mb->width[PLANE_CHROMA],
            mb->height[PLANE_CHROMA]);

        funcSet->pfXinBlockCopy[lgWidth] (
            predV[1],
            predStride,
            predV[0],
            predStride,
            mb->width[PLANE_CHROMA],
            mb->height[PLANE_CHROMA]);
    }

    fastBuf->syntaxRate += cabacEst->intraUvModeRate[XIN_CFL_DISALLOWED][lumaMode][bestMode];

    *uvSad = bestSad;

}

void Xin265pEncodeIntraMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf)
{
    xin_fast_md_buf *fastBuf;
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    UINT32          uvSad;
    UINT32          coefBitsY;
    UINT32          coefBitsU;
    UINT32          coefBitsV;
    UINT32          skipFlagBits;
    UINT64          ssdY[2];
    UINT64          ssdU[2];
    UINT64          ssdV[2];
    UINT64          lambda;
    SINT32          txSize;
    SINT32          txSizeUv;
    intptr_t        yCoefPos;
    intptr_t        uvCoefPos;

    seqSet    = secSet->seqSet;
    fastBuf   = fullBuf->fastBuf;
    funcSet   = secSet->funcSet;
    lambda    = secSet->sseLambda[PLANE_LUMA];
    txSize    = blockSize2TxSize[mb->blockSize];
    txSizeUv  = blockSize2TxSizeUv[mb->blockSize];
    yCoefPos  = mb->offX[PLANE_LUMA] + mb->offY[PLANE_LUMA]*fullBuf->coefStride[PLANE_LUMA];
    uvCoefPos = mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA]*fullBuf->coefStride[PLANE_CHROMA];

    Xin265pAnalyzeIntraChroma (
        secSet,
        mb,
        fastBuf,
        &uvSad);

    fastBuf->earlyStop = FALSE;
    fullBuf->tuNum     = 1;

    fullBuf->tranSize[PLANE_LUMA]   = txSize;
    fullBuf->tranSize[PLANE_CHROMA] = txSizeUv;
    fullBuf->tranType[PLANE_LUMA]   = XIN_DCT_DCT;
    fullBuf->tranType[PLANE_CHROMA] = Xin265pGetIntraUvTxType(fastBuf->intraUvMode, fullBuf->tranSize[PLANE_CHROMA], TRUE);

    Xin265pFullBufLoadCtx (
        fullBuf,
        mb);

    Xin265pTransformTx (
        secSet,
        fullBuf,
        mb->tu[PLANE_LUMA],
        PLANE_LUMA);

    Xin265pEstimateCoeff (
        secSet,
        mb,
        fullBuf,
        &coefBitsY,
        PLANE_LUMA);

    funcSet->pfXinComputeSsdFd[mb->lgWidth[PLANE_LUMA]] (
        fullBuf->tCoefBuf[PLANE_LUMA] + yCoefPos,
        fullBuf->coefStride[PLANE_LUMA],
        fullBuf->rCoefBuf[PLANE_LUMA] + yCoefPos,
        fullBuf->coefStride[PLANE_LUMA],
        mb->width[PLANE_LUMA],
        mb->height[PLANE_LUMA],
        txSize,
        ssdY);

    Xin265pTransformTx (
        secSet,
        fullBuf,
        mb->tu[PLANE_CHROMA_U],
        PLANE_CHROMA_U);

    Xin265pEstimateCoeff (
        secSet,
        mb,
        fullBuf,
        &coefBitsU,
        PLANE_CHROMA_U);

    funcSet->pfXinComputeSsdFd[mb->lgWidth[PLANE_CHROMA]] (
        fullBuf->tCoefBuf[PLANE_CHROMA_U] + uvCoefPos,
        fullBuf->coefStride[PLANE_CHROMA],
        fullBuf->rCoefBuf[PLANE_CHROMA_U] + uvCoefPos,
        fullBuf->coefStride[PLANE_CHROMA],
        mb->width[PLANE_CHROMA],
        mb->height[PLANE_CHROMA],
        txSizeUv,
        ssdU);

    Xin265pTransformTx (
        secSet,
        fullBuf,
        mb->tu[PLANE_CHROMA_V],
        PLANE_CHROMA_V);

    Xin265pEstimateCoeff (
        secSet,
        mb,
        fullBuf,
        &coefBitsV,
        PLANE_CHROMA_V);

    funcSet->pfXinComputeSsdFd[mb->lgWidth[PLANE_CHROMA]] (
        fullBuf->tCoefBuf[PLANE_CHROMA_V] + uvCoefPos,
        fullBuf->coefStride[PLANE_CHROMA],
        fullBuf->rCoefBuf[PLANE_CHROMA_V] + uvCoefPos,
        fullBuf->coefStride[PLANE_CHROMA],
        mb->width[PLANE_CHROMA],
        mb->height[PLANE_CHROMA],
        txSizeUv,
        ssdV);

    Xin265pEstimateCoeffSkipFlag (
        secSet,
        fullBuf,
        mb,
        &skipFlagBits);

    fastBuf->earlyStop = (seqSet->config.earlyStopMode > 1) ? fullBuf->skipCoeff : fastBuf->earlyStop;
    fullBuf->sseCost   = (ssdY[0] + ssdU[0] + ssdV[0]) << XIN_COST_FRACTION;
    fullBuf->sseCost  += CALC_RD_COST (lambda, coefBitsY + coefBitsU + coefBitsV + skipFlagBits);
    fullBuf->sseCost  += CALC_RD_COST (lambda, fastBuf->syntaxRate);

}


