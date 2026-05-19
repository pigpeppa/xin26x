/***************************************************************************//**
*
* @file          h265p_constant.h
* @brief         This file contains av1 constant defintions.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_constant_h_
#define _h265p_constant_h_

typedef enum xin_block_size
{
    XIN_BLOCK_4X4,
    XIN_BLOCK_4X8,
    XIN_BLOCK_8X4,
    XIN_BLOCK_8X8,
    XIN_BLOCK_8X16,
    XIN_BLOCK_16X8,
    XIN_BLOCK_16X16,
    XIN_BLOCK_16X32,
    XIN_BLOCK_32X16,
    XIN_BLOCK_32X32,
    XIN_BLOCK_32X64,
    XIN_BLOCK_64X32,
    XIN_BLOCK_64X64,
    XIN_BLOCK_64X128,
    XIN_BLOCK_128X64,
    XIN_BLOCK_128X128,
    XIN_BLOCK_4X16,
    XIN_BLOCK_16X4,
    XIN_BLOCK_8X32,
    XIN_BLOCK_32X8,
    XIN_BLOCK_16X64,
    XIN_BLOCK_64X16,
    XIN_BLOCK_SIZE_NUM,
    XIN_BLOCK_INVALID = 255,
} xin_block_size;

typedef enum xin_partition_type
{
    XIN_PARTITION_NONE,
    XIN_PARTITION_HORZ,
    XIN_PARTITION_VERT,
    XIN_PARTITION_SPLIT,
    XIN_PARTITION_HORZ_A,  // HORZ split and the top partition is split again
    XIN_PARTITION_HORZ_B,  // HORZ split and the bottom partition is split again
    XIN_PARTITION_VERT_A,  // VERT split and the left partition is split again
    XIN_PARTITION_VERT_B,  // VERT split and the right partition is split again
    XIN_PARTITION_HORZ_4,  // 4:1 horizontal partition
    XIN_PARTITION_VERT_4,  // 4:1 vertical partition
    XIN_EXT_PARTITION_TYPE_NUM,
    XIN_PARTITION_TYPE_NUM = XIN_PARTITION_SPLIT + 1,
    XIN_PARTITION_INVALID = 255
} xin_partition_type;

typedef enum xin_pred_mode
{
    XIN_DC_PRED = 0,    // Average of above and left pixels
    XIN_V_PRED,         // Vertical
    XIN_H_PRED,         // Horizontal
    XIN_D45_PRED,       // Directional 45  degree
    XIN_D135_PRED,      // Directional 135 degree
    XIN_D113_PRED,      // Directional 113 degree
    XIN_D157_PRED,      // Directional 157 degree
    XIN_D203_PRED,      // Directional 203 degree
    XIN_D67_PRED,       // Directional 67  degree
    XIN_SMOOTH_PRED,    // Combination of horizontal and vertical interpolation
    XIN_SMOOTH_V_PRED,  // Vertical interpolation
    XIN_SMOOTH_H_PRED,  // Horizontal interpolation
    XIN_PAETH_PRED,     // Predict from the direction of smallest gradient
    XIN_NEARESTMV,
    XIN_NEARMV,
    XIN_GLOBALMV,
    XIN_NEWMV,

    // Compound ref compound modes
    XIN_NEAREST_NEARESTMV,
    XIN_NEAR_NEARMV,
    XIN_NEAREST_NEWMV,
    XIN_NEW_NEARESTMV,
    XIN_NEAR_NEWMV,
    XIN_NEW_NEARMV,
    XIN_GLOBAL_GLOBALMV,
    XIN_NEW_NEWMV,
    XIN_MB_MODE_COUNT,
    XIN_INTRA_MODE_START = XIN_DC_PRED,
    XIN_INTRA_MODE_END = XIN_NEARESTMV,
    XIN_DIR_MODE_START = XIN_V_PRED,
    XIN_DIR_MODE_END = XIN_D67_PRED + 1,
    XIN_INTRA_MODE_NUM = XIN_INTRA_MODE_END - XIN_INTRA_MODE_START,
    XIN_SINGLE_INTER_MODE_START = XIN_NEARESTMV,
    XIN_SINGLE_INTER_MODE_END = XIN_NEAREST_NEARESTMV,
    XIN_SINGLE_INTER_MODE_NUM = XIN_SINGLE_INTER_MODE_END - XIN_SINGLE_INTER_MODE_START,
    XIN_COMP_INTER_MODE_START = XIN_NEAREST_NEARESTMV,
    XIN_COMP_INTER_MODE_END = XIN_MB_MODE_COUNT,
    XIN_COMP_INTER_MODE_NUM = XIN_COMP_INTER_MODE_END - XIN_COMP_INTER_MODE_START,
    XIN_INTER_MODE_START = XIN_NEARESTMV,
    XIN_INTER_MODE_END = XIN_MB_MODE_COUNT,
    XIN_INTRA_INVALID = XIN_MB_MODE_COUNT  // For uv_mode in inter blocks

} xin_pred_mode;

// An enum for single reference types (and some derived values).
typedef enum xin_single_ref
{
    XIN_NONE_FRAME = -1,
    XIN_INTRA_FRAME,
    XIN_LAST_FRAME,
    XIN_LAST2_FRAME,
    XIN_LAST3_FRAME,
    XIN_GOLDEN_FRAME,
    XIN_BWDREF_FRAME,
    XIN_ALTREF2_FRAME,
    XIN_ALTREF_FRAME,
    XIN_REF_FRAME_NUM,

    // Extra/scratch reference frame. It may be:
    // - used to update the ALTREF2_FRAME ref (see lshift_bwd_ref_frames()), or
    // - updated from ALTREF2_FRAME ref (see rshift_bwd_ref_frames()).
    XIN_EXTREF_FRAME = XIN_REF_FRAME_NUM,

    // Number of inter (non-intra) reference types.
    XIN_INTER_REF_NUM = XIN_ALTREF_FRAME - XIN_LAST_FRAME + 1,

    // Number of forward (aka past) reference types.
    XIN_FWD_REF_NUM = XIN_GOLDEN_FRAME - XIN_LAST_FRAME + 1,

    // Number of backward (aka future) reference types.
    XIN_BWD_REF_NUM = XIN_ALTREF_FRAME - XIN_BWDREF_FRAME + 1,

    XIN_SINGLE_REF_NUM = XIN_FWD_REF_NUM + XIN_BWD_REF_NUM,

} xin_single_ref;

typedef enum xin_compound_type
{
    XIN_COMPOUND_AVERAGE,
    XIN_COMPOUND_DISTWTD,
    XIN_COMPOUND_WEDGE,
    XIN_COMPOUND_DIFFWTD,
    XIN_COMPOUND_TYPE_NUM,
    XIN_MASKED_COMPOUND_TYPE_NUM = 2,
} xin_compound_type;

typedef enum xin_palette_size
{
    XIN_TWO_COLORS,
    XIN_THREE_COLORS,
    XIN_FOUR_COLORS,
    XIN_FIVE_COLORS,
    XIN_SIX_COLORS,
    XIN_SEVEN_COLORS,
    XIN_EIGHT_COLORS,
    XIN_PALETTE_SIZE_NUM
} xin_palette_size;

typedef enum xin_palette_color
{
    XIN_PALETTE_COLOR_ONE,
    XIN_PALETTE_COLOR_TWO,
    XIN_PALETTE_COLOR_THREE,
    XIN_PALETTE_COLOR_FOUR,
    XIN_PALETTE_COLOR_FIVE,
    XIN_PALETTE_COLOR_SIX,
    XIN_PALETTE_COLOR_SEVEN,
    XIN_PALETTE_COLOR_EIGHT,
    XIN_PALETTE_COLOR_NUM
} xin_palette_color;

typedef enum xin_unidir_comp_ref
{
    XIN_LAST_LAST2_FRAMES,      // { LAST_FRAME, LAST2_FRAME }
    XIN_LAST_LAST3_FRAMES,      // { LAST_FRAME, LAST3_FRAME }
    XIN_LAST_GOLDEN_FRAMES,     // { LAST_FRAME, GOLDEN_FRAME }
    XIN_BWDREF_ALTREF_FRAMES,   // { BWDREF_FRAME, ALTREF_FRAME }
    XIN_LAST2_LAST3_FRAMES,     // { LAST2_FRAME, LAST3_FRAME }
    XIN_LAST2_GOLDEN_FRAMES,    // { LAST2_FRAME, GOLDEN_FRAME }
    XIN_LAST3_GOLDEN_FRAMES,    // { LAST3_FRAME, GOLDEN_FRAME }
    XIN_BWDREF_ALTREF2_FRAMES,  // { BWDREF_FRAME, ALTREF2_FRAME }
    XIN_ALTREF2_ALTREF_FRAMES,  // { ALTREF2_FRAME, ALTREF_FRAME }
    XIN_TOTAL_UNIDIR_COMP_REF,
    // NOTE: UNIDIR_COMP_REFS is the number of uni-directional reference pairs
    //       that are explicitly signaled.
    XIN_UNIDIR_COMP_REF_NUM = XIN_BWDREF_ALTREF_FRAMES + 1,

} xin_unidir_comp_ref;

typedef enum xin_inter_intra_mode
{
    XIN_II_DC_PRED,
    XIN_II_V_PRED,
    XIN_II_H_PRED,
    XIN_II_SMOOTH_PRED,
    XIN_INTERINTRA_MODE_NUM
} xin_inter_intra_mode;

typedef enum xin_interp_filter
{
    XIN_EIGHTTAP_REGULAR,
    XIN_EIGHTTAP_SMOOTH,
    XIN_MULTITAP_SHARP,
    XIN_BILINEAR,
    XIN_INTERP_FILTERS_ALL,
    XIN_SWITCHABLE_FILTER_NUM = XIN_BILINEAR,
    XIN_SWITCHABLE = XIN_SWITCHABLE_FILTER_NUM + 1, /* the last switchable one */
    XIN_EXTRA_FILTERS = XIN_INTERP_FILTERS_ALL - XIN_SWITCHABLE_FILTER_NUM,
    XIN_INTERP_INVALID = 0xFF,
} xin_interp_filter;

