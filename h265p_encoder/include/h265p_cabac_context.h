/***************************************************************************//**
 *
 * @file          h265p_cabac_context.h
 * @brief         This file defines CABAC context related tables.
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
#ifndef _h265p_cabac_context_h_
#define _h265p_cabac_context_h_

#define XIN_CDF_PROB_BITS       15
#define XIN_CDF_PROB_TOP        (1 << XIN_CDF_PROB_BITS)
#define XIN_CDF_INIT_TOP        32768
#define XIN_CDF_SHIFT           (15 - XIN_CDF_PROB_BITS)
#define XIN_ICDF(x)             (XIN_CDF_PROB_TOP - (x))
#define XIN_EC_PROB_SHIFT       6
#define XIN_EC_MIN_PROB         4  // must be <= (1<<EC_PROB_SHIFT)/16

#define XIN_NZ_MAP_CTX_0        XIN_SIG_COEF_CONTEXTS_2D
#define XIN_NZ_MAP_CTX_5        (XIN_NZ_MAP_CTX_0 + 5)
#define XIN_NZ_MAP_CTX_10       (XIN_NZ_MAP_CTX_0 + 10)

// Pad 4 extra columns to remove horizontal availability check.
#define XIN_TX_PAD_HOR_LOG2     2
#define XIN_TX_PAD_HOR          4
#define XIN_TX_PAD_VER          4
// Pad 16 extra bytes to avoid reading overflow in SIMD optimization.
#define XIN_TX_PAD_END          16
#define XIN_TX_PAD_2D           ((32 + XIN_TX_PAD_HOR) * (32 + XIN_TX_PAD_VER) + XIN_TX_PAD_END)
#define XIN_TX_BUF_STRIDE       (32 + XIN_TX_PAD_HOR)

#define XIN_NUM_BASE_LEVEL      2
#define XIN_BR_CDF_SIZE         4
#define XIN_COEF_BASE_RANGE     (4 * (XIN_BR_CDF_SIZE - 1))

#if XIN_CDF_SHIFT == 0

#define XIN_CDF2(a0) XIN_ICDF(a0), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF3(a0, a1) XIN_ICDF(a0), XIN_ICDF(a1), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF4(a0, a1, a2) \
  XIN_ICDF(a0), XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF5(a0, a1, a2, a3) \
  XIN_ICDF(a0)                   \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF6(a0, a1, a2, a3, a4)                        \
  XIN_ICDF(a0)                                              \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF7(a0, a1, a2, a3, a4, a5)                                  \
  XIN_ICDF(a0)                                                            \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5), \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF8(a0, a1, a2, a3, a4, a5, a6)                              \
  XIN_ICDF(a0)                                                            \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5), \
      XIN_ICDF(a6), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF9(a0, a1, a2, a3, a4, a5, a6, a7)                          \
  XIN_ICDF(a0)                                                            \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5), \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF10(a0, a1, a2, a3, a4, a5, a6, a7, a8)                     \
  XIN_ICDF(a0)                                                            \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5), \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF11(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)                 \
  XIN_ICDF(a0)                                                            \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5), \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(a9),             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF12(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)               \
  XIN_ICDF(a0)                                                               \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5),    \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(a9), XIN_ICDF(a10), \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF13(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)          \
  XIN_ICDF(a0)                                                               \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5),    \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(a9), XIN_ICDF(a10), \
      XIN_ICDF(a11), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF14(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)     \
  XIN_ICDF(a0)                                                               \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5),    \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(a9), XIN_ICDF(a10), \
      XIN_ICDF(a11), XIN_ICDF(a12), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF15(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  XIN_ICDF(a0)                                                                \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5),     \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(a9), XIN_ICDF(a10),  \
      XIN_ICDF(a11), XIN_ICDF(a12), XIN_ICDF(a13), XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, \
                  a14)                                                        \
  XIN_ICDF(a0)                                                                \
  , XIN_ICDF(a1), XIN_ICDF(a2), XIN_ICDF(a3), XIN_ICDF(a4), XIN_ICDF(a5),     \
      XIN_ICDF(a6), XIN_ICDF(a7), XIN_ICDF(a8), XIN_ICDF(a9), XIN_ICDF(a10),  \
      XIN_ICDF(a11), XIN_ICDF(a12), XIN_ICDF(a13), XIN_ICDF(a14),             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0

#else
#define XIN_CDF2(a0)                                       \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 2) + \
            ((XIN_CDF_INIT_TOP - 2) >> 1)) /                   \
               ((XIN_CDF_INIT_TOP - 2)) +                      \
           1)                                              \
  , XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF3(a0, a1)                                       \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 3) +     \
            ((XIN_CDF_INIT_TOP - 3) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 3)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 3) + \
                ((XIN_CDF_INIT_TOP - 3) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 3)) +                      \
               2),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF4(a0, a1, a2)                                   \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 4) +     \
            ((XIN_CDF_INIT_TOP - 4) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 4)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 4) + \
                ((XIN_CDF_INIT_TOP - 4) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 4)) +                      \
               2),                                             \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 4) + \
                ((XIN_CDF_INIT_TOP - 4) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 4)) +                      \
               3),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF5(a0, a1, a2, a3)                               \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 5) +     \
            ((XIN_CDF_INIT_TOP - 5) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 5)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 5) + \
                ((XIN_CDF_INIT_TOP - 5) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 5)) +                      \
               2),                                             \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 5) + \
                ((XIN_CDF_INIT_TOP - 5) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 5)) +                      \
               3),                                             \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 5) + \
                ((XIN_CDF_INIT_TOP - 5) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 5)) +                      \
               4),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF6(a0, a1, a2, a3, a4)                           \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 6) +     \
            ((XIN_CDF_INIT_TOP - 6) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 6)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 6) + \
                ((XIN_CDF_INIT_TOP - 6) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 6)) +                      \
               2),                                             \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 6) + \
                ((XIN_CDF_INIT_TOP - 6) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 6)) +                      \
               3),                                             \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 6) + \
                ((XIN_CDF_INIT_TOP - 6) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 6)) +                      \
               4),                                             \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 6) + \
                ((XIN_CDF_INIT_TOP - 6) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 6)) +                      \
               5),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF7(a0, a1, a2, a3, a4, a5)                       \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 7) +     \
            ((XIN_CDF_INIT_TOP - 7) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 7)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 7) + \
                ((XIN_CDF_INIT_TOP - 7) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 7)) +                      \
               2),                                             \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 7) + \
                ((XIN_CDF_INIT_TOP - 7) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 7)) +                      \
               3),                                             \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 7) + \
                ((XIN_CDF_INIT_TOP - 7) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 7)) +                      \
               4),                                             \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 7) + \
                ((XIN_CDF_INIT_TOP - 7) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 7)) +                      \
               5),                                             \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 7) + \
                ((XIN_CDF_INIT_TOP - 7) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 7)) +                      \
               6),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF8(a0, a1, a2, a3, a4, a5, a6)                   \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) +     \
            ((XIN_CDF_INIT_TOP - 8) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 8)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) + \
                ((XIN_CDF_INIT_TOP - 8) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 8)) +                      \
               2),                                             \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) + \
                ((XIN_CDF_INIT_TOP - 8) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 8)) +                      \
               3),                                             \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) + \
                ((XIN_CDF_INIT_TOP - 8) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 8)) +                      \
               4),                                             \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) + \
                ((XIN_CDF_INIT_TOP - 8) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 8)) +                      \
               5),                                             \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) + \
                ((XIN_CDF_INIT_TOP - 8) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 8)) +                      \
               6),                                             \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 8) + \
                ((XIN_CDF_INIT_TOP - 8) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 8)) +                      \
               7),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF9(a0, a1, a2, a3, a4, a5, a6, a7)               \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) +     \
            ((XIN_CDF_INIT_TOP - 9) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 9)) +                          \
           1)                                                  \
  ,                                                            \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               2),                                             \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               3),                                             \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               4),                                             \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               5),                                             \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               6),                                             \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               7),                                             \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 9) + \
                ((XIN_CDF_INIT_TOP - 9) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 9)) +                      \
               8),                                             \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF10(a0, a1, a2, a3, a4, a5, a6, a7, a8)           \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) +     \
            ((XIN_CDF_INIT_TOP - 10) >> 1)) /                       \
               ((XIN_CDF_INIT_TOP - 10)) +                          \
           1)                                                   \
  ,                                                             \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               2),                                              \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               3),                                              \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               4),                                              \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               5),                                              \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               6),                                              \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               7),                                              \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               8),                                              \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 10) + \
                ((XIN_CDF_INIT_TOP - 10) >> 1)) /                   \
                   ((XIN_CDF_INIT_TOP - 10)) +                      \
               9),                                              \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF11(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)        \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +      \
            ((XIN_CDF_INIT_TOP - 11) >> 1)) /                        \
               ((XIN_CDF_INIT_TOP - 11)) +                           \
           1)                                                    \
  ,                                                              \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               2),                                               \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               3),                                               \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               4),                                               \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               5),                                               \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               6),                                               \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               7),                                               \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               8),                                               \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) +  \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               9),                                               \
      XIN_ICDF((((a9)-10) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 11) + \
                ((XIN_CDF_INIT_TOP - 11) >> 1)) /                    \
                   ((XIN_CDF_INIT_TOP - 11)) +                       \
               10),                                              \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF12(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)    \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +       \
            ((XIN_CDF_INIT_TOP - 12) >> 1)) /                         \
               ((XIN_CDF_INIT_TOP - 12)) +                            \
           1)                                                     \
  ,                                                               \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               2),                                                \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               3),                                                \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               4),                                                \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               5),                                                \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               6),                                                \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               7),                                                \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               8),                                                \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +   \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               9),                                                \
      XIN_ICDF((((a9)-10) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) +  \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               10),                                               \
      XIN_ICDF((((a10)-11) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 12) + \
                ((XIN_CDF_INIT_TOP - 12) >> 1)) /                     \
                   ((XIN_CDF_INIT_TOP - 12)) +                        \
               11),                                               \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF13(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +         \
            ((XIN_CDF_INIT_TOP - 13) >> 1)) /                           \
               ((XIN_CDF_INIT_TOP - 13)) +                              \
           1)                                                       \
  ,                                                                 \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               2),                                                  \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               3),                                                  \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               4),                                                  \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               5),                                                  \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               6),                                                  \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               7),                                                  \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               8),                                                  \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +     \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               9),                                                  \
      XIN_ICDF((((a9)-10) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +    \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               10),                                                 \
      XIN_ICDF((((a10)-11) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +   \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               11),                                                 \
      XIN_ICDF((((a11)-12) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 13) +   \
                ((XIN_CDF_INIT_TOP - 13) >> 1)) /                       \
                   ((XIN_CDF_INIT_TOP - 13)) +                          \
               12),                                                 \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF14(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +              \
            ((XIN_CDF_INIT_TOP - 14) >> 1)) /                                \
               ((XIN_CDF_INIT_TOP - 14)) +                                   \
           1)                                                            \
  ,                                                                      \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               2),                                                       \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               3),                                                       \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               4),                                                       \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               5),                                                       \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               6),                                                       \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               7),                                                       \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               8),                                                       \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +          \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               9),                                                       \
      XIN_ICDF((((a9)-10) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +         \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               10),                                                      \
      XIN_ICDF((((a10)-11) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +        \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               11),                                                      \
      XIN_ICDF((((a11)-12) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +        \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               12),                                                      \
      XIN_ICDF((((a12)-13) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 14) +        \
                ((XIN_CDF_INIT_TOP - 14) >> 1)) /                            \
                   ((XIN_CDF_INIT_TOP - 14)) +                               \
               13),                                                      \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF15(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +                   \
            ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                     \
               ((XIN_CDF_INIT_TOP - 15)) +                                        \
           1)                                                                 \
  ,                                                                           \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               2),                                                            \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               3),                                                            \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               4),                                                            \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               5),                                                            \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               6),                                                            \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               7),                                                            \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               8),                                                            \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +               \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               9),                                                            \
      XIN_ICDF((((a9)-10) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +              \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               10),                                                           \
      XIN_ICDF((((a10)-11) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +             \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               11),                                                           \
      XIN_ICDF((((a11)-12) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +             \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               12),                                                           \
      XIN_ICDF((((a12)-13) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +             \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               13),                                                           \
      XIN_ICDF((((a13)-14) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 15) +             \
                ((XIN_CDF_INIT_TOP - 15) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 15)) +                                    \
               14),                                                           \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0
#define XIN_CDF16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, \
                  a14)                                                        \
  XIN_ICDF((((a0)-1) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +                   \
            ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                     \
               ((XIN_CDF_INIT_TOP - 16)) +                                        \
           1)                                                                 \
  ,                                                                           \
      XIN_ICDF((((a1)-2) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               2),                                                            \
      XIN_ICDF((((a2)-3) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               3),                                                            \
      XIN_ICDF((((a3)-4) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               4),                                                            \
      XIN_ICDF((((a4)-5) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               5),                                                            \
      XIN_ICDF((((a5)-6) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               6),                                                            \
      XIN_ICDF((((a6)-7) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               7),                                                            \
      XIN_ICDF((((a7)-8) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               8),                                                            \
      XIN_ICDF((((a8)-9) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +               \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               9),                                                            \
      XIN_ICDF((((a9)-10) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +              \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               10),                                                           \
      XIN_ICDF((((a10)-11) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +             \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               11),                                                           \
      XIN_ICDF((((a11)-12) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +             \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               12),                                                           \
      XIN_ICDF((((a12)-13) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +             \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               13),                                                           \
      XIN_ICDF((((a13)-14) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +             \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               14),                                                           \
      XIN_ICDF((((a14)-15) * ((XIN_CDF_INIT_TOP >> XIN_CDF_SHIFT) - 16) +             \
                ((XIN_CDF_INIT_TOP - 16) >> 1)) /                                 \
                   ((XIN_CDF_INIT_TOP - 16)) +                                    \
               15),                                                           \
      XIN_ICDF(XIN_CDF_PROB_TOP), 0

#endif

#endif

