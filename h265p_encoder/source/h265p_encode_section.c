/***************************************************************************//**
 *
 * @file          h265p_encode_section.c
 * @brief         Encode a picture section in a frame.
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
#include "stdlib.h"
#include "string.h"
#include "xin26x_params.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_picture_struct.h"
#include "h265p_seq_struct.h"
#include "h265p_trans_context.h"
#include "h265p_pic_struct.h"
#include "h265p_trans_context.h"
#include "h265p_common_data.h"
#include "h265p_cabac_struct.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h265p_enc_init.h"
#include "h265p_encode_sb.h"

void Xin265pEncodeSection (
    xin_sec_struct *secSet)
{
    xin_pic_struct   *picSet;
    xin_seq_struct   *seqSet;
    xin265p_tile_dim *tileDim;
    UINT32           firstTsAddr;
    UINT32           sbNumInTile;
    UINT32           sbIndex;
    UINT32           sbRsAddr;
    xin_sb_struct    *sb;

    Xin265pSectionInit (
        secSet);

    picSet      = secSet->picSet;
    seqSet      = secSet->seqSet;
    tileDim     = secSet->tileDim;
    firstTsAddr = tileDim->firstTsSb;
    sbNumInTile = tileDim->sbNumInTile;

    for (sbIndex = 0; sbIndex < sbNumInTile; sbIndex++)
    {
        sbRsAddr = seqSet->sbTsToRsAddrMap[firstTsAddr + sbIndex];
        sb       = picSet->sb + sbRsAddr;

        Xin265pSbInit (
            secSet,
            sb);

        Xin265pReadInputSb (
            secSet,
            sb);

        Xin265pEncodeSb (
            secSet,
            sb);

        Xin265pComputeBsSb (
            secSet);

        Xin265pWriteSb (
            secSet,
            sb);

    }

    Xin265pLfCdefRestSection (
        secSet);

}

