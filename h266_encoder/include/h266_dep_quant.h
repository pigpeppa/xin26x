/***************************************************************************//**
 *
 * @file          h266_dep_quant.h
 * @brief         This file declares functions related to dependent quantization.
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
#ifndef _h266_dep_quant_h_
#define _h266_dep_quant_h_

SINT32 Xin266CreateDepQuantRom (
    xin_seq_struct *seqSet);

void Xin266DeleteDepQuantRom (
        xin_seq_struct *seqSet);

void Xin266DepQuant (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_struct   *tu,
    COEFF           *tCoeff,
    COEFF           *rCoeff,
    COEFF           *qCoeff,
    intptr_t        coeffStride,
    UINT32          qp,
    SINT32          width,
    SINT32          height,
    UINT64          *nzCGBitMapRs,
    UINT32          compId);

void Xin266DepQuant_AVX2 (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_struct   *tu,
    COEFF           *tCoeff,
    COEFF           *rCoeff,
    COEFF           *qCoeff,
    intptr_t        coeffStride,
    UINT32          qp,
    SINT32          width,
    SINT32          height,
    UINT64          *nzCGBitMapRs,
    UINT32          compId);

SINT32 Xin266CreateDepQuant (
    xin_sec_struct *secSet);

void Xin266PreQuantCoeff (
    SINT32       absCoeff,
    xin_pq_data  *pqData,
    xin_dp_param *dpParam);

void Xin266InitContext (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_param    *tuParam,
    xin_prob_model  *context,
    UINT32          compId);

void Xin266DepQuantReset (
    xin_dep_quant *depQuant,
    xin_tu_param  *tuParam);

#endif

