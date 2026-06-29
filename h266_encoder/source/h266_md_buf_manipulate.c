/***************************************************************************//**
 *
 * @file          h266_md_buf_manipulate.c
 * @brief         Mode decision buffer manipulation.
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
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h26x_trans_context.h"
#include "h266_bit_stream_struct.h"
#include "h266_picture_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "basic_macro.h"
#include "assert.h"

void Xin266SortMdBufSad (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum)
{
    UINT32  bufIdx0;
    UINT32  bufIdx1;

    if (bufNum <= 1)
    {
        return;
    }

    for (bufIdx0 = 0; bufIdx0 < bufNum - 1; bufIdx0++)
    {
        for (bufIdx1 = 0; bufIdx1 < bufNum - bufIdx0 - 1; bufIdx1++)
        {
            if (mdBuf[bufIdx1]->sadCost > mdBuf[bufIdx1+1]->sadCost)
            {
                XIN_SWAP(xin_fast_md_buf *, mdBuf[bufIdx1], mdBuf[bufIdx1 + 1]);
            }
        }
    }

}

void Xin266SortMdBufSse (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum)
{
    UINT32  bufIdx0;
    UINT32  bufIdx1;

    if (bufNum <= 1)
    {
        return;
    }

    for (bufIdx0 = 0; bufIdx0 < bufNum - 1; bufIdx0++)
    {
        for (bufIdx1 = 0; bufIdx1 < bufNum - bufIdx0 - 1; bufIdx1++)
        {
            if (mdBuf[bufIdx1]->sseCost > mdBuf[bufIdx1+1]->sseCost)
            {
                XIN_SWAP(xin_fast_md_buf *, mdBuf[bufIdx1], mdBuf[bufIdx1 + 1]);
            }
        }
    }

}

void Xin266FindHighestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **highestBuf)
{
    UINT32          bufIdx;
    xin_fast_md_buf *highestMdBuf;
    UINT64          maxSadCost;

    maxSadCost   = 0;
    highestMdBuf = mdBuf[0];

    for (bufIdx = 0; bufIdx < bufNum; bufIdx++)
    {
        if (mdBuf[bufIdx]->sadCost > maxSadCost)
        {
            maxSadCost   = mdBuf[bufIdx]->sadCost;
            highestMdBuf = mdBuf[bufIdx];

            if (maxSadCost == XIN_MAX_U64_COST)
            {
                break;
            }
        }
    }

    assert(highestMdBuf);

    *highestBuf = highestMdBuf;

}

void Xin266FindLowestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **lowestBuf)
{
    UINT32          bufIdx;
    xin_fast_md_buf *curBuf;

    curBuf = mdBuf[0];

    for (bufIdx = 1; bufIdx < bufNum; bufIdx++)
    {
        if (mdBuf[bufIdx]->sadCost < curBuf->sadCost)
        {
            curBuf = mdBuf[bufIdx];
        }
    }

    *lowestBuf = curBuf;

}

void Xin266FindFreeSseBuf (
    xin_full_md_buf **mdBuf,
    UINT32          bufNum,
    xin_full_md_buf **freeBuf)
{
    UINT32  bufIdx;

    *freeBuf = NULL;

    for (bufIdx = 0; bufIdx < bufNum; bufIdx++)
    {
        if (mdBuf[bufIdx]->sseCost == XIN_MAX_U64_COST)
        {
            *freeBuf = mdBuf[bufIdx];

            break;
        }
    }

    if (*freeBuf == NULL)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "There is no free sse buffer.\n"); 
    }

}


