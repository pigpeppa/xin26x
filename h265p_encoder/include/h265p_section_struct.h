/***************************************************************************//**
 *
 * @file          h265p_section_struct.h
 * @brief         This file contains av1 section level structure.
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
#ifndef _h265p_section_struct_h_
#define _h265p_section_struct_h_

#include "h265p_pic_struct.h"
#include "h265p_func_struct.h"
#include "h26x_thread_struct.h"
#include "h265p_me_struct.h"

typedef struct xin_sec_struct
{
    xin_seq_struct    *seqSet;
    xin_pic_struct    *picSet;
    xin_func_struct   *funcSet;

    xin_tu_struct     *tu;

    UINT32            sectionIdx;

    xin265p_tile_dim  *tileDim;

    UINT32            fastMdBufStart[XIN_MODE_NUM];
    
    xin_fast_md_buf   *fastMdBuf[XIN_PART_NUM][XIN_MAX_MB_DEPTH];
    UINT32            fastMdBufNum[XIN_PART_NUM][XIN_MAX_MB_DEPTH];
    xin_full_md_buf   *fullMdBuf[XIN_PART_NUM][XIN_MAX_MB_DEPTH];
    UINT32            fullMdBufNum[XIN_PART_NUM][XIN_MAX_MB_DEPTH];
    
    xin_me_struct     *meSet;

    PIXEL             *inputSb[PLANE_NUM];
    PIXEL             *inputMb[PLANE_NUM];
    intptr_t          inputYStride;
    intptr_t          inputUvStride;

    PIXEL             *reconSb[PLANE_NUM];
    PIXEL             *reconMb[PLANE_NUM];
    intptr_t          reconStride[PLANE_TYPE];

    PIXEL             *reconData[PLANE_NUM];
    intptr_t          reconDataStride[PLANE_TYPE];

    xin_mi_struct     *miData;
    SINT32            miDataStride;

    UINT8             *topCtx[XIN_MAX_MB_DEPTH][XIN_PART_NUM][PLANE_NUM];
    UINT8             *lftCtx[XIN_MAX_MB_DEPTH][XIN_PART_NUM][PLANE_NUM];

    UINT8             *sbTopCtx[PLANE_NUM];
    UINT8             *sbLftCtx[PLANE_NUM];

    xin_mv_u          minMv;
    xin_mv_u          maxMv;

    UINT32            qp;
    UINT32            uvQp;
    UINT32            secQp;
    UINT32            refQp;
    UINT32            chromaWeight;
    BOOL              codingDeltaQp;

    double            sbLambda;
    UINT32            sadLambda[2];
    UINT64            sseLambda[2];

    xin_mb_struct     *pqMbData[XIN_MAX_QT_DEPTH];
    xin_mb_struct     *phMbData[XIN_MAX_BT_DEPTH];
    xin_mb_struct     *pvMbData[XIN_MAX_BT_DEPTH];
    UINT32            sbAvailField;

    xin_mv_u          parentMv[2];
    SINT32            parentRefIdx[2];

    xin_mi_struct     neighMi;

    xin_sb_struct     *sb;
    xin_mb_struct     *mb;
    xin_mb_struct     *lftMb;
    xin_mb_struct     *topMb;
    
    xin_sb_struct     *lftSb;
    xin_sb_struct     *topSb;
    xin_sb_struct     *topLftSb;
    xin_sb_struct     *topRgtSb;

    xin_cabac_est     cabacEst;
    
    xin_cabac_context *cabacSet;

    PIXEL             *intraBuf;
    PIXEL             *intraLftBuf[PLANE_NUM];
    PIXEL             *intraTopBuf[PLANE_NUM];
    
    UINT8             *tempBuffer;
    UINT8             *predBuffer;

} xin_sec_struct;

typedef struct xin265p_encoder_struct
{
    xin_sec_struct  *secSet[XIN_MAX_THREAD_POOL_NUM];
    xin_pic_struct  *picSet[XIN_MAX_FRAME_THREAD];
    xin_pic_struct  *picList[XIN_MAX_FRAME_THREAD];
    UINT32          picListNum;
    UINT32          laListNum;
    xin_ref_picture *reconList[XIN_MAX_FRAME_THREAD];
    UINT32          reconListNum;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_rc_struct   *rcSet;
}xin265p_encoder_struct;

#endif

