/***************************************************************************//**
 *
 * @file          h266_unit_tree.c
 * @brief         h266 encoder unit tree computation.
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
#include "stdio.h"
#include "string.h"
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
#include "h26x_calc_log.h"
#include "h26x_common_data.h"
#include "h26x_thread_wrapper.h"
#include "h26x_thread_pool.h"
#include "h26x_unit_tree.h"

static void Xin266CalcGopCost (
    xin_seq_struct *seqSet)
{
    UINT64            gopCost;
    SINT32            encodeIdx;
    SINT32            queueSize;
    UINT64            totalPCost;
    UINT64            totalBCost;
    UINT32            pFrameNum;
    UINT32            bFrameNum;
    xin_input_picture *encodePicture;
    xin_input_picture *inputPicture;

    gopCost      = 0;
    inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx);
    bFrameNum    = 0;
    pFrameNum    = 0;
    totalPCost   = 0;
    totalBCost   = 0;
    queueSize    = XIN_QUEUE_SIZE (seqSet->lookaheadIdx, seqSet->preEncodeIdx);

    for (encodeIdx = 1; encodeIdx < queueSize; encodeIdx++)
    {
        encodePicture = XIN_QUEUE_DATA (seqSet->encodeQueue, encodeIdx + seqSet->preEncodeIdx);

        if (encodePicture->frameType >= XIN_I_FRAME)
        {
            continue;
        }

        if ((encodePicture->frameType >= XIN_P_FRAME) || ((encodePicture->frameType == XIN_B_FRAME) && (encodePicture->predGopIdx == 0)))
        {
            totalPCost += encodePicture->totalCost;
            pFrameNum  += 1;
        }
        else
        {
            totalBCost += encodePicture->totalCost;
            bFrameNum += 1;
        }

    }

    inputPicture->avgPCost  = pFrameNum ? totalPCost / pFrameNum : 0;
    inputPicture->avgBCost  = bFrameNum ? totalBCost / bFrameNum : 0;

    queueSize = XIN_MIN (inputPicture->predGopSize, queueSize);

    for (encodeIdx = 0; encodeIdx < queueSize; encodeIdx++)
    {
        encodePicture = XIN_QUEUE_DATA (seqSet->encodeQueue, encodeIdx + seqSet->preEncodeIdx);
        gopCost      += encodePicture->totalCost;
        
    }

    inputPicture->gopCost = queueSize ? gopCost / queueSize : 0;

}

void Xin266UnitTree (
    xin_seq_struct *seqSet)
{
    SINT32            inputIdx;
    xin_input_picture *inputPicture;
    SINT32            queueSize;

    Xin266CalcGopCost (
        seqSet);

    queueSize = XIN_QUEUE_SIZE (seqSet->lookaheadIdx, seqSet->preEncodeIdx);
    queueSize = XIN_MIN (queueSize, seqSet->config.lookAhead);

    for (inputIdx = queueSize - 1; inputIdx >= 0; inputIdx--)
    {
        inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx + inputIdx);

        memset (inputPicture->propCost, 0, sizeof(UINT16)*inputPicture->laTotalUnit);

    }

    for (inputIdx = queueSize - 1; inputIdx > 0; inputIdx--)
    {
        inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx + inputIdx);

        Xin26xUnitPropagate (
            inputPicture,
            seqSet->laUnitSize);
    }

    inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx);
    queueSize    = XIN_MIN (queueSize, (SINT32)inputPicture->predGopSize);

    for (inputIdx = 0; inputIdx < queueSize; inputIdx++)
    {
        inputPicture = XIN_QUEUE_DATA (seqSet->encodeQueue, seqSet->preEncodeIdx + inputIdx);

        Xin26xUnitTreeFinish (
            inputPicture);
    }

}


