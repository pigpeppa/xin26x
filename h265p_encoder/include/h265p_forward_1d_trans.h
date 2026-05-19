/***************************************************************************//**
*
* @file          h265p_forward_1d_trans.h
* @brief         This file declares av1 forward transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_forward_1d_trans_h_
#define _h265p_forward_1d_trans_h_

typedef void (*Xin265pFdct) (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

typedef void (*Xin265pFdctOpt) (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFdct8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFdct16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFdct32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFdct64 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFadst4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFadst8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFadst16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFidentity4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFidentity8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFidentity16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);
    
void Xin265pFidentity32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit);

void Xin265pFdct4x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadst4x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentity4x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct8x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadst8x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentity8x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentity8x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct4x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadst4x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct8x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadst8x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct16x64_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct32_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct64_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct16x32_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentity16x32_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct16x16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadst16x16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentity16x16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdct8x16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadst8x16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentity8x16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFdctDual8x8_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFadstDual8x8_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pFidentityDual8x8_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

#endif