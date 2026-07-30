/***************************************************************************//**
 *
 * @file          h26x_look_ahead.h
 * @brief         This file declares h26x look ahead subroutines.
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
#ifndef _h26x_look_ahead_h_
#define _h26x_look_ahead_h_

void Xin26xLookaheadFuncInit (
    xin_la_struct *laSet,
    UINT32        cpuFeature);

void Xin26xLookaheadFrame (
    xin_la_struct *laSet);

#endif

