/***************************************************************************//**
 *
 * @file          h26x_encoder_struct.h
 * @brief         This file contains h26x encoder common data structures and function pointer types.
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
