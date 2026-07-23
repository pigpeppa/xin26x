/***************************************************************************//**
 *
 * @file          h266_get_neighbour_mv.h
 * @brief         This file declares merge or skip MV candidate derivation subroutines.
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
#ifndef _h266_get_neighbour_mv_h_
#define _h266_get_neighbour_mv_h_

void Xin266FillMergeCandB (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *mergeCand,
    UINT32           *validMvs);

void Xin266FillMergeCandP (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *mergeCand,
    UINT32           *validMvs);

void Xin266FillMergeCandB (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *mergeCand,
    UINT32           *validMvs);

void Xin266FillAmvpCand (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    SINT32         listIdx,
    SINT32         refIdx,
    SINT32         amvrIdx,
    xin_mv32_u     *amvpCand,
    UINT32         *validMvs);

void Xin266ChangeMv2Me (
    xin_mv32_u *inputMv,
    xin_mv_u   *ouputMv);

void Xin266GetTMvp (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *neighbourMv);

void Xin266AddMvpCandHmvpLut (
    xin_sec_struct   *secSet,
    xin_neighbour_mv *neighbourMv);

void Xin266GetAffineMergeCand (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_affine_cpmv  *affMergeCand,
    UINT32           *mvNum);

void Xin266RoundAffineMv (
    SINT32 *mvx,
    SINT32 *mvy,
    SINT32 nShift);

void Xin266FillAffineMotionVector (
    xin_pu_struct   *pu,
    xin_mv32_u      affineMv[XIN_LIST_NUM][XIN_MAX_AFFINE_CPMV_NUM],
    SINT8           refIdx[XIN_LIST_NUM],
    UINT32          affineType,
    UINT32          listIdx,
    xin_mv32_u      *subBlockMv);

void Xin266ChangeMv32Prec (
    xin_mv32_u *inputMv,
    SINT32     inputPrec,
    SINT32     outputPrec);

void Xin266ChangeMvPrec (
    xin_mv_u *inputMv,
    SINT32   inputPrec,
    SINT32   outputPrec);

#endif

