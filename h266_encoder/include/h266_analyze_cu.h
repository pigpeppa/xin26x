/***************************************************************************//**
 *
 * @file          h266_analyze_cu.h
 * @brief         This file contains CU analysis subroutines declaration.
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
#ifndef _h266_analyze_cu_h_
#define _h266_analyze_cu_h_

void Xin266AnalyzeSkipCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu);

void Xin266EncodeMergeCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu);

void Xin266EncodeSkipCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu);

void Xin266AnalyzeInterCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    SINT32          amvrIdx);

void Xin266EncodeInterCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu);

void Xin266AnalyzeIntraCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *interMdBuf);

void Xin266AnalyzeCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu);

void Xin266EncodeIntraCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf **outputBuf,
    xin_cu_struct   *cu);

void Xin266ChromaCompensation (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu);

#endif

