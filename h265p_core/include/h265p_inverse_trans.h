/***************************************************************************//**
*
* @file          h265p_inverse_trans.c
* @brief         inverse transform subroutines declaration and definitions.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_inverse_trans_h_
#define _h265p_inverse_trans_h_

#include "h265p_trans_context.h"

extern const SINT8 invTxShift[XIN_TX_SIZE_NUM][2];
extern const SINT8 invCosBitHor[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1];
extern const SINT8 invCosBitVer[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1];

void Xin265pIDctWxH (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);


void Xin265pIDctGt8xGt8_AVX2 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pIDct4x4_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pIDct4x8_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pIDct8x4_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

void Xin265pIDct8x8_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);


#endif
