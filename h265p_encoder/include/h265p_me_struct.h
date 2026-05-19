/***************************************************************************//**
*
* @file          h265_me_struct.h
* @brief         This file contins h265 data structure related to motion estimation.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_me_struct_h_
#define _h265p_me_struct_h_

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

#define PRED_BUF_SIZE   (64*64)
#define PRED_BUF_STRIDE (64)

typedef struct xin_me_struct
{
    xin_mv_u    minMv;
    xin_mv_u    maxMv;

    xin_mv_u    meCand[8];
    BOOL        biMe;
    
    xin_mv_u    bestMv;
    xin_mv_u    predMv;

    UINT32      bestCost;
    UINT32      bestSad;

    SINT32      searchWidth;
    SINT32      searchHeight;

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

    PIXEL       *halfPel[POS_NUM];
    PIXEL       *qBufA[POS_NUM+1][POS_NUM];
    PIXEL       *qBufB[POS_NUM+1][POS_NUM];
    
}xin_me_struct;

#endif


