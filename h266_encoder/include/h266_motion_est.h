/***************************************************************************//**
 *
 * @file          h266_motion_est.h
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
#ifndef _h266_motion_est_h_
#define _h266_motion_est_h_

void Xin266FullSubPelMe (
    xin_sec_struct *secSet,
    SINT32         listIdx,
    SINT32         refIdx);

void Xin266SadCostSelect8 (
    UINT32  *sadCost,
    UINT32  *bestCost,
    SINT32  mvdX,
    SINT32  mvdY,
    UINT32  lambda,
    SINT32  mvFrac,
    UINT32  *bestPos);

void Xin266FastSubPelMeVar (
    xin_me_struct *meSet,
    SINT32        listIdx,
    SINT32        refIdx);

#endif
