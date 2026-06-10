/***************************************************************************//**
 *
 * @file          h265p_md_buf_manipulate.c
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
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "h265p_definition.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h26x_definition.h"
#include "h265p_picture_struct.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_me_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_md_buffer_struct.h"
#include "basic_macro.h"

void Xin265pSortMdBufSad (
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

void Xin265pSortMdBufSse (
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

void Xin265pFindHighestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **highestBuf)
{
    UINT32          bufIdx;
    xin_fast_md_buf *curBuf;

    curBuf = mdBuf[0];
    
    for (bufIdx = 1; bufIdx < bufNum; bufIdx++)
    {
        if (mdBuf[bufIdx]->sadCost > curBuf->sadCost)
        {
            curBuf = mdBuf[bufIdx];
        }
    }

    *highestBuf = curBuf;
    
}

void Xin265pFindLowestSadBuf (
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

