/***************************************************************************//**
 *
 * @file          h26x_look_ahead_struct.h
 * @brief         This file contains h26x look ahead structures.
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
#ifndef _h26x_la_struct_h_
#define _h26x_la_struct_h_

#include "h26x_look_ahead_func_struct.h"

typedef struct xin_la_section xin_la_section;

typedef struct xin_la_struct
{
    xin_la_func       *funcSet;
    xin_la_section    *laSection[4];
    xin_job_desc      *jobSubSample;
    xin_job_desc      *jobLookahead[4];
    xin_job_desc      *jobPostInit;
    xin_input_picture *pictureRead[XIN_LIST_NUM];
    xin_input_picture *pictureWrite;
    xin_thread_queue  *threadQueue;

    BOOL              laSatdMd;
    BOOL              laSubMe;
    
    UINT32            sectionNum;
    UINT32            totalUnit;
    UINT32            wdtInUnit;
    UINT32            hgtInUnit;
    UINT32            frameWidth;
    UINT32            frameHeight;
    intptr_t          frameStride;

    UINT32            laUnitSize;
    UINT32            lgUnitSize;
    UINT32            lambda;
    BOOL              isBusy;

}xin_la_struct;

typedef struct xin_la_section
{
    xin_la_struct     *laSet;
    xin_me_struct     *meSet;
    UINT32            unitX;
    UINT32            unitY;
    UINT32            sectionIdx;
    UINT32            sectionUnitY;
    UINT32            sectionHgtInUnit;
}xin_la_section;

#endif

