/***************************************************************************//**
 *
 * @file          h265p_trans_utility_sse2.c
 * @brief         av1 transform-related common subroutines (SSE2).
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
#include "emmintrin.h"
#include "xin_typedef.h"

void transpose_16bit_4x4 (
    SINT16 *input,
    SINT16 *output)
{
    __m128i *in;
    __m128i *out;

    in  = (__m128i *)input;
    out = (__m128i *)output;

    // Unpack 16 bit elements. Goes from:
    // in[0]: 00 01 02 03  XX XX XX XX
    // in[1]: 10 11 12 13  XX XX XX XX
    // in[2]: 20 21 22 23  XX XX XX XX
    // in[3]: 30 31 32 33  XX XX XX XX
    // to:
    // a0:    00 10 01 11  02 12 03 13
    // a1:    20 30 21 31  22 32 23 33
    const __m128i a0 = _mm_unpacklo_epi16 (in[0], in[1]);
    const __m128i a1 = _mm_unpacklo_epi16 (in[2], in[3]);

    // Unpack 32 bit elements resulting in:
    // out[0]: 00 10 20 30
    // out[1]: 01 11 21 31
    // out[2]: 02 12 22 32
    // out[3]: 03 13 23 33
    out[0] = _mm_unpacklo_epi32 (a0, a1);
    out[1] = _mm_srli_si128 (out[0], 8);
    out[2] = _mm_unpackhi_epi32 (a0, a1);
    out[3] = _mm_srli_si128 (out[2], 8);

}

void transpose_16bit_8x8 (
    SINT16 *input,
    SINT16 *output)
{
    __m128i *in;
    __m128i *out;

    in = (__m128i *)input;
    out = (__m128i *)output;

    // Unpack 16 bit elements. Goes from:
    // in[0]: 00 01 02 03  04 05 06 07
    // in[1]: 10 11 12 13  14 15 16 17
    // in[2]: 20 21 22 23  24 25 26 27
    // in[3]: 30 31 32 33  34 35 36 37
    // in[4]: 40 41 42 43  44 45 46 47
    // in[5]: 50 51 52 53  54 55 56 57
    // in[6]: 60 61 62 63  64 65 66 67
    // in[7]: 70 71 72 73  74 75 76 77
    // to:
    // a0:    00 10 01 11  02 12 03 13
    // a1:    20 30 21 31  22 32 23 33
    // a2:    40 50 41 51  42 52 43 53
    // a3:    60 70 61 71  62 72 63 73
    // a4:    04 14 05 15  06 16 07 17
    // a5:    24 34 25 35  26 36 27 37
    // a6:    44 54 45 55  46 56 47 57
    // a7:    64 74 65 75  66 76 67 77
    const __m128i a0 = _mm_unpacklo_epi16 (in[0], in[1]);
    const __m128i a1 = _mm_unpacklo_epi16 (in[2], in[3]);
    const __m128i a2 = _mm_unpacklo_epi16 (in[4], in[5]);
    const __m128i a3 = _mm_unpacklo_epi16 (in[6], in[7]);
    const __m128i a4 = _mm_unpackhi_epi16 (in[0], in[1]);
    const __m128i a5 = _mm_unpackhi_epi16 (in[2], in[3]);
    const __m128i a6 = _mm_unpackhi_epi16 (in[4], in[5]);
    const __m128i a7 = _mm_unpackhi_epi16 (in[6], in[7]);

    // Unpack 32 bit elements resulting in:
    // b0: 00 10 20 30  01 11 21 31
    // b1: 40 50 60 70  41 51 61 71
    // b2: 04 14 24 34  05 15 25 35
    // b3: 44 54 64 74  45 55 65 75
    // b4: 02 12 22 32  03 13 23 33
    // b5: 42 52 62 72  43 53 63 73
    // b6: 06 16 26 36  07 17 27 37
    // b7: 46 56 66 76  47 57 67 77
    const __m128i b0 = _mm_unpacklo_epi32 (a0, a1);
    const __m128i b1 = _mm_unpacklo_epi32 (a2, a3);
    const __m128i b2 = _mm_unpacklo_epi32 (a4, a5);
    const __m128i b3 = _mm_unpacklo_epi32 (a6, a7);
    const __m128i b4 = _mm_unpackhi_epi32 (a0, a1);
    const __m128i b5 = _mm_unpackhi_epi32 (a2, a3);
    const __m128i b6 = _mm_unpackhi_epi32 (a4, a5);
    const __m128i b7 = _mm_unpackhi_epi32 (a6, a7);

    // Unpack 64 bit elements resulting in:
    // out[0]: 00 10 20 30  40 50 60 70
    // out[1]: 01 11 21 31  41 51 61 71
    // out[2]: 02 12 22 32  42 52 62 72
    // out[3]: 03 13 23 33  43 53 63 73
    // out[4]: 04 14 24 34  44 54 64 74
    // out[5]: 05 15 25 35  45 55 65 75
    // out[6]: 06 16 26 36  46 56 66 76
    // out[7]: 07 17 27 37  47 57 67 77
    out[0] = _mm_unpacklo_epi64 (b0, b1);
    out[1] = _mm_unpackhi_epi64 (b0, b1);
    out[2] = _mm_unpacklo_epi64 (b4, b5);
    out[3] = _mm_unpackhi_epi64 (b4, b5);
    out[4] = _mm_unpacklo_epi64 (b2, b3);
    out[5] = _mm_unpackhi_epi64 (b2, b3);
    out[6] = _mm_unpacklo_epi64 (b6, b7);
    out[7] = _mm_unpackhi_epi64 (b6, b7);

}

void transpose_16bit_4x8 (
    SINT16 *input,
    SINT16 *output)
{
    __m128i *in;
    __m128i *out;

    in = (__m128i *)input;
    out = (__m128i *)output;

    // Unpack 16 bit elements. Goes from:
    // in[0]: 00 01 02 03  XX XX XX XX
    // in[1]: 10 11 12 13  XX XX XX XX
    // in[2]: 20 21 22 23  XX XX XX XX
    // in[3]: 30 31 32 33  XX XX XX XX
    // in[4]: 40 41 42 43  XX XX XX XX
    // in[5]: 50 51 52 53  XX XX XX XX
    // in[6]: 60 61 62 63  XX XX XX XX
    // in[7]: 70 71 72 73  XX XX XX XX
    // to:
    // a0:    00 10 01 11  02 12 03 13
    // a1:    20 30 21 31  22 32 23 33
    // a2:    40 50 41 51  42 52 43 53
    // a3:    60 70 61 71  62 72 63 73
    const __m128i a0 = _mm_unpacklo_epi16(in[0], in[1]);
    const __m128i a1 = _mm_unpacklo_epi16(in[2], in[3]);
    const __m128i a2 = _mm_unpacklo_epi16(in[4], in[5]);
    const __m128i a3 = _mm_unpacklo_epi16(in[6], in[7]);

    // Unpack 32 bit elements resulting in:
    // b0: 00 10 20 30  01 11 21 31
    // b1: 40 50 60 70  41 51 61 71
    // b2: 02 12 22 32  03 13 23 33
    // b3: 42 52 62 72  43 53 63 73
    const __m128i b0 = _mm_unpacklo_epi32(a0, a1);
    const __m128i b1 = _mm_unpacklo_epi32(a2, a3);
    const __m128i b2 = _mm_unpackhi_epi32(a0, a1);
    const __m128i b3 = _mm_unpackhi_epi32(a2, a3);

    // Unpack 64 bit elements resulting in:
    // out[0]: 00 10 20 30  40 50 60 70
    // out[1]: 01 11 21 31  41 51 61 71
    // out[2]: 02 12 22 32  42 52 62 72
    // out[3]: 03 13 23 33  43 53 63 73
    out[0] = _mm_unpacklo_epi64(b0, b1);
    out[1] = _mm_unpackhi_epi64(b0, b1);
    out[2] = _mm_unpacklo_epi64(b2, b3);
    out[3] = _mm_unpackhi_epi64(b2, b3);

}

void transpose_16bit_8x4 (
    SINT16 *input,
    SINT16 *output)
{
    __m128i *in;
    __m128i *out;

    in = (__m128i *)input;
    out = (__m128i *)output;

    // Unpack 16 bit elements. Goes from:
    // in[0]: 00 01 02 03  04 05 06 07
    // in[1]: 10 11 12 13  14 15 16 17
    // in[2]: 20 21 22 23  24 25 26 27
    // in[3]: 30 31 32 33  34 35 36 37

    // to:
    // a0:    00 10 01 11  02 12 03 13
    // a1:    20 30 21 31  22 32 23 33
    // a4:    04 14 05 15  06 16 07 17
    // a5:    24 34 25 35  26 36 27 37
    const __m128i a0 = _mm_unpacklo_epi16(in[0], in[1]);
    const __m128i a1 = _mm_unpacklo_epi16(in[2], in[3]);
    const __m128i a4 = _mm_unpackhi_epi16(in[0], in[1]);
    const __m128i a5 = _mm_unpackhi_epi16(in[2], in[3]);

    // Unpack 32 bit elements resulting in:
    // b0: 00 10 20 30  01 11 21 31
    // b2: 04 14 24 34  05 15 25 35
    // b4: 02 12 22 32  03 13 23 33
    // b6: 06 16 26 36  07 17 27 37
    const __m128i b0 = _mm_unpacklo_epi32(a0, a1);
    const __m128i b2 = _mm_unpacklo_epi32(a4, a5);
    const __m128i b4 = _mm_unpackhi_epi32(a0, a1);
    const __m128i b6 = _mm_unpackhi_epi32(a4, a5);

    // Unpack 64 bit elements resulting in:
    // out[0]: 00 10 20 30  XX XX XX XX
    // out[1]: 01 11 21 31  XX XX XX XX
    // out[2]: 02 12 22 32  XX XX XX XX
    // out[3]: 03 13 23 33  XX XX XX XX
    // out[4]: 04 14 24 34  XX XX XX XX
    // out[5]: 05 15 25 35  XX XX XX XX
    // out[6]: 06 16 26 36  XX XX XX XX
    // out[7]: 07 17 27 37  XX XX XX XX
    const __m128i zeros = _mm_setzero_si128();
    out[0] = _mm_unpacklo_epi64(b0, zeros);
    out[1] = _mm_unpackhi_epi64(b0, zeros);
    out[2] = _mm_unpacklo_epi64(b4, zeros);
    out[3] = _mm_unpackhi_epi64(b4, zeros);
    out[4] = _mm_unpacklo_epi64(b2, zeros);
    out[5] = _mm_unpackhi_epi64(b2, zeros);
    out[6] = _mm_unpacklo_epi64(b6, zeros);
    out[7] = _mm_unpackhi_epi64(b6, zeros);

}

