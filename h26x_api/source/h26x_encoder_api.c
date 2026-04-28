/***************************************************************************//**
*
* @file          h26x_encoder_api.c
* @brief         h26x encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "stdlib.h"
#include "memory.h"
#include "basic_macro.h"
#include "h26x_definition.h"
#include "xin26x_params.h"
#include "xin26x_logger.h"
#include "h265_encoder_api.h"
#include "h265p_encoder_api.h"
#include "h266_encoder_api.h"
#include "h26x_encoder_struct.h"

DLLEXPORT SINT32 Xin26xEncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config)
{
    xin_encoder_struct *h26xEncoder;
    UINT32              algMode;
    SINT32              result;

    algMode = config->algorithmMode;

    if ((_XIN_LOGGER == NULL) && (config->pfXinLogEntry != NULL))
    {
        _XIN_LOGGER = config->pfXinLogEntry;
    }
    else if (_XIN_LOGGER == NULL)
    {
        _XIN_LOGGER = Xin26xLogEntry;
    }

    XIN_MALLOC_CHECK (h26xEncoder, sizeof(xin_encoder_struct))

    h26xEncoder->pfXinEncoderCreate   = NULL;
    h26xEncoder->pfXinEncodeFrame     = NULL;
    h26xEncoder->pfXinGetReconFrame   = NULL;
    h26xEncoder->pfXinControlOption   = NULL;
    h26xEncoder->pfXinEncoderDelete   = NULL;
    h26xEncoder->pfXinEncoderReconfig = NULL;

    switch (algMode)
    {
    case XIN_ALG_H265:

#ifdef ENABLE_H265_ENCODER
        h26xEncoder->pfXinEncoderCreate   = Xin265EncoderCreate;
        h26xEncoder->pfXinEncodeFrame     = Xin265EncodeFrame;
        h26xEncoder->pfXinGetReconFrame   = Xin265GetReconFrame;
        h26xEncoder->pfXinControlOption   = Xin265ControlOption;
        h26xEncoder->pfXinEncoderDelete   = Xin265EncoderDelete;
#else
        _XIN_LOGGER (XIN_LOGGER_ERROR, "H265 is not enabled in this build!\n");
#endif
        break;

    case XIN_ALG_AV1:

#ifdef ENABLE_AV1_ENCODER
        h26xEncoder->pfXinEncoderCreate   = Xin265pEncoderCreate;
        h26xEncoder->pfXinEncodeFrame     = Xin265pEncodeFrame;
        h26xEncoder->pfXinGetReconFrame   = Xin265pGetReconFrame;
        h26xEncoder->pfXinControlOption   = Xin265pControlOption;
        h26xEncoder->pfXinEncoderDelete   = Xin265pEncoderDelete;
#else
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Av1 is not enabled in this build!\n");
#endif
        break;

    case XIN_ALG_H266:

#ifdef ENABLE_H266_ENCODER
        h26xEncoder->pfXinEncoderCreate = Xin266EncoderCreate;
        h26xEncoder->pfXinEncodeFrame   = Xin266EncodeFrame;
        h26xEncoder->pfXinGetReconFrame = Xin266GetReconFrame;
        h26xEncoder->pfXinControlOption = Xin266ControlOption;
        h26xEncoder->pfXinEncoderDelete = Xin266EncoderDelete;
#else
        _XIN_LOGGER (XIN_LOGGER_ERROR, "H266 is not enabled in this build!\n");
#endif
        break;

    default:
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid algorithm mode!\n");
        break;

    }

    *encoderHandle = (XIN_HANDLE)h26xEncoder;

    if (h26xEncoder->pfXinEncoderCreate)
    {
        result = h26xEncoder->pfXinEncoderCreate (
                     &h26xEncoder->encoderHandle,
                     config);
    }
    else
    {
        result = XIN_FAIL;
    }

    return result;

}

DLLEXPORT void Xin26xEncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf)
{
    xin_encoder_struct *h26xEncoder;

    h26xEncoder = (xin_encoder_struct *)encoderHandle;

    h26xEncoder->pfXinEncodeFrame (
        h26xEncoder->encoderHandle,
        inputBuf,
        outputBuf);
}

DLLEXPORT void Xin26xControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam)
{
    xin_encoder_struct *h26xEncoder;

    h26xEncoder = (xin_encoder_struct *)encoderHandle;

    h26xEncoder->pfXinControlOption (
        h26xEncoder->encoderHandle,
        dynParam);
}

DLLEXPORT void Xin26xSetDefaultParam (
    xin26x_params *param)
{

    memset (param, 0, sizeof(xin26x_params));

    param->inputWidth  = 1280;
    param->inputHeight = 720;
    param->frameRate   = 30.0;
    param->bitRate     = 1000000;
    param->minQp       = 10;
    param->maxQp       = 51;
    param->rcMode      = XIN_RC_ABR;

    param->outputFormat = 1;
    param->refFrameNum  = 1;
    param->bitDepth     = 8;
    param->minCbSize    = 8;
    param->maxCbSize    = 64;
    param->minTbSize    = 4;
    param->maxTbSize    = 32; 
    param->interTbDepth = 0;
    param->intraTbDepth = 0;
    param->minQtSize    = 8;
    param->ctuSize      = 128;
    param->qp           = 32;
    param->enableSao    = TRUE;
    param->enableTMvp   = TRUE;
    param->bFrameNum    = 15;
    param->logLevel     = XIN26X_LOG_SEQ;
    param->refreshType  = XIN_IDR_REFRESH;

    param->maxMttDepth  = 1;
    param->ctuSize      = 128;
    param->minCuSize    = 8;
    param->minQtSize    = 8;
    param->maxBtSize    = 64;
    param->maxTtSize    = 64;
    param->lumaTrSize64 = TRUE;
    param->enableAlf    = TRUE;
    
    param->numTileCols  = 1;
    param->numTileRows  = 1;
    param->enableWpp    = TRUE;
    param->enableFpp    = TRUE;
    param->unitTree     = TRUE;
    param->enableGpb    = TRUE;
    param->clientId     = 0;

    param->motionSearchMode = XIN_ME_BBDGS_SEARCH;
    param->searchRange      = 64;
    param->adaptiveBFrame   = TRUE;
    param->twoPassEncoder   = FALSE;
    param->enableMctf       = TRUE;
    param->enableBim        = TRUE;
    param->enableSceneCut   = TRUE;
    param->chromaQpOffset   = -1;
    
    param->sbSize               = 128;
    param->enableRectPartType   = TRUE;
    param->frameToBeEncoded     = 0;
    param->enableSignDataHiding = TRUE;
    param->temporalLayerNum     = 1;
    param->enableCuQpDelta      = TRUE;
    param->lookAhead            = 48;
    param->threadNum            = 0;
    param->unitTreeStrength     = 2.0;
    param->enableDeblock        = TRUE;

}

DLLEXPORT void Xin26xEncoderReconfig (
    XIN_HANDLE encoderHandle)
{
    xin_encoder_struct *h26xEncoder;

    h26xEncoder = (xin_encoder_struct *)encoderHandle;

    h26xEncoder->pfXinEncoderReconfig (
        h26xEncoder->encoderHandle);
}

DLLEXPORT void Xin26xGetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum)
{
    xin_encoder_struct *h26xEncoder;

    h26xEncoder = (xin_encoder_struct *)encoderHandle;

    h26xEncoder->pfXinGetReconFrame (
        h26xEncoder->encoderHandle,
        reconBuf,
        reconBufNum);
}

DLLEXPORT void Xin26xEncoderDelete (
    XIN_HANDLE encoderHandle)
{
    xin_encoder_struct *h26xEncoder;

    h26xEncoder = (xin_encoder_struct *)encoderHandle;

    h26xEncoder->pfXinEncoderDelete (
        h26xEncoder->encoderHandle);

    free (h26xEncoder);
}


