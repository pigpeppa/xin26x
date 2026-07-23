/***************************************************************************//**
 *
 * @file          h266_pic_struct.h
 * @brief         This file contains h266 picture level structure.
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
#ifndef _h266_pic_struct_h_
#define _h266_pic_struct_h_

#include "h266_seq_struct.h"
#include "h266_ct_unit_struct.h"
#include "h266_lmcs_struct.h"
#include "h26x_rate_control_context.h"

typedef struct xin_sec_struct xin_sec_struct;
typedef struct xin_rc_context xin_rc_context;
typedef struct xin_pic_struct xin_pic_struct;

typedef struct xin_pic_struct
{
    xin_seq_struct    *seqSet;
    xin_func_struct   *funcSet;
    xin_sec_struct    *secSet;
    
    BOOL              isBusy;
    xin_job_desc      *jobCtuEnc;
    xin_job_desc      *jobCtuLpf;
    xin_job_desc      *jobCtuSao;
    xin_job_desc      *jobCtuAlfStat;
    xin_job_desc      *jobDeriveAlf;
    xin_job_desc      *jobCtuAlf;
    xin_job_desc      *jobCtuCcAlfStat;
    xin_job_desc      *jobDeriveCcAlf;
    xin_job_desc      *jobCtuCcAlf;
    xin_job_desc      *jobLmcsFrame;

    xin_ref_picture   *pictureRef[XIN_MAX_DPB_FRAMES];
    UINT32            validRefFrame;
    xin_ref_picture   *pictureRead[XIN_LIST_NUM][XIN_MAX_REF_FRAMES];
    UINT32            adaSearchRange[XIN_LIST_NUM][XIN_MAX_REF_FRAMES];
    xin_ref_picture   *pictureWrite;
    xin_input_picture *inputPicture;

    UINT32            entryPointOffset[XIN_MAX_TILE_NUM];
    xin_prob_model    *cabacContext[XIN_MAX_TILE_NUM];
    xin_cabac_context *cabacSet[XIN_MAX_TILE_NUM];
    SINT32            ctuRowRefQp[XIN_MAX_TILE_NUM + 1];

    xin_neighbour_mv  hmvpLut[XIN_MAX_TILE_NUM][XIN_MAX_MERGE_MV_NUM];
    SINT32            hmvpNum[XIN_MAX_TILE_NUM];

    PIXEL             *saoTopBuffer[PLANE_NUM];
    PIXEL             *saoTopBuf[PLANE_NUM];
    PIXEL             *saoLftBuf[XIN_MAX_TILE_NUM][PLANE_NUM];

    XIN_HANDLE        listLock;

    xin_cu_list       *firstCuChunk;
    xin_cu_list       *lastCuChunk;

    xin_tu_list       *firstTuChunk;
    xin_tu_list       *lastTuChunk;
    
    xin_cu_struct     **cuList[XIN_MAX_TILE_NUM];
    UINT32            cuListSize[XIN_MAX_TILE_NUM];
    UINT32            cuListIdx[XIN_MAX_TILE_NUM];

    xin_tu_struct     **tuList[XIN_MAX_TILE_NUM];
    UINT32            tuListSize[XIN_MAX_TILE_NUM];
    UINT32            tuListIdx[XIN_MAX_TILE_NUM];

    xin_ctu_struct    *ctu;

    COEFF             *coeffBuf[PLANE_NUM];
    UINT16            *gt0Buf[PLANE_NUM];
    
    xin_bs_struct     *bitstream;
    xin_mv_u          *subMvdMap;
    intptr_t          subMvdStride;
    xin_affine_mv     *affineMvMap;
    intptr_t          affineMvStride;

    xin_lmcs_struct   *lmcsSet;
    
    UINT32            qp;
    SINT32            picQp;
    BOOL              codingFrame;

    xin_alf_struct    *alfSet;

    UINT32            vpsId;
    UINT32            spsId;
    UINT32            ppsId;

    BOOL              saoEnabledFlag[2];
    BOOL              enableAlf;
    BOOL              enableCcAlf;

    BOOL              offlineMode;

    xin_rc_context    rcContext;
    UINT32            temporalId;
    UINT32            depth;
    UINT32            inputNumber;
        
}xin_pic_struct;

#endif

