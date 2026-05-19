/***************************************************************************//**
*
* @file          h265p_inverse_1d_trans.h
* @brief         This file declare av1 invserse transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_inverse_1d_trans_h_
#define _h265p_inverse_1d_trans_h_

typedef void (*Xin265pIdct) (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

typedef void (*Xin265pIdctOpt) (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pIdct4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIdct8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIdct16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIdct32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIdct64 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIadst4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIadst8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIadst16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIadst32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIidentity4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIidentity8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIidentity16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIidentity32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIidentity64 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit);

void Xin265pIdct16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pIadst16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pIdct32_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit);

void Xin265pIdct64_AVX2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIdct4P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIadst4P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIdct4P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIadst4P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIidentity4_SSSE3 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIdct8P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIadst8P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIidentity8_SSSE3 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIdct8P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIadst8P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIdct16_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);

void Xin265pIadst16_SSSE3 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit);


#endif

