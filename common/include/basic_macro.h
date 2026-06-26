/***************************************************************************//**
 *
 * @file          basic_macro.h
 * @brief         Basic macro definitions.
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
#ifndef _basic_macro_h_
#define _basic_macro_h_

#include <math.h>

// Absolute value of X
#define  XIN_ABS(X)                     (((X) < 0) ? -(X) : (X))

// Maximum value between A and B
#define  XIN_MAX(A,B)                   (((A) > (B)) ? (A) : (B))

// Minimum value between A and B
#define  XIN_MIN(A,B)                   (((A) < (B)) ? (A) : (B))

// Clamp VAL into LOW and HIGH
#define  XIN_CLIP(VAL,LOW,HIGH)         XIN_MIN (XIN_MAX((VAL),(LOW)), (HIGH))

// Swap A and B
#define XIN_SWAP(X, A, B)               { X tmpSwap = (A); (A) = (B); (B) = tmpSwap; }

// Sign of X
#define XIN_SIGN(X)                     (((X) > 0) - ((X) < 0))

// Signed Round Divsion
#define XIN_SIGNED_ROUND_DIV(X, Y)      (((X) >= 0) ? (((X) + ((Y)/2))/(Y)) : (((X) - ((Y)/2))/(Y)))

// Unsigned Round Divsion
#define XIN_UNSIGNED_ROUND_DIV(X, Y)    (((Y) != 0) ? (((X) + ((Y)/2))/(Y)) : (0))

// Shift down with rounding for use when n >= 0, value >= 0
#define XIN_ROUND_POWER2(X, Y)          (((X) + (((1 << (Y)) >> 1))) >> (Y))

// Malloc check
#define XIN_MALLOC_CHECK(PTR, SIZE)     {(PTR) = malloc (SIZE); if ((PTR) == NULL) {return XIN_FAIL;}}

// Re-Malloc check
#define XIN_REMALLOC_CHECK(PTR, SIZE)   {(PTR) = realloc (PTR, SIZE); if ((PTR) == NULL) {return XIN_FAIL;}}

// LOG2
#define XIN_LOG2F(X)                    (log(X)/log(2))

// Shift right or left depending on sign of n
#define XIN_SIGNED_SHIFT(value, n)      ((n) < 0 ? ((value) << (-(n))) : ((value) >> (n)))

#endif