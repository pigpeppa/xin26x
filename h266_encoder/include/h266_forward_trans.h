/***************************************************************************//**
 *
 * @file          h266_forward_trans.h
 * @brief         h.266 forward transform subroutines.
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
#ifndef _h266_forward_trans_h_
#define _h266_forward_trans_h_

void Xin266FDctWxH (
    xin_sec_struct *secSet,
    COEFF          *input,
    intptr_t       inputStride,
    COEFF          *output,
    intptr_t       outputStride,
    UINT32         compId,
    UINT32         width,
    UINT32         height,
    UINT32         mtsIdx,
    BOOL           isIntra);

#endif

