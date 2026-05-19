/***************************************************************************//**
*
* @file          h265p_tx_init_level_sse4.c
* @brief         Av1 transform level initialization.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "assert.h"
#include "smmintrin.h"
#include "xin_typedef.h"
#include "basic_macro.h"
#include "h265p_cabac_context.h"

void Xin265pTxInitLevel4xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride)
{
    SINT32  row;
    __m128i zero128;
    __m128i coeff16x4;
    __m128i coeff8x4;
    __m128i abs8x4;
    UINT8   *levelBuf;
    UINT32  writeByte;

    assert (XIN_TX_PAD_HOR == 4);

    (void)width;
    zero128  = _mm_setzero_si128 ();
    levelBuf = level + levelStride * height;

    for (writeByte = 0; writeByte < (XIN_TX_PAD_VER * levelStride + XIN_TX_PAD_END); writeByte += 16)
    {
        _mm_storeu_si128((__m128i *)(levelBuf + writeByte), zero128);
    }

    for (row = 0; row < height; row++)
    {
        coeff16x4 = _mm_loadl_epi64 ((__m128i *)(coeff));
        coeff8x4  = _mm_packs_epi16 (coeff16x4, zero128);
        abs8x4    = _mm_abs_epi8 (coeff8x4);

        _mm_storel_epi64 ((__m128i *)level, abs8x4);

        level += levelStride;
        coeff += coeffStride;
    }

}

void Xin265pTxInitLevel8xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride)
{
    SINT32  row;
    __m128i zero128;
    __m128i coeff16x8;
    __m128i abs16x8;
    __m128i abs8x8;
    UINT8   *levelBuf;
    UINT32  writeByte;

    assert(XIN_TX_PAD_HOR == 4);

    (void)width;
    zero128 = _mm_setzero_si128();
    levelBuf = level + levelStride * height;

    for (writeByte = 0; writeByte < (XIN_TX_PAD_VER * levelStride + XIN_TX_PAD_END); writeByte += 16)
    {
        _mm_storeu_si128((__m128i *)(levelBuf + writeByte), zero128);
    }

    for (row = 0; row < height; row++)
    {
        coeff16x8 = _mm_loadu_si128 ((__m128i *)(coeff));
        abs16x8   = _mm_abs_epi16 (coeff16x8);
        abs8x8    = _mm_packs_epi16 (abs16x8, zero128);

        _mm_storel_epi64 ((__m128i *)level, abs8x8);
        *((SINT32 *)(level + 8)) = 0;

        level += levelStride;
        coeff += coeffStride;
    }

}

void Xin265pTxInitLevel16xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride)
{
    SINT32  row;
    __m128i zero128;
    __m128i coeff0x8, coeff1x8;
    __m128i coeffx16;
    __m128i absx16;
    UINT8   *levelBuf;
    UINT32  writeByte;

    assert(XIN_TX_PAD_HOR == 4);

    (void)width;
    zero128  = _mm_setzero_si128();
    levelBuf = level + levelStride * height;

    for (writeByte = 0; writeByte < (XIN_TX_PAD_VER * levelStride + XIN_TX_PAD_END); writeByte += 16)
    {
        _mm_storeu_si128((__m128i *)(levelBuf + writeByte), zero128);
    }

    for (row = 0; row < height; row++)
    {
        coeff0x8 = _mm_loadu_si128 ((__m128i *)(coeff));
        coeff1x8 = _mm_loadu_si128 ((__m128i *)(coeff + 8));
        coeffx16 = _mm_packs_epi16 (coeff0x8, coeff1x8);
        absx16   = _mm_abs_epi8 (coeffx16);

        _mm_storeu_si128 ((__m128i *)level, absx16);
        *((SINT32 *)(level + 16)) = 0;

        level += levelStride;
        coeff += coeffStride;
    }

}

void Xin265pTxInitLevel32xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride)
{
    SINT32  row;
    __m128i zero128;
    __m128i coeff0x8, coeff1x8, coeff2x8, coeff3x8;
    __m128i coeff0x16, coeff1x16;
    __m128i abs0x16, abs1x16;
    UINT8   *levelBuf;
    UINT32  writeByte;

    assert(XIN_TX_PAD_HOR == 4);

    (void)width;
    zero128 = _mm_setzero_si128();
    levelBuf = level + levelStride * height;

    for (writeByte = 0; writeByte < (XIN_TX_PAD_VER * levelStride + XIN_TX_PAD_END); writeByte += 16)
    {
        _mm_storeu_si128((__m128i *)(levelBuf + writeByte), zero128);
    }

    for (row = 0; row < height; row++)
    {
        coeff0x8 = _mm_loadu_si128 ((__m128i *)(coeff));
        coeff1x8 = _mm_loadu_si128 ((__m128i *)(coeff + 8));
        coeff2x8 = _mm_loadu_si128 ((__m128i *)(coeff + 16));
        coeff3x8 = _mm_loadu_si128 ((__m128i *)(coeff + 24));

        coeff0x16 = _mm_packs_epi16 (coeff0x8, coeff1x8);
        coeff1x16 = _mm_packs_epi16 (coeff2x8, coeff3x8);

        abs0x16 = _mm_abs_epi8 (coeff0x16);
        abs1x16 = _mm_abs_epi8 (coeff1x16);

        _mm_storeu_si128 ((__m128i *)(level),      abs0x16);
        _mm_storeu_si128 ((__m128i *)(level + 16), abs1x16);
        *((SINT32 *)(level + 32)) = 0;

        level += levelStride;
        coeff += coeffStride;
    }

}

void Xin265pTxInitLevel64xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride)
{
    SINT32  row;
    __m128i zero128;
    __m128i coeff0x8, coeff1x8, coeff2x8, coeff3x8;
    __m128i coeff4x8, coeff5x8, coeff6x8, coeff7x8;
    __m128i coeff0x16, coeff1x16;
    __m128i coeff2x16, coeff3x16;
    __m128i abs0x16, abs1x16;
    __m128i abs2x16, abs3x16;
    UINT8   *levelBuf;
    UINT32  writeByte;

    assert(XIN_TX_PAD_HOR == 4);

    (void)width;
    zero128 = _mm_setzero_si128();
    levelBuf = level + levelStride * height;

    for (writeByte = 0; writeByte < (XIN_TX_PAD_VER * levelStride + XIN_TX_PAD_END); writeByte += 16)
    {
        _mm_storeu_si128((__m128i *)(levelBuf + writeByte), zero128);
    }

    for (row = 0; row < height; row++)
    {
        coeff0x8 = _mm_loadu_si128 ((__m128i *)(coeff));
        coeff1x8 = _mm_loadu_si128 ((__m128i *)(coeff + 8));
        coeff2x8 = _mm_loadu_si128 ((__m128i *)(coeff + 16));
        coeff3x8 = _mm_loadu_si128 ((__m128i *)(coeff + 24));
        coeff4x8 = _mm_loadu_si128 ((__m128i *)(coeff + 32));
        coeff5x8 = _mm_loadu_si128 ((__m128i *)(coeff + 40));
        coeff6x8 = _mm_loadu_si128 ((__m128i *)(coeff + 48));
        coeff7x8 = _mm_loadu_si128 ((__m128i *)(coeff + 56));

        coeff0x16 = _mm_packs_epi16 (coeff0x8, coeff1x8);
        coeff1x16 = _mm_packs_epi16 (coeff2x8, coeff3x8);
        coeff2x16 = _mm_packs_epi16 (coeff4x8, coeff5x8);
        coeff3x16 = _mm_packs_epi16 (coeff6x8, coeff7x8);

        abs0x16 = _mm_abs_epi8 (coeff0x16);
        abs1x16 = _mm_abs_epi8 (coeff1x16);
        abs2x16 = _mm_abs_epi8 (coeff2x16);
        abs3x16 = _mm_abs_epi8 (coeff3x16);

        _mm_storeu_si128 ((__m128i *)(level),      abs0x16);
        _mm_storeu_si128 ((__m128i *)(level + 16), abs1x16);
        _mm_storeu_si128 ((__m128i *)(level + 32), abs2x16);
        _mm_storeu_si128 ((__m128i *)(level + 16), abs3x16);
        *((SINT32 *)(level + 64)) = 0;

        level += levelStride;
        coeff += coeffStride;

    }

}



