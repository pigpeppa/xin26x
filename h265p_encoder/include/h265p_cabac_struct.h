/***************************************************************************//**
 *
 * @file          h265p_cabac_struct.h
 * @brief         This file contains CABAC related data structure and definitions.
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
#ifndef _h265p_cabac_struct_h_
#define _h265p_cabac_struct_h_

#include "h265p_cabac_context.h"
#include "h265p_bit_stream_struct.h"
#include "xin_video_common.h"

#define XIN_CDF_SIZE(x)                 ((x) + 1)
#define XIN_MIN_PROB                    4
#define XIN_PROB_RATE_SHIFT             9
#define XIN_RATE_LITERAL(n)             ((n) * (1 << XIN_PROB_RATE_SHIFT))

#define XIN_SIG_COEF_CONTEXTS_2D        26
#define XIN_SIG_COEF_CONTEXTS_1D        16
#define XIN_BR_CDF_SIZE                 4
#define XIN_INTER_COMPOUND_MODE_NUM     (1 + XIN_NEW_NEWMV - XIN_NEAREST_NEARESTMV)
#define XIN_DIRECTIONAL_MODE_NUM        8
#define XIN_MAX_ANGLE_DELTA             3
#define XIN_BLOCK_SIZE_GROUP_NUM        4
#define XIN_SEG_TREE_PROB_NUM           (XIN_MAX_SEGMENT_NUM - 1)
#define XIN_SPATIAL_PREDICTION_PROB_NUM 3
#define XIN_DELTA_LF_PROB_NUM           (XIN_DELTA_LF_SMALL)

#define XIN_TOKEN_CDF_Q_CTXS            4
#define XIN_PARTITION_PLOFFSET          4

#define XIN_NUM_PARTITION_CTX           20
#define XIN_NUM_TXB_SKIP_CTX            13
#define XIN_NUM_EOB_COEF_CTX            9
#define XIN_NUM_DC_SIGN_CTX             3
#define XIN_NUM_SIG_COEF_EOB_CTX        4
#define XIN_NUM_SIG_COEF_CTX            (XIN_SIG_COEF_CONTEXTS_2D + XIN_SIG_COEF_CONTEXTS_1D)
#define XIN_NUM_LEVEL_CTX               21
#define XIN_NUM_NEWMV_MODE_CTX          6
#define XIN_NUM_GLOBALMV_MODE_CTX       2
#define XIN_NUM_REFMV_MODE_CTX          6
#define XIN_NUM_DRL_MODE_CTX            3
#define XIN_NUM_COMP_NEWMV_CTX          5
#define XIN_NUM_INTER_MODE_CTX          8
#define XIN_NUM_PALATTE_BSIZE_CTX       7
#define XIN_NUM_PALETTE_COLOR_IDX_CTX   5
#define XIN_NUM_PALETTE_UV_MODE_CTX     2
#define XIN_NUM_COMP_INTER_CTX          5
#define XIN_NUM_REF_CTX                 3
#define XIN_NUM_COMP_REF_TYPE_CTX       5
#define XIN_NUM_UNI_COMP_REF_CTX        3
#define XIN_NUM_TXFM_PARTITION_CTX      ((XIN_TX_SIZE_TYPE - XIN_TX_8X8) * 6 - 3)
#define XIN_NUM_COMP_INDEX_CTX          6
#define XIN_NUM_COMP_GROUP_IDX_CTX      6
#define XIN_NUM_SKIP_CTX                3
#define XIN_NUM_SKIP_MODE_CTX           3
#define XIN_NUM_INTRA_INTER_CTX         4
#define XIN_NUM_KF_MODE_CTX             5
#define XIN_NUM_TX_SIZE_CTX             3
#define XIN_NUM_SEG_TEMPORAL_PRED_CTX   3
#define XIN_NUM_SWITCHABLE_FILTER_CTX   ((XIN_SWITCHABLE_FILTER_NUM + 1) * 4)
#define XIN_NUM_CFL_ALPHA_CTX           (XIN_CFL_JOINT_SIGN_NUM + 1 - XIN_CFL_SIGN_NUM)
#define XIN_NUM_PALETTE_COLOR_INDEX_CTX 5
#define XIN_NUM_PALETTE_Y_MODE_CTX      3

typedef enum xin_tx_class
{
    XIN_TX_CLASS_2D = 0,
    XIN_TX_CLASS_HORIZ = 1,
    XIN_TX_CLASS_VERT = 2,
    XIN_TX_CLASSE_NUM = 3,
} xin_tx_class;

// Object used to write the unescaped CABAC bitstream.
typedef struct xin_cabac_struct
{
    xin_bs_struct bitstream;
    SINT32        low;
    UINT32        range;
    UINT32        bufferedByte;
    SINT32        numBufferedBytes;
    SINT32        bitCnt;
    UINT16        *preBufBase;
    UINT16        *preBuf;
    SINT32        preBufSize;
} xin_cabac_struct;

typedef struct xin_scan_order
{
    const SINT16 *scan;
    const SINT16 *iscan;
} xin_scan_order;

typedef struct xin_cdf_prob
{
    UINT16 txbSkipCdf[XIN_TX_SIZE_NUM][XIN_NUM_TXB_SKIP_CTX][XIN_CDF_SIZE(2)];
    UINT16 eobExtraCdf[XIN_TX_SIZE_NUM][PLANE_TYPE][XIN_NUM_EOB_COEF_CTX][XIN_CDF_SIZE(2)];
    UINT16 dcSignCdf[PLANE_TYPE][XIN_NUM_DC_SIGN_CTX][XIN_CDF_SIZE(2)];
    UINT16 eobFlagCdf16[PLANE_TYPE][2][XIN_CDF_SIZE(5)];
    UINT16 eobFlagCdf32[PLANE_TYPE][2][XIN_CDF_SIZE(6)];
    UINT16 eobFlagCdf64[PLANE_TYPE][2][XIN_CDF_SIZE(7)];
    UINT16 eobFlagCdf128[PLANE_TYPE][2][XIN_CDF_SIZE(8)];
    UINT16 eobFlagCdf256[PLANE_TYPE][2][XIN_CDF_SIZE(9)];
    UINT16 eobFlagCdf512[PLANE_TYPE][2][XIN_CDF_SIZE(10)];
    UINT16 eobFlagCdf1024[PLANE_TYPE][2][XIN_CDF_SIZE(11)];
    UINT16 coefBaseEobCdf[XIN_TX_SIZE_NUM][PLANE_TYPE][XIN_NUM_SIG_COEF_EOB_CTX][XIN_CDF_SIZE(3)];
    UINT16 coefBaseCdf[XIN_TX_SIZE_NUM][PLANE_TYPE][XIN_NUM_SIG_COEF_CTX][XIN_CDF_SIZE(4)];
    UINT16 coefBrCdf[XIN_TX_SIZE_NUM][PLANE_TYPE][XIN_NUM_LEVEL_CTX][XIN_CDF_SIZE(XIN_BR_CDF_SIZE)];

    UINT16 newMvCdf[XIN_NUM_NEWMV_MODE_CTX][XIN_CDF_SIZE(2)];
    UINT16 zeroMvCdf[XIN_NUM_GLOBALMV_MODE_CTX][XIN_CDF_SIZE(2)];
    UINT16 refMvCdf[XIN_NUM_REFMV_MODE_CTX][XIN_CDF_SIZE(2)];
    UINT16 drlCdf[XIN_NUM_DRL_MODE_CTX][XIN_CDF_SIZE(2)];

    UINT16 interCompoundModeCdf[XIN_NUM_INTER_MODE_CTX][XIN_CDF_SIZE(XIN_INTER_COMPOUND_MODE_NUM)];
    UINT16 compoundTypeCdf[XIN_BLOCK_SIZE_NUM][XIN_CDF_SIZE(XIN_MASKED_COMPOUND_TYPE_NUM)];
    UINT16 wedgeIdxCdf[XIN_BLOCK_SIZE_NUM][XIN_CDF_SIZE(16)];
    UINT16 interIntraCdf[XIN_BLOCK_SIZE_GROUP_NUM][XIN_CDF_SIZE(2)];
    UINT16 wedgeInterIntraCdf[XIN_BLOCK_SIZE_NUM][XIN_CDF_SIZE(2)];
    UINT16 interIntraModeCdf[XIN_BLOCK_SIZE_GROUP_NUM][XIN_CDF_SIZE(XIN_INTERINTRA_MODE_NUM)];
    UINT16 motionModeCdf[XIN_BLOCK_SIZE_NUM][XIN_CDF_SIZE(XIN_MOTION_MODE_NUM)];
    UINT16 obmcCdf[XIN_BLOCK_SIZE_NUM][XIN_CDF_SIZE(2)];
    UINT16 paletteYSizeCdf[XIN_NUM_PALATTE_BSIZE_CTX][XIN_CDF_SIZE(XIN_PALETTE_SIZE_NUM)];
    UINT16 paletteUvSizeCdf[XIN_NUM_PALATTE_BSIZE_CTX][XIN_CDF_SIZE(XIN_PALETTE_SIZE_NUM)];
    UINT16 paletteYColorIndexCdf[XIN_PALETTE_SIZE_NUM][XIN_NUM_PALETTE_COLOR_IDX_CTX][XIN_CDF_SIZE(XIN_PALETTE_COLOR_NUM)];
    UINT16 paletteUvColorIndexCdf[XIN_PALETTE_SIZE_NUM][XIN_NUM_PALETTE_COLOR_IDX_CTX][XIN_CDF_SIZE(XIN_PALETTE_COLOR_NUM)];
    UINT16 paletteYModeCdf[XIN_NUM_PALATTE_BSIZE_CTX][XIN_NUM_PALETTE_Y_MODE_CTX][XIN_CDF_SIZE(2)];
    UINT16 paletteUvModeCdf[XIN_NUM_PALETTE_UV_MODE_CTX][XIN_CDF_SIZE(2)];
    UINT16 compInterCdf[XIN_NUM_COMP_INTER_CTX][XIN_CDF_SIZE(2)];
    UINT16 singleRefCdf[XIN_NUM_REF_CTX][XIN_SINGLE_REF_NUM - 1][XIN_CDF_SIZE(2)];
    UINT16 compRefTypeCdf[XIN_NUM_COMP_REF_TYPE_CTX][XIN_CDF_SIZE(2)];
    UINT16 uniCompRefCdf[XIN_NUM_UNI_COMP_REF_CTX][XIN_UNIDIR_COMP_REF_NUM - 1][XIN_CDF_SIZE(2)];
    UINT16 compFwdRefCdf[XIN_NUM_REF_CTX][XIN_FWD_REF_NUM - 1][XIN_CDF_SIZE(2)];
    UINT16 compBwdrefCdf[XIN_NUM_REF_CTX][XIN_BWD_REF_NUM - 1][XIN_CDF_SIZE(2)];
    UINT16 txfmPartitionCdf[XIN_NUM_TXFM_PARTITION_CTX][XIN_CDF_SIZE(2)];
    UINT16 compoundIndexCdf[XIN_NUM_COMP_INDEX_CTX][XIN_CDF_SIZE(2)];
    UINT16 compGroupIdxCdf[XIN_NUM_COMP_GROUP_IDX_CTX][XIN_CDF_SIZE(2)];
    UINT16 skipModeCdf[XIN_NUM_SKIP_MODE_CTX][XIN_CDF_SIZE(2)];
    UINT16 skipCdf[XIN_NUM_SKIP_CTX][XIN_CDF_SIZE(2)];
    UINT16 intraInterCdf[XIN_NUM_INTRA_INTER_CTX][XIN_CDF_SIZE(2)];
    UINT16 jointsCdf[2][XIN_CDF_SIZE(XIN_MV_JOINT_NUM)];
    UINT16 classesCdf[2][XIN_CDF_SIZE(XIN_MV_CLASS_NUM)];
    UINT16 class0FpCdf[2][XIN_CLASS0_SIZE][XIN_CDF_SIZE(XIN_MV_FP_SIZE)];
    UINT16 fpCdf[2][XIN_CDF_SIZE(XIN_MV_FP_SIZE)];
    UINT16 signCdf[2][XIN_CDF_SIZE(2)];
    UINT16 class0HpCdf[2][XIN_CDF_SIZE(2)];
    UINT16 hpCdf[2][XIN_CDF_SIZE(2)];
    UINT16 class0Cdf[2][XIN_CDF_SIZE(XIN_CLASS0_SIZE)];
    UINT16 bitsCdf[2][XIN_MV_OFFSET_BITS][XIN_CDF_SIZE(2)];

    UINT16 intrabcCdf[XIN_CDF_SIZE(2)];
    UINT16 filterIntraCdf[XIN_BLOCK_SIZE_NUM][XIN_CDF_SIZE(2)];
    UINT16 filterIntraModeCdf[XIN_CDF_SIZE(XIN_FILTER_INTRA_MODE_NUM)];
    UINT16 switchableRestoreCdf[XIN_CDF_SIZE(XIN_RESTORE_SWITCHABLE_TYPE_NUM)];
    UINT16 wienerRestoreCdf[XIN_CDF_SIZE(2)];
    UINT16 sgrprojRestoreCdf[XIN_CDF_SIZE(2)];
    UINT16 yModeCdf[XIN_BLOCK_SIZE_GROUP_NUM][XIN_CDF_SIZE(XIN_INTRA_MODE_NUM)];
    UINT16 uvModeCdf[XIN_CFL_ALLOWED_TYPE_NUM][XIN_INTRA_MODE_NUM][XIN_CDF_SIZE(XIN_UV_INTRA_MODE_NUM)];
    UINT16 partitionCdf[XIN_NUM_PARTITION_CTX][XIN_CDF_SIZE(XIN_EXT_PARTITION_TYPE_NUM)];
    UINT16 switchableInterpCdf[XIN_NUM_SWITCHABLE_FILTER_CTX][XIN_CDF_SIZE(XIN_SWITCHABLE_FILTER_NUM)];
    /* kf_y_cdf is discarded after use, so does not require persistent storage.
       However, we keep it with the other CDFs in this struct since it needs to
       be copied to each tile to support parallelism just like the others.
    */
    UINT16 kfYCdf[XIN_NUM_KF_MODE_CTX][XIN_NUM_KF_MODE_CTX][XIN_CDF_SIZE(XIN_INTRA_MODE_NUM)];

    UINT16 angleDeltaCdf[XIN_DIRECTIONAL_MODE_NUM][XIN_CDF_SIZE(2 * XIN_MAX_ANGLE_DELTA + 1)];
    UINT16 txSizeCdf[XIN_MAX_TX_CAT_NUM][XIN_NUM_TX_SIZE_CTX][XIN_CDF_SIZE(XIN_MAX_TX_DEPTH + 1)];
    UINT16 deltaQCdf[XIN_CDF_SIZE(XIN_DELTA_Q_PROB_NUM + 1)];
    UINT16 deltaLfMultiCdf[XIN_FRAME_LF_COUNT][XIN_CDF_SIZE(XIN_DELTA_LF_PROB_NUM + 1)];
    UINT16 deltaLfCdf[XIN_CDF_SIZE(XIN_DELTA_LF_PROB_NUM + 1)];
    UINT16 intraExtTxCdf[XIN_EXT_TX_SETS_INTRA][XIN_EXT_TX_SIZE_NUM][XIN_INTRA_MODE_NUM][XIN_CDF_SIZE(XIN_TX_TYPE_NUM)];
    UINT16 interExtTxCdf[XIN_EXT_TX_SETS_INTER][XIN_EXT_TX_SIZE_NUM][XIN_CDF_SIZE(XIN_TX_TYPE_NUM)];
    UINT16 cflSignCdf[XIN_CDF_SIZE(XIN_CFL_JOINT_SIGN_NUM)];
    UINT16 cflAlphaCdf[XIN_NUM_CFL_ALPHA_CTX][XIN_CDF_SIZE(XIN_CFL_ALPHABET_SIZE)];

} xin_cdf_prob;

