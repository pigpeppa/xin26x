/***************************************************************************//**
 *
 * @file          xin_video_common.h
 * @brief         This file contains basic video input and output interface
 *                data structure definitions.
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
#ifndef _xin_video_common_h_
#define _xin_video_common_h_

typedef struct xin_lb_struct
{
    void   *base;
    UINT32 size;
    UINT32 index;
} xin_lb_struct;

typedef enum xin_yuv_plane
{
    PLANE_LUMA     = 0,
    PLANE_CHROMA   = 1,
    PLANE_CHROMA_U = 1,
    PLANE_CHROMA_V = 2,
    PLANE_TYPE     = 2,
    PLANE_NUM      = 3
}xin_yuv_plane;

typedef struct xin_frame_struct
{
    PIXEL      *yuvBuf[PLANE_NUM];

    UINT32     lumaWidth;
    UINT32     lumaHeight;
    
    intptr_t   lumaStride;
    intptr_t   chromaStride;

} xin_frame_struct;

typedef struct xin_mv_s
{
    SINT16  mvX;
    SINT16  mvY;
} xin_mv_s;

typedef union xin_mv_u
{
   SINT32   s32x1;
   SINT16   s16x2[2];
   xin_mv_s mv;
} xin_mv_u;

typedef struct xin_mv32_s
{
    SINT32  mv32X;
    SINT32  mv32Y;
} xin_mv32_s;

typedef union xin_mv32_u
{
   SINT64     s64x1;
   SINT32     s32x2[2];
   xin_mv32_s mv;
} xin_mv32_u;

#endif

