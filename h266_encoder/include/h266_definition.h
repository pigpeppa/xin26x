/***************************************************************************//**
 *
 * @file          h266_definition.h
 * @brief         This file contains h.266 definitions.
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
#ifndef _h266_definition_h_
#define _h266_definition_h_

#define XIN_CU_NO_SPLIT             0
#define XIN_CU_QUAD_SPLIT           1
#define XIN_CU_HORZ_SPLIT           2
#define XIN_CU_VERT_SPLIT           3
#define XIN_CU_TRIH_SPLIT           4
#define XIN_CU_TRIV_SPLIT           5
#define XIN_CU_SPLIT_NUM            (XIN_CU_TRIV_SPLIT + 1)
#define XIN_CU_CTU_LEVEL            15

#define XIN_ALF_APS                 0
#define XIN_LMCS_APS                1
#define XIN_SCALING_APS             2

#define XIN_CAN_NO_SPLIT            (1 << XIN_CU_NO_SPLIT)
#define XIN_CAN_QUAD_SPLIT          (1 << XIN_CU_QUAD_SPLIT)
#define XIN_CAN_HORZ_SPLIT          (1 << XIN_CU_HORZ_SPLIT)
#define XIN_CAN_VERT_SPLIT          (1 << XIN_CU_VERT_SPLIT)
#define XIN_CAN_TRIH_SPLIT          (1 << XIN_CU_TRIH_SPLIT)
#define XIN_CAN_TRIV_SPLIT          (1 << XIN_CU_TRIV_SPLIT)

#define XIN_CU_QUAD_PART            (XIN_CU_QUAD_SPLIT - XIN_CU_QUAD_SPLIT)
#define XIN_CU_HORZ_PART            (XIN_CU_HORZ_SPLIT - XIN_CU_QUAD_SPLIT)
#define XIN_CU_VERT_PART            (XIN_CU_VERT_SPLIT - XIN_CU_QUAD_SPLIT)
#define XIN_CU_TRIH_PART            (XIN_CU_TRIH_SPLIT - XIN_CU_QUAD_SPLIT)
#define XIN_CU_TRIV_PART            (XIN_CU_TRIV_SPLIT - XIN_CU_QUAD_SPLIT)
#define XIN_CU_PART_NUM             (XIN_CU_TRIV_PART + 1)

#define XIN_CU_TREE_L               0
#define XIN_CU_TREE_C               1
#define XIN_CU_TREE_L_MASK          (1<<XIN_CU_TREE_L)
#define XIN_CU_TREE_C_MASK          (1<<XIN_CU_TREE_C)
#define XIN_CU_TREE_D_MASK          ((XIN_CU_TREE_L_MASK) | (XIN_CU_TREE_C_MASK))

// Cu type defitions
#define XIN_SKIP_MODE               0
#define XIN_MERGE_MODE              1
#define XIN_INTER_MODE              2
#define XIN_INTRA_MODE              3
#define XIN_MODE_NUM                4
#define XIN_INVALID_MODE            0xFF

#define XIN_INTRA_CHROMA_NUM        5
#define XIN_INTRA_CHROMA_DM         5

#define XIN_ME_BUF_STRIDE           200
#define XIN_ME_FILTER_TAP           4

#define XIN_MAX_TU_NUM              4096
#define XIN_MAX_CU_NUM              4096

#define XIN_SECTION_CHUNK_SIZE      256

#define XIN_MAX_CTU_SIZE            128
#define XIN_MIN_CTU_SIZE            32

#define XIN_MAX_CU_SIZE             128
#define XIN_MIN_CU_SIZE             4

#define XIN_MAX_LG_CU_SIZE          7
#define XIN_MIN_LG_CU_SIZE          2

#define XIN_MIN_WIDTH_IN_TILE       1
#define XIN_MAX_WIDTH_IN_TILE       2
#define XIN_MIN_HEIGHT_IN_TILE      1
#define XIN_MAX_HEIGHT_IN_TILE      2

#define XIN_MAX_MERGE_MV_NUM        6
#define XIN_MAX_AFF_MERGE_MV_NUM    5

#define XIN_MAX_DPB_FRAMES          8
#define XIN_REF_PRED0_NUM           3

#define XIN_MAX_DPB_SIZE            32
#define XIN_MAX_RPL_NUM             256

#define XIN_MAX_TT_SIZE             16
#define XIN_MAX_BT_SIZE             32
#define XIN_MIN_TT_SIZE             16
#define XIN_MIN_BT_SIZE             8
#define XIN_MAX_BT_CU_DEPTH         1
#define XIN_MAX_TT_CU_DEPTH         1
#define XIN_MAX_QT_CU_DEPTH         6

#define XIN_MAX_LG_CG_SIZE          4

#define XIN_LG_TMP_UNIT_SIZE        3

#define XIN_MAX_INPUT_SIZE          255

// The CB is not completely outside the frame.
#define XIN_CB_PRESENT              (1<<1)

// The CB split is mandatory.
#define XIN_CB_FORBIDDEN            (1<<2)

// The CB is split in four child CBs.
#define XIN_CB_SPLIT                (1<<3)

#define PRED_BUF_SIZE               (XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE)
#define PRED_BUF_STRIDE             (XIN_MAX_CU_SIZE)

#define XIN_MIN_QP                  0
#define XIN_MAX_QP                  51
#define XIN_QP_NUM                  52
#define XIN_QP_SHIFT                12

#define XIN_BLOCK_SIZE              4
#define XIN_LOG_BLOCK_SIZE          2

#define XIN_MAX_AMVP_CAND_NUM       2
#define XIN_MAX_HMVP_AVMP_CAND_NUM  4
#define XIN_MAX_HMVP_CAND_NUM       5

#define XIN_ATMVP_SUB_BLOCK_SIZE    8
#define XIN_AFFINE_SUB_BLOCK_SIZE   4

#define XIN_MAX_AFFINE_CPMV_NUM     3
#define XIN_AFFINE_MODEL_4PARAM     0
#define XIN_AFFINE_MODEL_6PARAM     1
#define XIN_AFFINE_SBTMVP           2

#define XIN_LMCS_ANALYZE_CW_BINS    32
#define XIN_LMCS_ENCODE_CW_BINS     16
#define XIN_LMCS_SEG_MUM            32

// Surrounding ctu availability 
#define XIN_LFT_CTU_AVAIL           (1 << 0)
#define XIN_RGT_CTU_AVAIL           (1 << 1)
#define XIN_TOP_CTU_AVAIL           (1 << 2)
#define XIN_BOT_CTU_AVAIL           (1 << 3)

#define XIN_ENTROPY_HEADER_SIZE     0x001000

#define XIN_MIN_TEMPORAL_LAYER      1
#define XIN_MAX_TEMPORAL_LAYER      4

#define XIN_UV_WEIGHT_FRAC          8
#define XIN_RATE_FRACTION           15
#define XIN_SEE_LAMBDA_FRAC         16
#define XIN_SAD_LAMBDA_FRAC         8

#define XIN_SSE_SHIFT               (XIN_RATE_FRACTION + XIN_SEE_LAMBDA_FRAC - XIN_COST_FRACTION)
#define XIN_SSE_OFFSET              (1 << (XIN_SSE_SHIFT-1))

#define XIN_SAD_SHIFT               (XIN_RATE_FRACTION + XIN_SAD_LAMBDA_FRAC - XIN_COST_FRACTION)
#define XIN_SAD_OFFSET              (1 << (XIN_SAD_SHIFT-1))

#define XIN_UV_DIST_SHIFT           (XIN_COST_FRACTION - XIN_UV_WEIGHT_FRAC)

#define XIN_MV_BIT_NUM              18
#define XIN_MAX_MV                  ((1<<XIN_MV_BIT_NUM) - 1)
#define XIN_MIN_MV                  (-(1<<XIN_MV_BIT_NUM))
#define XIN_SBH_THRESHOLD           4

#define XIN_QUANT_SHIFT             14
#define XIN_IQUANT_SHIFT            6

#define XIN_INPUT_COPY_STAGE        0
#define XIN_SCENE_CUT_STAGE         1
#define XIN_MCTF_STAGE              2
#define XIN_LOOK_AHEAD_STAGE        3
#define XIN_ENCODE_STAGE            4
#define XIN_STAGE_NUM               5

#define XIN_CU_GRAD_HOR             0
#define XIN_CU_GRAD_VER             1
#define XIN_CU_GRAD_DOW             2
#define XIN_CU_GRAD_DUP             3
#define XIN_CU_GRAD_NUM             4

#define PEL_XY_TO_BLOCK_INDEX(PELX, PELY, BLOCKINDEX, FRAMEWIDTH, LGBS) \
   (BLOCKINDEX) = ((PELY)>>(LGBS))*(FRAMEWIDTH) + ((PELX)>>(LGBS))

#define IS_MV_DIFF_P(MV0, MV1)   \
    (((MV0)->mv[XIN_LIST_0].s64x1 != (MV1)->mv[XIN_LIST_0].s64x1) || ((MV0)->refIdx[XIN_LIST_0] != (MV1)->refIdx[XIN_LIST_0]))

#define IS_MV_DIFF_B(MV0, MV1)   \
    (((MV0)->mv[XIN_LIST_0].s64x1 != (MV1)->mv[XIN_LIST_0].s64x1) || ((MV0)->refIdx[XIN_LIST_0] != (MV1)->refIdx[XIN_LIST_0])   \
    || ((MV0)->mv[XIN_LIST_1].s64x1 != (MV1)->mv[XIN_LIST_1].s64x1) || ((MV0)->refIdx[XIN_LIST_1] != (MV1)->refIdx[XIN_LIST_1]))

#define CALC_SSE_COST(lambda, bit)   (((((UINT64)(lambda))*((UINT64)(bit))) + XIN_SSE_OFFSET) >> XIN_SSE_SHIFT)
#define CALC_SAD_COST(lambda, bit)   (((((UINT64)(lambda))*((UINT64)(bit))) + XIN_SAD_OFFSET) >> XIN_SAD_SHIFT)

#endif
