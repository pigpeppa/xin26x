/***************************************************************************//**
 *
 * @file          h26x_picture_struct.h
 * @brief         This file contains input picture description.
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
#ifndef _h26x_picture_struct_h_
#define _h26x_picture_struct_h_

typedef struct xin_rps_struct 
{
    SINT32  numOfNegPics;
    SINT32  numOfPosPics;
    SINT32  temporalId;
    BOOL    isRefFrame;
    
    SINT32  deltaNegPos[XIN_MAX_REF_FRAMES];
    SINT32  deltaPosPos[XIN_MAX_REF_FRAMES];
    BOOL    usedByNegPicFlag[XIN_MAX_REF_FRAMES];
    BOOL    usedByPosPicFlag[XIN_MAX_REF_FRAMES];
    
}xin_rps_struct;

typedef struct xin_fp_stat
{
    SINT64 intraError;
    SINT64 codedError;
    SINT64 secondCodedError;
    SINT32 mvCount;
    SINT32 interCount;
    SINT32 secondRefCount;
    double neutralCount;
    SINT32 intraSkipCount;
    SINT32 newMvCount;
    SINT32 sumInVectors;
    SINT32 sumMvY;
    SINT32 sumMvX;
    SINT32 sumMvYAbs;
    SINT32 sumMvXAbs;
    SINT64 sumSMvY;
    SINT64 sumSMvX;
    double intraFactor;
    double brightnessFactor;
} xin_fp_stat;

typedef struct xin_input_picture
{
    PIXEL           *inputBuffer;
    PIXEL           *inputBuf[PLANE_NUM];
    intptr_t        inputStride[2];
    SINT32          inputWidth;
    SINT32          inputHeight;
    SINT32          inputMarginX;
    SINT32          inputMarginY;
    
    PIXEL           *lowerBuffer;
    PIXEL           *lowerBuf;
    SINT32          lowerWidth;
    SINT32          lowerHeight;
    intptr_t        lowerStride;
    SINT32          lowerMarginX;
    SINT32          lowerMarginY;
    
    double          activity;

    double          *dqpMap;
    intptr_t        dqpMapStride;
    BOOL            isSceneCut;
    BOOL            isMctfFrame;
    xin_mv_u        *laMv[2];
    UINT8           *interDir;
    UINT64          totalCost;
    UINT32          *intraCost;
    UINT32          *interCost;
    UINT32          intraUnitNum;
    UINT64          gopCost;
    UINT64          avgBCost;
    UINT64          avgPCost;

    UINT32          encodeIdx;
    UINT32          predGopIdx;
    UINT32          temporalId;
    SINT32          predGopSize;
    SINT32          inputNumber;
    UINT32          frameType;
    UINT32          bufStage;
    double          avgOffset;
    UINT64          totalIntraCost;
    UINT32          interUnitNum;
    xin_rps_struct  rps;
    void            *pictureRead[XIN_LIST_NUM];

    UINT32          *rawMotionError;    
    xin_mv_u        *fpMv[2];
    intptr_t        fpMvStride;
    xin_fp_stat     *fpStat;
    
    double          *qpOffset;
    UINT16          *propCost;
    UINT32          laHgtInUnit;
    UINT32          laWdtInUnit;
    UINT32          laTotalUnit;
    UINT32          fpHgtInUnit;
    UINT32          fpWdtInUnit;
    UINT32          fpTotalUnit;
    XIN_HANDLE      fpLock;
    
}xin_input_picture;

#endif
