/***************************************************************************//**
*
* @file          h265_block_utility.h
* @brief         This file block define block basic function.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h26x_block_utility_h_
#define _h26x_block_utility_h_

void Xin26xBlockCopy (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy4xH_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy8xH_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy16xH_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy32xH_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy64xH_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy128xH_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy128xH_AVX2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);
    
void Xin26xBlockCopy64xH_AVX2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy32xH_AVX2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy4xH_Neon (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy8xH_Neon (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy16xH_Neon (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy32xH_Neon (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopy64xH_Neon (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockCopyCoeff (
    COEFF    *src,
    intptr_t srcStride,
    COEFF    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockSubForDct (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    COEFF    *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockSub (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockSub8xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockSub16xH_SSE2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockSub32xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockSub64xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

void Xin26xComputeVar8x8 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT32   *sse,
    SINT32   *var);

void Xin26xComputeVar8x8_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *recon,
    intptr_t reconStride,
    SINT32   *sse,
    SINT32   *var);

void Xin26xBlockRecon (
    COEFF    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockRecon8xH_SSE2 (
    SINT16   *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockRecon16xH_SSE2 (
    SINT16   *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockRecon32xH_AVX2 (
    SINT16   *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockRecon64xH_AVX2 (
    SINT16   *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

void Xin26xBlockAverage (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockAverage8xH_SSE2 (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockAverage16xH_SSE2 (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockAverage32xH_AVX2 (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockAverage64xH_AVX2 (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockPad (
    PIXEL    *buf,
    intptr_t bufStride,
    UINT32   width,
    UINT32   height,
    UINT32   padWidth,
    UINT32   padHeight);

void Xin26xBlockPad2 (
    PIXEL    *buf,
    intptr_t bufStride,
    UINT32   width,
    UINT32   height,
    UINT32   padWidth,
    UINT32   padHeight);

void Xin26xBlockPad1 (
    PIXEL    *buf,
    intptr_t bufStride,
    UINT32   width,
    UINT32   height,
    UINT32   padWidth,
    UINT32   padHeight);

#endif