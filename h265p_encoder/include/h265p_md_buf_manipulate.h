/***************************************************************************//**
 *
 * @file          h265p_md_buf_manipulate.h
 * @brief         This file declares mode decision buffer manipulation subroutines.
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
#ifndef _h265p_md_buf_manipulate_h_
#define _h265p_md_buf_manipulate_h_

void Xin265pSortMdBufSad (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum);

void Xin265pSortMdBufSse (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum);

void Xin265pFindHighestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **highestBuf);

void Xin265pFindLowestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **lowestBuf);

#endif

