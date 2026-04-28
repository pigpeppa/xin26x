/***************************************************************************//**
*
* @file          h265p_encoder_api.h
* @brief         av1 encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin265p_encoder_api_h_
#define _xin265p_encoder_api_h_

SINT32 Xin265pEncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config);

void Xin265pEncoderDelete (
    XIN_HANDLE encoderHandle);

void Xin265pEncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf);

void Xin265pGetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum);

void Xin265pEncoderReconfig (
    XIN_HANDLE encoderHandle);

void Xin265pSetDefaultParam (
    xin26x_params * param);

void Xin265pControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam);

#endif

