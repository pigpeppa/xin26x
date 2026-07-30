/***************************************************************************//**
 *
 * @file          h26x_rate_control.h
 * @brief         This file declares subroutines related to rate control.
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
#ifndef _h26x_rate_control_h_
#define _h26x_rate_control_h_

void Xin26xRcCbrGop (
    xin_rc_context *rcContext,
    UINT32         gopSize);

BOOL Xin26xRcCbrSkipDec (
    xin_rc_context *rcContext);

void Xin26xRcCbrPic (
    xin_rc_context *rcContext);

void Xin26xRcCbrUpdateSkip (
    xin_rc_context *rcContext);

void Xin26xRcCbrUpdatePic (
    xin_rc_context *rcContext,
    SINT32         outputBits,
    SINT32         headerBits);

SINT32 Xin26xRcCbrCreate (
    xin_rc_struct   *rcSet);

SINT32 Xin26xRcCbrCtu (
    xin_rc_context *rcContext,
    UINT32         ctuAddr);

void Xin26xRcCbrUpdateCtu (
    xin_rc_context *rcContext,
    UINT32         ctuAddr);

void Xin26xRcCbrDelete (
    xin_rc_struct *rcSet);

void Xin26xRcAbrGop (
    xin_rc_context *rcContext,
    UINT32         gopSize);

BOOL Xin26xRcAbrSkipDec (
    xin_rc_context *rcContext);

void Xin26xRcAbrPic (
    xin_rc_context *rcContext);

void Xin26xRcAbrUpdateSkip (
    xin_rc_context *rcContext);

void Xin26xRcAbrUpdatePic (
    xin_rc_context *rcContext,
    SINT32         outputBits,
    SINT32         headerBits);

SINT32 Xin26xRcAbrCreate (
    xin_rc_struct  *rcSet);

SINT32 Xin26xRcAbrCtu (
    xin_rc_context *rcContext,
    UINT32         ctuAddr);

void Xin26xRcAbrUpdateCtu (
    xin_rc_context *rcContext,
    UINT32         ctuAddr);

void Xin26xRcAbrDelete (
    xin_rc_struct *rcSet);

#endif
