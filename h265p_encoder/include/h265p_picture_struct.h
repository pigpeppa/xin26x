/***************************************************************************//**
*
* @file          h265p_picture_struct.h
* @brief         This file contins av1 inter reference data on frame level.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_picture_struct_h_
#define _h265p_picture_struct_h_

#include "h26x_picture_struct.h"

typedef struct xin_mi_struct
{
    xin_mv_u mv[2];
    SINT8    refFrame[2];
    UINT8    blockSize;
    UINT8    partType;

    UINT8    txSize;
    UINT8    predMode;
    UINT8    coeffSkip;
}xin_mi_struct;

typedef struct xin_ref_picture
{
    PIXEL             *refBuffer;
    PIXEL             *refBuf[PLANE_NUM];
    intptr_t          refStride[2];

    PIXEL             *ref1Buffer;
    PIXEL             *ref1Buf;
    intptr_t          ref1Stride;
    PIXEL             *ref2Buffer;
    PIXEL             *ref2Buf;
    intptr_t          ref2Stride;

    PIXEL             *inputBuf[PLANE_NUM];
    intptr_t          inputStride[2];

    UINT32            inputWidth;
    UINT32            inputHeight;
    UINT32            lumaWidth;
    UINT32            lumaHeight;
    UINT32            paddingWidth;
    UINT32            paddingHeight;
    UINT32            widthInPel4;
    UINT32            widthInPel8;
    UINT32            heightInPel4;
    UINT32            heightInPel8;
    UINT32            widthInMi;
    UINT32            heightInMi;
    BOOL              isFree;
    BOOL              isReferenced;
    BOOL              isAvail;
    SINT32            framePoc;
    UINT32            refFramePoc[2][XIN_MAX_REF_FRAMES];
    xin_frame_type    frameType;
    UINT32            refMode;
    UINT32            txMode;
    BOOL              isIntraFrame;
    BOOL              showFrame;
    BOOL              showableFrame;
    UINT32            obuType;
    
    xin_mi_struct     *miBuffer;
    xin_mi_struct     *miBuf;
    intptr_t          miStride;
    intptr_t          miSize;

    xin_rps_struct    *rps;

    UINT8             *lumaFlt[2];
    intptr_t          lumaFltStride[2];
    UINT8             *chromaFlt[2];
    intptr_t          chromaFltStride[2];
    SINT32            fltLvl[2];
    SINT32            fltLvlU;
    SINT32            fltLvlV;
    SINT32            sharpLvl;
    
    UINT8             *qpMap;

    SINT16            *qpOffset;
    UINT8             *qpNum;
    UINT8             refreshFrameFlags;
    BOOL              checkLDC;
    UINT32            temporalId;
    UINT32            gopIdx;
    UINT32            predIdxInGop;
    UINT32            predGopSize;
    SINT32            numOfNegPics;
    SINT32            numOfRefs[2];
    SINT32            numOfPosPics;

    UINT32            refPicNum[2];
    double            psnrYuv[PLANE_NUM];
    
}xin_ref_picture;


#endif

