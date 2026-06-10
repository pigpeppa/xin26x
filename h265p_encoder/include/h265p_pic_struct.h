/***************************************************************************//**
 *
 * @file          h265p_pic_struct.h
 * @brief         This file contains av1 picture level structure.
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
#ifndef _h265p_pic_struct_h_
#define _h265p_pic_struct_h_

#include "h265p_seq_struct.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_super_block_struct.h"
#include "h265p_cabac_struct.h"

typedef struct xin_sec_struct xin_sec_struct;
typedef struct xin_rc_struct xin_rc_struct;

typedef struct xin_lf_thresh
{
    UINT8 mblim[XIN_SIMD_WIDTH];
    UINT8 lim[XIN_SIMD_WIDTH];
    UINT8 hevThr[XIN_SIMD_WIDTH];
} xin_lf_thresh;

typedef struct xin_lf_info
{
    xin_lf_thresh lfThr[XIN_MAX_LOOP_FILTER + 1];
    UINT8         lvl[PLANE_NUM][2];
} xin_lf_info;

typedef struct xin_pic_struct
{
    xin_seq_struct    *seqSet;
    xin_func_struct   *funcSet;

    BOOL              isFree;

    xin_ref_picture   *pictureRef[XIN_MAX_DPB_FRAMES];
    UINT32            validRefFrame;
    xin_ref_picture   *pictureRead[2][XIN_MAX_REF_FRAMES];
    xin_ref_picture   *pictureWrite;
    xin_input_picture *inputPicture;

    xin_ref_picture   *refFrameMap[XIN_REF_FRAME_NUM];
    UINT8             refFrameIdx[XIN_REF_FRAME_NUM];

    UINT32            entryPointOffset[XIN_MAX_TILE_NUM];
    XIN_HANDLE        wppProcSem[XIN_MAX_TILE_NUM];
    xin_cabac_context *cabacSet[XIN_MAX_TILE_NUM];

    xin_sb_struct     *sb;

    xin_bs_struct     *bitstream;

    xin_quant_param   *quantParam;
    SINT32            baseQIdx;
    SINT32            deltaQDc[2];
    SINT32            deltaQAc[2];
    BOOL              codingFrame;
    BOOL              showExistingFrame;

    xin_lf_info       lfInfo;

    UINT32            vpsId;
    UINT32            spsId;
    UINT32            ppsId;

    UINT8             *topCtx[PLANE_NUM];

    XIN_HANDLE        rcMutexHandle;
    xin_rc_struct     *rcSet;
    void              *rcPic;
    UINT32            temporalId;

    BOOL              inputFlush;
    UINT32            entryIdx;
    BOOL              frameDone;

} xin_pic_struct;

#endif

