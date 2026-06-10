/***************************************************************************//**
 *
 * @file          h265p_quant_inv_quant.c
 * @brief         av1 forward quantization and inverse quantization.
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
#include "basic_macro.h"
#include "h26x_definition.h"
#include "h265p_definition.h"

static const SINT16 dcQuantQtx[XIN_QP_NUM] =
{
    4,    8,    8,    9,    10,  11,  12,  12,  13,  14,  15,   16,   17,   18,
    19,   19,   20,   21,   22,  23,  24,  25,  26,  26,  27,   28,   29,   30,
    31,   32,   32,   33,   34,  35,  36,  37,  38,  38,  39,   40,   41,   42,
    43,   43,   44,   45,   46,  47,  48,  48,  49,  50,  51,   52,   53,   53,
    54,   55,   56,   57,   57,  58,  59,  60,  61,  62,  62,   63,   64,   65,
    66,   66,   67,   68,   69,  70,  70,  71,  72,  73,  74,   74,   75,   76,
    77,   78,   78,   79,   80,  81,  81,  82,  83,  84,  85,   85,   87,   88,
    90,   92,   93,   95,   96,  98,  99,  101, 102, 104, 105,  107,  108,  110,
    111,  113,  114,  116,  117, 118, 120, 121, 123, 125, 127,  129,  131,  134,
    136,  138,  140,  142,  144, 146, 148, 150, 152, 154, 156,  158,  161,  164,
    166,  169,  172,  174,  177, 180, 182, 185, 187, 190, 192,  195,  199,  202,
    205,  208,  211,  214,  217, 220, 223, 226, 230, 233, 237,  240,  243,  247,
    250,  253,  257,  261,  265, 269, 272, 276, 280, 284, 288,  292,  296,  300,
    304,  309,  313,  317,  322, 326, 330, 335, 340, 344, 349,  354,  359,  364,
    369,  374,  379,  384,  389, 395, 400, 406, 411, 417, 423,  429,  435,  441,
    447,  454,  461,  467,  475, 482, 489, 497, 505, 513, 522,  530,  539,  549,
    559,  569,  579,  590,  602, 614, 626, 640, 654, 668, 684,  700,  717,  736,
    755,  775,  796,  819,  843, 869, 896, 925, 955, 988, 1022, 1058, 1098, 1139,
    1184, 1232, 1282, 1336,
};

static const SINT16 acQuantQtx[XIN_QP_NUM] =
{
    4,    8,    9,    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
    33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
    46,   47,   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,
    59,   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,
    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
    98,   99,   100,  101,  102,  104,  106,  108,  110,  112,  114,  116,  118,
    120,  122,  124,  126,  128,  130,  132,  134,  136,  138,  140,  142,  144,
    146,  148,  150,  152,  155,  158,  161,  164,  167,  170,  173,  176,  179,
    182,  185,  188,  191,  194,  197,  200,  203,  207,  211,  215,  219,  223,
    227,  231,  235,  239,  243,  247,  251,  255,  260,  265,  270,  275,  280,
    285,  290,  295,  300,  305,  311,  317,  323,  329,  335,  341,  347,  353,
    359,  366,  373,  380,  387,  394,  401,  408,  416,  424,  432,  440,  448,
    456,  465,  474,  483,  492,  501,  510,  520,  530,  540,  550,  560,  571,
    582,  593,  604,  615,  627,  639,  651,  663,  676,  689,  702,  715,  729,
    743,  757,  771,  786,  801,  816,  832,  848,  864,  881,  898,  915,  933,
    951,  969,  988,  1007, 1026, 1046, 1066, 1087, 1108, 1129, 1151, 1173, 1196,
    1219, 1243, 1267, 1292, 1317, 1343, 1369, 1396, 1423, 1451, 1479, 1508, 1537,
    1567, 1597, 1628, 1660, 1692, 1725, 1759, 1793, 1828,
};

void Xin265pQuantInvQuantB (
    COEFF    *qCoeff,
    SINT32   *tCoeff,
    SINT32   *rCoeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    SINT32   logScale,
    SINT32   *qAdd,
    SINT32   *qMult,
    SINT32   *qzBin,
    SINT32   *qShift,
    SINT32   *iqMult,
    UINT32   *nonZeroCount)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  tCoef;
    SINT32  qCoef;
    SINT32  rCoef;
    SINT32  absCoef;
    BOOL    isAc;
    UINT32  nzCount;
    SINT32  zBin[2];
    SINT32  add[2];

    nzCount = 0;
    zBin[0] =  XIN_ROUND_POWER2 (qzBin[0], logScale);
    zBin[1] =  XIN_ROUND_POWER2 (qzBin[1], logScale);
    add[0]  =  XIN_ROUND_POWER2 (qAdd[0],  logScale);
    add[1]  =  XIN_ROUND_POWER2 (qAdd[1],  logScale);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            isAc    = rowIdx || colIdx;
            tCoef   = tCoeff[rowIdx*coeffStride + colIdx];
            absCoef = XIN_ABS (tCoef);
            qCoef   = 0;
            rCoef   = 0;

            if (absCoef >= zBin[isAc])
            {
                absCoef = XIN_CLIP (absCoef + add[isAc], XIN_MIN_S16, XIN_MAX_S16);
                absCoef = ((absCoef * qMult[isAc]) >> 16) + absCoef;
                absCoef = (absCoef * qShift[isAc]) >> (16 - logScale);
                qCoef   = (tCoef > 0) ? absCoef : -absCoef;
                rCoef   = (absCoef*iqMult[isAc]) >> logScale;
                rCoef   = (tCoef > 0) ? rCoef : -rCoef;
            }

            nzCount += (qCoef != 0);

            qCoeff[rowIdx*coeffStride + colIdx] = (COEFF)qCoef;
            rCoeff[rowIdx*coeffStride + colIdx] = rCoef;

        }

    }

    *nonZeroCount = nzCount;

}

void Xin265pQuantInvQuantFp (
    COEFF    *qCoeff,
    SINT32   *tCoeff,
    SINT32   *rCoeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    SINT32   logScale,
    SINT32   *qAdd,
    SINT32   *qMult,
    SINT32   *qShift,
    SINT32   *iqMult,
    UINT32   *nonZeroCount)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  tCoef;
    SINT32  qCoef;
    SINT32  rCoef;
    SINT32  absCoef;
    BOOL    isAc;
    UINT32  nzCount;

    (void)qShift;

    nzCount = 0;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            isAc    = rowIdx || colIdx;
            tCoef   = tCoeff[rowIdx*coeffStride + colIdx];
            absCoef = XIN_ABS (tCoef);
            absCoef = XIN_CLIP (absCoef + XIN_ROUND_POWER2(qAdd[isAc], logScale), XIN_MIN_S16, XIN_MAX_S16);
            absCoef = ((absCoef * qMult[isAc]) >> (16 - logScale));
            qCoef   = (tCoef > 0) ? absCoef : -absCoef;
            rCoef   = (absCoef*iqMult[isAc]) >> logScale;
            rCoef   = (tCoef > 0) ? rCoef : -rCoef;

            nzCount += (qCoef != 0);

            qCoeff[rowIdx*coeffStride + colIdx] = (COEFF)qCoef;
            rCoeff[rowIdx*coeffStride + colIdx] = rCoef;
        }

    }

    *nonZeroCount = nzCount;

}


