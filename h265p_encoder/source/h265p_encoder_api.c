/***************************************************************************//**
*
* @file          h265p_encoder_api.c
* @brief         h265p encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "stdlib.h"
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
#include "h26x_calc_log.h"
#include "xin26x_logger.h"
#include "h265p_common_data.h"
#include "h26x_block_utility.h"
#include "xin26x_params.h"
#include "h265p_enc_init.h"

void CopyPicture (
    xin_func_struct   *funcSet,
    xin_frame_desc    *srcBuf,
    xin_input_picture *dstbuf)
{
    PIXEL   *srcY;
    PIXEL   *dstY;
    PIXEL   *srcU;
    PIXEL   *dstU;
    PIXEL   *srcV;
    PIXEL   *dstV;

    (void)funcSet;

    srcY = srcBuf->yuvBuf[PLANE_LUMA];
    srcU = srcBuf->yuvBuf[PLANE_CHROMA_U];
    srcV = srcBuf->yuvBuf[PLANE_CHROMA_V];

    dstY = dstbuf->inputBuf[PLANE_LUMA];
    dstU = dstbuf->inputBuf[PLANE_CHROMA_U];
    dstV = dstbuf->inputBuf[PLANE_CHROMA_V];

    Xin26xBlockCopy (
        srcY,
        srcBuf->lumaStride,
        dstY,
        dstbuf->inputStride[PLANE_LUMA],
        srcBuf->lumaWidth,
        srcBuf->lumaHeight);

    Xin26xBlockCopy (
        srcU,
        srcBuf->chromaStride,
        dstU,
        dstbuf->inputStride[PLANE_CHROMA],
        srcBuf->lumaWidth/2,
        srcBuf->lumaHeight/2);

    Xin26xBlockCopy (
        srcV,
        srcBuf->chromaStride,
        dstV,
        dstbuf->inputStride[PLANE_CHROMA],
        srcBuf->lumaWidth/2,
        srcBuf->lumaHeight/2);

}

static xin_pic_struct* PickupFreePicSet (
    xin265p_encoder_struct *h265pEncoder)
{
    UINT32          frameThreadIdx;
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;

    seqSet = h265pEncoder->seqSet;

    for (frameThreadIdx = 0; frameThreadIdx < seqSet->config.frameThreadNum; frameThreadIdx++)
    {
        picSet = h265pEncoder->picSet[frameThreadIdx];

        if (picSet->isFree)
        {
            return picSet;
        }
    }

    return NULL;

}

void Xin265pPrepareFrame (
    xin_seq_struct    *seqSet,
    xin_frame_desc    *inputBuf,
    xin_input_picture **dblOutputBuf)
{
    UINT32             predGopIdx;
    UINT32             predGopSize;
    xin_input_picture  *outputPicture;
    xin_rps_struct     *rps;
    UINT32             framePOC;
    UINT32             intraPeriod;

    predGopSize   = seqSet->predGopSize;
    intraPeriod   = seqSet->config.intraPeriod;
    *dblOutputBuf = NULL;

    if ((seqSet->lookaheadNumber == seqSet->inputNumber) && (inputBuf == NULL))
    {
        return;
    }

    outputPicture = seqSet->allocInputPic;
    *dblOutputBuf = outputPicture;
    framePOC      = seqSet->frameNumber;
    predGopIdx    = framePOC % predGopSize;

    if ((intraPeriod > 0) && ((framePOC % intraPeriod) == 0))
    {
        seqSet->intraRefresh |= TRUE;
    }

    outputPicture->inputNumber = framePOC;
    outputPicture->predGopIdx  = predGopIdx;
    outputPicture->predGopSize = predGopSize;

    outputPicture->inputBuf[PLANE_LUMA]     = inputBuf->yuvBuf[PLANE_LUMA];
    outputPicture->inputBuf[PLANE_CHROMA_U] = inputBuf->yuvBuf[PLANE_CHROMA_U];
    outputPicture->inputBuf[PLANE_CHROMA_V] = inputBuf->yuvBuf[PLANE_CHROMA_V];

    outputPicture->inputStride[PLANE_LUMA]   = inputBuf->lumaStride;
    outputPicture->inputStride[PLANE_CHROMA] = inputBuf->chromaStride;

    if ((framePOC == 0) || ((seqSet->intraRefresh) && (predGopIdx == 0)))
    {
        outputPicture->frameType   = XIN_IDR_FRAME;
        seqSet->intraRefresh       = FALSE;
        seqSet->frameNumber        = 0;
        seqSet->inputFilled        = TRUE;
    }
    else
    {
        outputPicture->frameType = XIN_P_FRAME;
    }

    if (inputBuf)
    {
        seqSet->inputNumber++;
    }

    if (outputPicture)
    {
        predGopIdx = outputPicture->predGopIdx;
        rps        = seqSet->rpsSet + predGopIdx;
        rps       += (outputPicture->frameType == XIN_I_FRAME) ? seqSet->predGopSize : 0;

        memcpy (&outputPicture->rps, rps, sizeof(xin_rps_struct));
    }

}

DLLEXPORT void Xin265pEncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf)
{
    xin265p_encoder_struct *h265pEncoder;
    xin_pic_struct         *picSet;
    xin_pic_struct         **picList;
    xin_ref_picture        **reconList;
    xin_ref_picture        *pictureWrite;
    xin_seq_struct         *seqSet;
    xin_input_picture      *inputPicture;
    xin_input_picture      *encodePicture;
    UINT32                 sectionIdx;
    UINT32                 frameIdx;

    h265pEncoder = (xin265p_encoder_struct *)encoderHandle;
    seqSet       = h265pEncoder->seqSet;
    reconList    = h265pEncoder->reconList;
    picList      = h265pEncoder->picList;

    if ((inputBuf == NULL) && (seqSet->inputNumber == seqSet->encodedNumber))
    {
        outputBuf->bitsBuf       = NULL;
        outputBuf->bytesGenerate = -1;
        outputBuf->temporalLayer = 0;

        return;
    }

    Xin265pPrepareFrame (
        seqSet,
        inputBuf,
        &inputPicture);

    if (!seqSet->inputFilled)
    {
        outputBuf->bitsBuf       = NULL;
        outputBuf->bytesGenerate = 0;
        outputBuf->temporalLayer = 0;

        return;
    }

    encodePicture = inputPicture;
    seqSet->frameNumber++;

    picSet = PickupFreePicSet (h265pEncoder);

    if (picSet == NULL)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Picture Set is NULL.\n");
    }

    Xin265pFrameInit (
        picSet,
        encodePicture);

    pictureWrite = picSet->pictureWrite;

    if (!picSet->codingFrame)
    {
        pictureWrite->isFree     = TRUE;
        picSet->isFree           = TRUE;
        encodePicture->bufStage  = XIN_BUF_INVALID;
        outputBuf->bitsBuf       = NULL;
        outputBuf->bytesGenerate = 0;
        outputBuf->temporalLayer = 0;

        return;
    }

    for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
    {
        h265pEncoder->secSet[0]->sectionIdx = sectionIdx;
        h265pEncoder->secSet[0]->picSet     = picSet;
            
        Xin265pEncodeSection (
            h265pEncoder->secSet[0]);
    }

    picList[h265pEncoder->picListNum] = picSet;
    h265pEncoder->picListNum++;

    for (frameIdx = 0; frameIdx < h265pEncoder->picListNum; frameIdx++)
    {
        Xin265pFramePostInit (
            picList[frameIdx]);

        picList[frameIdx]->isFree = TRUE;
        inputPicture              = picList[frameIdx]->inputPicture;
        inputPicture->bufStage    = XIN_BUF_INVALID;
        reconList[frameIdx]       = picList[frameIdx]->pictureWrite;
    }

    h265pEncoder->reconListNum = h265pEncoder->picListNum;
    h265pEncoder->picListNum   = 0;

    seqSet->encodedNumber++;

    if (seqSet->inputFlush)
    {
        seqSet->flushIndex++;
    }

    outputBuf->bitsBuf       = seqSet->outputBuf->base;
    outputBuf->bytesGenerate = seqSet->outputBuf->index;
    outputBuf->temporalLayer = pictureWrite->temporalId;

}

void Xin265pGetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum)
{
    xin265p_encoder_struct *h265pEncoder;
    xin_ref_picture        **reconList;
    UINT32                 frameIdx;

    h265pEncoder  = (xin265p_encoder_struct *)encoderHandle;
    reconList     = h265pEncoder->reconList;
    *reconBufNum  = h265pEncoder->reconListNum;

    for (frameIdx = 0; frameIdx < h265pEncoder->reconListNum; frameIdx++)
    {
        reconBuf->yuvBuf[PLANE_LUMA]     = reconList[frameIdx]->refBuf[PLANE_LUMA];
        reconBuf->yuvBuf[PLANE_CHROMA_U] = reconList[frameIdx]->refBuf[PLANE_CHROMA_U];
        reconBuf->yuvBuf[PLANE_CHROMA_V] = reconList[frameIdx]->refBuf[PLANE_CHROMA_V];

        reconBuf->lumaStride   = reconList[frameIdx]->refStride[PLANE_LUMA];
        reconBuf->chromaStride = reconList[frameIdx]->refStride[PLANE_CHROMA];

        reconBuf->lumaWidth  = reconList[frameIdx]->inputWidth;
        reconBuf->lumaHeight = reconList[frameIdx]->inputHeight;

        reconBuf++;
    }

}

void Xin265pSetDefaultParam (
    xin26x_params *param)
{
    memset (param, 0, sizeof(xin26x_params));

    param->inputWidth  = 1280;
    param->inputHeight = 720;
    param->frameRate   = 30.0;
    param->bitRate     = 1000000;
    param->minQp       = 10;
    param->maxQp       = 51;
    param->rcMode      = 1;

    param->refFrameNum  = 1;
    param->bitDepth     = 8;
    param->minCbSize    = 8;
    param->maxCbSize    = 64;
    param->minTbSize    = 4;
    param->maxTbSize    = 32;
    param->interTbDepth = 0;
    param->intraTbDepth = 0;
    param->qp           = 32;
    param->enableSao    = TRUE;
    param->enableTMvp   = TRUE;
    param->bFrameNum    = 7;

    param->numTileCols  = 1;
    param->numTileRows  = 1;
    param->enableWpp    = TRUE;
    param->enableFpp    = TRUE;
    param->threadNum    = 128;
    param->unitTree     = TRUE;

    param->motionSearchMode = XIN_ME_BBDGS_SEARCH;
    param->searchRange      = 64;

    param->frameToBeEncoded     = 0;
    param->enableSignDataHiding = TRUE;
    param->temporalLayerNum     = 1;
    param->enableCuQpDelta      = TRUE;
    param->lookAhead            = 40;
    param->unitTreeStrength     = 2.0;

}

void Xin265pControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam)
{
    xin265p_encoder_struct *h265pEncoder;
    xin_seq_struct         *seqSet;

    h265pEncoder = (xin265p_encoder_struct *)encoderHandle;
    seqSet       = h265pEncoder->seqSet;

    switch (dynParam->optionId)
    {
    case XIN26X_OPTION_SET_IDR:
    {
        seqSet->intraRefresh |= TRUE;

        break;
    }
    case XIN26X_OPTION_GET_PSNR:
    {
        if (seqSet->encodedNumber)
        {
            dynParam->psnrYuv[PLANE_LUMA]     = seqSet->psnrYuv[PLANE_LUMA]/seqSet->encodedNumber;
            dynParam->psnrYuv[PLANE_CHROMA_U] = seqSet->psnrYuv[PLANE_CHROMA_U]/seqSet->encodedNumber;
            dynParam->psnrYuv[PLANE_CHROMA_V] = seqSet->psnrYuv[PLANE_CHROMA_V]/seqSet->encodedNumber;
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

