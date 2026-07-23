/***************************************************************************//**
 *
 * @file          h266_coding_unit_struct.h
 * @brief         This file defines coding unit related structures.
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
#ifndef _h266_coding_unit_struct_h_
#define _h266_coding_unit_struct_h_

#define XIN_MAX_SUB_MV_STRIDE   16
#define XIN_MAX_SUB_MV_SIZE     (XIN_MAX_SUB_MV_STRIDE*XIN_MAX_SUB_MV_STRIDE)

typedef struct xin266_cu_info
{
    UINT32  width;
    UINT32  height;
    UINT32  offX;
    UINT32  offY;
    SINT32  childIdx;
    SINT32  parentIdx;
    UINT32  partIdx;
}xin266_cu_info;

typedef struct xin_affine_mv
{
    xin_mv32_u  mv[XIN_LIST_NUM][XIN_MAX_AFFINE_CPMV_NUM];
}xin_affine_mv;

typedef struct xin_cu_struct xin_cu_struct;

typedef struct xin_mode_struct
{
    BOOL             skipIntra;
    UINT64           sseCost;
    UINT64           sadCost;
    UINT64           sse;
    UINT32           sad;
    UINT32           rate;
    UINT8            lastSplit;
    UINT8            qtBeforeBt;
    UINT8            continuousSkip;
    SINT8            doMoreSplits;
    BOOL             didHorzSplit;
    BOOL             didVertSplit;
    UINT64           bestHorzCost;
    UINT64           bestVertCost;
    UINT64           bestUnsplitCost;
    SINT8            refIdx[XIN_LIST_NUM];
    xin_mv32_u       subMv[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM];
    SINT8            subRefIdx[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM];
    SINT8            bestBufIdx;
    BOOL             mvRefine;
    xin_mv_u         mvdL0SubPu[64];
    xin_mv32_u       affineMv[XIN_LIST_NUM][XIN_MAX_AFFINE_CPMV_NUM];
    xin_mv32_u       affinePredMv[XIN_LIST_NUM][XIN_MAX_AFFINE_CPMV_NUM];
    SINT32           cuGrad[XIN_CU_GRAD_NUM];
}xin_mode_struct;

typedef struct xin_cu_struct
{
    UINT32          cuPelX;
    UINT32          cuPelY;

    UINT32          offX;
    UINT32          offY;

    UINT32          treeMask;

    xin_cu_struct   *childCu[5];
    xin_cu_struct   *parentCu;

    UINT32          qp;

    xin_fast_md_buf *bestBuf;
    xin_mode_struct *modeCtrl;
    
    UINT8           type;
    UINT8           canSplit;
    UINT8           width;
    UINT8           height;
    UINT8           lgWidth;
    UINT8           lgHeight;
    UINT8           depth;
    UINT8           qtDepth;
    UINT8           mtDepth;
    
    UINT8           lgTuWidth[2];
    UINT8           lgTuHeight[2];

    UINT8           splitType;
    UINT8           partType;
    
    UINT16          partIdx;

    UINT8           partNum;
    UINT8           rootCbf;

    UINT8           geomFlag;
    
    UINT8           splitCtx;
    UINT8           qtCtx;
    UINT8           hvCtx;
    UINT8           horBtCtx;
    UINT8           verBtCtx;
    UINT32          skipContext;
    UINT32          affineContext;
    UINT32          predModeContext;

    xin_pu_struct   pu;
    
    xin_tu_struct   *tu[4];
    UINT32          tuNum;
    
}xin_cu_struct;

typedef struct xin_cu_list
{
    xin_cu_struct      *cuBuf;
    struct xin_cu_list *nextList;
}xin_cu_list;

#endif

