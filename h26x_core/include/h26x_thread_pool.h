/***************************************************************************//**
*
* @file          h26x_thread_pool.h
* @brief         This file declare thread pool implementation subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h26x_thread_pool_h_
#define _h26x_thread_pool_h_

SINT32 Xin26xMemListCreate (
    xin_mem_list *memList,
    void         **memBuf,
    UINT32       memSize);

SINT32 Xin26xMemListCreate (
    xin_mem_list *memList,
    void         **memBuf,
    UINT32       memSize);

void Xin26xMemListDelete (
    xin_mem_list *memList);

void* Xin26xMemListPop (
    xin_mem_list *memList);

void Xin26xMemListPush (
    xin_mem_list *memList,
    void         *mem);

SINT32 Xin26xThreadQueueCreate (
    xin_thread_queue *threadQueue,
    UINT32           threadNum);

void Xin26xThreadPushJob (
    xin_thread_queue *threadQueue,
    xin_job_desc     *job);

xin_job_desc* Xin26xThreadPopJob (
    xin_thread_queue *threadQueue);

void Xin26xThreadSubmit (
    xin_thread_queue *threadQueue,
    xin_job_desc     *job);

void Xin26xThreadJobDepAdd (
    xin_job_desc *jobExe,
    xin_job_desc *jobDep);

void Xin26xJobInit (
    xin_job_desc *job,
    void         (*func)(void *),
    void         *arg);

void Xin26xThreadWaitFor (
    xin_thread_queue *threadQueue,  
    xin_job_desc     *job);

void Xin26xThreadQueueDelete (
    xin_thread_queue *threadQueue);

void Xin26xTheadQueueStop (
    xin_thread_queue *threadQueue);

SINT32 Xin26xJobCreate (
    xin_job_desc **jobDescQueue,
    UINT32       jobQueueSize,
    UINT32       rDependsSize);

void Xin26xJobDelete (
    xin_job_desc *jobDescQueue,
    UINT32       jobQueueSize);

#endif