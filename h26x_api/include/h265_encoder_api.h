/***************************************************************************//**
 *
 * @file          h265_encoder_api.h
 * @brief         h265 encoder Application Programming Interface.
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