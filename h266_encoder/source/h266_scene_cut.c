/***************************************************************************//**
 *
 * @file          h266_scene_cut.c
 * @brief         Scene cut detection.
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
#include "h266_func_struct.h"

void Xin266SceneCutFrame (
    xin_sc_struct *scSet)
{
    xin_input_picture *pictureWrite;
    xin_input_picture *pictureRead;
    xin_func_struct   *funcSet;

    pictureRead  = scSet->pictureRead;
    pictureWrite = scSet->pictureWrite;
    funcSet      = scSet->funcSet;

    funcSet->pfXinDownscale2x2 (
        pictureWrite->inputBuf[0],
        pictureWrite->inputStride[0],
        FALSE,
        FALSE,
        pictureWrite->lowerBuf,
        pictureWrite->lowerStride,
        pictureWrite->inputWidth / 2,
        pictureWrite->inputHeight / 2);

    if (!pictureWrite->inputNumber)
    {
        return;
    }

    funcSet->pfXinComputeFrameAct (
        pictureWrite->lowerBuf,
        pictureWrite->lowerStride,
        pictureRead->lowerBuf,
        pictureRead->lowerStride,
        pictureWrite->inputWidth >> 1,
        pictureWrite->inputHeight >> 1,
        &pictureWrite->activity);

    if ((pictureWrite->activity * 64 > pictureRead->activity * 181) && (pictureWrite->inputNumber - scSet->lastSceneCut >= 16))
    {
        pictureWrite->isSceneCut = TRUE;
        scSet->lastSceneCut      = pictureWrite->inputNumber;
    }

}

void Xin266SceneCutFrameWrapper (
        void *opaque)
{
    xin_sc_struct *scSet;

    scSet = (xin_sc_struct *)opaque;
    
    Xin266SceneCutFrame (
        scSet);
}

void Xin266SceneCutStage (
    xin266_encoder_struct *h266Encoder)
{
    xin_seq_struct   *seqSet;
    xin_sc_struct    *scSet;
    xin_job_desc     *jobSceneCut;
    UINT32           inputIdx;
    UINT32           scenecutIdx;
    BOOL             performStage;

    seqSet = h266Encoder->seqSet;
    scSet  = h266Encoder->scSet;

    // wait for previous thread to end
    if (scSet->isBusy)
    {
        Xin26xThreadWaitFor (
            seqSet->threadQueue,
            scSet->jobSceneCut);

        seqSet->scenecutIdx++;
    }

    inputIdx      = seqSet->inputIdx;
    scenecutIdx   = seqSet->scenecutIdx;
    performStage  = !!XIN_QUEUE_SIZE(inputIdx, scenecutIdx);
    scSet->isBusy = performStage;

    if (performStage)
    {
        scSet->pictureWrite = XIN_QUEUE_DATA (seqSet->inputQueue, scenecutIdx);
        scSet->pictureRead  = XIN_QUEUE_DATA (seqSet->inputQueue, scenecutIdx - 1);
        scSet->isBusy       = TRUE;
        jobSceneCut         = scSet->jobSceneCut;

        Xin26xJobInit (
            jobSceneCut,
            Xin266SceneCutFrameWrapper,
            scSet);

        Xin26xThreadSubmit (
            seqSet->threadQueue,
            jobSceneCut);
    }

}
