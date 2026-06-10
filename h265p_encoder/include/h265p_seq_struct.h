/***************************************************************************//**
 *
 * @file          h265p_seq_struct.h
 * @brief         This file contains av1 sequence level structure.
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
#ifndef _h265p_seq_struct_h_
#define _h265p_seq_struct_h_

#include "h265p_cfg_api.h"
#include "h265p_func_struct.h"
#include "h26x_picture_struct.h"

typedef struct xin265p_tile_dim
{
    UINT32  tileIndex;
    UINT32  tileWidthInSb;
    UINT32  tileHeightInSb;
    UINT32  tileWidth;
    UINT32  tileHeight;
    UINT32  tilePelX;
    UINT32  tilePelY;
    UINT32  tileSbX;
    UINT32  tileSbY;
    UINT32  firstTsSb;
    UINT32  firstRsSb;
    UINT32  sbNumInTile;
} xin265p_tile_dim;

typedef struct xin_quant_param
{
    SINT32    zBin[2];
    SINT32    quant[2];
    SINT32    quantShift[2];
    SINT32    quantFp[2];
    SINT32    roundFp[2];
    SINT32    round[2];
    SINT32    dequant[2];
} xin_quant_param;

typedef struct xin_seq_struct
{
    xin265p_cfg_api   config;
    xin_func_struct   *funcSet;

    UINT32            vpsId;
    UINT32            spsId;
    UINT32            ppsId;

    UINT32            cpuFeature;

    UINT32            frameWidth;
    UINT32            frameHeight;

    UINT32            frameWidthInSb;
    UINT32            frameHeightInSb;
    UINT32            frameSizeInSb;

    UINT32            frameWidthInMi;
    UINT32            frameHeightInMi;

    UINT32            sbSize;
    UINT32            lgSbSize;

    UINT32            splitMask[XIN_MAX_MB_DEPTH];

    xin_rps_struct    *rpsSet;
    UINT32            rpsSize;
    UINT32            predGopSize;
    UINT32            *sbTsToRsAddrMap;

    xin_input_picture *allocInputPic;
    UINT32            allocInputPicNum;
    xin_input_picture *inputQueue[XIN_MAX_INPUT_SIZE];
    UINT32            inputSize;

    xin_ref_picture   *allocPictures;
    UINT32            allocPicNum;
    xin_ref_picture   *dpbQueue[XIN_MAX_DPB_SIZE];
    UINT32            dpbSize;

    xin_ref_picture   *refFrameMap[XIN_REF_FRAME_NUM];

    xin_lb_struct     *outputBuf;

    UINT32            rcGopSize;

    UINT8             *cabacContext;

    UINT32            bitsForPOC;

    UINT32            maxDepth;
    UINT32            cuNumInCtu;

    xin265p_tile_dim  tileDim[XIN_MAX_TILE_NUM];
    UINT32            tileNum;

    BOOL              intraRefresh;
    BOOL              inputFilled;
    UINT32            inputNumber;
    UINT32            frameNumber;
    UINT32            lookaheadNumber;
    UINT32            encodedNumber;
    BOOL              inputFlush;
    UINT32            flushIndex;
    UINT32            flushNumber;
    UINT32            inputIdrNumber;
    UINT32            frameIdrNumber;
    SINT32            craFramePoc;

    double            psnrYuv[PLANE_NUM];

} xin_seq_struct;

#endif
