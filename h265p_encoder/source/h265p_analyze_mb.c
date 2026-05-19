/***************************************************************************//**
*
* @file          h265p_analyze_mb.c
* @brief         Analyze a block for all possible mode.
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
#include "h265p_md_buf_manipulate.h"
#include "h265p_enc_init.h"
#include "h265p_common_data.h"

static void Xin265pContructFullBuf (
    xin_sec_struct  *secSet,
    UINT32          depth,
    UINT32          partType,
    xin_fast_md_buf **dblFastBuf,
    UINT32          inBufNum,
    xin_full_md_buf **dblFullBuf,
    UINT32          *outBufNum)
{
    xin_full_md_buf *fullBuf;
    xin_fast_md_buf *fastBuf;
    UINT32          fastBufIdx;
    UINT32          fullBufIdx;

    fullBufIdx = 0;

    for (fastBufIdx = 0; fastBufIdx < inBufNum; fastBufIdx++)
    {
        fastBuf = dblFastBuf[fastBufIdx];
        fullBuf = secSet->fullMdBuf[partType][depth] + fullBufIdx;

        fullBuf->fastBuf       = fastBuf;
        dblFullBuf[fullBufIdx] = fullBuf;
        fullBufIdx++;
    }

    *outBufNum = fullBufIdx;

}

static void Xin265pComputeMbCtx (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    UINT8         lftSkip;
    UINT8         topSkip;
    xin_mi_struct *lftMi;
    xin_mi_struct *topMi;

    (void)secSet;

    lftMi   = mb->lftMi;
    topMi   = mb->topMi;

    lftSkip = lftMi->coeffSkip;
    topSkip = topMi->coeffSkip;
    lftSkip = (lftSkip == 0xFF) ? 0 : lftSkip;
    topSkip = (topSkip == 0xFF) ? 0 : topSkip;

    mb->skipCoeffCtx = lftSkip + topSkip;

}

void Xin265pAnalyzeMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb)
{
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    BOOL            analyzeIntra;
    BOOL            analyzeInter;
    xin_fast_md_buf *intraMdBuf[11];
    xin_fast_md_buf *fastMdBuf[20];
    xin_full_md_buf *fullMdBuf[20];
    xin_fast_md_buf *intraBaseBuf;
    xin_fast_md_buf *fastBuf;
    xin_full_md_buf *fullBuf;
    xin_full_md_buf *bestBuf;
    UINT32          fastBufCnt;
    UINT32          fullBufCnt;
    UINT32          bufIdx;
    UINT32          intraRdoNum;
    UINT32          partType;

    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    partType     = mb->partType;
    pictureWrite = picSet->pictureWrite;
    analyzeIntra = (mb->width[PLANE_LUMA] <= XIN_MAX_TX_SIZE) && (mb->height[PLANE_LUMA] <= XIN_MAX_TX_SIZE);
    analyzeInter = (pictureWrite->frameType < XIN_I_FRAME);
    intraBaseBuf = secSet->fastMdBuf[partType][mb->depth] + secSet->fastMdBufStart[XIN_INTRA_MODE];
    intraRdoNum  = seqSet->config.intraRdoNum;
    fastBufCnt   = 0;
    bestBuf      = NULL;

    Xin265pComputeMbCtx (
        secSet,
        mb);

    if ((analyzeInter == FALSE) && (analyzeIntra == FALSE))
    {
        return;
    }

    if (analyzeIntra)
    {
        Xin265pSortMdBufSad (
            fastMdBuf,
            fastBufCnt);

        for (bufIdx = 0; bufIdx < intraRdoNum + 1; bufIdx++)
        {
            intraMdBuf[bufIdx] = intraBaseBuf + bufIdx;
        }

        Xin265pAnalyzeIntraMb (
            secSet,
            mb,
            intraMdBuf,
            intraRdoNum + 1,
            NULL);

        for (bufIdx = 0; bufIdx < intraRdoNum; bufIdx++)
        {
            fastMdBuf[fastBufCnt] = intraMdBuf[bufIdx];
            fastBufCnt++;
        }

    }

    Xin265pSortMdBufSad (
        fastMdBuf,
        fastBufCnt);

    fastBufCnt = XIN_MIN (seqSet->config.maxMdCandNum, fastBufCnt);

    if (!fastBufCnt)
    {
        return;
    }

    Xin265pContructFullBuf (
        secSet,
        mb->depth,
        mb->partType,
        fastMdBuf,
        fastBufCnt,
        fullMdBuf,
        &fullBufCnt);

    for (bufIdx = 0; bufIdx < fullBufCnt; bufIdx++)
    {
        fullBuf = fullMdBuf[bufIdx];
        fastBuf = fullBuf->fastBuf;

        if (fastBuf->type == XIN_INTRA_MODE)
        {
            Xin265pEncodeIntraMb (
                secSet,
                mb,
                fullBuf);
        }
        else
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid block type!\n");
        }

        if ((bestBuf == NULL) || (fullBuf->sseCost < bestBuf->sseCost))
        {
            bestBuf = fullBuf;
        }

    }

    mb->bestBuf = bestBuf;

    Xin265pMbPostInit (
        secSet,
        mb);

}

