/***************************************************************************//**
*
* @file          h26x_definition.h
* @brief         This file contains definitions for all video coding mode.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h26x_definition_h_
#define _h26x_definition_h_

#define XIN_ALG_H265             0
#define XIN_ALG_AV1              1
#define XIN_ALG_H266             2

#define XIN_CLIENT_UNKNOWN       0
#define XIN_CLIENT_001           1
#define XIN_CLIENT_002           2
#define XIN_CLIENT_003           3
#define XIN_CLIENT_004           4
#define XIN_CLIENT_NUM           5

#define XIN_BUF_INPUT            0
#define XIN_BUF_LOOKAHEAD        1
#define XIN_BUF_ENCODE           2
#define XIN_BUF_INVALID          0xFF

#define XIN_MAX_TILE_NUM         34
#define XIN_MAX_FRAME_THREAD     6
#define XIN_MAX_FP_THREAD        128
#define XIN_MAX_LA_THREAD        8
#define XIN_MAX_THREAD_POOL_NUM  128
#define XIN_MAX_DEP_JOB_NUM      16

#define XIN_JOB_STATE_PAUSED     0
#define XIN_JOB_STATE_WAITING    1
#define XIN_JOB_STATE_READY      2
#define XIN_JOB_STATE_RUNNING    3
#define XIN_JOB_STATE_DONE       4

#define XIN_RC_OFF               0
#define XIN_RC_CARMERA           1
#define XIN_RC_SCREEN            2
#define XIN_RC_CBR               3
#define XIN_RC_HM                4
#define XIN_RC_ABR               5
#define XIN_RC_CRF               6
#define XIN_RC_VBR               7
#define XIN_RC_MODE_NUM          8
#define XIN_RC_MAX_ENTRY         5
#define XIN_RC_MAX_GEAR          8
#define XIN_RC_MAX_GOP_SIZE      16
#define XIN_RC_MAX_PIC_SIZE      18
#define XIN_RC_ALLOC_PIC_NUM     32

#define XIN_MIN_REF_FRAMES       1
#define XIN_MAX_REF_FRAMES       6

#define XIN_LIST_0               0
#define XIN_LIST_1               1
#define XIN_LIST_NUM             2

#define XIN_CRA_REFRESH          0
#define XIN_IDR_REFRESH          1

#define XIN_STABLE_REGION        0
#define XIN_HIGH_VAR_REGION      1
#define XIN_SCENECUT_REGION      2
#define XIN_BLENDING_REGION      3
#define XIN_REGION_NUM           4

#define XIN_COST_FRACTION        8
#define XIN_MAX_UV_QP_DIF        10

#define XIN_ME_FULL_SEARCH       0
#define XIN_ME_BBDGS_SEARCH      1
#define XIN_ME_HIER_SEARCH       2
#define XIN_ME_TZ_SEARCH         3
#define XIN_ME_DIAMOND_SEARCH    4

#define XIN_MAX_FRAME_WIDTH      3840
#define XIN_MAX_FRAME_HEIGHT     2160

#define XIN_MAX_FRAME_NUM        2000
#define XIN_MAX_FP_FRAME_NUM     150

#define XIN_PADDING_OFFSET_X     16
#define XIN_PADDING_OFFSET_Y     16

#define XIN_INPUT_PADDING_X      128
#define XIN_INPUT_PADDING_Y      128

#define XIN_LOWER_PADDING_X      16
#define XIN_LOWER_PADDING_Y      16
#define XIN_LA_INTRA_NUM         5
#define XIN_LA_MAX_SIZE          16

#define XIN_LA_INTRA_PLANAR      0
#define XIN_LA_INTRA_DC          1
#define XIN_LA_INTRA_HOR         2
#define XIN_LA_INTRA_DIG         3
#define XIN_LA_INTRA_VEC         4
#define XIN_LA_INTRA_NUM         5

#define XIN_TF_ME_LEVEL_NUM      4
#define XIN_TF_MAX_REF_NUM       8
#define XIN_TF_MAX_SECTION_NUM   4
#define XIN_TF_SCALE_BIT_NUM     15
#define XIN_TF_SCALE_OFFSET      (1<<(XIN_TF_SCALE_BIT_NUM - 1))
#define XIN_TF_RANGE             (XIN_TF_MAX_REF_NUM>>1)
#define XIN_TF_UNIT_SIZE         16

#define XIN_MAX_U64_COST         ((UINT64)1 << 60)
#define XIN_MAX_U32_COST         (1 << 29)
#define XIN_MAX_U16              ((1 << 16) - 1)
#define XIN_MAX_S8               ((1 << 7) - 1)
#define XIN_MAX_S16              ((1 << 15) - 1)
#define XIN_MAX_S32              (((SINT64)1 << 31) - 1)
#define XIN_MIN_S8               (-XIN_MAX_S8 - 1)
#define XIN_MIN_S16              (-XIN_MAX_S16 - 1)
#define XIN_MIN_S32              (-XIN_MAX_S32 - 1)
#define XIN_MAX_U32              (0xFFFFFFFF)

#define XIN_8_BIT_DEPTH          8
#define XIN_10_BIT_DEPTH         10
#define XIN_12_BIT_DEPTH         12

#ifdef ENABLE_10BIT_ENCODER
#define XIN_INTERNAL_BIT_DEPTH   XIN_10_BIT_DEPTH
#else
#define XIN_INTERNAL_BIT_DEPTH   XIN_8_BIT_DEPTH
#endif

#define XIN_LUMA_MASK            (1<<PLANE_LUMA)
#define XIN_CHROMA_U_MASK        (1<<PLANE_CHROMA_U)
#define XIN_CHROMA_V_MASK        (1<<PLANE_CHROMA_V)
#define XIN_CHROMA_MASK          ((1<<PLANE_CHROMA_U)|(1<<PLANE_CHROMA_V))
#define XIN_ALL_PLANE_MASK       ((XIN_LUMA_MASK) | (XIN_CHROMA_MASK))

#define XIN_B_FRAME              0
#define XIN_P_FRAME              1
#define XIN_I_FRAME              2
#define XIN_IDR_FRAME            3
#define XIN_FRAME_TYPE           4

#define XIN_SIMD_WIDTH           16

#define XIN_BLOCK_1xH            0
#define XIN_BLOCK_2xH            1
#define XIN_BLOCK_4xH            2
#define XIN_BLOCK_8xH            3
#define XIN_BLOCK_16xH           4
#define XIN_BLOCK_32xH           5
#define XIN_BLOCK_64xH           6
#define XIN_BLOCK_128xH          7
#define XIN_BLOCK_NUM            8

#define _XIN_LOGGER              pfXin26xLogEntry

#define CALC_MVD_BIT(mvdX, mvdY)              (ExpGolombBits[(mvdX) + 16383] + ExpGolombBits[(mvdY) + 16383])
#define CALC_MVD_BIT_SHIFT(mvdX, mvdY, shift) (ExpGolombBits[((mvdX)>>(shift)) + 16383] + ExpGolombBits[((mvdY)>>(shift)) + 16383])

#if defined(__linux__) || defined(__ANDROID__) || defined(__APPLE__)
#define DLLEXPORT __attribute__((visibility("default")))
#else
#define DLLEXPORT
#endif

#define XIN_QUEUE_SIZE(WRIDX, RDIDX) ((WRIDX >= RDIDX) ? (WRIDX - RDIDX) : (WRIDX + XIN_MAX_FRAME_NUM - RDIDX))
#define XIN_QUEUE_DATA(QUEUE, IDX)   QUEUE[((IDX) + XIN_MAX_FRAME_NUM) % XIN_MAX_FRAME_NUM]
#define XIN_QUEUE_IDX(IDX)           ((IDX) % XIN_MAX_FRAME_NUM)

#endif