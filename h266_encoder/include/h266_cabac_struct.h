/***************************************************************************//**
 *
 * @file          h266_cabac_struct.h
 * @brief         This file contains CABAC-related data structures and definitions.
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
#ifndef _h266_cabac_struct_h_
#define _h266_cabac_struct_h_

#include "h266_bit_stream_struct.h"
#include "h26x_trans_context.h"

#define XIN_NUM_SPLIT_FLAG_CTX              9   // number of context models for split flag
#define XIN_NUM_SPLIT_QT_FLAG_CTX           6   // number of context models for split qt flag
#define XIN_NUM_SPLIT_HV_FLAG_CTX           5   // number of context models for split hv flag
#define XIN_NUM_SPLIT_12_FLAG_CTX           4   // number of context models for split 12 flag
#define XIN_NUM_MODE_CONS_FLAG_CTX          2

#define XIN_NUM_SKIP_FLAG_CTX               3   // number of context models for skip flag
#define XIN_NUM_MERGE_FLAG_CTX              1   // number of context models for merge flag of merge extended
#define XIN_NUM_REGULAR_MERGE_FLAG_CTX      2

#define XIN_NUM_MERGE_IDX_CTX               1   // number of context models for merge index of merge extended
#define XIN_NUM_MMVD_FLAG_CTX               1
#define XIN_NUM_MMVD_MERGE_FLAG_CTX         1   // number of context models for merge mode flag with MVD
#define XIN_NUM_MMVD_MERGE_IDX_CTX          1   // number of context models for merge mode idx with MVD
#define XIN_NUM_MMVD_STEP_MVP_IDX_CTX       1   // number of context models for merge mode idx with MVD

#define XIN_NUM_PRED_MODE_CTX               2   // number of context models for prediction mode
#define XIN_NUM_MULTI_REF_LINE_IDX_CTX      2   //
#define XIN_NUM_INTRA_LUMA_MPM_FLAG_CTX     1
#define XIN_NUM_INTRA_LUMA_PLANAR_FLAG_CTX  2
#define XIN_NUM_CCLM_MODE_FLAG_CTX          1
#define XIN_NUM_CCLM_MODE_IDX_CTX           1
#define XIN_NUM_INTRA_CHROMA_PRED_MODE_CTX  1
#define XIN_NUM_MIP_FLAG_CTX                4
#define XIN_NUM_DELTA_QP_CTX                2
#define XIN_NUM_INTER_DIR_CTX               6
#define XIN_NUM_REF_PIC_CTX                 2
#define XIN_NUM_SUBBLOCK_MERGE_FLAG_CTX     3
#define XIN_NUM_AFFINE_FLAG_CTX             3
#define XIN_NUM_AFFINE_TYPE_CTX             1
#define XIN_NUM_AFF_MERGE_IDX_CTX           1
#define XIN_NUM_BCW_IDX_CTX                 1
#define XIN_NUM_MVD_CTX                     2
#define XIN_NUM_BDPCM_MODE_CTX              4
#define XIN_NUM_QT_ROOT_CBF_CTX             1
#define XIN_NUM_QT_CBF_Y_CTX                4
#define XIN_NUM_QT_CBF_U_CTX                2
#define XIN_NUM_QT_CBF_V_CTX                3
#define XIN_NUM_SIG_COEFF_GROUP_CTX         2
#define XIN_NUM_SIG_FLAG_STATE_LUMA_CTX     12
#define XIN_NUM_SIG_FLAG_LUMA_CTX           36
#define XIN_NUM_SIG_FLAG_STATE_CHROMA_CTX   8
#define XIN_NUM_SIG_FLAG_CHROMA_CTX         24
#define XIN_NUM_PAR_FLAG_LUMA_CTX           21
#define XIN_NUM_PAR_FLAG_CHROMA_CTX         11
#define XIN_NUM_GT2_FLAG_LUMA_CTX           21
#define XIN_NUM_GT2_FLAG_CHROMA_CTX         11
#define XIN_NUM_GT1_FLAG_LUMA_CTX           21
#define XIN_NUM_GT1_FLAG_CHROMA_CTX         11
#define XIN_NUM_LAST_XY_LUMA_CTX            20
#define XIN_NUM_LAST_XY_CHROMA_CTX          3
#define XIN_NUM_MVP_IDX_CTX                 1
#define XIN_NUM_SMVD_FLAG_CTX               1
#define XIN_NUM_SAO_MERGE_FLAG_CTX          1
#define XIN_NUM_SAO_TYPE_IDX_CTX            1
#define XIN_NUM_TRANSQUANT_BYPASS_FLAG_CTX  1
#define XIN_NUM_LFNST_IDX_CTX               3
#define XIN_NUM_PLT_IDX_CTX                 1
#define XIN_NUM_ROTATION_FLAG_CTX           1
#define XIN_NUM_RUN_TYPE_FLAG_CTX           1
#define XIN_NUM_IDX_RUN_MODEL_CTX           5
#define XIN_NUM_COPY_RUN_MODEL_CTX          3
#define XIN_NUM_RDPCM_FLAG_CTX              2
#define XIN_NUM_RDPCM_DIR_CTX               2
#define XIN_NUM_MTS_INDEX_CTX               11
#define XIN_NUM_ISP_MODE_CTX                2
#define XIN_NUM_SBT_FLAG_CTX                2
#define XIN_NUM_QUAD_FLAG_CTX               1
#define XIN_NUM_SBT_HOR_FLAG_CTX            3
#define XIN_NUM_SBT_POS_FLAG_CTX            1
#define XIN_NUM_CROSS_COMP_PRED_CTX         10
#define XIN_NUM_CHROMA_QP_ADJ_FLAG_CTX      1
#define XIN_NUM_CHROMA_QP_ADJ_IDC_CTX       1
#define XIN_NUM_IMV_FLAG_CTX                5
#define XIN_NUM_CTB_ALF_FLAG_CTX            9
#define XIN_NUM_CTB_ALF_ALT_CTX             2
#define XIN_NUM_ALF_USE_TEMPO_FLT_CTX       1
#define XIN_NUM_CC_ALF_FLT_CTRL_FLAG_CTX    6
#define XIN_NUM_MH_INTRA_FLAG_CTX           1
#define XIN_NUM_IBC_FLAG_CTX                3
#define XIN_NUM_JOINT_CB_CR_FLAG_CTX        3
#define XIN_NUM_TRANSFORM_SKIP_FLAG_CTX     2
#define XIN_NUM_TS_SIG_COEFF_GROUP_CTX      3
#define XIN_NUM_TS_SIG_FLAG_CTX             3
#define XIN_NUM_TS_PAR_FLAG_CTX             1
#define XIN_NUM_TS_GTX_FLAG_CTX             5
#define XIN_NUM_TS_GT1_FLAG_CTX             4
#define XIN_NUM_TS_RESIDUAL_SIGN_CTX        6
#define CNU                                 35  // dummy initialization value for unused context models 'Context model Not Used'
#define DWS                                 8

#define XIN_NUM_SIG_LUMA_PER_STATE          12
#define XIN_NUM_SIG_CHROMA_PER_STATE        9

#define XIN_PROB_BIT_NUM                    15
#define XIN_PROB_BIT_0_NUM                  10
#define XIN_PROB_BIT_1_NUM                  14
#define XIN_CABAC_STATE_MASK_0              (~(~0u << XIN_PROB_BIT_0_NUM) << (XIN_PROB_BIT_NUM - XIN_PROB_BIT_0_NUM))
#define XIN_CABAC_STATE_MASK_1              (~(~0u << XIN_PROB_BIT_1_NUM) << (XIN_PROB_BIT_NUM - XIN_PROB_BIT_1_NUM))

#define XIN_CO_SPLIT_FLAG                   (0)
#define XIN_CO_SPLIT_QT_FLAG                (XIN_CO_SPLIT_FLAG              +   XIN_NUM_SPLIT_FLAG_CTX)
#define XIN_CO_SPLIT_HV_FLAG                (XIN_CO_SPLIT_QT_FLAG           +   XIN_NUM_SPLIT_QT_FLAG_CTX)
#define XIN_CO_SPLIT_12_FLAG                (XIN_CO_SPLIT_HV_FLAG           +   XIN_NUM_SPLIT_HV_FLAG_CTX)
#define XIN_CO_MODE_CONS_FLAG               (XIN_CO_SPLIT_12_FLAG           +   XIN_NUM_SPLIT_12_FLAG_CTX)
#define XIN_CO_SKIP_FLAG                    (XIN_CO_MODE_CONS_FLAG          +   XIN_NUM_MODE_CONS_FLAG_CTX)
#define XIN_CO_MERGE_FLAG                   (XIN_CO_SKIP_FLAG               +   XIN_NUM_SKIP_FLAG_CTX)
#define XIN_CO_REGULAR_MERGE_FLAG           (XIN_CO_MERGE_FLAG              +   XIN_NUM_MERGE_FLAG_CTX)
#define XIN_CO_MERGE_IDX                    (XIN_CO_REGULAR_MERGE_FLAG      +   XIN_NUM_REGULAR_MERGE_FLAG_CTX)
#define XIN_CO_MMVD_FLAG_CTX                (XIN_CO_MERGE_IDX               +   XIN_NUM_MERGE_IDX_CTX)
#define XIN_CO_MMVD_MERGE_FLAG              (XIN_CO_MMVD_FLAG_CTX           +   XIN_NUM_MMVD_FLAG_CTX)
#define XIN_CO_MMVD_MERGE_IDX               (XIN_CO_MMVD_MERGE_FLAG         +   XIN_NUM_MMVD_MERGE_FLAG_CTX)
#define XIN_CO_MMVD_STEP_MVP_IDX            (XIN_CO_MMVD_MERGE_IDX          +   XIN_NUM_MMVD_MERGE_IDX_CTX)
#define XIN_CO_PRED_MODE                    (XIN_CO_MMVD_STEP_MVP_IDX       +   XIN_NUM_MMVD_STEP_MVP_IDX_CTX)
#define XIN_CO_MULTI_REF_LINE_IDX           (XIN_CO_PRED_MODE               +   XIN_NUM_PRED_MODE_CTX)
#define XIN_CO_INTRA_LUMA_MPM_FLAG          (XIN_CO_MULTI_REF_LINE_IDX      +   XIN_NUM_MULTI_REF_LINE_IDX_CTX)
#define XIN_CO_INTRA_LUMA_PLANAR_FLAG       (XIN_CO_INTRA_LUMA_MPM_FLAG     +   XIN_NUM_INTRA_LUMA_MPM_FLAG_CTX)
#define XIN_CO_CCLM_MODE_FLAG               (XIN_CO_INTRA_LUMA_PLANAR_FLAG  +   XIN_NUM_INTRA_LUMA_PLANAR_FLAG_CTX)
#define XIN_CO_CCLM_MODE_IDX                (XIN_CO_CCLM_MODE_FLAG          +   XIN_NUM_CCLM_MODE_FLAG_CTX)
#define XIN_CO_INTRA_CHROMA_PRED_MODE       (XIN_CO_CCLM_MODE_IDX           +   XIN_NUM_CCLM_MODE_IDX_CTX)
#define XIN_CO_MIP_FLAG                     (XIN_CO_INTRA_CHROMA_PRED_MODE  +   XIN_NUM_INTRA_CHROMA_PRED_MODE_CTX)
#define XIN_CO_DELTA_QP                     (XIN_CO_MIP_FLAG                +   XIN_NUM_MIP_FLAG_CTX)
#define XIN_CO_INTER_DIR                    (XIN_CO_DELTA_QP                +   XIN_NUM_DELTA_QP_CTX)
#define XIN_CO_REF_PIC                      (XIN_CO_INTER_DIR               +   XIN_NUM_INTER_DIR_CTX)
#define XIN_CO_SUBBLOCK_MERGE_FLAG          (XIN_CO_REF_PIC                 +   XIN_NUM_REF_PIC_CTX)
#define XIN_CO_AFFINE_FLAG                  (XIN_CO_SUBBLOCK_MERGE_FLAG     +   XIN_NUM_SUBBLOCK_MERGE_FLAG_CTX)
#define XIN_CO_AFFINE_TYPE                  (XIN_CO_AFFINE_FLAG             +   XIN_NUM_AFFINE_FLAG_CTX)
#define XIN_CO_AFF_MERGE_IDX                (XIN_CO_AFFINE_TYPE             +   XIN_NUM_AFFINE_TYPE_CTX)
#define XIN_CO_BCW_IDX                      (XIN_CO_AFF_MERGE_IDX           +   XIN_NUM_AFF_MERGE_IDX_CTX)
#define XIN_CO_MVD                          (XIN_CO_BCW_IDX                 +   XIN_NUM_BCW_IDX_CTX)
#define XIN_CO_BDPCM_MODE                   (XIN_CO_MVD                     +   XIN_NUM_MVD_CTX)
#define XIN_CO_QT_ROOT_CBF                  (XIN_CO_BDPCM_MODE              +   XIN_NUM_BDPCM_MODE_CTX)
#define XIN_CO_QT_CBF_Y                     (XIN_CO_QT_ROOT_CBF             +   XIN_NUM_QT_ROOT_CBF_CTX)
#define XIN_CO_QT_CBF_U                     (XIN_CO_QT_CBF_Y                +   XIN_NUM_QT_CBF_Y_CTX)
#define XIN_CO_QT_CBF_V                     (XIN_CO_QT_CBF_U                +   XIN_NUM_QT_CBF_U_CTX)
#define XIN_CO_SIG_COEFF_GROUP_LUMA         (XIN_CO_QT_CBF_V                +   XIN_NUM_QT_CBF_V_CTX)
#define XIN_CO_SIG_COEFF_GROUP_CHROMA       (XIN_CO_SIG_COEFF_GROUP_LUMA    +   XIN_NUM_SIG_COEFF_GROUP_CTX)
#define XIN_CO_SIG_FLAG_LUMA                (XIN_CO_SIG_COEFF_GROUP_CHROMA  +   XIN_NUM_SIG_COEFF_GROUP_CTX)
#define XIN_CO_SIG_FLAG_CHROMA              (XIN_CO_SIG_FLAG_LUMA           +   XIN_NUM_SIG_FLAG_LUMA_CTX)
#define XIN_CO_PAR_FLAG_LUMA                (XIN_CO_SIG_FLAG_CHROMA         +   XIN_NUM_SIG_FLAG_CHROMA_CTX)
#define XIN_CO_PAR_FLAG_CHROMA              (XIN_CO_PAR_FLAG_LUMA           +   XIN_NUM_PAR_FLAG_LUMA_CTX)
#define XIN_CO_GT2_FLAG_LUMA                (XIN_CO_PAR_FLAG_CHROMA         +   XIN_NUM_PAR_FLAG_CHROMA_CTX)
#define XIN_CO_GT2_FLAG_CHROMA              (XIN_CO_GT2_FLAG_LUMA           +   XIN_NUM_GT2_FLAG_LUMA_CTX)
#define XIN_CO_GT1_FLAG_LUMA                (XIN_CO_GT2_FLAG_CHROMA         +   XIN_NUM_GT2_FLAG_CHROMA_CTX)
#define XIN_CO_GT1_FLAG_CHROMA              (XIN_CO_GT1_FLAG_LUMA           +   XIN_NUM_GT1_FLAG_LUMA_CTX)
#define XIN_CO_LAST_X_LUMA                  (XIN_CO_GT1_FLAG_CHROMA         +   XIN_NUM_GT1_FLAG_CHROMA_CTX)
#define XIN_CO_LAST_X_CHROMA                (XIN_CO_LAST_X_LUMA             +   XIN_NUM_LAST_XY_LUMA_CTX)
#define XIN_CO_LAST_Y_LUMA                  (XIN_CO_LAST_X_CHROMA           +   XIN_NUM_LAST_XY_CHROMA_CTX)
#define XIN_CO_LAST_Y_CHROMA                (XIN_CO_LAST_Y_LUMA             +   XIN_NUM_LAST_XY_LUMA_CTX)
#define XIN_CO_MVP_IDX                      (XIN_CO_LAST_Y_CHROMA           +   XIN_NUM_LAST_XY_CHROMA_CTX)
#define XIN_CO_SMVD_FLAG                    (XIN_CO_MVP_IDX                 +   XIN_NUM_MVP_IDX_CTX)
#define XIN_CO_SAO_MERGE_FLAG               (XIN_CO_SMVD_FLAG               +   XIN_NUM_SMVD_FLAG_CTX)
#define XIN_CO_SAO_TYPE_IDX                 (XIN_CO_SAO_MERGE_FLAG          +   XIN_NUM_SAO_MERGE_FLAG_CTX)
#define XIN_CO_TRANSQUANT_BYPASS_FLAG       (XIN_CO_SAO_TYPE_IDX            +   XIN_NUM_SAO_TYPE_IDX_CTX)
#define XIN_CO_LFNST_IDX                    (XIN_CO_TRANSQUANT_BYPASS_FLAG  +   XIN_NUM_TRANSQUANT_BYPASS_FLAG_CTX)
#define XIN_CO_PLT_IDX                      (XIN_CO_LFNST_IDX               +   XIN_NUM_LFNST_IDX_CTX)
#define XIN_CO_ROTATION_FLAG                (XIN_CO_PLT_IDX                 +   XIN_NUM_PLT_IDX_CTX)
#define XIN_CO_RUN_TYPE_FLAG                (XIN_CO_ROTATION_FLAG           +   XIN_NUM_ROTATION_FLAG_CTX)
#define XIN_CO_IDX_RUN_MODEL                (XIN_CO_RUN_TYPE_FLAG           +   XIN_NUM_RUN_TYPE_FLAG_CTX)
#define XIN_CO_COPY_RUN_MODEL               (XIN_CO_IDX_RUN_MODEL           +   XIN_NUM_IDX_RUN_MODEL_CTX)
#define XIN_CO_RDPCM_FLAG                   (XIN_CO_COPY_RUN_MODEL          +   XIN_NUM_COPY_RUN_MODEL_CTX)
#define XIN_CO_RDPCM_DIR                    (XIN_CO_RDPCM_FLAG              +   XIN_NUM_RDPCM_FLAG_CTX)
#define XIN_CO_MTS_INDEX                    (XIN_CO_RDPCM_DIR               +   XIN_NUM_RDPCM_DIR_CTX)
#define XIN_CO_ISP_MODE                     (XIN_CO_MTS_INDEX               +   XIN_NUM_MTS_INDEX_CTX)
#define XIN_CO_SBT_FLAG                     (XIN_CO_ISP_MODE                +   XIN_NUM_ISP_MODE_CTX)
#define XIN_CO_QUAD_FLAG                    (XIN_CO_SBT_FLAG                +   XIN_NUM_SBT_FLAG_CTX)
#define XIN_CO_SBT_HOR_FLAG                 (XIN_CO_QUAD_FLAG               +   XIN_NUM_QUAD_FLAG_CTX)
#define XIN_CO_SBT_POS_FLAG                 (XIN_CO_SBT_HOR_FLAG            +   XIN_NUM_SBT_HOR_FLAG_CTX)
#define XIN_CO_CROSS_COMP_PRED              (XIN_CO_SBT_POS_FLAG            +   XIN_NUM_SBT_POS_FLAG_CTX)
#define XIN_CO_CHROMA_QP_ADJ_FLAG           (XIN_CO_CROSS_COMP_PRED         +   XIN_NUM_CROSS_COMP_PRED_CTX)
#define XIN_CO_CHROMA_QP_ADJ_IDC            (XIN_CO_CHROMA_QP_ADJ_FLAG      +   XIN_NUM_CHROMA_QP_ADJ_FLAG_CTX)
#define XIN_CO_IMV_FLAG                     (XIN_CO_CHROMA_QP_ADJ_IDC       +   XIN_NUM_CHROMA_QP_ADJ_IDC_CTX)
#define XIN_CO_CTB_ALF_FLAG                 (XIN_CO_IMV_FLAG                +   XIN_NUM_IMV_FLAG_CTX)
#define XIN_CO_CTB_ALF_ALT                  (XIN_CO_CTB_ALF_FLAG            +   XIN_NUM_CTB_ALF_FLAG_CTX)
#define XIN_CO_USE_TEMPO_FLT                (XIN_CO_CTB_ALF_ALT             +   XIN_NUM_CTB_ALF_ALT_CTX)
#define XIN_CO_CC_ALF_FLT_CTRL_FLAG         (XIN_CO_USE_TEMPO_FLT           +   XIN_NUM_ALF_USE_TEMPO_FLT_CTX)
#define XIN_CO_MH_INTRA_FLAG                (XIN_CO_CC_ALF_FLT_CTRL_FLAG    +   XIN_NUM_CC_ALF_FLT_CTRL_FLAG_CTX)
#define XIN_CO_IBC_FLAG                     (XIN_CO_MH_INTRA_FLAG           +   XIN_NUM_MH_INTRA_FLAG_CTX)
#define XIN_CO_JOINT_CB_CR_FLAG             (XIN_CO_IBC_FLAG                +   XIN_NUM_IBC_FLAG_CTX)
#define XIN_CO_TRANSFORM_SKIP_FLAG          (XIN_CO_JOINT_CB_CR_FLAG        +   XIN_NUM_JOINT_CB_CR_FLAG_CTX)
#define XIN_CO_TS_SIG_COEFF_GROUP           (XIN_CO_TRANSFORM_SKIP_FLAG     +   XIN_NUM_TRANSFORM_SKIP_FLAG_CTX)
#define XIN_CO_TS_SIG_FLAG                  (XIN_CO_TS_SIG_COEFF_GROUP      +   XIN_NUM_TS_SIG_COEFF_GROUP_CTX)
#define XIN_CO_TS_PAR_FLAG                  (XIN_CO_TS_SIG_FLAG             +   XIN_NUM_TS_SIG_FLAG_CTX)
#define XIN_CO_TS_GTX_FLAG                  (XIN_CO_TS_PAR_FLAG             +   XIN_NUM_TS_PAR_FLAG_CTX)
#define XIN_CO_TS_GT1_FLAG                  (XIN_CO_TS_GTX_FLAG             +   XIN_NUM_TS_GTX_FLAG_CTX)
#define XIN_CO_TS_RESIDUAL_SIGN             (XIN_CO_TS_GT1_FLAG             +   XIN_NUM_TS_GT1_FLAG_CTX)
#define XIN_NUM_OF_CTX                      (XIN_CO_TS_RESIDUAL_SIGN        +   XIN_NUM_TS_RESIDUAL_SIGN_CTX)

// Object used to write the unescaped CABAC bitstream.
typedef struct xin_cabac_struct
{
    xin_bs_struct bitstream;
    UINT32        low;
    UINT32        range;
    UINT32        bufferedByte;
    SINT32        numBufferedBytes;
    SINT32        bitsLeft;
} xin_cabac_struct;

typedef struct xin_prob_model
{
    UINT16  state[2];
    UINT8   rate;
} xin_prob_model;

typedef struct xin_cabac_context
{
    xin_cabac_struct cabac;
    xin_prob_model   context[XIN_NUM_OF_CTX];
} xin_cabac_context;

typedef struct xin_coeff_context
{
    SINT32  planeIdx;
    BOOL    sbhOn;
    SINT32  regBinLimit;
    SINT32  width;
    SINT32  height;
    SINT32  cgHeight;
    SINT32  cgWidth;
    SINT32  lgCGSize;
    SINT32  lastScanIdx;
    SINT32  minSubIdx;
    SINT32  maxSubIdx;
    SINT32  subCGIdx;
    SINT32  sigGrpCtx;
    SINT32  cgPosX;
    SINT32  cgPosY;
    SINT32  sumAbs1;
    BOOL    onlyLastSigGrp;
} xin_coeff_context;

#define XIN_NUM_SIG_FLAG_CTX        (XIN_NUM_SIG_FLAG_LUMA_CTX + XIN_NUM_SIG_FLAG_CHROMA_CTX)
#define XIN_NUM_LAST_XY_CTX         14
#define XIN_NUM_GT1_FLAG_CTX        (XIN_NUM_GT1_FLAG_LUMA_CTX + XIN_NUM_GT1_FLAG_CHROMA_CTX)
#define XIN_NUM_PAR_FLAG_CTX        (XIN_NUM_PAR_FLAG_LUMA_CTX + XIN_NUM_PAR_FLAG_CHROMA_CTX)
#define XIN_NUM_GT2_FLAG_CTX        (XIN_NUM_GT2_FLAG_LUMA_CTX + XIN_NUM_GT2_FLAG_CHROMA_CTX)
#define XIN_NUM_QT_CBF_CTX          (XIN_NUM_QT_CBF_Y_CTX + XIN_NUM_QT_CBF_U_CTX + XIN_NUM_QT_CBF_V_CTX)

typedef struct xin_cabac_est
{
    UINT32  sigCoeffGrpBits[XIN_NUM_SIG_COEFF_GROUP_CTX*2][2]; /*Flag = [0|1]*/
    UINT32  sigCoeffBits[XIN_NUM_SIG_FLAG_CTX][2]; /*Flag = [0|1]*/
    UINT32  lastXBits[2][XIN_MAX_LG_TU_SIZE + 1][XIN_NUM_LAST_XY_CTX];
    UINT32  lastYBits[2][XIN_MAX_LG_TU_SIZE + 1][XIN_NUM_LAST_XY_CTX];
    BOOL    lastXValid[2][XIN_MAX_LG_TU_SIZE + 1];
    BOOL    lastYValid[2][XIN_MAX_LG_TU_SIZE + 1];
    UINT32  greaterOneBits[XIN_NUM_GT1_FLAG_CTX][2]; /*Flag = [0|1]*/
    UINT32  parFlagBits[XIN_NUM_PAR_FLAG_CTX][2];
    UINT32  greaterTwoBits[XIN_NUM_GT2_FLAG_CTX][2]; /*Flag = [0|1]*/
    UINT32  blockCbpBits[XIN_NUM_QT_CBF_CTX][2]; /*Flag = [0|1]*/
    UINT32  blockRootCbpBits[2][2]; /*Flag = [0|1]*/
    BOOL    estValid[2];
} xin_cabac_est;

#endif
