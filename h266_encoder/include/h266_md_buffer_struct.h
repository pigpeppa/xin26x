/***************************************************************************//**
 *
 * @file          h266_md_buffer_struct.h
 * @brief         This file contains h266 mode decision buffer structure definitions.
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
#ifndef _h266_md_buffer_struct_h_
#define _h266_md_buffer_struct_h_

#include "h26x_trans_context.h"

typedef struct xin_motion_data
{
    xin_mv32_u  bestMv[XIN_LIST_NUM];
    xin_mv32_u  predMv[XIN_LIST_NUM];
    UINT64      bestCost[XIN_LIST_NUM];
    UINT32      bestSad[XIN_LIST_NUM];
    SINT8       refIdx[XIN_LIST_NUM];
    UINT32      mvpIdx[XIN_LIST_NUM];
    PIXEL       *predBuf[XIN_LIST_NUM];
    UINT32      mvdBit[XIN_LIST_NUM];
    UINT32      idxBit[XIN_LIST_NUM];

    xin_mv32_u  tempMv[XIN_LIST_NUM][XIN_MAX_REF_FRAMES];
    xin_mv32_u  tempPredMv[XIN_LIST_NUM][XIN_MAX_REF_FRAMES];
    UINT32      tempMvpIdx[XIN_LIST_NUM][XIN_MAX_REF_FRAMES];
} xin_motion_data;

typedef struct xin_full_md_buf
{
    void            *bufBase[XIN_MTS_IDX_NUM];
    COEFF           *qCoefBuf[XIN_MTS_IDX_NUM][PLANE_NUM];
    COEFF           *tCoefBuf[XIN_MTS_IDX_NUM][PLANE_NUM];
    COEFF           *rCoefBuf[XIN_MTS_IDX_NUM][PLANE_NUM];
    PIXEL           *reconBuf[XIN_MTS_IDX_NUM][PLANE_NUM];

    intptr_t        coeffStride[2];

    UINT16          *coeffSign[XIN_MTS_IDX_NUM][PLANE_NUM];
    UINT16          *gt0BitMap[XIN_MTS_IDX_NUM][PLANE_NUM];
    UINT64          nzCGMapEs[XIN_MTS_IDX_NUM][4][PLANE_NUM];
    UINT64          nzCGMapRs[XIN_MTS_IDX_NUM][4][PLANE_NUM];
    UINT8           mtsIdx[PLANE_NUM];

    UINT64          sseCost;
    UINT64          sse;
    UINT32          rate;

    UINT32          yuvCbf[XIN_MTS_IDX_NUM][PLANE_NUM];
    UINT32          rootCbf;

    UINT32          tuNum;

} xin_full_md_buf;

typedef struct xin_fast_md_buf
{
    xin_full_md_buf *fullBuf;
    void            *bufBase;
    PIXEL           *predBuf[PLANE_NUM];

    intptr_t        lumaStride;
    intptr_t        chromaStride;

    UINT64          sadCost;
    UINT64          sseCost;

    UINT32          syntaxRate;

    UINT32          sad;
    UINT64          sse;

    BOOL            mergeFlag;
    BOOL            mvRefine;
    BOOL            affine;
    UINT32          affineType;
    UINT32          mergeIndex;
    SINT32          imvIdx;
    SINT32          bcwIdx;
    UINT8           didChromaMc;

    UINT32          mvpIndex[XIN_LIST_NUM];
    SINT8           refIdx[XIN_LIST_NUM];
    xin_mv32_u      mv[XIN_LIST_NUM];
    xin_mv32_u      predMv[XIN_LIST_NUM];
    xin_mv_s        mvdL0SubPu[64];
    xin_mv32_u      affineMv[XIN_LIST_NUM][3];
    xin_mv32_u      affinePredMv[XIN_LIST_NUM][3];
    UINT32          intraLumaMode;
    UINT32          multiRefIdx;
    UINT32          intraChromaMode;
    xin_motion_data meData;

    //UINT32          chromaCandList[8];

    UINT32          type;
} xin_fast_md_buf;

#endif

