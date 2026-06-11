/***************************************************************************//**
 *
 * @file          h26x_thread_struct.h
 * @brief         This file contains thread-related structures.
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
#ifndef _h26x_thread_struct_h_
#define _h26x_thread_struct_h_

#include "h26x_definition.h"

typedef struct xin_job_desc
{
    XIN_HANDLE          lock;
    UINT32              state;
    SINT32              nDepends;
    struct xin_job_desc **rDepends;
    SINT32              rDependsCount;
    SINT32              rDependsSize;
    void                (*func)(void *);
    void                *arg;
    struct xin_job_desc *next;
} xin_job_desc;

typedef struct xin_mem_list
{
    void        *list[XIN_MAX_THREAD_POOL_NUM];
    int         maxListSize;
    int         listSize;
    XIN_HANDLE  lock;
}xin_mem_list;

typedef struct xin_thread_queue
{
    XIN_HANDLE   lock;
    XIN_HANDLE   jobAvailable;
    XIN_HANDLE   jobDone;
    XIN_HANDLE   threads[XIN_MAX_THREAD_POOL_NUM];
    SINT32       threadCount;
    SINT32       threadRunningCount;
    BOOL         stop;
    xin_job_desc *first;
    xin_job_desc *last;
} xin_thread_queue;

#endif