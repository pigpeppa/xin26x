/***************************************************************************//**
 *
 * @file          h266_encoder_api.h
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
