/***************************************************************************//**
 *
 * @file          h26x_me_struct.h
 * @brief         This file contains h26x data structures related to motion estimation.
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
#ifndef _h26x_me_struct_h_
#define _h26x_me_struct_h_

#include "h26x_me_func_struct.h"

#define LFT_POS         0
#define RGT_POS         1
#define TOP_POS         2
#define BOT_POS         3
#define TOP_LFT_POS     4
#define TOP_RGT_POS     5
#define BOT_LFT_POS     6
#define BOT_RGT_POS     7
#define CEN_POS         8
#define POS_NUM         8

#define XIN_ME_FRAC_BIT_NUM  2
#define XIN_ME_PREC_QUARTER  4
#define XIN_ME_PREC_HALF     3
#define XIN_ME_PREC_INT      2
#define XIN_ME_PREC_4PEL     0
#define XIN_ME_PREC_INTERNAL (2 + XIN_ME_FRAC_BIT_NUM)

#define XIN_IMV_OFF          0
#define XIN_IMV_FPEL         1
#define XIN_IMV_4PEL         2
#define XIN_IMV_HPEL         3

// The maximum number of steps in a step search given the largest
// allowed initial step
#define XIN_MAX_SEARCH_ROUND    11
// Maximum size of the first step in full pel units
#define XIN_MAX_FIRST_STEP      (1 << (XIN_MAX_SEARCH_ROUND - 1))

typedef struct xin_check_point 
{
    UINT32      pointNum;
    xin_mv_s    offset[POS_NUM];
}xin_check_point;

typedef struct xin_me_struct
{
    xin_me_func funcSet;
    UINT32      width;
    UINT32      height;
    
    xin_mv_u    minMv;
    xin_mv_u    maxMv;

    xin_mv_u    meCand[12];
    UINT32      candNum;
    BOOL        biMe;
    BOOL        oneDimMe;
    
    xin_mv_u    bestMv;
    xin_mv_u    predMv;

    UINT32      bestCost;
    UINT32      bestSad;
    UINT32      bestPos;
    
    SINT32      searchWidth;
    SINT32      searchHeight;

    SINT32      iMvMode;

    // test zone
    SINT32      bestRound;
    SINT32      bestDist;
    SINT32      maxLgTZDist;
    SINT32      rasterDist;
    SINT32      bestMvX;
    SINT32      bestMvY;

    // diamond
    SINT32      numSearchRound;
    xin_mv_s    searchOffset[16][13];
    UINT32      searchRadius[16];
    UINT32      searchPoints[16];
    UINT32      skippedSR;
    UINT32      refinedSR;
    
    UINT32      sadLambda;
    UINT64      sseLambda;

    SINT32      refIdx;
    SINT32      listIdx;
    PIXEL       *input;
    intptr_t    inputStride;
    PIXEL       *input1;
    intptr_t    input1Stride;
    PIXEL       *input2;
    intptr_t    input2Stride;
    PIXEL       *ref;
    intptr_t    refStride;
    PIXEL       *ref1;
    intptr_t    ref1Stride;
    PIXEL       *ref2;
    intptr_t    ref2Stride;

    PIXEL       *interpBuf; // Allocate XIN_ME_BUF_STRIDExXIN_ME_BUF_STRIDEx4
    PIXEL       *halfPelH;
    PIXEL       *halfPelV;
    PIXEL       *halfPelHv;
    PIXEL       *integPel;
    intptr_t    interpStride;

    PIXEL       *halfPel[POS_NUM+1];
    PIXEL       *qBufA[POS_NUM+1][POS_NUM];
    PIXEL       *qBufB[POS_NUM+1][POS_NUM];
    
}xin_me_struct;

#endif