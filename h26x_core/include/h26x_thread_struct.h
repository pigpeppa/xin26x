/***************************************************************************//**
*
* @file          h26x_thread_struct.h
* @brief         This file contains thread related structure.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
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