/***************************************************************************//**
 *
 * @file          h266_encoder_api.c
 * @brief         h266 encoder Application Programming Interface.
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
#include "xin26x_params.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
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
#include "h266_coding_unit_struct.h"
#include "h266_enc_init.h"
#include "h26x_thread_wrapper.h"
#include "h26x_thread_pool.h"
#include "h266_encoder_create.h"
#include "h26x_block_utility.h"
#include "h266_lookahead_frame.h"
#include "h26x_common_data.h"
#include "h26x_extend_picture.h"
#include "h26x_frame_operation.h"
#include "h266_func_struct.h"

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

extern xin_rps_struct* xin266RpsRandom[XIN_MAX_PRED_HIER_NUM];
extern xin_rps_struct* xin266RpsLowDelay[XIN_MAX_TEMPORAL_LAYER+1];
extern UINT32* xin266encOrder[XIN_MAX_PRED_HIER_NUM];

static void Xin266CopyPicture (
    xin_seq_struct    *seqSet,
    xin_frame_desc    *srcBuf,
    xin_input_picture *dstbuf)
{
    void   *srcY;
    PIXEL  *dstY;
    void   *srcU;
    PIXEL  *dstU;
    void   *srcV;
    PIXEL  *dstV;

    xin_func_struct *funcSet;

    funcSet = seqSet->funcSet;

    srcY = srcBuf->yuvBuf[PLANE_LUMA];
    srcU = srcBuf->yuvBuf[PLANE_CHROMA_U];
    srcV = srcBuf->yuvBuf[PLANE_CHROMA_V];

    dstY = dstbuf->inputBuf[PLANE_LUMA];
    dstU = dstbuf->inputBuf[PLANE_CHROMA_U];
    dstV = dstbuf->inputBuf[PLANE_CHROMA_V];

    funcSet->pfXinPictureScaleCopy (
        (void *)srcY,
        srcBuf->lumaStride,
        (void *)dstY,
        dstbuf->inputStride[PLANE_LUMA],
        seqSet->config.inputBitDepth,
        seqSet->config.internalBitDepth,
        srcBuf->lumaWidth,
        srcBuf->lumaHeight);

    funcSet->pfXinPictureScaleCopy (
        (void *)srcU,
        srcBuf->chromaStride,
        (void *)dstU,
        dstbuf->inputStride[PLANE_CHROMA],
        seqSet->config.inputBitDepth,
        seqSet->config.internalBitDepth,
        srcBuf->lumaWidth/2,
        srcBuf->lumaHeight/2);

    funcSet->pfXinPictureScaleCopy (
        (void *)srcV,
        srcBuf->chromaStride,
        (void *)dstV,
        dstbuf->inputStride[PLANE_CHROMA],
        seqSet->config.inputBitDepth,
        seqSet->config.internalBitDepth,
        srcBuf->lumaWidth/2,
        srcBuf->lumaHeight/2);

}

static void Xin266CopyInputFrame (
    xin_seq_struct    *seqSet,
    xin_frame_desc    *inputBuf)
{
    xin_input_picture  *inputPicture;
    xin_input_picture  **inputQueue;

    inputPicture = NULL;
    inputQueue   = seqSet->inputQueue;

    if (inputBuf == NULL)
    {
        seqSet->flushIdx = seqSet->flushIdx < 0 ? seqSet->inputIdx : seqSet->flushIdx;

        return;
    }

    if (seqSet->config.bFrameNum || seqSet->config.lookAhead)
    {
        Xin26xFramePop (
            (void **)seqSet->inputList,
            &seqSet->inputListNum,
            (void **)&inputPicture);

        if (inputPicture == NULL)
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "There is no input buffer available.\n");

            return;
        }

        Xin266CopyPicture (
            seqSet,
            inputBuf,
            inputPicture);

        Xin26xExtendPicture (
            inputPicture->inputBuf[0],
            inputPicture->inputStride[0],
            seqSet->config.inputWidth,
            seqSet->config.inputHeight,
            inputPicture->inputMarginX,
            inputPicture->inputMarginY);

        Xin26xExtendPicture (
            inputPicture->inputBuf[1],
            inputPicture->inputStride[1],
            seqSet->config.inputWidth/2,
            seqSet->config.inputHeight/2,
            inputPicture->inputMarginX/2,
            inputPicture->inputMarginY/2);

        Xin26xExtendPicture (
            inputPicture->inputBuf[2],
            inputPicture->inputStride[1],
            seqSet->config.inputWidth/2,
            seqSet->config.inputHeight/2,
            inputPicture->inputMarginX/2,
            inputPicture->inputMarginY/2);

        inputPicture->predGopIdx   = XIN_MAX_GOP_SIZE;
        inputPicture->bufStage     = XIN_BUF_INPUT;
        inputPicture->inputNumber  = seqSet->inputNumber;
        inputPicture->intraUnitNum = 0;
        inputPicture->isSceneCut   = !inputPicture->inputNumber;
        inputPicture->isMctfFrame  = FALSE;

    }
    else
    {
        inputPicture = seqSet->inputFrame;

        inputPicture->inputNumber = seqSet->inputNumber;
        inputPicture->predGopIdx  = XIN_MAX_GOP_SIZE;
        inputPicture->isSceneCut  = !inputPicture->inputNumber;
        inputPicture->isMctfFrame = FALSE;

        inputPicture->inputBuf[PLANE_LUMA]     = inputBuf->yuvBuf[PLANE_LUMA];
        inputPicture->inputBuf[PLANE_CHROMA_U] = inputBuf->yuvBuf[PLANE_CHROMA_U];
        inputPicture->inputBuf[PLANE_CHROMA_V] = inputBuf->yuvBuf[PLANE_CHROMA_V];

        inputPicture->inputStride[PLANE_LUMA]   = inputBuf->lumaStride;
        inputPicture->inputStride[PLANE_CHROMA] = inputBuf->chromaStride;

        inputPicture->inputWidth  = inputBuf->lumaWidth;
        inputPicture->inputHeight = inputBuf->lumaHeight;

    }

    inputQueue[seqSet->inputIdx] = inputPicture;

    seqSet->inputIdx++;

    seqSet->inputNumber++;

}


void Xin266ProFrameWrapper (
    void *opaque)
{
    xin_pic_struct *picSet;

    picSet = (xin_pic_struct *)opaque;

    Xin266ProFrame (
        picSet);
}

void Xin266PreFrameWrapper (
    void *opaque)
{
    xin_pic_struct *picSet;

    picSet = (xin_pic_struct *)opaque;

    Xin266PreFrame (
        picSet);
}

void Xin266AddRefDep (
    xin_pic_struct *picSet,
    xin_job_desc   *jobPreFrame)
{
    xin_ref_picture *pictureWrite;
    xin_ref_picture *pictureRead;
    SINT32          refIdx;
    SINT32          refNum;

    pictureWrite = picSet->pictureWrite;

    if (pictureWrite->frameType >= XIN_I_FRAME)
    {
        return;
    }

    refNum = (pictureWrite->predIdxInGop == 0) ? pictureWrite->numOfRefs[XIN_LIST_0] : picSet->validRefFrame;

    for (refIdx = 0; refIdx < refNum; refIdx++)
    {
        pictureRead = picSet->pictureRef[refIdx];

        Xin26xThreadJobDepAdd (
            jobPreFrame,
            pictureRead->jobProFrame);

    }

}

void Xin266AddRunningDep (
    xin266_encoder_struct *h266Encoder,
    xin_job_desc          *jobPreFrame)
{
    xin_ref_picture *pictureWrite;
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    SINT32          frameNum;
    SINT32          frameIdx;
    SINT32          baseEncIdx;
    SINT32          currEncIdx;

    seqSet     = h266Encoder->seqSet;
    frameNum   = seqSet->config.frameThreadNum - 1;
    baseEncIdx = seqSet->currEncoder;

    for (frameIdx = 0; frameIdx < frameNum; frameIdx++)
    {
        currEncIdx   = (frameIdx + baseEncIdx) % seqSet->config.frameThreadNum;
        picSet       = h266Encoder->picSet[currEncIdx];

        if (picSet->isBusy)
        {
            pictureWrite = picSet->pictureWrite;

            Xin26xThreadJobDepAdd (
                jobPreFrame,
                pictureWrite->jobProFrame);
        }

    }

}

void Xin266FrameEncode (
    xin266_encoder_struct *h266Encoder,
    xin_pic_struct        *picSet)
{
    xin_seq_struct   *seqSet;
    xin_ref_picture  *pictureWrite;
    UINT32           sectionIdx;
    xin_thread_queue *threadQueue;

    seqSet       = picSet->seqSet;
    threadQueue  = seqSet->threadQueue;
    pictureWrite = picSet->pictureWrite;

    Xin26xJobInit (
        pictureWrite->jobPreFrame,
        Xin266PreFrameWrapper,
        (void *)picSet);

    if (seqSet->config.frameThreadNum > 1)
    {
        if (pictureWrite->predIdxInGop)
        {
            Xin266AddRefDep (
                picSet,
                pictureWrite->jobPreFrame);
        }
        else
        {
            Xin266AddRunningDep (
                h266Encoder,
                pictureWrite->jobPreFrame);
        }
    }

    Xin26xThreadSubmit (
        threadQueue,
        pictureWrite->jobPreFrame);

    for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
    {
        Xin266EncodeSection (
            picSet,
            sectionIdx);
    }

    for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
    {
        Xin266LpfSection (
            picSet,
            sectionIdx);
    }

    for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
    {
        Xin266SaoSection (
            picSet,
            sectionIdx);
    }

    if (picSet->enableAlf)
    {
        for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
        {
            Xin266AlfStatSection (
                picSet,
                sectionIdx);
        }

        Xin266DeriveAlfFrame (
            picSet);

        for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
        {
            Xin266AlfSection (
                picSet,
                sectionIdx);
        }

        for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
        {
            Xin266CcAlfStatSection (
                picSet,
                sectionIdx);
        }

        Xin266DeriveCcAlfFrame (
            picSet);

        for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
        {
            Xin266CcAlfSection (
                picSet,
                sectionIdx);
        }

        Xin26xJobInit (
            pictureWrite->jobProFrame,
            Xin266ProFrameWrapper,
            (void *)picSet);

        Xin26xThreadJobDepAdd (
            pictureWrite->jobProFrame,
            picSet->jobCtuCcAlf + seqSet->frameSizeInCtu - 1);

        
    }
    else
    {

        Xin26xJobInit (
            pictureWrite->jobProFrame,
            Xin266ProFrameWrapper,
            (void *)picSet);

        Xin26xThreadJobDepAdd (
            pictureWrite->jobProFrame,
            picSet->jobCtuSao + seqSet->frameSizeInCtu - 1);
        
    }

    Xin26xThreadSubmit (
        threadQueue,
        pictureWrite->jobProFrame);

}

static void Xin266DpbPreUpdate (
    xin_pic_struct *picSet)
{
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_ref_picture **dpbQueue;

    seqSet        = picSet->seqSet;
    dpbQueue      = seqSet->dpbQueue;
    pictureWrite  = picSet->pictureWrite;

    if (pictureWrite->isReferenced)
    {
        memmove(dpbQueue + 1, dpbQueue, seqSet->dpbSize*sizeof(xin_ref_picture *));

        dpbQueue[0] = pictureWrite;
        seqSet->dpbSize++;
    }

}

static void Xin266DpbPostUpdate (
    xin_pic_struct *picSet)
{
    xin_ref_picture *pictureWrite;
    xin_ref_picture *pictureReturn;
    xin_seq_struct  *seqSet;
    UINT32          rdIdx;
    UINT32          wrIdx;
    BOOL            findCurrPic;
    xin_ref_picture **dpbQueue;

    pictureWrite  = picSet->pictureWrite;
    pictureReturn = NULL;
    seqSet        = picSet->seqSet;
    dpbQueue      = seqSet->dpbQueue;
    findCurrPic   = FALSE;

    if (pictureWrite->isReferenced)
    {
        // Remove all previous gop pictures, whose gop index is not zero
        if ((seqSet->config.bFrameNum > 0) && (pictureWrite->predIdxInGop == 0))
        {
            wrIdx = 0;

            for (rdIdx = 0; rdIdx < seqSet->dpbSize; rdIdx++)
            {
                findCurrPic |= (pictureWrite == dpbQueue[rdIdx]);

                if ((dpbQueue[rdIdx]->predIdxInGop == 0) || (findCurrPic == FALSE))
                {
                    dpbQueue[wrIdx] = dpbQueue[rdIdx];

                    wrIdx++;
                }
                else
                {
                    Xin26xFramePush (
                        (void **)seqSet->refList,
                        &seqSet->refListNum,
                        (void *)dpbQueue[rdIdx]);
                }
            }

            seqSet->dpbSize = wrIdx;

        }

        // If DPB size exceed allocated picture size,
        // then we shrink DPB buffer size.
        if ((seqSet->dpbSize + seqSet->config.frameThreadNum - 1) >= seqSet->refFrameNum)
        {
            pictureReturn = dpbQueue[seqSet->dpbSize - 1];
            seqSet->dpbSize--;
        }

        if (pictureReturn)
        {
            Xin26xFramePush (
                (void **)seqSet->refList,
                &seqSet->refListNum,
                (void *)pictureReturn);
        }

    }
    else
    {
        Xin26xFramePush (
            (void **)seqSet->refList,
            &seqSet->refListNum,
            (void *)pictureWrite);
    }

}


static void Xin266GopDecision (
    xin266_encoder_struct *h266Encoder)
{
    xin_seq_struct    *seqSet;
    SINT32            frameIdx;
    UINT32            predGopIdx;
    SINT32            maxGopSize;
    SINT32            gopSize;
    xin_input_picture *currPicture;
    xin_input_picture *pictureQueue[XIN_MAX_GOP_SIZE];
    SINT32            gopDecIdx;
    SINT32            mctfIdx;
    UINT32            lgGopSize;
    UINT32            *encoderOrder;

    seqSet      = h266Encoder->seqSet;
    maxGopSize  = seqSet->config.bFrameNum + 1;
    gopDecIdx   = seqSet->gopDecIdx;
    mctfIdx     = seqSet->mctfIdx;
    currPicture = XIN_QUEUE_DATA (seqSet->inputQueue, mctfIdx - 1);
    maxGopSize  = (mctfIdx != gopDecIdx) && (seqSet->flushIdx == mctfIdx) ? XIN_MIN ((mctfIdx - gopDecIdx), maxGopSize) : maxGopSize;
    maxGopSize  = XIN_MAX (1, maxGopSize);

    // output frame to lookahead
    if (XIN_QUEUE_SIZE (mctfIdx, gopDecIdx) >= maxGopSize)
    {
        for (frameIdx = 0; frameIdx < maxGopSize; frameIdx++)
        {
            currPicture = XIN_QUEUE_DATA (seqSet->inputQueue, gopDecIdx + frameIdx);

            if ((currPicture->isSceneCut) || (currPicture->inputNumber == 0) || (((currPicture->inputNumber - seqSet->gopLastIdr) == (SINT32)seqSet->config.intraPeriod) && (seqSet->config.refreshType == XIN_IDR_REFRESH)))
            {
                break;
            }
        }

        gopSize   = 1 << calcLog2[frameIdx];
        lgGopSize = calcLog2[gopSize];

        for (frameIdx = 0; frameIdx < gopSize; frameIdx++)
        {
            pictureQueue[frameIdx] = XIN_QUEUE_DATA (seqSet->inputQueue, gopDecIdx + frameIdx);
            currPicture            = pictureQueue[frameIdx];
            predGopIdx             = (frameIdx + 1) & (gopSize - 1);

            if ((currPicture->inputNumber == 0) || (currPicture->isSceneCut))
            {
                currPicture->frameType = XIN_IDR_FRAME;

                seqSet->gopLastIdr = currPicture->inputNumber;
            }
            else if ((currPicture->inputNumber - seqSet->gopLastIdr) == (SINT32)seqSet->config.intraPeriod)
            {
                currPicture->frameType = (seqSet->config.refreshType == XIN_IDR_REFRESH) ? XIN_IDR_FRAME : XIN_I_FRAME;

                seqSet->gopLastIdr = currPicture->inputNumber;
            }
            else
            {
                if (seqSet->config.enableGpb)
                {
                    currPicture->frameType = XIN_B_FRAME;
                }
                else
                {
                    currPicture->frameType = (seqSet->config.bFrameNum && predGopIdx != 0) ? XIN_B_FRAME : XIN_P_FRAME;
                }
            }

            currPicture->predGopSize = gopSize;
            currPicture->predGopIdx  = predGopIdx;

            memcpy(&currPicture->rps, &xin266RpsRandom[lgGopSize][predGopIdx], sizeof(xin_rps_struct));

            currPicture->temporalId = currPicture->rps.temporalId;

        }

        encoderOrder = xin266encOrder[lgGopSize];

        for (frameIdx = 0; frameIdx < gopSize; frameIdx++)
        {
            XIN_QUEUE_DATA(seqSet->encodeQueue, gopDecIdx + frameIdx) = pictureQueue[encoderOrder[frameIdx] - 1];
        }

        seqSet->gopDecIdx += gopSize;

    }

}

static void Xin266WriteReconFrame (
    xin_seq_struct   *seqSet,
    xin_ref_picture  *pictureWrite,
    xin_frame_struct *reconFrame)
{
    PIXEL           *srcY;
    PIXEL           *dstY;
    PIXEL           *srcU;
    PIXEL           *dstU;
    PIXEL           *srcV;
    PIXEL           *dstV;
    xin_func_struct *funcSet;

    funcSet = seqSet->funcSet;

    srcY = pictureWrite->refBuf[PLANE_LUMA];
    srcU = pictureWrite->refBuf[PLANE_CHROMA_U];
    srcV = pictureWrite->refBuf[PLANE_CHROMA_V];

    dstY = reconFrame->yuvBuf[PLANE_LUMA];
    dstU = reconFrame->yuvBuf[PLANE_CHROMA_U];
    dstV = reconFrame->yuvBuf[PLANE_CHROMA_V];

    funcSet->pfXinPictureCopy (
        srcY,
        pictureWrite->refStride[0],
        dstY,
        reconFrame->lumaStride,
        pictureWrite->lumaWidth,
        pictureWrite->lumaHeight);

    funcSet->pfXinPictureCopy (
        srcU,
        pictureWrite->refStride[1],
        dstU,
        reconFrame->chromaStride,
        pictureWrite->lumaWidth/2,
        pictureWrite->lumaHeight/2);

    funcSet->pfXinPictureCopy (
        srcV,
        pictureWrite->refStride[1],
        dstV,
        reconFrame->chromaStride,
        pictureWrite->lumaWidth/2,
        pictureWrite->lumaHeight/2);

}

static void Xin266CodingStageWait (
    xin_pic_struct   *picSet,
    xin_frame_struct *reconFrame)
{
    xin_ref_picture   *pictureEncode;
    xin_input_picture *inputPicture;
    xin_seq_struct    *seqSet;

    if (picSet->isBusy)
    {
        pictureEncode = picSet->pictureWrite;
        inputPicture  = picSet->inputPicture;
        seqSet        = picSet->seqSet;

        Xin26xThreadWaitFor (
            seqSet->threadQueue,
            pictureEncode->jobProFrame);

        Xin266FrameProInit (
            picSet);

        if (seqSet->config.needRecon)
        {
            Xin266WriteReconFrame (
                seqSet,
                pictureEncode,
                reconFrame);
        }

        Xin266DpbPostUpdate (
            picSet);

        if (!seqSet->config.zeroLatency)
        {
            Xin26xFramePush (
                (void **)seqSet->inputList,
                &seqSet->inputListNum,
                (void *)inputPicture);
        }

        seqSet->outputIdx++;

    }

}

static void Xin266CodingStage (
    xin266_encoder_struct *h266Encoder)
{
    xin_pic_struct    *picSet;
    xin_seq_struct    *seqSet;
    UINT32            encodeIdx;
    xin_ref_picture   *pictureEncode;
    xin_input_picture *inputPicture;
    BOOL              performStage;
    SINT32            currEncoder;

    seqSet        = h266Encoder->seqSet;
    currEncoder   = seqSet->currEncoder;
    picSet        = h266Encoder->picSet[currEncoder];
    pictureEncode = NULL;

    if (!seqSet->config.zeroLatency)
    {
        Xin266CodingStageWait (
            picSet,
            &h266Encoder->reconFrame);
    }

    if (seqSet->outputIdx == seqSet->flushIdx)
    {
        seqSet->encodeFinished = TRUE;
    }

    encodeIdx      = XIN_QUEUE_IDX (seqSet->encodeIdx);
    inputPicture   = seqSet->encodeQueue[encodeIdx];
    performStage   = !!XIN_QUEUE_SIZE (seqSet->preEncodeIdx, seqSet->encodeIdx);
    picSet->isBusy = performStage;

    seqSet->currEncoder = (currEncoder + 1) % seqSet->config.frameThreadNum;

    if (!performStage)
    {
        return;
    }

    Xin26xFramePop (
        (void **)seqSet->refList,
        &seqSet->refListNum,
        (void **)&pictureEncode);

    if (pictureEncode == NULL)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Can not find reference picture.\n");
    }

    Xin266FramePreInit (
        picSet,
        inputPicture,
        pictureEncode);

    Xin266FrameEncode (
        h266Encoder,
        picSet);

    Xin266DpbPreUpdate (
        picSet);

    seqSet->encodeIdx++;

    if (seqSet->config.zeroLatency)
    {
        Xin266CodingStageWait (
            picSet,
            &h266Encoder->reconFrame);
    }

}

static void Xin266PreEncode (
    xin266_encoder_struct *h266Encoder)
{
    xin_seq_struct    *seqSet;
    SINT32            gopDecIdx;
    SINT32            lookaheadIdx;
    SINT32            preEncodeIdx;
    xin_input_picture *inputPicture;
    SINT32            gopSize;
    SINT32            lookaheadSize;

    seqSet        = h266Encoder->seqSet;
    preEncodeIdx  = seqSet->preEncodeIdx;
    lookaheadSize = seqSet->config.lookAhead;

    if (lookaheadSize)
    {
        lookaheadIdx  = seqSet->lookaheadIdx;
        inputPicture  = XIN_QUEUE_DATA (seqSet->inputQueue, lookaheadIdx - 1);
        lookaheadSize = (lookaheadIdx != preEncodeIdx) && (seqSet->flushIdx == lookaheadIdx) ? lookaheadIdx - preEncodeIdx : lookaheadSize;
        lookaheadSize = XIN_MAX (1, lookaheadSize);

        if (XIN_QUEUE_SIZE (lookaheadIdx, preEncodeIdx) >= lookaheadSize)
        {
            Xin266UnitTree (
                seqSet);

            inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx);
            gopSize      = inputPicture->predGopSize;

            seqSet->preEncodeIdx += gopSize;
        }
    }
    else
    {
        gopDecIdx = seqSet->gopDecIdx;

        if (XIN_QUEUE_SIZE (gopDecIdx, preEncodeIdx))
        {
            inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx);
            gopSize      = inputPicture->predGopSize;

            seqSet->preEncodeIdx += gopSize;
        }
    }

}

DLLEXPORT void Xin266EncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf)
{
    xin266_encoder_struct *h266Encoder;
    xin_seq_struct        *seqSet;

    h266Encoder              = (xin266_encoder_struct *)encoderHandle;
    seqSet                   = h266Encoder->seqSet;
    seqSet->outputBuf->index = 0;

    Xin266CopyInputFrame (
        seqSet,
        inputBuf);

    if (seqSet->config.enableSceneCut)
    {
        Xin266SceneCutStage (
            h266Encoder);
    }
    else
    {
        seqSet->scenecutIdx = seqSet->inputIdx;
    }

    if (seqSet->config.enableMctf)
    {
        Xin266MctfStage (
            h266Encoder);
    }
    else
    {
        seqSet->mctfIdx = seqSet->scenecutIdx;
    }

    Xin266GopDecision (
        h266Encoder);

    if (seqSet->config.lookAhead > 0)
    {
        Xin266LookaheadStage (
            h266Encoder);
    }

    Xin266PreEncode (
        h266Encoder);

    Xin266CodingStage (
        h266Encoder);

    if ((seqSet->encodeFinished) && (seqSet->outputBuf->index == 0))
    {
        outputBuf->bitsBuf       = NULL;
        outputBuf->bytesGenerate = -1;
        outputBuf->temporalLayer = 0;
    }
    else
    {
        outputBuf->bitsBuf       = seqSet->outputBuf->base;
        outputBuf->bytesGenerate = seqSet->outputBuf->index;
        outputBuf->temporalLayer = 0;
    }

}

void Xin266GetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum)
{
    xin266_encoder_struct *h266Encoder;
    xin_frame_struct      *reconFrame;

    h266Encoder = (xin266_encoder_struct *)encoderHandle;
    reconFrame  = &h266Encoder->reconFrame;

    reconBuf->yuvBuf[PLANE_LUMA]     = reconFrame->yuvBuf[PLANE_LUMA];
    reconBuf->yuvBuf[PLANE_CHROMA_U] = reconFrame->yuvBuf[PLANE_CHROMA_U];
    reconBuf->yuvBuf[PLANE_CHROMA_V] = reconFrame->yuvBuf[PLANE_CHROMA_V];

    reconBuf->lumaStride   = reconFrame->lumaStride;
    reconBuf->chromaStride = reconFrame->chromaStride;

    reconBuf->lumaWidth  = reconFrame->lumaWidth;
    reconBuf->lumaHeight = reconFrame->lumaHeight;

    *reconBufNum = 1;

}


void Xin266ControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam)
{
    xin266_encoder_struct *h266Encoder;
    xin_seq_struct        *seqSet;

    (void)dynParam;
    h266Encoder = (xin266_encoder_struct *)encoderHandle;
    seqSet      = h266Encoder->seqSet;

    switch (dynParam->optionId)
    {
    case XIN26X_OPTION_SET_IDR:
    {
        seqSet->intraRefresh |= TRUE;

        break;
    }

    case XIN26X_OPTION_GET_PSNR:
    {
        if (seqSet->outputIdx)
        {
            dynParam->psnrYuv[PLANE_LUMA]     = seqSet->psnrYuv[PLANE_LUMA] / seqSet->outputIdx;
            dynParam->psnrYuv[PLANE_CHROMA_U] = seqSet->psnrYuv[PLANE_CHROMA_U] / seqSet->outputIdx;
            dynParam->psnrYuv[PLANE_CHROMA_V] = seqSet->psnrYuv[PLANE_CHROMA_V] / seqSet->outputIdx;
        }
        else
        {
            dynParam->psnrYuv[PLANE_LUMA]     = 0.0;
            dynParam->psnrYuv[PLANE_CHROMA_U] = 0.0;
            dynParam->psnrYuv[PLANE_CHROMA_V] = 0.0;
        }

        break;
    }

    default:
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "It is a invalid option.\n");

        break;
    }

    }

}

