/***************************************************************************//**
 *
 * @file          xin_avx2_linux_patch.h
 * @brief         This file contains AVX2 patch for Linux builds.
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
#ifndef _xin_avx2_linux_patch_h_
#define _xin_avx2_linux_patch_h_

#ifndef _mm256_storeu2_m128i
#define _mm256_storeu2_m128i(hiaddr, loaddr, a) _mm_storeu_si128((loaddr), _mm256_castsi256_si128(a)); _mm_storeu_si128((hiaddr), _mm256_extractf128_si256(a, 0x1));
#endif

#ifndef _mm256_set_m128i
#define _mm256_set_m128i(v0, v1)  _mm256_insertf128_si256(_mm256_castsi128_si256(v1), (v0), 1)
#endif

#ifndef _mm256_loadu2_m128i
#define _mm256_loadu2_m128i(hiaddr, loaddr) _mm256_set_m128i(_mm_loadu_si128(hiaddr), _mm_loadu_si128(loaddr))
#endif

#ifndef _mm256_setr_m128i
#define _mm256_setr_m128i(v0, v1) _mm256_set_m128i((v1), (v0))
#endif

#endif