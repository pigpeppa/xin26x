/***************************************************************************//**
 *
 * @file          h265p_cfg_api.h
 * @brief         This file contains av1 encoder configuration API.
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

#ifndef _h265p_cfg_api_h_
#define _h265p_cfg_api_h_

typedef struct xin265p_cfg_api
{
    UINT32      profile;
    UINT32      level;
    UINT32      inputWidth;
    UINT32      inputHeight;
    UINT32      frameToBeEncoded;
    UINT32      bFrameNum;
    UINT32      refFrameNum;
    UINT32      temporalLayerNum;
    UINT32      refreshType;
    UINT32      intraPeriod;

    UINT32      encoderMode;

    UINT32      bitDepth;

    BOOL        stillPicture;
    BOOL        screenContentMode;

    BOOL        enableSao;
    UINT32      sbSize;
    BOOL        enableIntraBC;
    BOOL        enableFilterIntra;
    BOOL        enableIntraEdgeFilter;
    BOOL        enableInterIntraCompound;
    BOOL        enableMaskedCompound;
    BOOL        enableWarpedMotion;
    BOOL        enableDualFilter;
    BOOL        enableOrderHint;
    UINT32      orderHintBits;
    BOOL        enableCdef;
    BOOL        enableRestoration;
    UINT32      enablePartMask;
    
    BOOL        enableRdoq;

    BOOL        transSkipFlag;
    UINT32      outputFormat;

    UINT32      motionSearchMode;
    BOOL        fastSubMe;
    BOOL        biPredMe;
    UINT32      searchRange;

    BOOL        enableSmp;

    UINT32      frameThreadNum;
    UINT32      threadNum;
    BOOL        enableTiles;
    UINT32      numTileCols;
    UINT32      numTileRows;

    BOOL        enableAmp;

    BOOL        enableWpp;

    UINT32      qp;

    BOOL        disableDeblock;

    BOOL        fastRateEst;
    BOOL        fastIntraMd;
    BOOL        satdMd;
    BOOL        fastSao;
    BOOL        cuDepthPred;
    BOOL        cuDepthQuit;
    BOOL        cuModeQuit;
    BOOL        cuEarlySkip;
    BOOL        skipIntraMode;
    BOOL        enableSkipMe;
    BOOL        enableLTRef;
    UINT32      earlyStopMode;
    UINT32      maxMergeCand;
    UINT32      maxMdCandNum;
    UINT32      intraRdoNum;
    UINT32      mergeRdoNum;
    UINT32      intra4x4RdoNum;
    BOOL        rdoBi2Nx2N;
    BOOL        chromaFastCost;
    BOOL        adaptiveIFrame;
    BOOL        oneDimMe;
    BOOL        disableCu64;

    BOOL        calcPsnr;
    BOOL        calcSsim;

    UINT32      bitRate;
    float       frameRate;
    UINT32      lookAhead;
    BOOL        unitTree;
    double      unitTreeStrength;
    BOOL        enableFrameSkip;
    UINT32      rateControlMode;
    UINT32      minQp;
    UINT32      maxQp;
    
}xin265p_cfg_api;

#endif

