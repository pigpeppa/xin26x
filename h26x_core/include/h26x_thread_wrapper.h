/***************************************************************************//**
 *
 * @file          h26x_thread_wrapper.h
 * @brief         Low-level thread function abstraction.
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
#ifndef _h26x_thread_wrapper_h_
#define _h26x_thread_wrapper_h_

XIN_HANDLE Xin26xConstructThread(
    void *threadFunc(void *),
    void *threadCtx);

SINT32 Xin26xDestructThread (
    XIN_HANDLE threadHandle);

XIN_HANDLE Xin26xConstructSem (
    UINT32     initialCount,
    UINT32     maxCount);

SINT32 Xin26xPostSem (
    XIN_HANDLE semHandle);

SINT32 Xin26xBlockOnSem (
    XIN_HANDLE semHandle);

SINT32 Xin26xDestructSem (
    XIN_HANDLE semHandle);

XIN_HANDLE Xin26xConstructMutex ( );

SINT32 Xin26xPostMutex(
    XIN_HANDLE mutexHandle);

SINT32 Xin26xBlockOnMutex(
    XIN_HANDLE mutexHandle);

SINT32 Xin26xDestructMutex(
    XIN_HANDLE mutexHandle);

XIN_HANDLE Xin26xConstructCond ( );

SINT32 Xin26xDestructCond (
    XIN_HANDLE condHandle);

SINT32 Xin26xWaitCond (
    XIN_HANDLE condHandle,
    XIN_HANDLE mutexHandle);

SINT32 Xin26xSignalCond (
    XIN_HANDLE condHandle);

SINT32 Xin26xBroadcastCond (
    XIN_HANDLE condHandle);

SINT32 Xin26xDestructCond (
    XIN_HANDLE condHandle);

XIN_HANDLE Xin26xConstructLock ( );

SINT32 Xin26xGetLock (
    XIN_HANDLE lockHandle);

SINT32 Xin26xReleaseLock (
    XIN_HANDLE mutexHandle);

SINT32 Xin26xDestructLock (
    XIN_HANDLE lockHandle);

#endif