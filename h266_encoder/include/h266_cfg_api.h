/***************************************************************************//**
 *
 * @file          h266_cfg_api.h
 * @brief         This file contains h266 encoder configuration API.
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

#ifndef _h266_cfg_api_h_
#define _h266_cfg_api_h_

typedef struct xin266_cfg_api
{
    UINT32      inputWidth;
    UINT32      inputHeight;
    SINT32      frameToBeEncoded;
    SINT32      bFrameNum;
    UINT32      refFrameNum;
    UINT32      temporalLayerNum;
    UINT32      refreshType;
    UINT32      intraPeriod;

    UINT32      inputBitDepth;
    UINT32      internalBitDepth;

    UINT32      encoderMode;
    UINT32      bitDepth;

    UINT32      ctuSize;
    UINT32      minQtSize;

    UINT32      maxBtSize;
    UINT32      minBtSize;

    UINT32      maxTtSize;
    UINT32      minTtSize;

    UINT32      maxMttDepth;
    BOOL        lumaTrSize64;

    UINT32      maxTrSize;
    UINT32      minCuSize;

    BOOL        screenContentMode;
    BOOL        offlineMode;
    BOOL        zeroLatency;
    BOOL        twoPassEncoder;
    BOOL        enableMctf;
    UINT32      mctfMode;
    UINT32      mctfRefNum;
    UINT32      mctfUnitSize;
    BOOL        enableMts;
    BOOL        enableSceneCut;
    UINT32      quitSkipDepths;

    BOOL        enableGpb;

    BOOL        enableSao;
    BOOL        enableAlf;
    BOOL        enableCcAlf;
    UINT32      alfMode;

    BOOL        enableStrongIntraSmoothing;
    BOOL        enableTMvp;

    BOOL        enableSignDataHiding;
    BOOL        enableRdoq;
    UINT32      rdoqThrVal;

    BOOL        constrainedIntraPredFlag;
    BOOL        enableCclm;
    BOOL        verCollocatedChroma;
    BOOL        enableDmvr;
    BOOL        enableSbtMvp;
    BOOL        enableAffine;
    UINT32      affineType;
    BOOL        enableDepQuant;
    BOOL        enableLmcs;
    BOOL        enableAmvr;
    BOOL        enableBcw;
    BOOL        enableBim;

    BOOL        transSkipFlag;
    UINT32      maxTrSkipLgSize;

    BOOL        enableCuQpDelta;
    UINT32      diffCuQpDeltaDepth;

    UINT32      motionSearchMode;
    UINT32      fastSubMe;
    BOOL        biPredMe;
    UINT32      searchRange;

    UINT32      frameThreadNum;
    UINT32      threadNum;
    BOOL        enableTiles;
    UINT32      numTileCols;
    UINT32      numTileRows;

    BOOL        enableWpp;

    UINT32      qp;

    UINT32      qpInVal[6];
    UINT32      qpOutVal[6];
    UINT32      numPtsInQpTable;

    BOOL        disableDeblock;
    
    BOOL        fastRateEst;
    BOOL        satdMd;
    BOOL        laSatdMd;
    BOOL        laSubMe;
    BOOL        fastSao;
    BOOL        cuDepthPred;
    BOOL        cuDepthQuit;
    BOOL        cuModeQuit;
    BOOL        cuEarlySkip;
    BOOL        skipIntraMode;
    BOOL        enableSkipMe;
    BOOL        disableBigInter;
    BOOL        enableLTRef;
    BOOL        fastNonRef;
    BOOL        fastNonQt;
    UINT32      fastIntraMd;
    UINT32      qtbttSpeedUp;
    UINT32      fastTTSplit;
    BOOL        gradientFastQtbt;
    UINT32      maxMergeCand;
    UINT32      maxAffineMergeCand;
    UINT32      maxMdCandNum;
    UINT32      intraRdoNum;
    BOOL        chromaFastCost;
    BOOL        unfoldRefList0;
    BOOL        adaptiveBFrame;
    BOOL        oneDimMe;
    UINT32      intraLgDec;
    UINT32      tuCoefDepthQuit;
    BOOL        cuSkipDepthQuit;

    BOOL        calcPsnr;
    BOOL        calcSsim;
    BOOL        needRecon;

    UINT32      bitRate;
    UINT32      crf;
    float       frameRate;
    SINT32      lookAhead;
    BOOL        unitTree;
    SINT32      vbvBufSize;
    SINT32      vbvMaxRate;
    double      unitTreeStrength;
    BOOL        enableFrameSkip;
    UINT32      rateControlMode;
    UINT32      minQp;
    UINT32      maxQp;
    SINT32      chromaQpOffset;
}xin266_cfg_api;

#endif

