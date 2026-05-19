/***************************************************************************//**
*
* @file          h265p_definition.h
* @brief         This file contains av1 defintions.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_definition_h_
#define _h265p_definition_h_

#define XIN_PART_SPLIT              0
#define XIN_PART_HORZ               1
#define XIN_PART_VERT               2
#define XIN_PART_HORZ_A             3
#define XIN_PART_HORZ_B             4
#define XIN_PART_VERT_A             5
#define XIN_PART_VERT_B             6
#define XIN_PART_HORZ_4             7
#define XIN_PART_VERT_4             8
#define XIN_PART_NUM                9

#define XIN_CAN_PART_HORZ           (1 << XIN_PART_HORZ)
#define XIN_CAN_PART_VERT           (1 << XIN_PART_VERT)
#define XIN_CAN_PART_SPLIT          (1 << XIN_PART_SPLIT)
#define XIN_CAN_PART_HORZ_A         (1 << XIN_PART_HORZ_A)
#define XIN_CAN_PART_HORZ_B         (1 << XIN_PART_HORZ_B)
#define XIN_CAN_PART_VERT_A         (1 << XIN_PART_VERT_A)
#define XIN_CAN_PART_VERT_B         (1 << XIN_PART_VERT_B)
#define XIN_CAN_PART_HORZ_4         (1 << XIN_PART_HORZ_4)
#define XIN_CAN_PART_VERT_4         (1 << XIN_PART_VERT_4)
#define XIN_CAN_PART_RECT           (XIN_CAN_PART_HORZ | XIN_CAN_PART_VERT)
#define XIN_CAN_PART_AB             (XIN_CAN_PART_HORZ_A | XIN_CAN_PART_HORZ_B | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B)
#define XIN_CAN_PART_1TO4           (XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4)

#define XIN_MAX_MB_DEPTH            6
#define XIN_MAX_QT_DEPTH            5
#define XIN_MAX_BT_DEPTH            5

#define XIN_MAX_SB_SIZE             128

#define XIN_MIN_TEMPORAL_LAYER      1
#define XIN_MAX_TEMPORAL_LAYER      4

#define XIN_MIN_WIDTH_IN_TILE       1
#define XIN_MAX_WIDTH_IN_TILE       2
#define XIN_MIN_HEIGHT_IN_TILE      1
#define XIN_MAX_HEIGHT_IN_TILE      2

// av1 mb type defitions
#define XIN_SKIP_MODE               0
#define XIN_INTER_MODE              1
#define XIN_INTRA_MODE              2
#define XIN_MODE_NUM                3
#define XIN_INVALID_MODE            0xFF

// Surrounding ctu availability 
#define XIN_LFT_SB_AVAIL           (1 << 0)
#define XIN_RGT_SB_AVAIL           (1 << 1)
#define XIN_TOP_SB_AVAIL           (1 << 2)
#define XIN_BOT_SB_AVAIL           (1 << 3)

#define XIN_MIN_QP                  0
#define XIN_MAX_QP                  255
#define XIN_QP_NUM                  (XIN_MAX_QP - XIN_MIN_QP + 1)

#define XIN_MAX_LG_BLOCK_SIZE       7
#define XIN_MAX_BLOCK_SIZE          (1 << XIN_MAX_LG_BLOCK_SIZE)
#define XIN_MIN_BLOCK_SIZE          4

#define XIN_B_FRAME                 0
#define XIN_P_FRAME                 1
#define XIN_I_FRAME                 2
#define XIN_IDR_FRAME               3

#define XIN_ME_FULL_SEARCH          0
#define XIN_ME_BBDGS_SEARCH         1
#define XIN_ME_HIER_SEARCH          2

#define XIN_MAX_DPB_FRAMES          8

#define XIN_MAX_SEGMENT_NUM         8
#define XIN_DELTA_LF_SMALL          3

#define XIN_TX_SIZE_LUMA_MIN        XIN_TX_4X4
#define XIN_TX_SIZE_CTX_MIN         (XIN_TX_SIZE_LUMA_MIN + 1)
#define XIN_MAX_TX_CAT_NUM          (XIN_TX_SIZE_NUM - XIN_TX_SIZE_CTX_MIN)
#define XIN_MAX_TX_DEPTH            2

#define XIN_CFL_ALPHABET_SIZE_LOG2  4
#define XIN_CFL_ALPHABET_SIZE       (1 << XIN_CFL_ALPHABET_SIZE_LOG2)

#define XIN_EXT_TX_SIZE_NUM         4 
#define XIN_EXT_TX_SETS_INTER       4  // Sets of transform selections for INTER
#define XIN_EXT_TX_SETS_INTRA       3  // Sets of transform selections for INTRA

#define XIN_DELTA_Q_PROB_NUM        (XIN_DELTA_Q_SMALL)

#define XIN_FRAME_LF_COUNT          4

#define XIN_COEF_CONTEXT_BIT        3
#define XIN_COEF_CONTEXT_MASK       ((1 << XIN_COEF_CONTEXT_BIT) - 1)

#define XIN_PROFILE_BIT_NUM         3
#define XIN_LEVEL_BIT_NUM           5

#define XIN_ENTROPY_HEADER_SIZE     0x001000

#define XIN_PRIMARY_REF_NONE        7

#define XIN_LOG_MI_SIZE             2
#define XIN_MI_SIZE                 (1 << XIN_LOG_MI_SIZE)

#define XIN_ME_BUF_STRIDE           200
#define XIN_ME_FILTER_TAP           4

// The block is not completely outside the frame.
#define XIN_MB_PRESENT              (1 << 0)
#define XIN_MB_HAS_HOR              (1 << 1)
#define XIN_MB_HAS_VER              (1 << 2)

#define XIN_CLASS0_BITS             1 /* bits at integer precision for class 0 */
#define XIN_CLASS0_SIZE             (1 << XIN_CLASS0_BITS)
#define XIN_MV_OFFSET_BITS          (XIN_MV_CLASS_NUM + XIN_CLASS0_BITS - 2)
#define XIN_MV_BITS_CONTEXTS        6
#define XIN_MV_FP_SIZE              4

