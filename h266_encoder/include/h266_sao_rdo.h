/***************************************************************************//**
 *
 * @file          h266_sao_rdo.h
 * @brief         This file declares SAO-related subroutines.
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
#ifndef _h266_sao_rdo_h_
#define _h266_sao_rdo_h_

void Xin266SaoCtuLuma (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266SaoCtuChroma (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266SaoRdoCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266SaoStatEo64xH_Neon (
    PIXEL     *input,
    intptr_t  inputStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diff,
    UINT16    *count);

void Xin266SaoStatEo32xH_Neon (
    PIXEL     *input,
    intptr_t  inputStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diff,
    UINT16    *count);

void Xin266SaoCtu (
    xin_ctu_struct *ctu);

#endif

