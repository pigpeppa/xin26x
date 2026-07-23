/***************************************************************************//**
 *
 * @file          h266_seq_struct.h
 * @brief         This file contains h266 sequence level structure.
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
#ifndef _h266_seq_struct_h_
#define _h266_seq_struct_h_

#include "h266_cfg_api.h"
#include "h266_picture_struct.h"
#include "xin26x_params.h"
#include "h266_cabac_struct.h"
#include "h26x_trans_context.h"
#include "h26x_thread_struct.h"
#include "h266_trans_unit_struct.h"

typedef struct xin_rc_struct xin_rc_struct;
typedef struct xin_sec_struct xin_sec_struct;
typedef struct xin_mode_list xin_mode_list;
typedef struct xin_mode_struct xin_mode_struct;
typedef struct xin_cu_list xin_cu_list;
typedef struct xin_cu_struct xin_cu_struct;
typedef struct xin_tu_list xin_tu_list;
typedef struct xin_tu_struct xin_tu_struct;
typedef struct xin_func_struct xin_func_struct;

typedef struct xin266_tile_dim 
{
    UINT32  tileIndex;
    UINT32  tileWidthInCtu;
    UINT32  tileHeightInCtu;
    UINT32  tileWidth;
    UINT32  tileHeight;
    UINT32  tilePelX;
    UINT32  tilePelY;
    UINT32  tileCtuX;
    UINT32  tileCtuY;
    UINT32  firstTsCtu;
    UINT32  firstRsCtu;
    UINT32  ctuNumInTile;
}xin266_tile_dim;

typedef struct xin_quant_param
{
    SINT32    qMult;          
    SINT32    qShift;       
    SINT32    iqMult;
    SINT32    iqShift;
}xin_quant_param;

typedef struct xin_nb_info_sbb
{
  UINT8   numInv;
  UINT8   invInPos[5];
}xin_nb_info_sbb;

typedef struct xin_nb_info_out
{
  UINT16  maxDist;
  UINT16  num;
  UINT16  outPos[5];
}xin_nb_info_out;

typedef struct xin_scan_info
{
    SINT32          sbbSize;
    SINT32          numSbb;
    SINT32          scanIdx;
    SINT32          rasterPos;
    SINT32          sbbPos;
    SINT32          insidePos;
    UINT32          spt;
    UINT32          sigCtxOffsetNext;
    UINT32          gtxCtxOffsetNext;
    SINT32          nextInsidePos;
    SINT32          nextSbbRight;
    SINT32          nextSbbBelow;
    SINT32          posX;
    SINT32          posY;
    xin_nb_info_sbb currNbInfoSbb;
} xin_scan_info;

typedef struct xin_tu_param
{
    UINT32          chType;
    UINT32          width;
    UINT32          height;
    SINT32          numCoeff;
    UINT32          numSbb;
    UINT32          log2SbbWidth;
    UINT32          log2SbbHeight;
    UINT32          log2SbbSize;
    SINT32          sbbSize;
    SINT32          sbbMask;
    SINT32          widthInSbb;
    SINT32          heightInSbb;
    xin_scan_pos    *scanSbbId2SbbPos;
    xin_scan_pos    *scanId2BlkPos;
    xin_nb_info_sbb *scanId2NbInfoSbb;
    xin_nb_info_out *scanId2NbInfoOut;
    xin_scan_info   *scanInfo;
}xin_tu_param;

typedef struct xin_dp_param
{
    SINT32  qShift;
    SINT32  qAdd;
    SINT32  qScale;
    SINT32  maxQIdx;
    SINT32  thresLast;
    SINT32  thresSSbb;
    SINT32  iqAdd;
    SINT32  iqShift;
    SINT32  iqScale;

    SINT32  distShift;
    SINT32  distAdd;
    SINT64  distStepAdd;
    SINT64  distOrgFact;
}xin_dp_param;

typedef struct xin_seq_struct
{
    xin266_cfg_api    config;
    xin_func_struct   *funcSet;
    xin_rc_struct     *rcSet;
    xin_thread_queue  *threadQueue;
    xin_sec_struct    **secSet;
    
    UINT32            vpsId;
    UINT32            dpsId;
    UINT32            spsId;
    UINT32            ppsId;
    
    UINT32            cpuFeature;
    UINT32            cpuCoreNum;

    UINT32            frameWidth;
    UINT32            frameHeight;

    UINT32            laWdtInUnit;
    UINT32            laHgtInUnit;
    UINT32            laTotalUnit;

    UINT32            lowerWidth;
    UINT32            lowerHeight;
    
    UINT32            frameWidthInCtu;
    UINT32            frameHeightInCtu;
    UINT32            frameSizeInCtu;

    UINT32            frameWidthInBlock;
    UINT32            frameHeightInBlock;
    
    UINT32            cuNumInCtu;

    UINT32            maxTuSize;
    UINT32            ctuSize;
    UINT32            lgCtuSize;

    UINT32            laUnitSize;
    UINT32            fpUnitSize;

    UINT32            blockSize;
    UINT32            lgBlockSize;
    UINT32            blockSetWidth;
    UINT32            blockSetHeight;

    xin_depth_range   *drConfig;

    xin_rpl_struct    *rplList[XIN_LIST_NUM];
    UINT32            rplNum[XIN_LIST_NUM];
    xin_rps_struct    *rpsSet;
    UINT32            rpsSize;
    UINT32            predGopSize;
    UINT32            initGopSize;
    UINT32            *ctuTsToRsAddrMap;

    xin_scan_pos      *scanOrder[XIN_MAX_LG_CG_SIZE+1][XIN_MAX_LG_CG_SIZE+1];
    xin_scan_pos      *scanOrderCG[XIN_MAX_LG_TU_SIZE+1][XIN_MAX_LG_TU_SIZE+1];

    xin_input_picture *inputFrame;
    UINT32            inputFrameNum;

    xin_input_picture *inputList[XIN_MAX_FRAME_NUM];
    UINT32            inputListNum;
    
    xin_input_picture *inputQueue[XIN_MAX_FRAME_NUM];
    xin_input_picture *encodeQueue[XIN_MAX_FRAME_NUM];

    XIN_HANDLE        *unitListLock;

    UINT32            inputIdx;
    UINT32            scenecutIdx;
    UINT32            mctfIdx;
    UINT32            gopDecIdx;
    UINT32            lookaheadIdx;
    UINT32            preEncodeIdx;
    UINT32            encodeIdx;
    SINT32            outputIdx;
    SINT32            flushIdx;
    BOOL              encodeFinished;
    
    SINT32            gopLastIdr;
    SINT32            mctfLastIdr;
    SINT32            encLastIdr;
    
    SINT8             gopSizeBuf[XIN_MAX_REF_FRAMES];

    xin_ref_picture   *refFrame;
    UINT32            refFrameNum;
    xin_ref_picture   *refList[XIN_MAX_FRAME_NUM];
    UINT32            refListNum;
    
    xin_ref_picture   *dpbQueue[XIN_MAX_DPB_SIZE];
    xin_ref_picture   *longTermRef;
    UINT32            dpbSize;

    xin_nb_info_sbb   *scanId2NbInfoSbb[XIN_MAX_LG_TU_SIZE+1][XIN_MAX_LG_TU_SIZE+1];
    xin_nb_info_out   *scanId2NbInfoOut[XIN_MAX_LG_TU_SIZE+1][XIN_MAX_LG_TU_SIZE+1];
    xin_tu_param      *tuParam[XIN_MAX_LG_TU_SIZE+1][XIN_MAX_LG_TU_SIZE+1][2];
    xin_dp_param      *dpParam[XIN_MAX_LG_TU_SIZE+XIN_MAX_LG_TU_SIZE+1];

    xin_quant_param   *quantParam[2];

    xin_lb_struct     *outputBuf;

    UINT32            encoderPass;
    UINT32            rcGopSize;
    UINT32            maxTemporalId;

    xin_prob_model    *cabacContext;

    UINT32            bitsForPOC;

    xin266_tile_dim   tileDim[XIN_MAX_TILE_NUM];
    UINT32            tileNum;

    xin266_tile_dim   frameDim;

    xin_mem_list      scratchMem;

    UINT32            chromaQpMap[XIN_QP_NUM];
    const UINT32      *chromaWeight;

    BOOL              intraRefresh;
    SINT32            currEncoder;
    SINT32            inputNumber;
    SINT32            craFramePoc;

    double            psnrYuv[PLANE_NUM];
    
}xin_seq_struct;
#endif

