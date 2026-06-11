/***************************************************************************//**
 *
 * @file          h266_alf_context.h
 * @brief         This file contains ALF-related definitions.
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
#ifndef _h266_alf_context_h_
#define _h266_alf_context_h_

#define XIN_ALF_CLS_BLK_SIZE            32  //non-normative, local buffer size
#define XIN_ALF_MAX_CLS_NUM             25
#define XIN_ALF_CLIP_NUM                4
#define XIN_ALF_MAX_LUMA_COEF_NUM       13
#define XIN_ALF_MAX_CHROMA_ALT_NUM      8
#define XIN_ALF_MAX_CHROMA_COEF_NUM     7
#define XIN_ALF_FIXED_FLT_SET_NUM       16
#define XIN_ALF_FIXED_FILTER_NUM        64
#define XIN_ALF_CTB_MAX_APS_NUM         8
#define XIN_ALF_MAX_FILTER_LENGTH       7
#define XIN_ALF_FRAME_PADDING           ((XIN_ALF_MAX_FILTER_LENGTH + 1) >> 1)
#define XIN_ALF_UNUSED_CLASS            255
#define XIN_ALF_UNUSED_TRANSPOS         255
#define XIN_ALF_LUMA_FILTER_LENGTH      7
#define XIN_ALF_CHROMA_FILTER_LENGTH    5
#define XIN_CC_ALF_MAX_COEFF_NUM        8
#define XIN_CC_ALF_MAX_FILTER_NUM       4
#define CCALF_BITS_PER_COEFF_LEVEL      3

typedef struct xin_alf_class
{
    UINT8 classIdx;
    UINT8 transposeIdx;
} xin_alf_class;

typedef struct xin_alf_filter
{
    UINT32  filterType;
    SINT32  filterLength;
    SINT32  numCoeff;
    SINT32  filterSize;
} xin_alf_filter;

typedef enum xin_alf_dir
{
    XIN_ALF_HOR,
    XIN_ALF_VER,
    XIN_ALF_DIAG0,
    XIN_ALF_DIAG1,
    XIN_ALF_DIR_NUM
} xin_alf_dir;

typedef enum xin_alf_shape
{
    XIN_ALF_FILTER_5,
    XIN_ALF_FILTER_7,
    XIN_ALF_CC_FILTER,
    XIN_ALF_FILTER_NUM
} xin_alf_shape;

typedef struct xin_alf_cov
{
    SINT32  numCoeff;
    SINT32  numBins;
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    FLOAT32 pixAcc;
} xin_alf_cov;

#endif

