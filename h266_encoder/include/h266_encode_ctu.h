/***************************************************************************//**
 *
 * @file          h266_encode_ctu.h
 * @brief         This file contains CTU level subroutine declarations.
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
#ifndef _h266_encode_ctu_h_
#define _h266_encode_ctu_h_

void Xin266ReadInputCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266EncodeCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266WriteCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu,
    BOOL           realEntropy);

void Xin266CtuInfoUpdate (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266ComputeBsCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266DeblockCtuHor (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu);

void Xin266DeblockCtuVer (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu);

void Xin266DeblockCtu (
    xin_ctu_struct *ctu);

void Xin266AnalyzeCuRec (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu);

void Xin266StoreCtuData (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

#endif

