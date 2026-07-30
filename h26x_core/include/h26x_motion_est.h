/***************************************************************************//**
 *
 * @file          h26x_motion_est.h
 * @brief         This file declares motion estimation subroutines.
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
#ifndef _h26x_motion_est_h_
#define _h26x_motion_est_h_

void Xin26xMotionEstBbdgs (
    xin_me_struct *meSet);

void Xin26xMotionEstFull (
    xin_me_struct *meSet);

void Xin26xMotionEstHier (
    xin_me_struct *meSet);

void Xin26xMotionEstTz (
    xin_me_struct *meSet);

void Xin26xMotionEstDia (
    xin_me_struct  *meSet);

void Xin26xFastSubPelMe (
    xin_me_struct *meSet,
    SINT32        listIdx,
    SINT32        refIdx);

void Xin26xMeFuncInit (
    xin_me_struct *meSet,
    UINT32        cpuFeature);

void Xin26xSadSelect8 (
    UINT32  *sadCost,
    UINT32  *bestCost,
    UINT32  *bestPos);

void Xin26xChangeMvPrec (
    xin_mv32_u *inputMv,
    SINT32     inputPrec,
    SINT32     outputPrec);

#endif