#define XIN_DELTA_Q_SMALL           3

#define XIN_CFL_JOINT_SIGN_NUM      (XIN_CFL_SIGN_NUM * XIN_CFL_SIGN_NUM - 1)
// CFL_SIGN_U is equivalent to (js + 1) / 3 for js in 0 to 8
#define XIN_CFL_SIGN_U(js)          (((js + 1) * 11) >> 5)
// CFL_SIGN_V is equivalent to (js + 1) % 3 for js in 0 to 8
#define XIN_CFL_SIGN_V(js)          ((js + 1) - XIN_CFL_SIGN_NUM * XIN_CFL_SIGN_U(js))

// There is no context when the alpha for a given plane is zero.
// So there are 2 fewer contexts than joint signs.
#define XIN_CFL_ALPHA_CONTEXT       (XIN_CFL_JOINT_SIGN_NUM + 1 - XIN_CFL_SIGN_NUM)
#define XIN_CFL_CONTEXT_U(js)       (js + 1 - XIN_CFL_SIGN_NUM)
// Also, the contexts are symmetric under swapping the planes.
#define XIN_CFL_CONTEXT_V(js)       (XIN_CFL_SIGN_V(js) * XIN_CFL_SIGN_NUM + XIN_CFL_SIGN_U(js) - XIN_CFL_SIGN_NUM)

#define XIN_MAX_LOOP_FILTER         63

#define XIN_OUTPUT_OBU              0
#define XIN_OUTPUT_IVF              1
#define XIN_IVF_FILE_HEADER_SIZE    32
#define XIN_IVF_FRAME_HEADER_SIZE   12

#define XIN_MAX_INPUT_SIZE          255
#define XIN_MAX_DPB_SIZE            16
#define XIN_MAX_RPS_NUM             64

#define XIN_LOG_UNIT_SIZE           4
#define XIN_LOW_UNIT_SIZE           (1 << XIN_LOG_UNIT_SIZE)
#define XIN_LOW_PADDING_SIZE        (XIN_LOW_UNIT_SIZE + 4)
#define XIN_PROP_FRAC_BITS          (XIN_LOG_UNIT_SIZE*2)
#define XIN_PROP_ROUNDING           (1 << (XIN_PROP_FRAC_BITS - 1))

#define XIN_RATE_FRACTION           9
#define XIN_LAMBDA_FRAC             7

#define XIN_DIST_SHIFT               (XIN_RATE_FRACTION + XIN_LAMBDA_FRAC - XIN_COST_FRACTION)
#define XIN_DIST_OFFSET              (1 << (XIN_DIST_SHIFT-1))
#define CALC_RD_COST(lambda, bit)   (((((UINT64)(lambda))*((UINT64)(bit))) + XIN_DIST_OFFSET) >> XIN_DIST_SHIFT)

#define PEL_XY_TO_BLOCK_INDEX(PELX, PELY, BLOCKINDEX, FRAMEWIDTH, LGBS) \
   (BLOCKINDEX) = ((PELY)>>(LGBS))*(FRAMEWIDTH) + ((PELX)>>(LGBS))

#endif

