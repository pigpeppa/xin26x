/***************************************************************************//**
*
* @file          h26x_encoder_struct.h
* @brief         h26x encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h26x_encoder_struct_h_
#define _h26x_encoder_struct_h_

typedef void (*XinEncodeFrame) (
    XIN_HANDLE       encoderHandle,
    xin_frame_desc   *inputBuf,
    xin_out_buf_desc *outputBuf);

typedef void (*XinControlOption) (
    XIN_HANDLE            encoderHandle,
    xin26x_dynamic_params *dynParam);

typedef SINT32 (*XinEncoderCreate) (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config);

typedef void (*XinEncoderReconfig) (
    XIN_HANDLE encoderHandle);

typedef void (*XinGetReconFrame) (
    XIN_HANDLE     encoderHandle,
    xin_frame_desc *reconBuf,
    UINT32         *reconBufNum);

typedef void (*XinEncoderDelete) (
    XIN_HANDLE encoderHandle);

typedef struct xin_encoder_struct
{
    XIN_HANDLE         encoderHandle;
    XinEncodeFrame     pfXinEncodeFrame;
    XinControlOption   pfXinControlOption;
    XinEncoderCreate   pfXinEncoderCreate;
    XinEncoderReconfig pfXinEncoderReconfig;
    XinGetReconFrame   pfXinGetReconFrame;
    XinEncoderDelete   pfXinEncoderDelete;
} xin_encoder_struct;

#endif
