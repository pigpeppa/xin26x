/***************************************************************************//**
 *
 * @file          h265p_idct_add.h
 * @brief         This file declares inverse transform and add prediction subroutines.
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
#ifndef _h265p_idct_add_h_
#define _h265p_idct_add_h_

void Xin265pIDctAdd (
    xin_sec_struct *secSet,
    SINT32         *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    PIXEL          *output,
    intptr_t       outputStride,
    UINT32         tranType,
    UINT32         tranSize);

#endif

