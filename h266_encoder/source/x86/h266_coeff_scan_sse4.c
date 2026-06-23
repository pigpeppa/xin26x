/***************************************************************************//**
 *
 * @file          h266_coeff_scan_sse4.c
 * @brief         Rearrange quantized coefficient layout for entropy write (SSE4).
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
#include "xin_typedef.h"
#include "h266_constant.h"
#include "h266_definition.h"
#include "xin_video_common.h"
#include "h26x_trans_context.h"
#include "h266_trans_unit_struct.h"
#include "h266_scan_order.h"
#include "smmintrin.h"

static const UINT8 coeffShuf[16] = 
{
    0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15
};

static const UINT8 diagonalScanOrder4x4[16] =
{
    0x0000, 0x0004, 0x0001, 0x0008,
    0x0005, 0x0002, 0x000C, 0x0009,
    0x0006, 0x0003, 0x000D, 0x000A,
    0x0007, 0x000E, 0x000B, 0x000F
};

void Xin266CoeffScan4x4_SSE4 (
    SINT16       *coefBuffer,
    intptr_t     coefStride,
    SINT32       cgWidth,
    SINT32       cgHeight,
    xin_scan_pos *scanOrder,
    UINT16       *coeffSign,
    UINT16       *gt0Buf)
{
    SINT32  gt0BitMap;
    SINT32  coefSign;
    __m128i allZero;
    __m128i levl0x4;
    __m128i levl1x4;
    __m128i levl2x4;
    __m128i levl3x4;
    __m128i levl0x8;
    __m128i levl1x8;
    __m128i levlLx16;
    __m128i levlHx16;
    __m128i levelx16;
    __m128i scanShuf;
    __m128i coefShuf;
    __m128i abs0x8;
    __m128i abs1x8;
    __m128i absx16;
    __m128i gt0x16;

    (void)cgWidth;
    (void)cgHeight;
	(void)scanOrder;
    allZero  = _mm_setzero_si128 ();
    scanShuf = _mm_lddqu_si128 ((__m128i *)(diagonalScanOrder4x4));
    coefShuf = _mm_lddqu_si128 ((__m128i *)(coeffShuf));
    levl0x4  = _mm_loadl_epi64 ((__m128i *) (coefBuffer));
    levl1x4  = _mm_loadl_epi64 ((__m128i *) (coefBuffer + coefStride*1));
    levl2x4  = _mm_loadl_epi64 ((__m128i *) (coefBuffer + coefStride*2));
    levl3x4  = _mm_loadl_epi64 ((__m128i *) (coefBuffer + coefStride*3));

    levl0x8 = _mm_unpacklo_epi64 (levl0x4, levl1x4);
    levl1x8 = _mm_unpacklo_epi64 (levl2x4, levl3x4);

    levl0x8 = _mm_shuffle_epi8 (levl0x8, coefShuf);
    levl1x8 = _mm_shuffle_epi8 (levl1x8, coefShuf);

    levlLx16 = _mm_unpacklo_epi64 (levl0x8, levl1x8);
    levlHx16 = _mm_unpackhi_epi64 (levl0x8, levl1x8);

    levlLx16 = _mm_shuffle_epi8 (levlLx16, scanShuf);
    levlHx16 = _mm_shuffle_epi8 (levlHx16, scanShuf);

    levl0x8  = _mm_unpacklo_epi8 (levlLx16, levlHx16);
    levl1x8  = _mm_unpackhi_epi8 (levlLx16, levlHx16);
    
    abs0x8   = _mm_abs_epi16 (levl0x8);
    abs1x8   = _mm_abs_epi16 (levl1x8);

    levelx16 = _mm_packs_epi16 (levl0x8, levl1x8);
    coefSign = _mm_movemask_epi8 (levelx16);
    
    absx16   = _mm_packs_epi16 (abs0x8, abs1x8);
    gt0x16   = _mm_cmpgt_epi8 (absx16, allZero);
    
    gt0BitMap  = _mm_movemask_epi8 (gt0x16);

    *gt0Buf    = (UINT16)gt0BitMap;
    *coeffSign = (UINT16)coefSign;

}