typedef enum xin_cfl_sign
{
    XIN_CFL_SIGN_ZERO,
    XIN_CFL_SIGN_NEG,
    XIN_CFL_SIGN_POS,
    XIN_CFL_SIGN_NUM
} xin_cfl_sign;

typedef enum xin_restoration_type
{
    XIN_RESTORE_NONE,
    XIN_RESTORE_WIENER,
    XIN_RESTORE_SGRPROJ,
    XIN_RESTORE_SWITCHABLE,
    XIN_RESTORE_SWITCHABLE_TYPE_NUM = XIN_RESTORE_SWITCHABLE,
    XIN_RESTORE_TYPE_NUM = 4,

} xin_restoration_type;

typedef enum xin_motion_mode
{
    XIN_SIMPLE_TRANSLATION,
    XIN_OBMC_CAUSAL,    // 2-sided OBMC
    XION_WARPED_CAUSAL,  // 2-sided WARPED
    XIN_MOTION_MODE_NUM

} xin_motion_mode;

typedef enum xin_uv_pred_mode
{
    XIN_UV_DC_PRED,        // Average of above and left pixels
    XIN_UV_V_PRED,         // Vertical
    XIN_UV_H_PRED,         // Horizontal
    XIN_UV_D45_PRED,       // Directional 45  degree
    XIN_UV_D135_PRED,      // Directional 135 degree
    XIN_UV_D113_PRED,      // Directional 113 degree
    XIN_UV_D157_PRED,      // Directional 157 degree
    XIN_UV_D203_PRED,      // Directional 203 degree
    XIN_UV_D67_PRED,       // Directional 67  degree
    XIN_UV_SMOOTH_PRED,    // Combination of horizontal and vertical interpolation
    XIN_UV_SMOOTH_V_PRED,  // Vertical interpolation
    XIN_UV_SMOOTH_H_PRED,  // Horizontal interpolation
    XIN_UV_PAETH_PRED,     // Predict from the direction of smallest gradient
    XIN_UV_CFL_PRED,       // Chroma-from-Luma
    XIN_UV_INTRA_MODE_NUM,
    XIN_UV_MODE_INVALID,  // For uv_mode in inter blocks
} xin_uv_pred_mode;

