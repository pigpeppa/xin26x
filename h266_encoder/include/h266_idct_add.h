/***************************************************************************//**
 *
 * @file          h266_idct_add.h
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
#ifndef _h266_idct_add_h_
#define _h266_idct_add_h_

void Xin266IDctAdd (
    xin_sec_struct *secSet,
    COEFF          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    PIXEL          *output,
    intptr_t       outputStride,
    BOOL           isIntra,
    UINT32         compId,
    UINT32         mtsIdx,
    UINT32         width,
    UINT32         height);

#endif

