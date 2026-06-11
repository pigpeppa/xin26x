/***************************************************************************//**
 *
 * @file          h26x_calc_psnr_ssim.h
 * @brief         Declares subroutines to compute PSNR or SSIM.
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
#ifndef _h26x_calc_psnr_ssim_h_
#define _h26x_calc_psnr_ssim_h_

void Xin26xCalcPsnr (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    double   *psnr);

#endif