typedef enum xin_seq_level
{
    XIN_SEQ_LEVEL_2_0,
    XIN_SEQ_LEVEL_2_1,
    XIN_SEQ_LEVEL_2_2,
    XIN_SEQ_LEVEL_2_3,
    XIN_SEQ_LEVEL_3_0,
    XIN_SEQ_LEVEL_3_1,
    XIN_SEQ_LEVEL_3_2,
    XIN_SEQ_LEVEL_3_3,
    XIN_SEQ_LEVEL_4_0,
    XIN_SEQ_LEVEL_4_1,
    XIN_SEQ_LEVEL_4_2,
    XIN_SEQ_LEVEL_4_3,
    XIN_SEQ_LEVEL_5_0,
    XIN_SEQ_LEVEL_5_1,
    XIN_SEQ_LEVEL_5_2,
    XIN_SEQ_LEVEL_5_3,
    XIN_SEQ_LEVEL_6_0,
    XIN_SEQ_LEVEL_6_1,
    XIN_SEQ_LEVEL_6_2,
    XIN_SEQ_LEVEL_6_3,
    XIN_SEQ_LEVEL_7_0,
    XIN_SEQ_LEVEL_7_1,
    XIN_SEQ_LEVEL_7_2,
    XIN_SEQ_LEVEL_7_3,
    XIN_SEQ_LEVEL_NUM,
    XIN_SEQ_LEVEL_MAX = 31
} xin_seq_level;

// Profile 0.  8-bit and 10-bit 4:2:0 and 4:0:0 only.
// Profile 1.  8-bit and 10-bit 4:4:4
// Profile 2.  8-bit and 10-bit 4:2:2
//            12 bit  4:0:0, 4:2:2 and 4:4:4
typedef enum xin_seq_profile
{
    XIN_PROFILE_0,
    XIN_PROFILE_1,
    XIN_PROFILE_2,
    XIN_PROFILE_MAX
} xin_seq_profile;

