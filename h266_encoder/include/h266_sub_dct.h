/***************************************************************************//**
 *
 * @file          h266_sub_dct.h
 * @brief         This file declares h.266 subtract and forward transform subroutines.
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
#ifndef _h266_sub_dct_h_
#define _h266_sub_dct_h_

void Xin266SubFDctWxH (
    xin_sec_struct *secSet,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    COEFF          *output,
    intptr_t       outputStride,
    UINT32         compId,
    UINT32         mtsIdx,
    SINT32         width,
    SINT32         height,
    BOOL           isIntra);

#endif

