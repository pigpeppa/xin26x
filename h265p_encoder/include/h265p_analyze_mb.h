/***************************************************************************//**
 *
 * @file          h265p_analyze_mb.h
 * @brief         This file contains block analyze subroutine declarations.
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
#ifndef _h265p_analyze_mb_h_
#define _h265p_analyze_mb_h_

void Xin265pAnalyzeMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb);

void Xin265pAnalyzeIntraMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf **fastBuf,
    UINT32          bufNum,
    xin_fast_md_buf *interMdBuf);

void Xin265pGetBlockAvail (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb);

void Xin265pEncodeIntraMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf);

#endif

