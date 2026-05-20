/***************************************************************************//**
*
* @file          h26x_compute_dist.c
* @brief         Sad or satd subroutines declare.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h26x_compute_dist_h_
#define _h26x_compute_dist_h_

#define XIN_DIST_LOG_WGT_BASE   3
#define XIN_DIST_WGT_BASE       (1 << XIN_DIST_LOG_WGT_BASE)
#define XIN_DIST_WGT_OFFSET     (1 << (XIN_DIST_LOG_WGT_BASE - 1))

void Xin26xComputeSad (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad4xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad128xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad128xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xH_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xH_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xH_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xH_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad4xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad8xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad16xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad32xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad64xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad128xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSadGt16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad8xH_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSadGt8xH_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSsdFd (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeBlockSsd (
    COEFF   *tCoeff,
    intptr_t coeffStride,
    UINT32   *ssd,
    UINT64   *totalSsd);

void Xin26xComputeSsdFd4xH_SSE2 (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdFd8xH_SSE2 (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdFdGt8xH_SSE2 (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdFdGt16xH_AVX2 (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdFd4xH_Neon (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdFdGt4xH_Neon (
    COEFF    *tCoeff,
    intptr_t tCoeffStride,
    COEFF    *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeBlockSsd_SSE2 (
    SINT16   *tCoeff,
    intptr_t coeffStride,
    UINT32   *ssd,
    UINT64   *totalSsd);

void Xin26xComputeSsd (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsd4xH_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsd8xH_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdGt8xH_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdGt16xH_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsd4xH_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsd8xH_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSsdGt8xH_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

void Xin26xComputeSadx8 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xHx8_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx8_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSadGt16xHx8_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSadGt16xHx8_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xHx8_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);


void Xin26xComputeSad32xHx8_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx8_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xHx8_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSadx5 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    PIXEL    *ref3,
    PIXEL    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xHx5_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx5_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx5_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xHx5_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xHx5_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad128xHx5_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xHx5_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xHx5_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx5_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xHx5_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    UINT8    *ref3,
    UINT8    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSadx3 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xHx3_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx3_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xHx3_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xHx3_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad128xHx3_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad8xHx3_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad16xHx3_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad32xHx3_Neon (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeSad64xHx3_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSadx8 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad8xHx8_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad16xHx8_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSadGt16xHx8_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSadGt16xHx8_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSad8xHx8_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

void Xin26xComputeAvgSadGt8xHx8_Neon (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);


void Xin26xComputeSatdGt4x4 (
    PIXEL     *input,
    intptr_t  inputStride,
    PIXEL     *pred,
    intptr_t  predStride,
    UINT32    width,
    UINT32    height,
    UINT32    *satd);

void Xin26xComputeSatd4x4 (
    PIXEL     *input,
    intptr_t  inputStride,
    PIXEL     *pred,
    intptr_t  predStride,
    UINT32    width,
    UINT32    height,
    UINT32    *satd);

void Xin26xComputeAvgSatdGt4x4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeAvgSatd4x4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatd4x4_SSSE3 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatd (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatdFast (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatd_SSSE3 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatd_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatdFast_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeAvgSatd (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeAvgSatdGt4x4_SSSE3 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeAvgSatdGt8x8_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatdGt4x4_SSSE3 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeSatdGt8x8_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *satd);

void Xin26xComputeHada8x8 (
    PIXEL    *input,
    intptr_t inputStride,
    UINT32   *satd);

void Xin26xComputeHada8x8_SSSE3 (
    PIXEL    *input,
    intptr_t inputStride,
    UINT32   *satd);

void Xin26xComputeSadS16 (
    SINT16   *input,
    intptr_t inputStride,
    SINT16   *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   subShift,
    UINT32   *sad);

void Xin26xComputeSadS168xH_AVX2 (
    SINT16   *input,
    intptr_t inputStride,
    SINT16   *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   subShift,
    UINT32   *sad);

void Xin26xComputeSadS1616xH_AVX2 (
    SINT16   *input,
    intptr_t inputStride,
    SINT16   *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   subShift,
    UINT32   *sad);

void Xin26xComputeVar (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeVar8xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeVar16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeVarGt16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeAvgVar (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *predA,
    PIXEL    *predB,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeAvgVar8xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *predA,
    PIXEL    *predB,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeAvgVar16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *predA,
    PIXEL    *predB,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeAvgVarGt16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *predA,
    PIXEL    *predB,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

void Xin26xComputeWeightSatd (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    SINT32   weightA,
    SINT32   weightB,
    UINT32   *satd);

void Xin26xComputeWeightSad (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    SINT32   weightA,
    SINT32   weightB,
    UINT32   *satd);

void Xin26xComputeWeightSatdGt8x8_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *refA,
    UINT8    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    SINT32   weightA,
    SINT32   weightB,
    UINT32   *satd);

void Xin26xComputeWeightSatdGt4x4_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *refA,
    UINT8    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    SINT32   weightA,
    SINT32   weightB,
    UINT32   *satd);

#endif
