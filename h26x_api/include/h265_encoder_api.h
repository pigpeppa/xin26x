/***************************************************************************//**
*
* @file          h265_encoder_api.h
* @brief         h265 encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin265_encoder_api_h_
#define _xin265_encoder_api_h_

SINT32 Xin265EncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config);

void Xin265EncoderDelete (
    XIN_HANDLE encoderHandle);

void Xin265EncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf);

void Xin265GetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum);

void Xin265EncoderReconfig (
    XIN_HANDLE encoderHandle);

void Xin265SetDefaultParam (
    xin26x_params * param);

void Xin265ControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam);

#endif