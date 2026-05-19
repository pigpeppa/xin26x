/***************************************************************************//**
*
* @file          h265p_compute_dist.h
* @brief         This file declare sad or sse computation.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_compute_dist_h_
#define _h265p_compute_dist_h_

void Xin265pComputeSsdFd (
    SINT32   *tCoeff,
    intptr_t tCoeffStride,
    SINT32   *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    SINT32   txShift,
    UINT64   *ssd);

void Xin265pComputeSsdFdGt16xH_AVX2 (
    SINT32   *tCoeff,
    intptr_t tCoeffStride,
    SINT32   *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    SINT32   txSize,
    UINT64   *ssd);

#endif