typedef struct lv_map_coef_rate
{
    SINT32  txbSkipRate[XIN_NUM_TXB_SKIP_CTX][2];
    SINT32  baseEobRate[XIN_NUM_SIG_COEF_EOB_CTX][3];
    SINT32  baseRate[XIN_NUM_SIG_COEF_CTX][8];
    SINT32  eobExtraRate[XIN_NUM_EOB_COEF_CTX][2];
    SINT32  dcSignRate[XIN_NUM_DC_SIGN_CTX][2];
    SINT32  lpsRate[XIN_NUM_LEVEL_CTX][XIN_COEF_BASE_RANGE + 1 + XIN_COEF_BASE_RANGE + 1];

} lv_map_coef_rate;

typedef struct lv_map_eob_rate
{
    SINT32 eobRate[2][11];
    
} lv_map_eob_rate;


typedef struct xin_cabac_est
{
    SINT32  partitionRate[XIN_NUM_PARTITION_CTX][XIN_EXT_PARTITION_TYPE_NUM];
    SINT32  skipModeRate[XIN_NUM_SKIP_CTX][2];
    SINT32  skipRate[XIN_NUM_SKIP_CTX][2];
    SINT32  mbModeRate[XIN_BLOCK_SIZE_GROUP_NUM][XIN_INTRA_MODE_NUM];
    SINT32  yModeRate[XIN_INTRA_MODE_NUM][XIN_INTRA_MODE_NUM][XIN_INTRA_MODE_NUM];
    SINT32  intraUvModeRate[XIN_CFL_ALLOWED_TYPE_NUM][XIN_INTRA_MODE_NUM][XIN_UV_INTRA_MODE_NUM];
    SINT32  filterIntraModeRate[XIN_FILTER_INTRA_MODE_NUM];
    SINT32  filterIntraRate[XIN_BLOCK_SIZE_NUM][2];
    SINT32  switchableInterpRate[XIN_NUM_SWITCHABLE_FILTER_CTX][XIN_SWITCHABLE_FILTER_NUM];
    SINT32  paletteYSizeRate[XIN_NUM_PALATTE_BSIZE_CTX][XIN_PALETTE_SIZE_NUM];
    SINT32  paletteUvSizeRate[XIN_NUM_PALATTE_BSIZE_CTX][XIN_PALETTE_SIZE_NUM];
    SINT32  paletteYColorRate[XIN_PALETTE_SIZE_NUM][XIN_NUM_PALETTE_COLOR_INDEX_CTX][XIN_PALETTE_COLOR_NUM];
    SINT32  paletteUvColorRate[XIN_PALETTE_SIZE_NUM][XIN_NUM_PALETTE_COLOR_INDEX_CTX][XIN_PALETTE_COLOR_NUM];
    SINT32  paletteYModeRate[XIN_NUM_PALATTE_BSIZE_CTX][XIN_NUM_PALETTE_Y_MODE_CTX][2];
    SINT32  paletteUvModeRate[XIN_NUM_PALETTE_UV_MODE_CTX][2];
    SINT32  cflRate[XIN_CFL_JOINT_SIGN_NUM][XIN_CFL_PRED_PLANE_NUM][XIN_CFL_ALPHABET_SIZE];
    SINT32  txSizeRate[XIN_TX_SIZE_NUM - 1][XIN_NUM_TX_SIZE_CTX][XIN_TX_SIZE_NUM];
    SINT32  txPartitionRate[XIN_NUM_TXFM_PARTITION_CTX][2];
    SINT32  interTxTypeRate[XIN_EXT_TX_SETS_INTER][XIN_EXT_TX_SIZE_NUM][XIN_TX_2D_NUM];
    SINT32  intraTxTypeRate[XIN_EXT_TX_SETS_INTRA][XIN_EXT_TX_SIZE_NUM][XIN_INTRA_MODE_NUM][XIN_TX_2D_NUM];
    SINT32  angleDeltaRate[XIN_DIRECTIONAL_MODE_NUM][2*XIN_MAX_ANGLE_DELTA + 1];
    SINT32  switchableRestoreRate[XIN_RESTORE_SWITCHABLE_TYPE_NUM];
    SINT32  wienerRestoreRate[2];
    SINT32  sgrprojRestoreRate[2];
    SINT32  intraBCRate[2];
    SINT32  compInterRate[XIN_NUM_COMP_INTER_CTX][2];
    SINT32  singleRefRate[XIN_NUM_REF_CTX][XIN_SINGLE_REF_NUM - 1][2];
    SINT32  compRefTypeRate[XIN_NUM_COMP_REF_TYPE_CTX][XIN_CDF_SIZE(XIN_COMP_REFERENCE_TYPE_NUM)];
    SINT32  uniCompRefRate[XIN_NUM_UNI_COMP_REF_CTX][XIN_UNIDIR_COMP_REF_NUM - 1][XIN_CDF_SIZE(2)];
    SINT32  compFwdRefRate[XIN_NUM_REF_CTX][XIN_FWD_REF_NUM - 1][2];
    // Cost for signaling ref_frame[1] (ALTREF_FRAME, ALTREF2_FRAME, or
    // BWDREF_FRAME) in bidir-comp mode.
    SINT32  compBwdRefRate[XIN_NUM_REF_CTX][XIN_BWD_REF_NUM - 1][2];
    SINT32  intraInterRate[XIN_NUM_INTRA_INTER_CTX][2];
    SINT32  newMvModeRate[XIN_NUM_NEWMV_MODE_CTX][2];
    SINT32  zeroMvModeRate[XIN_NUM_GLOBALMV_MODE_CTX][2];
    SINT32  refMvModeRate[XIN_NUM_REFMV_MODE_CTX][2];
    SINT32  drlModeRate0[XIN_NUM_DRL_MODE_CTX][2];
    SINT32  interCompoundModeRate[XIN_NUM_INTER_MODE_CTX][XIN_INTER_COMPOUND_MODE_NUM];
    SINT32  compoundTypeRate[XIN_BLOCK_SIZE_NUM][XIN_MASKED_COMPOUND_TYPE_NUM];
    SINT32  interIntraRate[XIN_BLOCK_SIZE_GROUP_NUM][2];
    SINT32  interIntraModeRate[XIN_BLOCK_SIZE_GROUP_NUM][XIN_INTERINTRA_MODE_NUM];
    SINT32  motionModeRate[XIN_BLOCK_SIZE_NUM][XIN_MOTION_MODE_NUM];
    SINT32  motionModeRate1[XIN_BLOCK_SIZE_NUM][2];
    SINT32  compIdxRate[XIN_NUM_COMP_INDEX_CTX][2];
    SINT32  compGroupIdxRate[XIN_NUM_COMP_GROUP_IDX_CTX][2];

    lv_map_coef_rate coefFacRate[XIN_TX_SIZE_NUM][PLANE_TYPE];
    lv_map_eob_rate  eobFracRate[7][2];
    
} xin_cabac_est;

typedef struct xin_cabac_context
{
    xin_cabac_struct cabac;
    xin_cdf_prob     cdfProb;
} xin_cabac_context;

#endif

