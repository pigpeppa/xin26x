/***************************************************************************//**
 *
 * @file          h266_cabac_context.c
 * @brief         This file defines CABAC context related tables.
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
#include "xin_typedef.h"
#include "h26x_definition.h"
#include "h266_cabac_struct.h"
#include "memory.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h266_definition.h"
#include "basic_macro.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"

static const UINT8 InitSplitFlag[4][XIN_NUM_SPLIT_FLAG_CTX] =
{
    {  18,  27,  15,  18,  28,  45,  26,   7,  23, },
    {  11,  35,  53,  12,   6,  30,  13,  15,  31, },
    {  19,  28,  38,  27,  29,  38,  20,  30,  31, },
    {  12,  13,   8,   8,  13,  12,   5,   9,   9, },
};

static const UINT8 InitSplitQtFlag[4][XIN_NUM_SPLIT_QT_FLAG_CTX] =
{
    {  26,  36,  38,  18,  34,  21, },
    {  20,  14,  23,  18,  19,   6, },
    {  27,   6,  15,  25,  19,  37, },
    {   0,   8,   8,  12,  12,   8, },
};

static const UINT8 InitSplitHvFlag[4][XIN_NUM_SPLIT_HV_FLAG_CTX] =
{
    {  43,  42,  37,  42,  44, },
    {  43,  35,  37,  34,  52, },
    {  43,  42,  29,  27,  44, },
    {   9,   8,   9,   8,   5, },
};

static const UINT8 InitSplit12Flag[4][XIN_NUM_SPLIT_12_FLAG_CTX] =
{
    {  28,  29,  28,  29, },
    {  43,  37,  21,  22, },
    {  36,  45,  36,  45, },
    {  12,  13,  12,  13, },
};

static const UINT8 InitModeConsFlag[4][XIN_NUM_MODE_CONS_FLAG_CTX] =
{
    {  25,  20, },
    {  25,  12, },
    { CNU, CNU, },
    {   1,   0, },
};

static const UINT8 InitSkipFlag[4][XIN_NUM_SKIP_FLAG_CTX] =
{
    {  57,  60,  46, },
    {  57,  59,  45, },
    {   0,  26,  28, },
    {   5,   4,   8, },
};

static const UINT8 InitMergeFlag[4][XIN_NUM_MERGE_FLAG_CTX] =
{
    {   6, },
    {  21, },
    {  26, },
    {   4, },
};

static const UINT8 InitRegularMergeFlag[4][XIN_NUM_REGULAR_MERGE_FLAG_CTX] =
{
    {  46,  15, },
    {  38,   7, },
    { CNU, CNU, },
    {   5,   5, },
};

static const UINT8 InitMergeIdx[4][XIN_NUM_MERGE_IDX_CTX] =
{
    {  18, },
    {  20, },
    {  34, },
    {   4, },
};

static const UINT8 InitMmvdFlag[4][XIN_NUM_MMVD_FLAG_CTX] =
{
    {  25, },
    {  26, },
    { CNU, },
    {   4, },
};

static const UINT8 InitMmvdMergeIdx[4][XIN_NUM_MMVD_MERGE_IDX_CTX] =
{
    {  43, },
    {  43, },
    { CNU, },
    {  10, },
};

static const UINT8 InitMmvdStepMvpIdx[4][XIN_NUM_MMVD_STEP_MVP_IDX_CTX] =
{
    {  59, },
    {  60, },
    { CNU, },
    {   0, },
};

static const UINT8 InitPredMode[4][XIN_NUM_PRED_MODE_CTX] =
{
    {  40,  35, },
    {  40,  35, },
    { CNU, CNU, },
    {   5,   1, },
};

static const UINT8 InitMultiRefLineIdx[4][XIN_NUM_MULTI_REF_LINE_IDX_CTX] =
{
    {  25,  59, },
    {  25,  58, },
    {  25,  60, },
    {   5,   8, },
};

static const UINT8 InitIntraLumaMpmFlag[4][XIN_NUM_INTRA_LUMA_MPM_FLAG_CTX] =
{
    {  44, },
    {  36, },
    {  45, },
    {   6, },
};

static const UINT8 InitIntraLumaPlanarFlag[4][XIN_NUM_INTRA_LUMA_PLANAR_FLAG_CTX] =
{
    {  13,   6, },
    {  12,  20, },
    {  13,  28, },
    {   1,   5, },
};

static const UINT8 InitCclmModeFlag[4][XIN_NUM_CCLM_MODE_FLAG_CTX] =
{
    {  26, },
    {  34, },
    {  59, },
    {   4, },
};

static const UINT8 InitCclmModeIdx[4][XIN_NUM_CCLM_MODE_IDX_CTX] =
{
    {  27, },
    {  27, },
    {  27, },
    {   9, },
};

static const UINT8 InitIntraChromaPredMode[4][XIN_NUM_INTRA_CHROMA_PRED_MODE_CTX] =
{
    {  25, },
    {  25, },
    {  34, },
    {   5, },
};

static const UINT8 InitMipFlag[4][XIN_NUM_MIP_FLAG_CTX] =
{
    {  56,  57,  50,  26, },
    {  41,  57,  58,  26, },
    {  33,  49,  50,  25, },
    {   9,  10,   9,   6, },
};

static const UINT8 InitDeltaQP[4][XIN_NUM_DELTA_QP_CTX] =
{
    { CNU, CNU, },
    { CNU, CNU, },
    { CNU, CNU, },
    { DWS, DWS, },
};

static const UINT8 InitInterDir[4][XIN_NUM_INTER_DIR_CTX] =
{
    {  14,  13,   5,   4,   3,  40, },
    {   7,   6,   5,  12,   4,  40, },
    { CNU, CNU, CNU, CNU, CNU, CNU, },
    {   0,   0,   1,   4,   4,   0, },
};

static const UINT8 InitRefPic[4][XIN_NUM_REF_PIC_CTX] =
{
    {   5,  35, },
    {  20,  35, },
    { CNU, CNU, },
    {   0,   4, },
};

static const UINT8 InitSubblockMergeFlag[4][XIN_NUM_SUBBLOCK_MERGE_FLAG_CTX] =
{
    {  25,  58,  45, },
    {  48,  57,  44, },
    { CNU, CNU, CNU, },
    {   4,   4,   4, },
};

static const UINT8 InitAffineFlag[4][XIN_NUM_AFFINE_FLAG_CTX] =
{
    {  19,  13,   6, },
    {  12,  13,  14, },
    { CNU, CNU, CNU, },
    {   4,   0,   0, },
};

static const UINT8 InitAffineType[4][XIN_NUM_AFFINE_TYPE_CTX] =
{
    {  35, },
    {  35, },
    { CNU, },
    {   4, },
};

static const UINT8 InitAffMergeIdx[4][XIN_NUM_AFF_MERGE_IDX_CTX] =
{
    {   4, },
    {   5, },
    { CNU, },
    {   0, },
};

static const UINT8 InitBcwIdx[4][XIN_NUM_BCW_IDX_CTX] =
{
    {   5, },
    {   4, },
    { CNU, },
    {   1, },
};

static const UINT8 InitMvd[4][XIN_NUM_MVD_CTX] =
{
    {  51,  36, },
    {  44,  43, },
    {  14,  45, },
    {   9,   5, },
};

static const UINT8 InitBDPCMMode[4][XIN_NUM_BDPCM_MODE_CTX] =
{
    {  19,  21,   0,  28, },
    {  40,  36,   0,  13, },
    {  19,  35,   1,  27, },
    {   1,   4,   1,   0, },
};

static const UINT8 InitQtRootCbf[4][XIN_NUM_QT_ROOT_CBF_CTX] =
{
    {  12, },
    {   5, },
    {   6, },
    {   4, },
};

static const UINT8 InitQtCbfY[4][XIN_NUM_QT_CBF_Y_CTX] =
{
    {  15,   6,   5,  14, },
    {  23,   5,  20,   7, },
    {  15,  12,   5,   7, },
    {   5,   1,   8,   9, },
};

static const UINT8 InitQtCbfU[4][XIN_NUM_QT_CBF_U_CTX] =
{
    {  25,  37, },
    {  25,  28, },
    {  12,  21, },
    {   5,   0, },
};

static const UINT8 InitQtCbfV[4][XIN_NUM_QT_CBF_V_CTX] =
{
    {   9,  36,  45, },
    {  25,  29,  45, },
    {  33,  28,  36, },
    {   2,   1,   0, },
};

static const UINT8 InitSigCoeffGroupLuma[4][XIN_NUM_SIG_COEFF_GROUP_CTX] =
{
    {  25,  45, },
    {  25,  30, },
    {  18,  31, },
    {   8,   5, },
};

static const UINT8 InitSigCoeffGroupChroma[4][XIN_NUM_SIG_COEFF_GROUP_CTX] =
{
    {  25,  14, },
    {  25,  45, },
    {  25,  15, },
    {   5,   8, },
};

static const UINT8 InitSigFlagLuma[4][XIN_NUM_SIG_FLAG_LUMA_CTX] =
{

    {  17,  41,  49,  36,   1,  49,  50,  37,  48,  51,  58,  45, 26,  45,  53,  46,  49,  54,  61,  39,  35,  39,  39,  39, 19,  54,  39,  39,  50,  39,  39,  39,   0,  39,  39,  39,},
    {  17,  41,  42,  29,  25,  49,  43,  37,  33,  58,  51,  30, 19,  38,  38,  46,  34,  54,  54,  39,   6,  39,  39,  39, 19,  39,  54,  39,  19,  39,  39,  39,  56,  39,  39,  39,},
    {  25,  19,  28,  14,  25,  20,  29,  30,  19,  37,  30,  38, 11,  38,  46,  54,  27,  39,  39,  39,  44,  39,  39,  39, 18,  39,  39,  39,  27,  39,  39,  39,   0,  39,  39,  39,},
    {  12,   9,   9,  10,   9,   9,   9,  10,   8,   8,   8,  10,  9,  13,   8,   8,   8,   8,   8,   5,   8,   0,   0,   0,  8,   8,   8,   8,   8,   0,   4,   4,   0,   0,   0,   0,},

};

static const UINT8 InitSigFlagChroma[4][XIN_NUM_SIG_FLAG_CHROMA_CTX] =
{
    {   9,  49,  50,  36,  48,  59,  59,  38, 34,  45,  38,  31,  58,  39,  39,  39, 34,  38,  54,  39,  41,  39,  39,  39,},
    {  17,  34,  35,  21,  41,  59,  60,  38, 35,  45,  53,  54,  44,  39,  39,  39, 34,  38,  62,  39,  26,  39,  39,  39,},
    {  25,  27,  28,  37,  34,  53,  53,  46, 19,  46,  38,  39,  52,  39,  39,  39, 11,  39,  39,  39,  19,  39,  39,  39,},
    {  12,  12,   9,  13,   4,   5,   8,   9,  8,  12,  12,   8,   4,   0,   0,   0,  8,   8,   8,   8,   4,   0,   0,   0,},
};

static const UINT8 InitParFlagLuma[4][XIN_NUM_PAR_FLAG_LUMA_CTX] =
{
    {  33,  40,  25,  41,  26,  42,  25,  33,  26,  34,  27,  25,  41,  42,  42,  35,  33,  27,  35,  42,  43, },
    {  18,  17,  33,  18,  26,  42,  25,  33,  26,  42,  27,  25,  34,  42,  42,  35,  26,  27,  42,  20,  20, },
    {  33,  25,  18,  26,  34,  27,  25,  26,  19,  42,  35,  33,  19,  27,  35,  35,  34,  42,  20,  43,  20, },
    {   8,   9,  12,  13,  13,  13,  10,  13,  13,  13,  13,  13,  13,  13,  13,  13,  10,  13,  13,  13,  13, },
};

static const UINT8 InitParFlagChroma[4][XIN_NUM_PAR_FLAG_CHROMA_CTX] =
{
    {  33,  25,  26,  34,  19,  27,  33,  42,  43,  35,  43, },
    {  25,  25,  26,  11,  19,  27,  33,  42,  35,  35,  43, },
    {  33,  25,  26,  42,  19,  27,  26,  50,  35,  20,  43, },
    {   8,  12,  12,  12,  13,  13,  13,  13,  13,  13,  13, },
};

static const UINT8 InitGt2FlagLuma[4][XIN_NUM_GT2_FLAG_LUMA_CTX] =
{
    {  25,   0,   0,  17,  25,  26,   0,   9,  25,  33,  19,   0,  25,  33,  26,  20,  25,  33,  27,  35,  22, },
    {  17,   0,   1,  17,  25,  18,   0,   9,  25,  33,  34,   9,  25,  18,  26,  20,  25,  18,  19,  27,  29, },
    {  25,   1,  40,  25,  33,  11,  17,  25,  25,  18,   4,  17,  33,  26,  19,  13,  33,  19,  20,  28,  22, },
    {   1,   5,   9,   9,   9,   6,   5,   9,  10,  10,   9,   9,   9,   9,   9,   9,   6,   8,   9,   9,  10, },
};

static const UINT8 InitGt2FlagChroma[4][XIN_NUM_GT2_FLAG_CHROMA_CTX] =
{
    {  25,   1,  25,  33,  26,  12,  25,  33,  27,  28,  37, },
    {  17,   9,  25,  10,  18,   4,  17,  33,  19,  20,  29, },
    {  40,   9,  25,  18,  26,  35,  25,  26,  35,  28,  37, },
    {   1,   5,   8,   8,   9,   6,   6,   9,   8,   8,   9, },
};

static const UINT8 InitGt1FlagLuma[4][XIN_NUM_GT1_FLAG_LUMA_CTX] =
{
    {   0,   0,  33,  34,  35,  21,  25,  34,  35,  28,  29,  40,  42,  43,  29,  30,  49,  36,  37,  45,  38, },
    {   0,  17,  26,  19,  35,  21,  25,  34,  20,  28,  29,  33,  27,  28,  29,  22,  34,  28,  44,  37,  38, },
    {  25,  25,  11,  27,  20,  21,  33,  12,  28,  21,  22,  34,  28,  29,  29,  30,  36,  29,  45,  30,  23, },
    {   9,   5,  10,  13,  13,  10,   9,  10,  13,  13,  13,   9,  10,  10,  10,  13,   8,   9,  10,  10,  13, },
};

static const UINT8 InitGt1FlagChroma[4][XIN_NUM_GT1_FLAG_CHROMA_CTX] =
{
    {   0,  40,  34,  43,  36,  37,  57,  52,  45,  38,  46, },
    {   0,  25,  19,  20,  13,  14,  57,  44,  30,  30,  23, },
    {  40,  33,  27,  28,  21,  37,  36,  37,  45,  38,  46, },
    {   8,   8,   9,  12,  12,  10,   5,   9,   9,   9,  13, },
};

static const UINT8 InitLastXLuma[4][XIN_NUM_LAST_XY_LUMA_CTX] =
{
    {   6,   6,  12,  14,   6,   4,  14,   7,   6,   4,  29,   7,   6,   6,  12,  28,   7,  13,  13,  35, },
    {   6,  13,  12,   6,   6,  12,  14,  14,  13,  12,  29,   7,   6,  13,  36,  28,  14,  13,   5,  26, },
    {  13,   5,   4,  21,  14,   4,   6,  14,  21,  11,  14,   7,  14,   5,  11,  21,  30,  22,  13,  42, },
    {   8,   5,   4,   5,   4,   4,   5,   4,   1,   0,   4,   1,   0,   0,   0,   0,   1,   0,   0,   0, },
};

static const UINT8 InitLastXChroma[4][XIN_NUM_LAST_XY_CHROMA_CTX] =
{
    {  19,   5,   4, },
    {  12,   4,  18, },
    {  12,   4,   3, },
    {   5,   4,   4, },
};

static const UINT8 InitLastYLuma[4][XIN_NUM_LAST_XY_LUMA_CTX] =
{
    {   5,   5,  20,  13,  13,  19,  21,   6,  12,  12,  14,  14,   5,   4,  12,  13,   7,  13,  12,  41, },
    {   5,   5,  12,   6,   6,   4,   6,  14,   5,  12,  14,   7,  13,   5,  13,  21,  14,  20,  12,  34, },
    {  13,   5,   4,   6,  13,  11,  14,   6,   5,   3,  14,  22,   6,   4,   3,   6,  22,  29,  20,  34, },
    {   8,   5,   8,   5,   5,   4,   5,   5,   4,   0,   5,   4,   1,   0,   0,   1,   4,   0,   0,   0, },
};

static const UINT8 InitLastYChroma[4][XIN_NUM_LAST_XY_CHROMA_CTX] =
{
    {  11,   5,  27, },
    {  11,   4,  18, },
    {  12,   4,   3, },
    {   6,   5,   5, },
};

static const UINT8 InitMVPIdx[4][XIN_NUM_MVP_IDX_CTX] =
{
    {  34, },
    {  34, },
    {  42, },
    {  12, },
};

static const UINT8 InitSmvdFlag[4][XIN_NUM_SMVD_FLAG_CTX] =
{
    {  28, },
    {  28, },
    { CNU, },
    {   5, },
};

static const UINT8 InitSaoMergeFlag[4][XIN_NUM_SAO_MERGE_FLAG_CTX] =
{
    {   2, },
    {  60, },
    {  60, },
    {   0, },
};

static const UINT8 InitSaoTypeIdx[4][XIN_NUM_SAO_TYPE_IDX_CTX] =
{
    {   2, },
    {   5, },
    {  13, },
    {   4, },
};

static const UINT8 InitTransquantBypassFlag[4][XIN_NUM_TRANSQUANT_BYPASS_FLAG_CTX] =
{
    { CNU, },
    { CNU, },
    { CNU, },
    { DWS, },
};

static const UINT8 InitLFNSTIdx[4][XIN_NUM_LFNST_IDX_CTX] =
{
    {  52,  37,  27, },
    {  37,  45,  27, },
    {  28,  52,  42, },
    {   9,   9,  10, },
};

static const UINT8 InitPLTFlag[4][XIN_NUM_PLT_IDX_CTX] =
{
    {  17, },
    {   0, },
    {  25, },
    {   1, },
};

static const UINT8 InitRotationFlag[4][XIN_NUM_ROTATION_FLAG_CTX] =
{
    {  35, },
    {  42, },
    {  42, },
    {   5, },
};

static const UINT8 InitRunTypeFlag[4][XIN_NUM_RUN_TYPE_FLAG_CTX] =
{
    {  50, },
    {  59, },
    {  42, },
    {   9, },
};

static const UINT8 InitIdxRunModel[4][XIN_NUM_IDX_RUN_MODEL_CTX] =
{
    {  58,  45,  45,  30,  38, },
    {  51,  30,  30,  38,  23, },
    {  50,  37,  45,  30,  46, },
    {   9,   6,   9,  10,   5, },
};

static const UINT8 InitCopyRunModel[4][XIN_NUM_COPY_RUN_MODEL_CTX] =
{
    {  45,  38,  46, },
    {  38,  53,  46, },
    {  45,  38,  46, },
    {   0,   9,   5, },
};

static const UINT8 InitRdpcmFlag[4][XIN_NUM_RDPCM_FLAG_CTX] =
{
    { CNU, CNU, },
    { CNU, CNU, },
    { CNU, CNU, },
    { DWS, DWS, },
};

static const UINT8 InitRdpcmDir[4][XIN_NUM_RDPCM_DIR_CTX] =
{
    { CNU, CNU, },
    { CNU, CNU, },
    { CNU, CNU, },
    { DWS, DWS, },
};

static const UINT8 InitMTSIndex[4][XIN_NUM_MTS_INDEX_CTX] =
{
    {  29, CNU, CNU, CNU, CNU, CNU,  33,  18,  27,   0, CNU, },
    {  29, CNU, CNU, CNU, CNU, CNU,  18,  33,  27,   0, CNU, },
    {  20, CNU, CNU, CNU, CNU, CNU,  33,   0,  42,   0, CNU, },
    {   8, DWS, DWS, DWS, DWS, DWS,   1,   0,   9,   0, DWS, },
};

static const UINT8 InitISPMode[4][XIN_NUM_ISP_MODE_CTX] =
{
    {  48,  43, },
    {  33,  43, },
    {  33,  43, },
    {   9,   2, },
};

static const UINT8 InitSbtFlag[4][XIN_NUM_SBT_FLAG_CTX] =
{
    {  41,  57, },
    {  56,  57, },
    { CNU, CNU, },
    {   1,   5, },
};

static const UINT8 InitSbtQuadFlag[4][XIN_NUM_QUAD_FLAG_CTX] =
{
    {  42, },
    {  42, },
    { CNU, },
    {  10, },
};

static const UINT8 InitSbtHorFlag[4][XIN_NUM_SBT_HOR_FLAG_CTX] =
{
    {  35,  51,  27, },
    {  20,  43,  12, },
    { CNU, CNU, CNU, },
    {   8,   4,   1, },
};

static const UINT8 InitSbtPosFlag[4][XIN_NUM_SBT_POS_FLAG_CTX] =
{
    {  28, },
    {  28, },
    { CNU, },
    {  13, },
};

static const UINT8 InitCrossCompPred[4][XIN_NUM_CROSS_COMP_PRED_CTX] =
{
    { CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, },
    { CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, },
    { CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, CNU, },
    { DWS, DWS, DWS, DWS, DWS, DWS, DWS, DWS, DWS, DWS, },
};

static const UINT8 InitChromaQpAdjFlag[4][XIN_NUM_CHROMA_QP_ADJ_FLAG_CTX] =
{
    { CNU, },
    { CNU, },
    { CNU, },
    { DWS, },
};

static const UINT8 InitChromaQpAdjIdc[4][XIN_NUM_CHROMA_QP_ADJ_IDC_CTX] =
{
    { CNU, },
    { CNU, },
    { CNU, },
    { DWS, },
};

static const UINT8 InitImvFlag[4][XIN_NUM_IMV_FLAG_CTX] =
{
    {  59,  26,  50,  60,  38, },
    {  59,  48,  58,  60,  60, },
    { CNU,  34, CNU, CNU, CNU, },
    {   0,   5,   0,   0,   4, },
};

static const UINT8 InitMHIntraFlag[4][XIN_NUM_MH_INTRA_FLAG_CTX] =
{
    {  58, },
    {  58, },
    { CNU, },
    {   1, },
};

static const UINT8 InitIBCFlag[4][XIN_NUM_IBC_FLAG_CTX] =
{
    {   0,  43,  30, },
    {   0,  42,  37, },
    {  17,  27,  36, },
    {   1,   5,   8, },
};

static const UINT8 InitJointCbCrFlag[4][XIN_NUM_JOINT_CB_CR_FLAG_CTX] =
{
    {  42,  43,  52, },
    {  27,  36,  45, },
    {  12,  21,  35, },
    {   1,   1,   0, },
};

static const UINT8 InitTransformSkipFlag[4][XIN_NUM_TRANSFORM_SKIP_FLAG_CTX] =
{
    {  25,  17, },
    {  25,   9, },
    {  25,   9, },
    {   1,   1, },
};

static const UINT8 InitTsSigCoeffGroup[4][XIN_NUM_TS_SIG_COEFF_GROUP_CTX] =
{
    {  18,  35,  45, },
    {  18,  12,  29, },
    {  18,  20,  38, },
    {   5,   8,   8, },
};

static const UINT8 InitTsSigFlag[4][XIN_NUM_TS_SIG_FLAG_CTX] =
{
    {  25,  50,  37, },
    {  40,  35,  44, },
    {  25,  28,  38, },
    {  13,  13,   8, },
};

static const UINT8 InitTsParFlag[4][XIN_NUM_TS_PAR_FLAG_CTX] =
{
    {  11, },
    {   3, },
    {  11, },
    {   6, },
};

static const UINT8 InitTsGtxFlag[4][XIN_NUM_TS_GTX_FLAG_CTX] =
{
    { CNU,   3,   4,   4,   5, },
    { CNU,   2,  10,   3,   3, },
    { CNU,  10,   3,   3,   3, },
    { DWS,   1,   1,   1,   1, },
};

static const UINT8 InitTsGt1Flag[4][XIN_NUM_TS_GT1_FLAG_CTX] =
{
    {  19,  11,   4,   6, },
    {  18,  11,   4,  28, },
    {  11,   5,   5,  14, },
    {   4,   2,   1,   6, },
};

static const UINT8 InitTsResidualSign[4][XIN_NUM_TS_RESIDUAL_SIGN_CTX] =
{
    {  35,  25,  46,  28,  33,  38, },
    {   5,  10,  53,  43,  25,  46, },
    {  12,  17,  46,  28,  25,  46, },
    {   1,   4,   4,   5,   8,   8, },
};

static const UINT8 InitCtbAlfFlag[4][XIN_NUM_CTB_ALF_FLAG_CTX] =
{
    {  33,  52,  46,  25,  61,  54,  25,  61,  54, },
    {  13,  23,  46,   4,  61,  54,  19,  46,  54, },
    {  62,  39,  39,  54,  39,  39,  31,  39,  39, },
    {   0,   0,   0,   4,   0,   0,   1,   0,   0, },
};

static const UINT8 InitCtbAlfAlt[4][XIN_NUM_CTB_ALF_ALT_CTX] =
{
    {  11,  26, },
    {  20,  12, },
    {  11,  11, },
    {   0,   0, },
};

static const UINT8 InitAlfUseTemporalFilt[4][XIN_NUM_ALF_USE_TEMPO_FLT_CTX] =
{
    {  46, },
    {  46, },
    {  46, },
    {   0, },
};

static const UINT8 InitCcAlfFltCtrlFlag[4][XIN_NUM_CC_ALF_FLT_CTRL_FLAG_CTX] = 
{
    {  25,  35,  38,  25,  28,  38, },
    {  18,  21,  38,  18,  21,  38, },
    {  18,  30,  31,  18,  30,  31, },
    {   4,   1,   4,   4,   1,   4, },
};

static void InitCabacContextBuffer (
    xin_prob_model *proModel,
    SINT32         qp,
    const UINT8    *contextModel,
    const UINT8    *rateInit,
    UINT32         size)
{
    UINT32 idx;
    SINT32 slope;
    SINT32 offset;
    SINT32 rate0;
    SINT32 rate1;
    SINT32 initState;
    UINT32 p1;

    for (idx = 0; idx < size; idx++)
    {
        slope     = (contextModel[idx] >> 3) - 4;
        offset    = ((contextModel[idx] & 7) * 18) + 1;
        initState = ((slope * (qp - 16)) >> 1) + offset;
        initState = XIN_MIN (XIN_MAX (1, initState), 127);
        p1        = (initState << 8);
        rate0     = 2 + ((rateInit[idx] >> 2) & 3);
        rate1     = 3 + rate0 + (rateInit[idx] & 3);

        proModel[idx].state[0] = p1 & XIN_CABAC_STATE_MASK_0;
        proModel[idx].state[1] = p1 & XIN_CABAC_STATE_MASK_1;
        proModel[idx].rate     = (UINT8)(16 * rate0 + rate1);
    }

}

void Xin266InitCabacContext (
    xin_prob_model *proModel,
    SINT32         sliceIdx,
    SINT32         qpIdx)
{
    InitCabacContextBuffer (
        proModel + XIN_CO_SPLIT_FLAG,
        qpIdx,
        InitSplitFlag[sliceIdx],
        InitSplitFlag[3],
        XIN_NUM_SPLIT_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SPLIT_QT_FLAG,
        qpIdx,
        InitSplitQtFlag[sliceIdx],
        InitSplitQtFlag[3],
        XIN_NUM_SPLIT_QT_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SPLIT_HV_FLAG,
        qpIdx,
        InitSplitHvFlag[sliceIdx],
        InitSplitHvFlag[3],
        XIN_NUM_SPLIT_HV_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SPLIT_12_FLAG,
        qpIdx,
        InitSplit12Flag[sliceIdx],
        InitSplit12Flag[3],
        XIN_NUM_SPLIT_12_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MODE_CONS_FLAG,
        qpIdx,
        InitModeConsFlag[sliceIdx],
        InitModeConsFlag[3],
        XIN_NUM_MODE_CONS_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SKIP_FLAG,
        qpIdx,
        InitSkipFlag[sliceIdx],
        InitSkipFlag[3],
        XIN_NUM_SKIP_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MERGE_FLAG,
        qpIdx,
        InitMergeFlag[sliceIdx],
        InitMergeFlag[3],
        XIN_NUM_MERGE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_REGULAR_MERGE_FLAG,
        qpIdx,
        InitRegularMergeFlag[sliceIdx],
        InitRegularMergeFlag[3],
        XIN_NUM_REGULAR_MERGE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MERGE_IDX,
        qpIdx,
        InitMergeIdx[sliceIdx],
        InitMergeIdx[3],
        XIN_NUM_MERGE_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MMVD_FLAG_CTX,
        qpIdx,
        InitMmvdFlag[sliceIdx],
        InitMmvdFlag[3],
        XIN_NUM_MMVD_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MMVD_MERGE_FLAG,
        qpIdx,
        InitMmvdFlag[sliceIdx],
        InitMmvdFlag[3],
        XIN_NUM_MMVD_MERGE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MMVD_MERGE_IDX,
        qpIdx,
        InitMmvdMergeIdx[sliceIdx],
        InitMmvdMergeIdx[3],
        XIN_NUM_MMVD_MERGE_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MMVD_STEP_MVP_IDX,
        qpIdx,
        InitMmvdStepMvpIdx[sliceIdx],
        InitMmvdStepMvpIdx[3],
        XIN_NUM_MMVD_STEP_MVP_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_PRED_MODE,
        qpIdx,
        InitPredMode[sliceIdx],
        InitPredMode[3],
        XIN_NUM_PRED_MODE_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MULTI_REF_LINE_IDX,
        qpIdx,
        InitMultiRefLineIdx[sliceIdx],
        InitMultiRefLineIdx[3],
        XIN_NUM_MULTI_REF_LINE_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_INTRA_LUMA_MPM_FLAG,
        qpIdx,
        InitIntraLumaMpmFlag[sliceIdx],
        InitIntraLumaMpmFlag[3],
        XIN_NUM_INTRA_LUMA_MPM_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_INTRA_LUMA_PLANAR_FLAG,
        qpIdx,
        InitIntraLumaPlanarFlag[sliceIdx],
        InitIntraLumaPlanarFlag[3],
        XIN_NUM_INTRA_LUMA_PLANAR_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_CCLM_MODE_FLAG,
        qpIdx,
        InitCclmModeFlag[sliceIdx],
        InitCclmModeFlag[3],
        XIN_NUM_CCLM_MODE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_CCLM_MODE_IDX,
        qpIdx,
        InitCclmModeIdx[sliceIdx],
        InitCclmModeIdx[3],
        XIN_NUM_CCLM_MODE_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_INTRA_CHROMA_PRED_MODE,
        qpIdx,
        InitIntraChromaPredMode[sliceIdx],
        InitIntraChromaPredMode[3],
        XIN_NUM_INTRA_CHROMA_PRED_MODE_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MIP_FLAG,
        qpIdx,
        InitMipFlag[sliceIdx],
        InitMipFlag[3],
        XIN_NUM_MIP_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_DELTA_QP,
        qpIdx,
        InitDeltaQP[sliceIdx],
        InitDeltaQP[3],
        XIN_NUM_DELTA_QP_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_INTER_DIR,
        qpIdx,
        InitInterDir[sliceIdx],
        InitInterDir[3],
        XIN_NUM_INTER_DIR_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_REF_PIC,
        qpIdx,
        InitRefPic[sliceIdx],
        InitRefPic[3],
        XIN_NUM_REF_PIC_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SUBBLOCK_MERGE_FLAG,
        qpIdx,
        InitSubblockMergeFlag[sliceIdx],
        InitSubblockMergeFlag[3],
        XIN_NUM_SUBBLOCK_MERGE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_AFFINE_FLAG,
        qpIdx,
        InitAffineFlag[sliceIdx],
        InitAffineFlag[3],
        XIN_NUM_AFFINE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_AFFINE_TYPE,
        qpIdx,
        InitAffineType[sliceIdx],
        InitAffineType[3],
        XIN_NUM_AFFINE_TYPE_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_AFF_MERGE_IDX,
        qpIdx,
        InitAffMergeIdx[sliceIdx],
        InitAffMergeIdx[3],
        XIN_NUM_AFF_MERGE_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_BCW_IDX,
        qpIdx,
        InitBcwIdx[sliceIdx],
        InitBcwIdx[3],
        XIN_NUM_BCW_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MVD,
        qpIdx,
        InitMvd[sliceIdx],
        InitMvd[3],
        XIN_NUM_MVD_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_BDPCM_MODE,
        qpIdx,
        InitBDPCMMode[sliceIdx],
        InitBDPCMMode[3],
        XIN_NUM_BDPCM_MODE_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_QT_ROOT_CBF,
        qpIdx,
        InitQtRootCbf[sliceIdx],
        InitQtRootCbf[3],
        XIN_NUM_QT_ROOT_CBF_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_QT_CBF_Y,
        qpIdx,
        InitQtCbfY[sliceIdx],
        InitQtCbfY[3],
        XIN_NUM_QT_CBF_Y_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_QT_CBF_U,
        qpIdx,
        InitQtCbfU[sliceIdx],
        InitQtCbfU[3],
        XIN_NUM_QT_CBF_U_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_QT_CBF_V,
        qpIdx,
        InitQtCbfV[sliceIdx],
        InitQtCbfV[3],
        XIN_NUM_QT_CBF_V_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SIG_COEFF_GROUP_LUMA,
        qpIdx,
        InitSigCoeffGroupLuma[sliceIdx],
        InitSigCoeffGroupLuma[3],
        XIN_NUM_SIG_COEFF_GROUP_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SIG_COEFF_GROUP_CHROMA,
        qpIdx,
        InitSigCoeffGroupChroma[sliceIdx],
        InitSigCoeffGroupChroma[3],
        XIN_NUM_SIG_COEFF_GROUP_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SIG_FLAG_LUMA,
        qpIdx,
        InitSigFlagLuma[sliceIdx],
        InitSigFlagLuma[3],
        XIN_NUM_SIG_FLAG_LUMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SIG_FLAG_CHROMA,
        qpIdx,
        InitSigFlagChroma[sliceIdx],
        InitSigFlagChroma[3],
        XIN_NUM_SIG_FLAG_CHROMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_PAR_FLAG_LUMA,
        qpIdx,
        InitParFlagLuma[sliceIdx],
        InitParFlagLuma[3],
        XIN_NUM_PAR_FLAG_LUMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_PAR_FLAG_CHROMA,
        qpIdx,
        InitParFlagChroma[sliceIdx],
        InitParFlagChroma[3],
        XIN_NUM_PAR_FLAG_CHROMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_GT2_FLAG_LUMA,
        qpIdx,
        InitGt2FlagLuma[sliceIdx],
        InitGt2FlagLuma[3],
        XIN_NUM_GT2_FLAG_LUMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_GT2_FLAG_CHROMA,
        qpIdx,
        InitGt2FlagChroma[sliceIdx],
        InitGt2FlagChroma[3],
        XIN_NUM_GT2_FLAG_CHROMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_GT1_FLAG_LUMA,
        qpIdx,
        InitGt1FlagLuma[sliceIdx],
        InitGt1FlagLuma[3],
        XIN_NUM_GT1_FLAG_LUMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_GT1_FLAG_CHROMA,
        qpIdx,
        InitGt1FlagChroma[sliceIdx],
        InitGt1FlagChroma[3],
        XIN_NUM_GT1_FLAG_CHROMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_LAST_X_LUMA,
        qpIdx,
        InitLastXLuma[sliceIdx],
        InitLastXLuma[3],
        XIN_NUM_LAST_XY_LUMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_LAST_X_CHROMA,
        qpIdx,
        InitLastXChroma[sliceIdx],
        InitLastXChroma[3],
        XIN_NUM_LAST_XY_CHROMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_LAST_Y_LUMA,
        qpIdx,
        InitLastYLuma[sliceIdx],
        InitLastYLuma[3],
        XIN_NUM_LAST_XY_LUMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_LAST_Y_CHROMA,
        qpIdx,
        InitLastYChroma[sliceIdx],
        InitLastYChroma[3],
        XIN_NUM_LAST_XY_CHROMA_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_MVP_IDX,
        qpIdx,
        InitMVPIdx[sliceIdx],
        InitMVPIdx[3],
        XIN_NUM_MVP_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SMVD_FLAG,
        qpIdx,
        InitSmvdFlag[sliceIdx],
        InitSmvdFlag[3],
        XIN_NUM_SMVD_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SAO_MERGE_FLAG,
        qpIdx,
        InitSaoMergeFlag[sliceIdx],
        InitSaoMergeFlag[3],
        XIN_NUM_SAO_MERGE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_SAO_TYPE_IDX,
        qpIdx,
        InitSaoTypeIdx[sliceIdx],
        InitSaoTypeIdx[3],
        XIN_NUM_SAO_TYPE_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TRANSQUANT_BYPASS_FLAG,
        qpIdx,
        InitTransquantBypassFlag[sliceIdx],
        InitTransquantBypassFlag[3],
        XIN_NUM_TRANSQUANT_BYPASS_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_LFNST_IDX,
        qpIdx,
        InitLFNSTIdx[sliceIdx],
        InitLFNSTIdx[3],
        XIN_NUM_LFNST_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_PLT_IDX,
        qpIdx,
        InitPLTFlag[sliceIdx],
        InitPLTFlag[3],
        XIN_NUM_PLT_IDX_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_ROTATION_FLAG,
        qpIdx,
        InitRotationFlag[sliceIdx],
        InitRotationFlag[3],
        XIN_NUM_ROTATION_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_RUN_TYPE_FLAG,
        qpIdx,
        InitRunTypeFlag[sliceIdx],
        InitRunTypeFlag[3],
        XIN_NUM_RUN_TYPE_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TRANSFORM_SKIP_FLAG,
        qpIdx,
        InitTransformSkipFlag[sliceIdx],
        InitTransformSkipFlag[3],
        XIN_NUM_TRANSFORM_SKIP_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TS_SIG_COEFF_GROUP,
        qpIdx,
        InitTsSigCoeffGroup[sliceIdx],
        InitTsSigCoeffGroup[3],
        XIN_NUM_TS_SIG_COEFF_GROUP_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TS_SIG_FLAG,
        qpIdx,
        InitTsSigFlag[sliceIdx],
        InitTsSigFlag[3],
        XIN_NUM_TS_SIG_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TS_PAR_FLAG,
        qpIdx,
        InitTsParFlag[sliceIdx],
        InitTsParFlag[3],
        XIN_NUM_TS_PAR_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TS_GTX_FLAG,
        qpIdx,
        InitTsGtxFlag[sliceIdx],
        InitTsGtxFlag[3],
        XIN_NUM_TS_GTX_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TS_GT1_FLAG,
        qpIdx,
        InitTsGt1Flag[sliceIdx],
        InitTsGt1Flag[3],
        XIN_NUM_TS_GT1_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_TS_RESIDUAL_SIGN,
        qpIdx,
        InitTsResidualSign[sliceIdx],
        InitTsResidualSign[3],
        XIN_NUM_TS_RESIDUAL_SIGN_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_IMV_FLAG,
        qpIdx,
        InitImvFlag[sliceIdx],
        InitImvFlag[3],
        XIN_NUM_IMV_FLAG_CTX);
    
    InitCabacContextBuffer (
        proModel + XIN_CO_CTB_ALF_FLAG,
        qpIdx,
        InitCtbAlfFlag[sliceIdx],
        InitCtbAlfFlag[3],
        XIN_NUM_CTB_ALF_FLAG_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_CTB_ALF_ALT,
        qpIdx,
        InitCtbAlfAlt[sliceIdx],
        InitCtbAlfAlt[3],
        XIN_NUM_CTB_ALF_ALT_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_USE_TEMPO_FLT,
        qpIdx,
        InitAlfUseTemporalFilt[sliceIdx],
        InitAlfUseTemporalFilt[3],
        XIN_NUM_ALF_USE_TEMPO_FLT_CTX);

    InitCabacContextBuffer (
        proModel + XIN_CO_CC_ALF_FLT_CTRL_FLAG,
        qpIdx,
        InitCcAlfFltCtrlFlag[sliceIdx],
        InitCcAlfFltCtrlFlag[3],
        XIN_NUM_CC_ALF_FLT_CTRL_FLAG_CTX);

}

void Xin266CabacContextInit (
    xin_cabac_context *cabacSet,
    xin_prob_model    *context,
    SINT32            sliceIdx,
    SINT32            qpIdx,
    BOOL              resetContext)
{
    if (resetContext)
    {
        context += (sliceIdx*XIN_QP_NUM + qpIdx)*XIN_NUM_OF_CTX;
        memcpy(cabacSet->context, context, sizeof(xin_prob_model)*XIN_NUM_OF_CTX);
    }

    cabacSet->cabac.range    = 510;
    cabacSet->cabac.low      = 0;
    cabacSet->cabac.bitsLeft = 23;

    cabacSet->cabac.bufferedByte     = 0xff;
    cabacSet->cabac.numBufferedBytes = 0;


}


