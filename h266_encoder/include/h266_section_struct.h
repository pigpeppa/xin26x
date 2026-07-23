/***************************************************************************//**
 *
 * @file          h266_section_struct.h
 * @brief         This file contains h266 section level structure.
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
#ifndef _h266_section_struct_h_
#define _h266_section_struct_h_

#include "h266_pic_struct.h"
#include "h26x_sao_context.h"
#include "h26x_thread_struct.h"
#include "h26x_look_ahead_struct.h"
#include "h26x_mctf_struct.h"
#include "h266_scene_cut_struct.h"
#include "h266_dep_quant_struct.h"

typedef struct xin_func_struct xin_func_struct;

typedef struct xin_mode_list
{
    xin_mode_struct      *modeBuf;
    struct xin_mode_list *nextList;
}xin_mode_list;

typedef struct xin_sec_struct
{
    xin_seq_struct     *seqSet;
    xin_pic_struct     *picSet;
    xin_func_struct    *funcSet;

    UINT32             sectionIdx;

    xin266_tile_dim    *tileDim;

    xin_fast_md_buf    *fastMdBuf[XIN_MAX_QT_CU_DEPTH*2];
    UINT32             interFastBufNum;
    UINT32             intraFastBufNum;
    xin_full_md_buf    *fullMdBuf[XIN_MAX_QT_CU_DEPTH*2];

    UINT32             fullMdBufNum;
    UINT32             fastMdBufNum;

    xin_fast_md_buf    *fastUvMdBuf;
    UINT32             fastUvMdBufNum;
    xin_full_md_buf    *fullUvMdBuf;
    UINT32             fullUvMdBufNum;

    xin_full_md_buf    *fullBuf[20];
    UINT32             fullBufNum;
    xin_full_md_buf    *fullUvBuf[20];
    UINT32             fullUvBufNum;

    xin_fast_md_buf    *fastUvBuf[20];
    UINT32             fastUvBufNum;

    xin_fast_md_buf    *interBuf[20];
    UINT32             interBufNum;
    xin_fast_md_buf    *intraBuf[20];
    UINT32             intraBufNum;

    xin_motion_data    meData;
    xin_me_struct      *meSet;

    xin_dep_quant      *depQuant;

    PIXEL              *inputCtu[PLANE_NUM];
    PIXEL              *inputCu[PLANE_NUM];
    PIXEL              *reshapeCtuY;
    PIXEL              *reshapeCu[PLANE_NUM];
    intptr_t           inputYStride;
    intptr_t           inputUvStride;

    PIXEL              *reconCtu[PLANE_NUM];
    PIXEL              *reconCu[PLANE_NUM];
    intptr_t           reconYStride;
    intptr_t           reconUvStride;

    PIXEL              *reconData[XIN_MAX_QT_CU_DEPTH+1][PLANE_NUM];
    intptr_t           reconDataStride[PLANE_TYPE];
    xin_block_struct   *blockData[XIN_MAX_QT_CU_DEPTH+1];
    intptr_t           blockDataStride;
    xin_mv_u           subMvdData[XIN_MAX_QT_CU_DEPTH][1024];
    xin_affine_mv      affineMvData[XIN_MAX_QT_CU_DEPTH][1024];
    xin_neighbour_mv   hmvpLutData[XIN_MAX_QT_CU_DEPTH][XIN_MAX_MERGE_MV_NUM];
    SINT32             hmvpNumData[XIN_MAX_QT_CU_DEPTH];

    xin_mv_u           minMv;
    xin_mv_u           maxMv;

    SINT32             qp;
    SINT32             uvQp;
    SINT32             refQp;
    UINT32             chromaWeight;
    BOOL               codingDeltaQp;
    SINT32             chromaResScaleInv;

    double             ctuLambda;
    UINT32             sadLambda[2];
    UINT64             sseLambda[2];
    UINT32             ctuAvailField;

    xin_neighbour_mv   mergeCand[XIN_MAX_MERGE_MV_NUM];
    xin_neighbour_mv   *hmvpLut;
    SINT32             hmvpNum;
    xin_mv32_u         amvpCand[XIN_MAX_AMVP_CAND_NUM + 1];
    UINT32             amvpNum;
    xin_mv32_u         aAmvpCand[XIN_MAX_AMVP_CAND_NUM + 1][3];
    UINT32             aAmvpNum;

    xin_affine_cpmv    affMergeCand[XIN_MAX_AFF_MERGE_MV_NUM];
    xin_neighbour_mv   centerMv;
    xin_mv32_u         *affMvBuf[XIN_MAX_AFF_MERGE_MV_NUM][XIN_LIST_NUM];

    xin_neighbour_mv   hmvpLutBuf[XIN_MAX_QT_CU_DEPTH][XIN_MAX_MERGE_MV_NUM];
    SINT32             hmvpNumBuf[XIN_MAX_QT_CU_DEPTH];

    xin_mv32_u         parentMv[XIN_LIST_NUM];
    SINT32             parentRefIdx[XIN_LIST_NUM];

    xin_ctu_struct     *ctu;
    xin_cu_struct      *cu;

    xin_ctu_struct     *lftCtu;
    xin_ctu_struct     *topCtu;
    xin_ctu_struct     *topLftCtu;
    xin_ctu_struct     *topRgtCtu;

    BOOL               qtBeforeBt;

    xin_block_struct   *curBlock;
    xin_block_struct   *topBlock;
    xin_block_struct   *lftTBlock;
    xin_block_struct   *lftBBlock;
    xin_block_struct   *topRgtBlock;
    xin_block_struct   *topRBlock;
    xin_block_struct   *topLBlock;
    xin_block_struct   *lftBotBlock;
    xin_block_struct   *topLftBlock;
    UINT64             intraAvailField;

    xin_cabac_est      cabacEst;

    PIXEL              *saoTopBuf[PLANE_NUM];

    xin_cabac_context  *cabacSet;

    PIXEL              nIntraBuf[4000];
    xin_block_struct   neighBlock;

    xin_mode_list     *firstModeChunk;
    xin_mode_list     *lastModeChunk;

    xin_mode_struct   **modeList;
    UINT32            modeListSize;
    UINT32            modeListIdx;

    PIXEL             *nIntraBufY;
    PIXEL             *nIntraFltBufY;
    PIXEL             *nIntraBufU;
    PIXEL             *nIntraBufV;

    UINT8             *tempBuffer;
    PIXEL             *predBuffer;
    PIXEL             *biMeInput;

    SINT32            boDiff[PLANE_NUM][XIN_NUM_SAO_BO_CLASS];
    UINT16            boCount[PLANE_NUM][XIN_NUM_SAO_BO_CLASS];

    SINT32            eoDiff[PLANE_NUM][XIN_NUM_SAO_EO][XIN_NUM_SAO_EO_CLASS];
    UINT16            eoCount[PLANE_NUM][XIN_NUM_SAO_EO][XIN_NUM_SAO_EO_CLASS];

} xin_sec_struct;

typedef struct xin266_encoder_struct
{
    xin_sec_struct   *secSet[XIN_MAX_THREAD_POOL_NUM];
    xin_pic_struct   *picSet[XIN_MAX_FRAME_THREAD];
    xin_mctf_struct  *mctfSet;
    xin_la_struct    *laSet;
    xin_sc_struct    *scSet;
    xin_frame_struct reconFrame;
    xin_seq_struct   *seqSet;
    xin_func_struct  *funcSet;
    xin_rc_struct    *rcSet;
    xin_thread_queue *threadQueue;
} xin266_encoder_struct;

#endif

