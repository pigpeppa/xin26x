/***************************************************************************//**
*
* @file          xin26x_encoder_api.h
* @brief         h26x encoder Application Programming Interface.
* @authors       Pig Peppa
* @copyright     (c) 2020, Pig Peppa <pig.peppa@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin26x_encoder_api_h_
#define _xin26x_encoder_api_h_

#ifdef __cplusplus
extern "C" {
#endif

SINT32 Xin26xEncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config);

void Xin26xEncoderDelete (
    XIN_HANDLE encoderHandle);

void Xin26xEncodeFrame (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf);

void Xin26xGetReconFrame (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum);

void Xin26xEncoderReconfig (
    XIN_HANDLE encoderHandle);

void Xin26xSetDefaultParam (
    xin26x_params * param);

void Xin26xControlOption (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam);

#ifdef __cplusplus
}
#endif
#endif