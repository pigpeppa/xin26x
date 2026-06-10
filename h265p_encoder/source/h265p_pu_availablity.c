/***************************************************************************//**
 *
 * @file          h265p_pu_availablity.c
 * @brief         Get one PU's neighbor block availability.
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
#include "assert.h"
#include "string.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"

void Xin265pGetBlockAvail (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    UINT32          puPelX;
    UINT32          puPelY;

    intptr_t        curMiIdx;
    SINT32          width;
    SINT32          height;

    xin_ref_picture *pictureWrite;
    xin_pic_struct  *picSet;

    xin_mi_struct   *topMi;
    xin_mi_struct   *lftMi;
    xin_mi_struct   *lftBotMi;
    xin_mi_struct   *topRgtMi;
    xin_mi_struct   *curMi;
    xin_mi_struct   *miBuf;
    intptr_t        miStride;

    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;

    width     = mb->width[PLANE_LUMA];
    height    = mb->height[PLANE_LUMA];

    puPelX    = (SINT32)(mb->mbPelX[PLANE_LUMA]);
    puPelY    = (SINT32)(mb->mbPelY[PLANE_LUMA]);

    miBuf    = pictureWrite->miBuf;
    miStride = pictureWrite->miStride;

    PEL_XY_TO_BLOCK_INDEX (puPelX, puPelY, curMiIdx, miStride, XIN_LOG_MI_SIZE);

    curMi    = miBuf + curMiIdx;
    lftMi    = curMi - 1;
    topMi    = curMi - miStride;
    topRgtMi = topMi + (width >> XIN_LOG_MI_SIZE);
    lftBotMi = lftMi + (height >> XIN_LOG_MI_SIZE);

    mb->lftBotMi = lftBotMi;
    mb->lftMi    = lftMi;
    mb->topRgtMi = topRgtMi;
    mb->topMi    = topMi;
    mb->curMi    = curMi;

}

