/***************************************************************************//**
*
* @file          h265p_md_buffer_struct.h
* @brief         This file contins h265 mode decsion buffer structure definition.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_md_buffer_struct_h_
#define _h265p_md_buffer_struct_h_

typedef struct xin_motion_data
{
    xin_mv_u    bestMv[2];
    xin_mv_u    predMv[2];
    UINT64      bestCost[2];
    UINT32      bestSad[2];
    SINT16      refIdx[2];
    UINT32      mvpIdx[2];
    UINT8       *predBuf[2];
    UINT32      mvdBit[2];
    
    xin_mv_u    tempMv[2][XIN_MAX_REF_FRAMES];
    xin_mv_u    tempPredMv[2][XIN_MAX_REF_FRAMES];
    UINT32      tempMvpIdx[2][XIN_MAX_REF_FRAMES];
}xin_motion_data;

typedef struct xin_fast_md_buf
{
    PIXEL           *predBuf[PLANE_NUM];

    intptr_t        predStride[2];
    
    UINT64          sadCost;
    UINT64          sseCost;

    UINT32          syntaxRate;
    UINT32          coeffRate;
    
    UINT32          sad;
    UINT64          sse;
    
    xin_motion_data meData;

    UINT32          type;
    UINT32          predMode;
    UINT32          intraUvMode;
    UINT32          intraFilterMode;
    SINT32          angleDelta[2];
    BOOL            earlyStop;
    
}xin_fast_md_buf;

typedef struct xin_full_md_buf
{
    xin_fast_md_buf *fastBuf;
    COEFF           *qCoefBuf[PLANE_NUM];
    SINT32          *tCoefBuf[PLANE_NUM];
    SINT32          *rCoefBuf[PLANE_NUM];
    PIXEL           *reconBuf[PLANE_NUM];

    intptr_t        coefStride[2];
    
    UINT64          sseCost;

    UINT64          testCost;

    UINT64          coeffRate;

    BOOL            skipCoeff;

    UINT8           topCtx[PLANE_NUM][XIN_MAX_SB_SIZE>>XIN_LOG_MI_SIZE];
    UINT8           lftCtx[PLANE_NUM][XIN_MAX_SB_SIZE>>XIN_LOG_MI_SIZE];

    UINT32          tuNum;

    UINT32          tranSize[2];
    UINT32          tranType[2];
    UINT32          nzCount[5][PLANE_NUM];
    UINT8           culLevel[5][PLANE_NUM];
    UINT32          eob[5][PLANE_NUM];

    UINT8           dcSignCtx[5][PLANE_NUM];
    UINT8           txSkipCtx[5][PLANE_NUM];
}xin_full_md_buf;

#endif

