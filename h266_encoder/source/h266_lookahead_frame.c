/***************************************************************************//**
 *
 * @file          h266_lookahead_frame.c
 * @brief         This file computes lower resolution frame inter/intra cost.
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
#include "h26x_definition.h"
#include "xin26x_params.h"
#include "stdlib.h"
#include "string.h"
#include "memory.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h266_definition.h"
#include "basic_macro.h"
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
#include "h26x_common_data.h"
#include "h266_intra_prediction.h"
#include "h266_motion_est.h"
#include "h26x_thread_pool.h"
#include "h266_inter_prediction.h"
#include "h26x_thread_wrapper.h"
#include "assert.h"
#include "h266_lookahead_frame.h"
#include "h26x_motion_est.h"
#include "h266_enc_init.h"
#include "h26x_extend_picture.h"
#include "h26x_look_ahead.h"

static void Xin266LookaheadConstructPictureRead (
    xin_la_struct  *laSet,
    xin_seq_struct *seqSet)
{
    SINT32            targetPoc;
    SINT32            anchorPoc;
    xin_rps_struct    *rps;
    xin_input_picture *pictureRef;
    xin_input_picture *pictureWrite;
    xin_input_picture *pictureRead;
    UINT32            refIdx;
    UINT32            queueSize;

    queueSize    = XIN_QUEUE_SIZE (seqSet->lookaheadIdx, seqSet->encodeIdx);
    pictureWrite = laSet->pictureWrite;
    rps          = &pictureWrite->rps;
    anchorPoc    = pictureWrite->inputNumber;
    targetPoc    = anchorPoc - rps->deltaNegPos[0];

    if (targetPoc >= 0)
    {
        pictureRead = NULL;

        for (refIdx = 0; refIdx < queueSize; refIdx++)
        {
            pictureRef = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->encodeIdx + refIdx);

            if (targetPoc == pictureRef->inputNumber)
            {
                pictureRead = pictureRef;

                break;
            }
        }

        if (pictureRead == NULL)
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find refernece picture.\n");
        }
        else
        {
            if (rps->usedByNegPicFlag[0])
            {
                laSet->pictureRead[XIN_LIST_0] = pictureRead;
            }
        }

    }

    if (rps->numOfPosPics)
    {
        targetPoc   = anchorPoc + rps->deltaPosPos[0];
        pictureRead = NULL;

        if (targetPoc >= 0)
        {
            for (refIdx = 0; refIdx < queueSize; refIdx++)
            {
                pictureRef = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->encodeIdx + refIdx);

                if (targetPoc == pictureRef->inputNumber)
                {
                    pictureRead = pictureRef;

                    break;
                }
            }

            if (pictureRead == NULL)
            {
                _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find refernece picture.\n");
            }
            else
            {
                if (rps->usedByPosPicFlag[0])
                {
                    laSet->pictureRead[XIN_LIST_1] = pictureRead;
                }
            }

        }

    }
    else
    {
        laSet->pictureRead[XIN_LIST_1] = NULL;
    }

}

void Xin266LookaheadFrameInit (
    xin_la_struct     *laSet,
    xin_seq_struct    *seqSet)
{
    xin_input_picture *pictureWrite;

    pictureWrite = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->lookaheadIdx);

    if (pictureWrite == NULL)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find input picture.\n");
    }

    laSet->pictureWrite = pictureWrite;

    // Construct picture read
    if (pictureWrite->frameType <= XIN_I_FRAME)
    {
        Xin266LookaheadConstructPictureRead (
            laSet,
            seqSet);

    }
    else
    {
        laSet->pictureRead[XIN_LIST_0] = NULL;
        laSet->pictureRead[XIN_LIST_1] = NULL;
    }

    pictureWrite->pictureRead[0] = laSet->pictureRead[XIN_LIST_0];
    pictureWrite->pictureRead[1] = laSet->pictureRead[XIN_LIST_1];
    pictureWrite->bufStage       = XIN_BUF_LOOKAHEAD;

}

void Xin266LookaheadStage (
    xin266_encoder_struct *h266Encoder)
{
    xin_seq_struct *seqSet;
    xin_la_struct  *laSet;
    UINT32         gopDecIdx;
    UINT32        lookaheadIdx;
    BOOL          performStage;

    seqSet = h266Encoder->seqSet;
    laSet  = h266Encoder->laSet;

    // wait for previous thread to end
    if (laSet->isBusy)
    {
        Xin26xThreadWaitFor (
            seqSet->threadQueue,
            laSet->jobPostInit);

        seqSet->lookaheadIdx++;

    }

    gopDecIdx     = seqSet->gopDecIdx;
    lookaheadIdx  = seqSet->lookaheadIdx;
    performStage  = !!XIN_QUEUE_SIZE(gopDecIdx, lookaheadIdx);
    laSet->isBusy = performStage;

    if (performStage)
    {
        Xin266LookaheadFrameInit (
            laSet,
            seqSet);

        Xin26xLookaheadFrame (
            laSet);
    }

}

