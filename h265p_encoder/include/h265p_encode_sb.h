/***************************************************************************//**
 *
 * @file          h265p_encode_sb.h
 * @brief         This file contains super block level subroutine declarations.
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
#ifndef _h265p_encode_sb_h_
#define _h265p_encode_sb_h_

void Xin265pReadInputSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pReadInputSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pEncodeSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pWriteSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pComputeBsSb (
    xin_sec_struct *secSet);

void Xin265pDeblockSbHor (
    xin_pic_struct *picSet,
    xin_sb_struct  *sb);

void Xin265pDeblockSbVer (
    xin_pic_struct *picSet,
    xin_sb_struct  *sb);

#endif
