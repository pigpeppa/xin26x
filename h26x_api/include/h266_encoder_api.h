/***************************************************************************//**
*
* @file          h266_encoder_api.h
* @brief         h266 encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin266_encoder_api_h_
#define _xin266_encoder_api_h_

SINT32 Xin266EncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config);

void Xin266EncoderDelete (
    XIN_HANDLE encoderHandle);

void Xin266EncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf);

void Xin266GetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum);

void Xin266EncoderReconfig (
    XIN_HANDLE encoderHandle);

void Xin266SetDefaultParam (
    xin26x_params * param);

void Xin266ControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam);

#endif