typedef enum xin_obu_type
{
    XIN_OBU_SEQUENCE_HEADER = 1,
    XIN_OBU_TEMPORAL_DELIMITER = 2,
    XIN_OBU_FRAME_HEADER = 3,
    XIN_OBU_TILE_GROUP = 4,
    XIN_OBU_METADATA = 5,
    XIN_OBU_FRAME = 6,
    XIN_OBU_REDUNDANT_FRAME_HEADER = 7,
    XIN_OBU_PADDING = 15,
} xin_obu_type;

typedef enum xin_frame_type
{
    XIN_KEY_FRAME = 0,
    XIN_INTER_FRAME = 1,
    XIN_INTRA_ONLY_FRAME = 2,  // replaces intra-only
    XIN_S_FRAME = 3,
    XIN_FRAME_TYPE_NUM,
} xin_frame_type;

/* Symbols for coding which components are zero jointly */
typedef enum xin_mv_joint_type
{
    XIN_MV_JOINT_ZERO   = 0,   /* Zero vector */
    XIN_MV_JOINT_HNZVZ  = 1,  /* Vert zero, hor nonzero */
    XIN_MV_JOINT_HZVNZ  = 2,  /* Hor zero, vert nonzero */
    XIN_MV_JOINT_HNZVNZ = 3, /* Both components nonzero */
    XIN_MV_JOINT_NUM    = 4
} xin_mv_joint_type;

typedef enum xin_mv_class_type
{
    XIN_MV_CLASS_0 = 0,   /* (0, 2]     integer pel */
    XIN_MV_CLASS_1 = 1,   /* (2, 4]     integer pel */
    XIN_MV_CLASS_2 = 2,   /* (4, 8]     integer pel */
    XIN_MV_CLASS_3 = 3,   /* (8, 16]    integer pel */
    XIN_MV_CLASS_4 = 4,   /* (16, 32]   integer pel */
    XIN_MV_CLASS_5 = 5,   /* (32, 64]   integer pel */
    XIN_MV_CLASS_6 = 6,   /* (64, 128]  integer pel */
    XIN_MV_CLASS_7 = 7,   /* (128, 256] integer pel */
    XIN_MV_CLASS_8 = 8,   /* (256, 512] integer pel */
    XIN_MV_CLASS_9 = 9,   /* (512, 1024] integer pel */
    XIN_MV_CLASS_10 = 10, /* (1024,2048] integer pel */
    XIN_MV_CLASS_NUM = 11,
} xin_mv_class_type;

typedef enum xin_filter_intra_mode
{
    XIN_FILTER_DC_PRED,
    XIN_FILTER_V_PRED,
    XIN_FILTER_H_PRED,
    XIN_FILTER_D157_PRED,
    XIN_FILTER_PAETH_PRED,
    XIN_FILTER_INTRA_MODE_NUM,
    XIN_FILTER_INVALID = 0xFF,
} xin_filter_intra_mode;

typedef enum xin_reference_mode
{
    XIN_SINGLE_REFERENCE = 0,
    XIN_COMPOUND_REFERENCE = 1,
    XIN_REFERENCE_MODE_SELECT = 2,
    XIN_REFERENCE_MODE_NUM = 3,
} xin_reference_mode;

// frame transform mode
typedef enum xin_tx_mode
{
    XIN_ONLY_4X4, // use only 4x4 transform
    XIN_TX_MODE_LARGEST, // transform size is the largest possible for pu size
    XIN_TX_MODE_SELECT, // transform specified for each block
    XIN_TX_MODE_NUM,
} xin_tx_mode;

typedef enum xin_cfl_allowed_type
{
    XIN_CFL_DISALLOWED,
    XIN_CFL_ALLOWED,
    XIN_CFL_ALLOWED_TYPE_NUM
} xin_cfl_allowed_type;

typedef enum xin_comp_reference_type
{
    XIN_UNIDIR_COMP_REFERENCE,
    XIN_BIDIR_COMP_REFERENCE,
    XIN_COMP_REFERENCE_TYPE_NUM,
} xin_comp_reference_type;

typedef enum xin_cfl_pred_type
{
    XIN_CFL_PRED_U,
    XIN_CFL_PRED_V,
    XIN_CFL_PRED_PLANE_NUM
} xin_cfl_pred_type;

typedef enum xin_mv_precision
{
    XIN_MV_NONE = -1,
    XIN_MV_LOW_PRECISION = 0,
    XIN_MVL_HIGH_PRECISION,
} xin_mv_precision;

#endif

