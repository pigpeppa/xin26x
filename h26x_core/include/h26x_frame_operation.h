/***************************************************************************//**
 *
 * @file          h26x_frame_operation.h
 * @brief         This file defines subroutines for push and pop frame from frame queue.
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
#ifndef _h26x_frame_operation_h_
#define _h26x_frame_operation_h_

void Xin26xFramePush (
    void    **frameQueue,
    UINT32  *frameQueueSize,
    void    *frameToPush);

void Xin26xFramePop (
    void    **frameQueue,
    UINT32  *frameQueueSize,
    void    **frameToPop);

#endif

