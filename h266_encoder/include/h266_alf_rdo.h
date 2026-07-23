/***************************************************************************//**
 *
 * @file          h266_alf_rdo.h
 * @brief         This file declares ALF-related subroutines.
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
#ifndef _h266_alf_rdo_h_
#define _h266_alf_rdo_h_

void Xin266AlfGetStatCtu (
    xin_ctu_struct *ctu);

void Xin266DeriveAlfFilter (
    xin_pic_struct *picSet);

void Xin266AlfFrame (
    xin_pic_struct *picSet);

void Xin266AlfGetStatFrame (
    xin_pic_struct *picSet);

void Xin266AlfPadPixel (
    xin_ctu_struct *ctu);

void Xin266AlfStatCtu (
    xin_ctu_struct *ctu);

void Xin266CcAlfCopyFrame (
    xin_pic_struct *picSet);

void Xin266DeriveStatsCcAlfFFrame (
    xin_pic_struct *picSet);

void Xin266DeriveCcAlfFilter (
    xin_alf_struct *alfSet,
    SINT32         compId);

void Xin266CcAlfFrame (
    xin_pic_struct *picSet);

void Xin266AlfCtu (
    xin_ctu_struct *ctu);

void Xin266CcAlfStatCtu (
    xin_ctu_struct *ctu);

void Xin266CcAlfCtu (
    xin_ctu_struct *ctu);

void Xin266ALfFuncInit (
    xin_alf_struct *alfSet,
    UINT32         cpuFeature);

#endif

