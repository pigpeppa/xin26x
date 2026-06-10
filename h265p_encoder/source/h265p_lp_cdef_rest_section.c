/***************************************************************************//**
 *
 * @file          h265p_lp_cdef_rest_section.c
 * @brief         Deblock, CDEF and restoration of a section in a frame.
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
#include "h265p_encode_sb.h"

void Xin265pDeblockSbRow (
    xin_sec_struct *secSet,
    xin_sb_struct  *sbRow,
    BOOL           isVert)
{
    UINT32           colIdx;
    xin_pic_struct   *picSet;
    xin265p_tile_dim *tileDim;
    UINT32           widthInSb;

    picSet    = secSet->picSet;
    tileDim   = secSet->tileDim;
    widthInSb = tileDim->tileWidthInSb;

    if (isVert)
    {
        for (colIdx = 0; colIdx < widthInSb; colIdx++)
        {
            Xin265pDeblockSbVer (
                picSet,
                sbRow + colIdx);
        }
    }
    else
    {
        for (colIdx = 0; colIdx < widthInSb; colIdx++)
        {
            Xin265pDeblockSbHor (
                picSet,
                sbRow + colIdx);
        }
    }

}

void Xin265pLfCdefRestSection (
    xin_sec_struct *secSet)
{
    UINT32           rowIdx;
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;
    xin265p_tile_dim *tileDim;
    UINT32           sbRowAddr;
    UINT32           firstTsAddr;
    UINT32           widthInSb;
    UINT32           heightInSb;

    sbRowAddr   = 0;
    seqSet      = secSet->seqSet;
    picSet      = secSet->picSet;
    tileDim     = secSet->tileDim;
    firstTsAddr = tileDim->firstTsSb;
    widthInSb   = tileDim->tileWidthInSb;
    heightInSb  = tileDim->tileHeightInSb;
    pictureWrite = picSet->pictureWrite;

    if ((pictureWrite->fltLvl[0] == 0) && (pictureWrite->fltLvl[1] == 0))
    {
        return;
    }

    for (rowIdx = 0; rowIdx < heightInSb; rowIdx++)
    {
        sbRowAddr = seqSet->sbTsToRsAddrMap[firstTsAddr + rowIdx*widthInSb];

        Xin265pDeblockSbRow (
            secSet,
            picSet->sb + sbRowAddr,
            TRUE);

        Xin265pDeblockSbRow (
            secSet,
            picSet->sb + sbRowAddr,
            FALSE);
    }
    
}
