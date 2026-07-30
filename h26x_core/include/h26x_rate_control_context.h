/***************************************************************************//**
 *
 * @file          h26x_rate_control_context.h
 * @brief         Shared rate-control core definitions (primarily for CBR and
 *                the common infrastructure).
 *
 *                As of the current refactoring step (moving CBR), only the
 *                structures needed by the shared CBR implementation and the
 *                top-level rate-control container are defined here.
 *
 *                The detailed per-mode structs for HM / ABR / VBR remain in
 *                the per-encoder headers for now (they are still used by
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

#ifndef _h26x_rate_control_context_h_
#define _h26x_rate_control_context_h_

typedef struct xin_rc_context xin_rc_context;
typedef struct xin_rc_struct xin_rc_struct;

typedef void    (*XinRcGop)          (xin_rc_context *rcContext, UINT32 gopSize);
typedef void    (*XinRcPic)          (xin_rc_context *rcContext);
typedef void    (*XinRcUpdatePic)  (xin_rc_context *rcContext, SINT32 outputBits, SINT32 headerBits);
typedef SINT32  (*XinRcCreate)      (xin_rc_struct  *rcSet);
typedef SINT32  (*XinRcCtu)          (xin_rc_context *rcContext, UINT32 ctuAddr);
typedef void    (*XinRcUpdateCtu)  (xin_rc_context *rcContext, UINT32 ctuAddr);
typedef BOOL    (*XinRcSkipDec)     (xin_rc_context *rcContext);
typedef void    (*XinRcUpdateSkip) (xin_rc_context *rcContext);
typedef void    (*XinRcDelete)      (xin_rc_struct  *rcSet);

typedef struct xin_rc_struct
{
    void              *rc;
    XinRcGop          pfXinRcGop;
    XinRcPic          pfXinRcPic;
    XinRcUpdatePic    pfXinRcUpdatePic;
    XinRcCreate       pfXinRcCreate;
    XinRcCtu          pfXinRcCtu;
    XinRcUpdateCtu    pfXinRcUpdateCtu;
    XinRcSkipDec      pfXinRcSkipDec;
    XinRcUpdateSkip   pfXinRcUpdateSkip;
    XinRcDelete       pfXinRcDelete;
    XIN_HANDLE        lockHandle;

    UINT32            bitRate;
    UINT32            vbvBufSize;
    UINT32            vbvMaxRate;
    double            frameRate;
    UINT32            rcGopSize;
    SINT32            bFrameNum;
    UINT32            frameSizeInCtu;
    UINT32            inputWidth;
    UINT32            inputHeight;
    SINT32            intraPeriod;
    BOOL              screenContentMode;
    UINT32            rateControlMode;
    UINT32            initialQp;
    UINT32            crf;
    UINT32            minQp;
    UINT32            maxQp;
    double            unitTreeStrength;
    UINT32            frameToBeEncoded;
    BOOL              unitTree;
    BOOL              laSatdMd;
    xin_input_picture **encodeQueue;
} xin_rc_struct;

typedef struct xin_rc_context 
{
    void              *pictureRc;
    xin_input_picture *pictureInput;
    xin_rc_struct     *rcSet;
    UINT32            picQp;
    UINT32            qp;
    UINT32            bitUsed;
    double            avgQpOffset;
    BOOL              isReferenced;
    double            unitTreeStrength;
}xin_rc_context;

#endif
