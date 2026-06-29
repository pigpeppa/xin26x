/***************************************************************************//**
 *
 * @file          h266_mctf.c
 * @brief         This file contains motion compensated temporal filter implementation.
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
#include "h266_enc_init.h"
#include "h26x_mctf_struct.h"
#include "h26x_mctf.h"
#include "h26x_extend_picture.h"

void Xin266MctfFrameInit (
    xin_mctf_struct *mctfSet,
    xin_seq_struct  *seqSet)
{
    xin_input_picture *inputPicture;
    xin_mctf_picture  *mctfPicture;
    UINT32            mctfIdx;
    UINT32            frameIdx;
    UINT32            mctfRange;

    mctfIdx      = seqSet->mctfIdx;
    inputPicture = XIN_QUEUE_DATA (seqSet->inputQueue, mctfIdx);
    mctfPicture  = mctfSet->pictureWrite;
    mctfRange    = mctfSet->mctfRefNum >> 1;

    mctfPicture->input[0]       = inputPicture->inputBuf[0];
    mctfPicture->input[1]       = inputPicture->inputBuf[1];
    mctfPicture->input[2]       = inputPicture->inputBuf[2];
    mctfPicture->inputStride[0] = inputPicture->inputStride[0];
    mctfPicture->inputStride[1] = inputPicture->inputStride[1];
    mctfPicture->inputNumber    = inputPicture->inputNumber;
    inputPicture->isMctfFrame   = TRUE;

    mctfSet->ctuDqpMap       = inputPicture->dqpMap;
    mctfSet->ctuDqpMapStride = inputPicture->dqpMapStride;

    memset (mctfSet->ctuDqpMap, 0, sizeof(double)*seqSet->frameSizeInCtu);

    if (inputPicture->inputNumber == seqSet->mctfLastIdr)
    {
        // future
        for (frameIdx = 0; frameIdx < mctfRange; frameIdx++)
        {
            inputPicture = XIN_QUEUE_DATA (seqSet->inputQueue, mctfIdx + 1 + frameIdx);
            mctfPicture  = mctfSet->pictureRead[frameIdx];

            mctfPicture->input[0]       = inputPicture->inputBuf[0];
            mctfPicture->input[1]       = inputPicture->inputBuf[1];
            mctfPicture->input[2]       = inputPicture->inputBuf[2];
            mctfPicture->inputStride[0] = inputPicture->inputStride[0];
            mctfPicture->inputStride[1] = inputPicture->inputStride[1];
            mctfPicture->inputNumber    = inputPicture->inputNumber;
        }

        mctfSet->validRefNum = mctfRange;
        mctfSet->strength    = 1.5;

    }
    else
    {
        // past
        for (frameIdx = 0; frameIdx < mctfRange; frameIdx++)
        {
            inputPicture = XIN_QUEUE_DATA (seqSet->inputQueue, mctfIdx - mctfRange + frameIdx);
            mctfPicture  = mctfSet->pictureRead[frameIdx];

            mctfPicture->input[0]       = inputPicture->inputBuf[0];
            mctfPicture->input[1]       = inputPicture->inputBuf[1];
            mctfPicture->input[2]       = inputPicture->inputBuf[2];
            mctfPicture->inputStride[0] = inputPicture->inputStride[0];
            mctfPicture->inputStride[1] = inputPicture->inputStride[1];
            mctfPicture->inputNumber    = inputPicture->inputNumber;
        }

        // future
        for (frameIdx = 0; frameIdx < mctfRange; frameIdx++)
        {
            inputPicture = XIN_QUEUE_DATA (seqSet->inputQueue, mctfIdx + 1 + frameIdx);
            mctfPicture  = mctfSet->pictureRead[frameIdx + mctfRange];

            mctfPicture->input[0]       = inputPicture->inputBuf[0];
            mctfPicture->input[1]       = inputPicture->inputBuf[1];
            mctfPicture->input[2]       = inputPicture->inputBuf[2];
            mctfPicture->inputStride[0] = inputPicture->inputStride[0];
            mctfPicture->inputStride[1] = inputPicture->inputStride[1];
            mctfPicture->inputNumber    = inputPicture->inputNumber;
        }

        mctfSet->validRefNum = mctfRange*2;
        mctfSet->strength    = 1.5;

    }

}
void Xin266MctfStage (
    xin266_encoder_struct *h266Encoder)
{
    xin_mctf_struct   *mctfSet;
    xin_seq_struct    *seqSet;
    xin_input_picture *pictureWrite;
    SINT32            scenecutIdx;
    SINT32            mctfIdx;
    SINT32            gopSize;
    BOOL              performStage;

    mctfSet  = h266Encoder->mctfSet;
    seqSet   = h266Encoder->seqSet;
    gopSize  = seqSet->config.bFrameNum + 1;
    gopSize  = XIN_MAX (gopSize, XIN_MAX_GOP_SIZE);

    // wait for previous thread to end
    if (mctfSet->isBusy)
    {
        Xin26xThreadWaitFor (
            seqSet->threadQueue,
            mctfSet->jobMctfBim);

        pictureWrite = XIN_QUEUE_DATA (seqSet->inputQueue, seqSet->mctfIdx);
        seqSet->mctfIdx++;
    }

    mctfIdx         = seqSet->mctfIdx;
    scenecutIdx     = seqSet->scenecutIdx;
    performStage    = XIN_QUEUE_SIZE (scenecutIdx, mctfIdx) >= (SINT32)(mctfSet->mctfRefNum + 1);
    mctfSet->isBusy = performStage;

    if (performStage)
    {
        pictureWrite = XIN_QUEUE_DATA (seqSet->inputQueue, mctfIdx);

        if (pictureWrite->isSceneCut || (pictureWrite->inputNumber - seqSet->mctfLastIdr) == (SINT32)seqSet->config.intraPeriod)
        {
            seqSet->mctfLastIdr = pictureWrite->inputNumber;
        }

        if ((pictureWrite->inputNumber - seqSet->mctfLastIdr) % gopSize == 0)
        {
            Xin266MctfFrameInit (
                mctfSet,
                seqSet);

            Xin26xMctfFrame (
                mctfSet);
        }
    }
    else if ((mctfIdx != scenecutIdx) && (seqSet->flushIdx == scenecutIdx))
    {
        seqSet->mctfIdx++;
    }

}
