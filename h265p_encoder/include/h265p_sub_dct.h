/***************************************************************************//**
 *
 * @file          h265p_sub_dct.h
 * @brief         This file declares av1 subtract and forward transform subroutine.
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
#ifndef _h265p_sub_dct_h_
#define _h265p_sub_dct_h_

void Xin265pSubFDct (
    xin_sec_struct *secSet,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         *output,
    intptr_t       outputStride,
    UINT32         tranType,
    UINT32         tranSize);

#endif

