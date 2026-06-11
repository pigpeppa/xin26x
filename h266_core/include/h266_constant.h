/***************************************************************************//**
 *
 * @file          h266_constant.h
 * @brief         This file contains h.266 constant definitions.
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
#ifndef _h266_constant_h_
#define _h266_constant_h_

#define XIN_MV_FRAC_BIT_NUM     4
#define XIN_BCW_LOG_WGT_BASE    3
#define XIN_BCW_WGT_BASE        (1 << XIN_BCW_LOG_WGT_BASE)
#define XIN_BCW_NUM             5
#define XIN_BCW_SIZE_CONSTRAINT 256
#define XIN_BCW_DEFAULT         (XIN_BCW_NUM >> 1)
#define XIN_MAX_CU_DEPTH        7

typedef enum NalUnitType
{
    NAL_UNIT_CODED_SLICE_TRAIL = 0,   // 0
    NAL_UNIT_CODED_SLICE_STSA,        // 1
    NAL_UNIT_CODED_SLICE_RADL,        // 2
    NAL_UNIT_CODED_SLICE_RASL,        // 3

    NAL_UNIT_RESERVED_VCL_4,
    NAL_UNIT_RESERVED_VCL_5,
    NAL_UNIT_RESERVED_VCL_6,

    NAL_UNIT_CODED_SLICE_IDR_W_RADL,  // 7
    NAL_UNIT_CODED_SLICE_IDR_N_LP,    // 8
    NAL_UNIT_CODED_SLICE_CRA,         // 9
    NAL_UNIT_CODED_SLICE_GDR,         // 10

    NAL_UNIT_RESERVED_IRAP_VCL_11,
    NAL_UNIT_RESERVED_IRAP_VCL_12,

    NAL_UNIT_DPS,                     // 13
    NAL_UNIT_VPS,                     // 14
    NAL_UNIT_SPS,                     // 15
    NAL_UNIT_PPS,                     // 16
    NAL_UNIT_PREFIX_APS,              // 17
    NAL_UNIT_SUFFIX_APS,              // 18
    NAL_UNIT_PH,                      // 19
    NAL_UNIT_ACCESS_UNIT_DELIMITER,   // 20
    NAL_UNIT_EOS,                     // 21
    NAL_UNIT_EOB,                     // 22
    NAL_UNIT_PREFIX_SEI,              // 23
    NAL_UNIT_SUFFIX_SEI,              // 24
    NAL_UNIT_FD,                      // 25

    NAL_UNIT_RESERVED_NVCL_26,
    NAL_UNIT_RESERVED_NVCL_27,

    NAL_UNIT_UNSPECIFIED_28,
    NAL_UNIT_UNSPECIFIED_29,
    NAL_UNIT_UNSPECIFIED_30,
    NAL_UNIT_UNSPECIFIED_31,
    NAL_UNIT_INVALID
} NalUnitType;

typedef enum xin_intra_mode
{
    XIN_INTRA_PLANAR = 0,
    XIN_INTRA_DC     = 1,
    XIN_INTRA_2      = 2,
    XIN_INTRA_3      = 3,
    XIN_INTRA_4      = 4,
    XIN_INTRA_5      = 5,
    XIN_INTRA_6      = 6,
    XIN_INTRA_7      = 7,
    XIN_INTRA_8      = 8,
    XIN_INTRA_9      = 9,
    XIN_INTRA_10     = 10,
    XIN_INTRA_11     = 11,
    XIN_INTRA_12     = 12,
    XIN_INTRA_13     = 13,
    XIN_INTRA_14     = 14,
    XIN_INTRA_15     = 15,
    XIN_INTRA_16     = 16,
    XIN_INTRA_17     = 17,
    XIN_INTRA_HOR    = 18,
    XIN_INTRA_19     = 19,
    XIN_INTRA_20     = 20,
    XIN_INTRA_21     = 21,
    XIN_INTRA_22     = 22,
    XIN_INTRA_23     = 23,
    XIN_INTRA_24     = 24,
    XIN_INTRA_25     = 25,
    XIN_INTRA_26     = 26,
    XIN_INTRA_27     = 27,
    XIN_INTRA_28     = 28,
    XIN_INTRA_29     = 29,
    XIN_INTRA_30     = 30,
    XIN_INTRA_31     = 31,
    XIN_INTRA_32     = 32,
    XIN_INTRA_33     = 33,
    XIN_INTRA_DIA    = 34,
    XIN_INTRA_35     = 35,
    XIN_INTRA_36     = 36,
    XIN_INTRA_37     = 37,
    XIN_INTRA_38     = 38,
    XIN_INTRA_39     = 39,
    XIN_INTRA_40     = 40,
    XIN_INTRA_41     = 41,
    XIN_INTRA_42     = 42,
    XIN_INTRA_43     = 43,
    XIN_INTRA_44     = 44,
    XIN_INTRA_45     = 45,
    XIN_INTRA_46     = 46,
    XIN_INTRA_47     = 47,
    XIN_INTRA_48     = 48,
    XIN_INTRA_49     = 49,
    XIN_INTRA_VER    = 50,
    XIN_INTRA_51     = 51,
    XIN_INTRA_52     = 52,
    XIN_INTRA_53     = 53,
    XIN_INTRA_54     = 54,
    XIN_INTRA_55     = 55,
    XIN_INTRA_56     = 56,
    XIN_INTRA_57     = 57,
    XIN_INTRA_58     = 58,
    XIN_INTRA_59     = 59,
    XIN_INTRA_60     = 60,
    XIN_INTRA_61     = 61,
    XIN_INTRA_62     = 62,
    XIN_INTRA_63     = 63,
    XIN_INTRA_64     = 64,
    XIN_INTRA_65     = 65,
    XIN_INTRA_VDIA   = 66,
    XIN_INTRA_NUM    = 67
} xin_intra_mode;

typedef enum xin_angular_intra_func
{
    XIN_INTRA_FUNC_2     = 0,
    XIN_INTRA_FUNC_3_17  = 1,
    XIN_INTRA_FUNC_18    = 2,
    XIN_INTRA_FUNC_19_13 = 3,
    XIN_INTRA_FUNC_34    = 4,
    XIN_INTRA_FUNC_35_49 = 5,
    XIN_INTRA_FUNC_50    = 6,
    XIN_INTRA_FUNC_51_65 = 7,
    XIN_INTRA_FUNC_66    = 8,
    XIN_ANG_IF_NUM       = 9
} xin_angular_intra_func;

typedef enum xin_nb_dir
{
    XIN_NB_TOP_LFT = 0,
    XIN_NB_TOP,
    XIN_NB_TOP_RGT,
    XIN_NB_LFT,
    XIN_NB_BOT_LFT,
    XIN_NB_COL,
    XIN_NB_NUM
} xin_nb_dir;

typedef enum xin_tmvp_pos
{
    XIN_TMVP_COL_BOT_RGT = 0,
    XIN_TMVP_COL_CENTRE
} xin_tmvp_pos;

typedef enum xin_reshape_signal_type
{
  XIN_RESHAPE_SIGNAL_SDR  = 0,
  XIN_RESHAPE_SIGNAL_PQ   = 1,
  XIN_RESHAPE_SIGNAL_HLG  = 2,
  XIN_RESHAPE_SIGNAL_NULL = 100,
}xin_reshape_signal_type;

typedef enum xin_mv_prec
{
  XIN_MV_PREC_4PEL      = 0,      // 4-pel
  XIN_MV_PREC_INT       = 2,      // 1-pel, shift 2 bits from 4-pel
  XIN_MV_PREC_HALF      = 3,      // 1/2-pel
  XIN_MV_PREC_QUARTER   = 4,      // 1/4-pel (the precision of regular MV difference signaling), shift 4 bits from 4-pel
  XIN_MV_PREC_SIXTEENTH = 6,     // 1/16-pel (the precision of internal MV), shift 6 bits from 4-pel
  XIN_MV_PREC_INTERNAL  = 2 + XIN_MV_FRAC_BIT_NUM,
}xin_mv_prec;

#endif

