/***************************************************************************//**
 *
 * @file          h26x_mctf_struct.h
 * @brief         This file contains h26x temporal filter process structures.
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
#ifndef _h26x_mctf_struct_h_
#define _h26x_mctf_struct_h_

#include "h26x_mctf_func_struct.h"
#include "h26x_thread_struct.h"
#include "h26x_picture_struct.h"

typedef struct xin_mctf_struct xin_mctf_struct;

typedef struct xin_mctf_picture
{
    xin_mctf_struct  *mctfSet;

    xin_job_desc     *jobSubSample;

    PIXEL            *input[3];
    intptr_t         inputStride[2];

    SINT32           inputWidth;
    SINT32           inputHeight;

    PIXEL            *inputSub2Buf;
    PIXEL            *inputSub2;
    intptr_t         inputSub2Stride;

    SINT32           inputSub2Width;
    SINT32           inputSub2Height;

    PIXEL            *inputSub4Buf;
    PIXEL            *inputSub4;
    intptr_t         inputSub4Stride;

    SINT32           inputSub4Width;
    SINT32           inputSub4Height;

    SINT32           marginX;
    SINT32           marginY;
    SINT32           inputNumber;

} xin_mctf_picture;

typedef struct xin_mctf_me
{
    xin_mctf_struct  *mctfSet;
    xin_mctf_picture *mctfPicture;

    UINT32           unitX;
    UINT32           unitY;
    UINT32           sectionIdx;
    UINT32           sectionUnitY;
    UINT32           sectionHgtInUnit;

    intptr_t         inputStride;
    PIXEL            *input;
    intptr_t         refStride;
    PIXEL            *ref;
    xin_mv32_u       *currMv;
    intptr_t         currMvStride;
    xin_mv32_u       *lowerMv;
    intptr_t         lowerMvStride;
    SINT32           *currError;
    intptr_t         errorStride;
    SINT32           *variance;
    intptr_t         varianceStride;
    SINT32           factor;

    UINT32           searchLevel;
    UINT32           blockSize;
    UINT32           refIdx;
    SINT32           frameWidth;
    SINT32           frameHeight;
} xin_mctf_me;

typedef struct xin_mctf_mc
{
    xin_mctf_struct  *mctfSet;
    xin_mctf_picture *mctfPicture;
    UINT32           unitX;
    UINT32           unitY;
    UINT32           sectionIdx;
    UINT32           sectionUnitY;
    UINT32           sectionHgtInUnit;

    intptr_t         inputStride;
    PIXEL            *input;
    intptr_t         refStride;
    PIXEL            *ref;
    xin_mv32_u       *currMv;
    intptr_t         currMvStride;
    xin_mv32_u       *lowerMv;
    intptr_t         lowerMvStride;
    SINT32           factor;
    PIXEL            *filter[XIN_TF_MAX_REF_NUM][3];
} xin_mctf_mc;

typedef struct xin_mctf_struct
{
    xin_mctf_func       *funcSet;
    xin_thread_queue    *threadQueue;
    xin_mctf_picture    *pictureWrite;
    xin_mctf_picture    *pictureRead[XIN_TF_MAX_REF_NUM];
    xin_job_desc        *jobMctfVar;
    xin_job_desc        *jobMctfMc[XIN_TF_MAX_SECTION_NUM];
    xin_job_desc        *jobMctfMe[XIN_TF_MAX_REF_NUM][XIN_TF_ME_LEVEL_NUM][XIN_TF_MAX_SECTION_NUM];
    xin_mctf_me         *mctfMe[XIN_TF_MAX_REF_NUM][XIN_TF_ME_LEVEL_NUM][XIN_TF_MAX_SECTION_NUM];
    xin_mctf_mc         *mctfMc[XIN_TF_MAX_SECTION_NUM];
    xin_job_desc        *jobMctfBim;
    UINT32              validRefNum;
    double              strength;

    SINT32              mctfUnitSize;

    BOOL                isBusy;

    UINT32              sectionNum;
    UINT32              mctfMode;
    UINT32              mctfRefNum;
    BOOL                bimEnabled;
    SINT32              ctuSize;

    SINT32              inputWidth;
    SINT32              inputHeight;

    xin_mv32_u          *l0Mv[XIN_TF_MAX_REF_NUM];
    intptr_t            l0MvStride;

    xin_mv32_u          *l1Mv[XIN_TF_MAX_REF_NUM];
    intptr_t            l1MvStride;

    xin_mv32_u          *l2Mv[XIN_TF_MAX_REF_NUM];
    intptr_t            l2MvStride;

    xin_mv32_u          *mv[XIN_TF_MAX_REF_NUM];
    intptr_t            mvStride;

    SINT32              *error[XIN_TF_MAX_REF_NUM];
    intptr_t            errorStride;

    SINT32              *variance;
    intptr_t            varianceStride;

    double              *unitDqpMap;
    intptr_t            unitDqpMapStride;

    double              *ctuDqpMap;
    intptr_t            ctuDqpMapStride;

} xin_mctf_struct;

#endif

