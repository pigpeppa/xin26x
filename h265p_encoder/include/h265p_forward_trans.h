/***************************************************************************//**
*
* @file          h265p_forward_trans.h
* @brief         av1 forward transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_forward_trans_h_
#define _h265p_forward_trans_h_

extern const SINT8 fwdCosBitHor[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1];
extern const SINT8 fwdCosBitVer[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1];
extern const SINT8 fwdTxShift[XIN_TX_SIZE_NUM][3];

void Xin265pFDctWxH (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct4x4_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct8x4_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct4x8_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct8x8_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct64x64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct32x32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct16x16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct32x16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct16x32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct64x32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct32x64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct8x16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pFDct16x8_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

#endif

