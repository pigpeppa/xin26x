/***************************************************************************//**
 *
 * @file          h265p_macro_block_struct.h
 * @brief         This file defines av1 macro block related structure.
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
#ifndef _h265p_macro_block_struct_h_
#define _h265p_macro_block_struct_h_

#include "h265p_md_buffer_struct.h"

typedef struct xin_mb_struct xin_mb_struct;

typedef struct xin_mb_struct
{
    UINT16          mbPelX[2];
    UINT16          mbPelY[2];

    UINT8           offX[2];
    UINT8           offY[2];

    UINT8           blockSize;
    UINT8           lgWidth[2];
    UINT8           lgHeight[2];
    UINT8           width[2];
    UINT8           height[2];
    UINT8           depth;
    UINT8           geomFlag;
    SINT8           rgtEdgeMi;
    SINT8           botEdgeMi;

    xin_full_md_buf *bestBuf;
    xin_mb_struct   *childMb[4];
    xin_mb_struct   *parentMb;

    UINT8           *mbTopCtx[PLANE_NUM];
    UINT8           *mbLftCtx[PLANE_NUM];

    UINT8           nebRefCount[XIN_REF_FRAME_NUM];

    xin_mi_struct   *curMi;
    xin_mi_struct   *topMi;
    xin_mi_struct   *lftMi;
    xin_mi_struct   *topRgtMi;
    xin_mi_struct   *lftBotMi;
    xin_tu_struct   *tu[PLANE_NUM];
    UINT8           txType;

    BOOL            skipCoeff;
    UINT8           mbType;
    UINT8           predMode;
    UINT8           intraFilterMode;
    UINT8           intraUvMode;
    SINT32          intraAngleDelta[2];

    xin_mv_u        predMv[2];
    xin_mv_u        mv[2];
    SINT8           refFrame[2];

    UINT32          intraLftAvail;
    UINT32          intraTopAvail;

    UINT8           txSize;
    UINT8           tuNum;

    UINT8           partitionCtx;
    UINT8           skipModeCtx;
    UINT8           skipCoeffCtx;
    UINT8           intraInterCtx;
    UINT8           refModeCtx;

    UINT64          sseCost;
    UINT64          sadCost;

    UINT8           splitType;
    UINT8           partType;
    UINT8           partIdx;
    UINT32          canSplit;

} xin_mb_struct;

#endif

