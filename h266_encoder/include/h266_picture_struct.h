/***************************************************************************//**
 *
 * @file          h266_picture_struct.h
 * @brief         This file contains h266 inter reference data on frame level.
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
#ifndef _h266_picture_struct_h_
#define _h266_picture_struct_h_

#include "h26x_picture_struct.h"
#include "h26x_thread_struct.h"

typedef struct xin_block_struct
{
    xin_mv32_u mv[XIN_LIST_NUM];
    SINT8      refIdx[XIN_LIST_NUM];
    UINT16     cuPelX;
    UINT16     cuPelY;
    UINT8      height;
    UINT8      width;
    UINT8      qtDepth;
    UINT8      iLMode;
    UINT8      type;
    SINT8      affine;
    UINT8      affineType;
    UINT8      imvIdx;
    UINT8      bcwIdx;
}xin_block_struct;

typedef struct xin_neighbour_mv
{
    xin_mv32_u  mv[XIN_LIST_NUM];
    SINT8       refIdx[XIN_LIST_NUM];
    UINT8       bcwIdx;
    BOOL        useAltHpelIf;
} xin_neighbour_mv;

typedef struct xin_affine_cpmv
{
    xin_mv32_u  mv[XIN_LIST_NUM][XIN_MAX_AFFINE_CPMV_NUM];
    SINT8       refIdx[XIN_LIST_NUM];
    UINT8       affineType;
    UINT8       bcwIdx;
} xin_affine_cpmv;

typedef struct xin_depth_range
{
    UINT32  minDepth;
    UINT32  maxDepth;
}xin_depth_range;

typedef struct xin_rpl_struct 
{
    SINT32  numOfPics;
    SINT32  deltaPos[XIN_MAX_REF_FRAMES];
    
}xin_rpl_struct;

typedef struct xin_ref_picture
{
    PIXEL            *refBuffer;
    PIXEL            *refBuf[PLANE_NUM];
    intptr_t         refStride[2];

    PIXEL            *ref1Buffer;
    PIXEL            *ref1Buf;
    intptr_t         ref1Stride;
    PIXEL            *ref2Buffer;
    PIXEL            *ref2Buf;
    intptr_t         ref2Stride;

    PIXEL            *inputBuf[PLANE_NUM];
    intptr_t         inputStride[2];

    UINT32           inputWidth;
    UINT32           inputHeight;
    UINT32           lumaWidth;
    UINT32           lumaHeight;
    UINT32           paddingWidth;
    UINT32           paddingHeight;
    UINT32           widthInPel4;
    UINT32           widthInPel8;
    UINT32           heightInPel4;
    UINT32           heightInPel8;
    UINT32           widthInBlock;
    UINT32           heightInBlock;
    UINT32           blockSize;
    UINT32           lgBlockSize;
    BOOL             isReferenced;
    BOOL             colFromL0Flag;
    SINT32           framePoc;
    SINT32           refFramePoc[XIN_LIST_NUM][XIN_MAX_REF_FRAMES];
    UINT32           frameType;
    UINT32           nalType;
    xin_block_struct *blockSetMap;
    UINT32           blockSetWidth;
    UINT32           blockSetHeight;
    UINT32           blockSetSize;
    xin_rps_struct   *rps;
    xin_rpl_struct   rpl[XIN_LIST_NUM];

    xin_job_desc     *jobProFrame;
    xin_job_desc     *jobPreFrame;
    
    UINT8            *cbfMap;
    
    UINT8            *horBs;
    UINT8            *verBs;
    
    UINT8            *qpMap;
    UINT8            *qpUvMap;
    xin_depth_range  *drMap;

    double           *qpOffset;
    UINT8            *qpNum;
    double           avgQpOffset;
    BOOL             useRpsInSps[XIN_LIST_NUM];
    BOOL             checkLDC;
    BOOL             mvdL1Zero;
    UINT32           temporalId;
    UINT32           gopIdx;
    UINT32           predGopSize;
    UINT32           predIdxInGop;
    UINT32           numOfNegPics;
    UINT32           numOfPosPics;
    UINT32           numOfRefs[XIN_LIST_NUM];

    UINT32           refPicNum[XIN_LIST_NUM];
    
    double           psnrYuv[PLANE_NUM];
    
}xin_ref_picture;

#endif

