/***************************************************************************//**
 *
 * @file          h26x_downscale_subs.h
 * @brief         This file declares downscale subroutines.
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
#ifndef _h26x_downscale_subs_h_
#define _h26x_downscale_subs_h_

void Xin26xDownscale2x2 (
    PIXEL       *input,
    intptr_t    inputStride,
    BOOL        haveInputAbove,
    BOOL        haveInputBelow,
    PIXEL       *output,    
    intptr_t    outputStride,   
    UINT32      outputWidth,    
    UINT32      outputHeight);

void Xin26xDownscale2x2_AVX2 (
    PIXEL       *input,
    intptr_t    inputStride,
    BOOL        haveInputAbove,
    BOOL        haveInputBelow,
    PIXEL       *output,    
    intptr_t    outputStride,   
    UINT32      outputWidth,    
    UINT32      outputHeight);

#endif