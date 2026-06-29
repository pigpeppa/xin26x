/***************************************************************************//**
 *
 * @file          h266_rate_control.c
 * @brief         Subroutines related to rate control.
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
#include "xin_typedef.h"
#include "h26x_common_data.h"
#include "stdlib.h"
#include "string.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "h266_constant.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_section_struct.h"
#include "h266_rate_control.h"
#include "h26x_thread_wrapper.h"
#include "h26x_rate_control_struct.h"
#include "h26x_calc_log.h"
#include "h26x_rate_control.h"

SINT32 Xin266RcCreate (
    xin_seq_struct *seqSet)
{
    xin_rc_struct *rcSet;

    XIN_MALLOC_CHECK (rcSet, sizeof(xin_rc_struct));

    rcSet->rcGopSize         = seqSet->predGopSize;
    rcSet->bitRate           = seqSet->config.bitRate;
    rcSet->vbvBufSize        = seqSet->config.vbvBufSize;
    rcSet->vbvMaxRate        = seqSet->config.vbvMaxRate;
    rcSet->frameRate         = seqSet->config.frameRate;
    rcSet->bFrameNum         = seqSet->config.bFrameNum;
    rcSet->frameSizeInCtu    = seqSet->frameSizeInCtu;
    rcSet->inputWidth        = seqSet->config.inputWidth;
    rcSet->inputHeight       = seqSet->config.inputHeight;
    rcSet->intraPeriod       = seqSet->config.intraPeriod;
    rcSet->screenContentMode = seqSet->config.screenContentMode;
    rcSet->rateControlMode   = seqSet->config.rateControlMode;
    rcSet->initialQp         = seqSet->config.qp;
    rcSet->crf               = seqSet->config.crf;
    rcSet->minQp             = seqSet->config.minQp;
    rcSet->maxQp             = seqSet->config.maxQp;
    rcSet->unitTreeStrength  = seqSet->config.unitTreeStrength;
    rcSet->frameToBeEncoded  = seqSet->config.frameToBeEncoded;
    rcSet->unitTree          = seqSet->config.unitTree;
    rcSet->encodeQueue       = seqSet->encodeQueue;
    rcSet->laSatdMd          = seqSet->config.laSatdMd;

    if ((seqSet->config.rateControlMode == XIN_RC_ABR) || (seqSet->config.rateControlMode == XIN_RC_CRF))
    {
        rcSet->pfXinRcGop        = Xin26xRcAbrGop;
        rcSet->pfXinRcPic        = Xin26xRcAbrPic;
        rcSet->pfXinRcUpdatePic  = Xin26xRcAbrUpdatePic;
        rcSet->pfXinRcCreate     = Xin26xRcAbrCreate;
        rcSet->pfXinRcDelete     = Xin26xRcAbrDelete;
        rcSet->pfXinRcCtu        = Xin26xRcAbrCtu;
        rcSet->pfXinRcUpdateCtu  = Xin26xRcAbrUpdateCtu;
        rcSet->pfXinRcSkipDec    = Xin26xRcAbrSkipDec;
        rcSet->pfXinRcUpdateSkip = Xin26xRcAbrUpdateSkip;
    }
    else
    {
        rcSet->pfXinRcGop        = Xin26xRcCbrGop;
        rcSet->pfXinRcPic        = Xin26xRcCbrPic;
        rcSet->pfXinRcUpdatePic  = Xin26xRcCbrUpdatePic;
        rcSet->pfXinRcCreate     = Xin26xRcCbrCreate;
        rcSet->pfXinRcDelete     = Xin26xRcCbrDelete;
        rcSet->pfXinRcCtu        = Xin26xRcCbrCtu;
        rcSet->pfXinRcUpdateCtu  = Xin26xRcCbrUpdateCtu;
        rcSet->pfXinRcSkipDec    = Xin26xRcCbrSkipDec;
        rcSet->pfXinRcUpdateSkip = Xin26xRcCbrUpdateSkip;
    }

    rcSet->pfXinRcCreate (
        rcSet);

    rcSet->lockHandle = Xin26xConstructLock ();

    seqSet->rcSet = rcSet;

    return TRUE;

}

void Xin266RcPic (
    xin_pic_struct *picSet)
{
    xin_rc_context  *rcContext;
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_rc_struct   *rcSet;
    SINT32          ctuIdx;

    seqSet       = picSet->seqSet;
    rcSet        = seqSet->rcSet;
    rcContext    = &picSet->rcContext;
    pictureWrite = picSet->pictureWrite;

    rcContext->rcSet = rcSet;

    rcSet->pfXinRcPic (
        rcContext);

    if (seqSet->config.unitTree)
    {
        for (ctuIdx = 0; ctuIdx < (SINT32)seqSet->frameSizeInCtu; ctuIdx++)
        {
            pictureWrite->qpOffset[ctuIdx] = pictureWrite->qpOffset[ctuIdx] * rcContext->unitTreeStrength - rcContext->avgQpOffset;
        }

    }

    picSet->picQp = rcContext->picQp;
    
}

void Xin266RcUpdatePic (
    xin_pic_struct *picSet,
    SINT32         outputBits,
    SINT32         headerBits)
{
    xin_rc_context *rcContext;
    xin_rc_struct  *rcSet;

    rcContext = &picSet->rcContext;
    rcSet     = rcContext->rcSet;

    rcSet->pfXinRcUpdatePic (
        rcContext,
        outputBits,
        headerBits);
}

void Xin266RcSkipDec (
    xin_pic_struct *picSet)
{
    xin_rc_context *rcContext;
    xin_rc_struct  *rcSet;

    rcContext = &picSet->rcContext;
    rcSet     = rcContext->rcSet;

    picSet->codingFrame = rcSet->pfXinRcSkipDec (rcContext);
}

void Xin266RcGop (
    xin_pic_struct *picSet,
    UINT32         gopSize)
{
    xin_rc_context *rcContext;
    xin_rc_struct  *rcSet;

    rcContext = &picSet->rcContext;
    rcSet     = rcContext->rcSet;

    rcSet->pfXinRcGop (
        rcContext,
        gopSize);
}

void Xin266RcCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_rc_context  *rcContext;
    xin_ref_picture *pictureWrite;
    xin_rc_struct   *rcSet;
    xin_rc_abr_pic  *pictureRc;
    xin_rc_abr_ctu  *ctuRc;

    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    rcContext    = &picSet->rcContext;
    rcSet        = rcContext->rcSet;
    pictureRc    = (xin_rc_abr_pic *)rcContext->pictureRc;
    pictureWrite = picSet->pictureWrite;
    secSet->qp   = rcSet->pfXinRcCtu (rcContext, ctu->ctuAddr);

    if (seqSet->config.unitTree)
    {
        secSet->qp += (SINT32)pictureWrite->qpOffset[ctu->ctuAddr];
        secSet->qp  = XIN_CLIP (secSet->qp, XIN_MIN_QP, XIN_MAX_QP);
    }

    if (seqSet->config.rateControlMode == XIN_RC_ABR)
    {
        ctuRc     = pictureRc->rcCtu + ctu->ctuAddr;
        ctuRc->qp = secSet->qp;
    }
    
}

void Xin266RcUpdateCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_rc_context *rcContext;
    xin_rc_cbr_ctu *ctuCbr;
    xin_rc_cbr_pic *pictureCbr;
    xin_pic_struct *picSet;
    xin_rc_struct  *rcSet;

    picSet    = secSet->picSet;
    rcContext = &picSet->rcContext;
    rcSet     = rcContext->rcSet;

    if (rcSet->rateControlMode == XIN_RC_CBR)
    {
        pictureCbr = (xin_rc_cbr_pic *)rcContext->pictureRc;
        ctuCbr     = pictureCbr->rcCtu + ctu->ctuAddr;

        ctuCbr->qp      = secSet->qp;
        ctuCbr->bitUsed = ctu->bitUsed;
    }

    rcSet->pfXinRcUpdateCtu (
        rcContext,
        ctu->ctuAddr);

}

void Xin266RcUpdateSkip (
    xin_pic_struct *picSet)
{
    xin_rc_context *rcContext;
    xin_rc_struct  *rcSet;

    rcContext = &picSet->rcContext;
    rcSet     = rcContext->rcSet;

    rcSet->pfXinRcUpdateSkip (
        rcContext);
}

void Xin266RcDelete (
    xin_seq_struct *seqSet)
{
    xin_rc_struct *rcSet;

    rcSet = seqSet->rcSet;

    rcSet->pfXinRcDelete (
        rcSet);

    Xin26xDestructLock (
        rcSet->lockHandle);

    free (rcSet);
}

