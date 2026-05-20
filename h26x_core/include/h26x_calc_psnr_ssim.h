/***************************************************************************//**
*
* @file          h26x_calc_psnr_ssim.h
* @brief         Declare subroutines to compute psnr or ssim.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
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