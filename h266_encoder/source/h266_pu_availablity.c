/***************************************************************************//**
 *
 * @file          h266_pu_availablity.c
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
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
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
#include "h266_coding_unit_struct.h"

static xin_block_struct nonAvailBlock =
{
    {{0}, {0}},
    {0, 0},
    0,
    0,
    0xFF,
    0xFF,
    0,
    XIN_INTRA_DC,
    XIN_INVALID_MODE,
    0,
    0,
    XIN_BCW_DEFAULT
};

/*
   // B2 |                B1 | B0
   //  -----------------------
   //    |                   |
   //    |                   |
   //    |                   |
   //    |                   |
   //    |                   |
   //    |                   |
   //    |                   |
   // A1 |                   |
   //  --|-------------------|
   // A0
*/
void Xin266GetBlockAvail (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu)
{
    UINT32           puPelX;
    UINT32           puPelY;
    SINT32           puLftPelX;
    SINT32           puTopPelY;
    SINT32           puRgtPelX;
    SINT32           puBotPelY;

    UINT32           curBlockIdx;
    UINT32           lftBotBlockIdx;
    UINT32           lftBlockIdx;
    UINT32           topLftBlockIdx;
    UINT32           topBlockIdx;
    UINT32           topRgtBlockIdx;
    SINT32           width;
    SINT32           height;

    UINT32           curCtuIdx;
    UINT32           lftBotCtuIdx;
    UINT32           lftCtuIdx;
    UINT32           topLftCtuIdx;
    UINT32           topCtuIdx;
    UINT32           topRgtCtuIdx;
    UINT32           sliceIdx;
    BOOL             enableWpp;

    SINT32           frameWidthInCtu;
    SINT32           blockSetWidth;
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;

    xin_block_struct *lftBotBlock;
    xin_block_struct *lftBBlock;
    xin_block_struct *topLftBlock;
    xin_block_struct *topRBlock;
    xin_block_struct *topRgtBlock;
    xin_block_struct *curBlock;
    xin_block_struct *blockMap;

    xin_ctu_struct   *lftBotCtu;
    xin_ctu_struct   *lftBCtu;
    xin_ctu_struct   *topLftCtu;
    xin_ctu_struct   *topRCtu;
    xin_ctu_struct   *topRgtCtu;
    xin_ctu_struct   *curCtu;
    xin_ctu_struct   *ctuMap;

    SINT32           frameHeight;
    SINT32           frameWidth;

    seqSet          = secSet->seqSet;
    picSet          = secSet->picSet;
    frameHeight     = seqSet->frameHeight;
    frameWidth      = seqSet->frameWidth;
    frameWidthInCtu = seqSet->frameWidthInCtu;
    pictureWrite    = picSet->pictureWrite;
    blockSetWidth   = pictureWrite->blockSetWidth;
    enableWpp       = seqSet->config.enableWpp || (!seqSet->config.enableTiles);

    width     = pu->width;
    height    = pu->height;

    puPelX    = (SINT32)(secSet->cu->cuPelX);
    puPelY    = (SINT32)(secSet->cu->cuPelY);
    puLftPelX = puPelX - 1;
    puTopPelY = puPelY - 1;
    puRgtPelX = puPelX + width;
    puBotPelY = puPelY + height;

    PEL_XY_TO_BLOCK_INDEX (puPelX,      puPelY,      curBlockIdx,    blockSetWidth, seqSet->lgBlockSize);
    PEL_XY_TO_BLOCK_INDEX (puLftPelX,   puBotPelY,   lftBotBlockIdx, blockSetWidth, seqSet->lgBlockSize);
    PEL_XY_TO_BLOCK_INDEX (puLftPelX,   puBotPelY-1, lftBlockIdx,    blockSetWidth, seqSet->lgBlockSize);
    PEL_XY_TO_BLOCK_INDEX (puRgtPelX,   puTopPelY,   topRgtBlockIdx, blockSetWidth, seqSet->lgBlockSize);
    PEL_XY_TO_BLOCK_INDEX (puLftPelX,   puTopPelY,   topLftBlockIdx, blockSetWidth, seqSet->lgBlockSize);
    PEL_XY_TO_BLOCK_INDEX (puRgtPelX-1, puTopPelY,   topBlockIdx,    blockSetWidth, seqSet->lgBlockSize);

    blockMap    = pictureWrite->blockSetMap;
    curBlock    = blockMap + curBlockIdx;
    lftBBlock   = blockMap + lftBlockIdx;
    topRBlock   = blockMap + topBlockIdx;
    topLftBlock = blockMap + topLftBlockIdx;
    topRgtBlock = blockMap + topRgtBlockIdx;
    lftBotBlock = blockMap + lftBotBlockIdx;

    PEL_XY_TO_BLOCK_INDEX (puPelX,      puPelY,      curCtuIdx,    frameWidthInCtu, seqSet->lgCtuSize);
    PEL_XY_TO_BLOCK_INDEX (puLftPelX,   puBotPelY,   lftBotCtuIdx, frameWidthInCtu, seqSet->lgCtuSize);
    PEL_XY_TO_BLOCK_INDEX (puLftPelX,   puBotPelY-1, lftCtuIdx,    frameWidthInCtu, seqSet->lgCtuSize);
    PEL_XY_TO_BLOCK_INDEX (puRgtPelX,   puTopPelY,   topRgtCtuIdx, frameWidthInCtu, seqSet->lgCtuSize);
    PEL_XY_TO_BLOCK_INDEX (puLftPelX,   puTopPelY,   topLftCtuIdx, frameWidthInCtu, seqSet->lgCtuSize);
    PEL_XY_TO_BLOCK_INDEX (puRgtPelX-1, puTopPelY,   topCtuIdx,    frameWidthInCtu, seqSet->lgCtuSize);

    ctuMap    = picSet->ctu;
    curCtu    = ctuMap + curCtuIdx;
    lftBCtu   = ctuMap + lftCtuIdx;
    topRCtu   = ctuMap + topCtuIdx;
    topLftCtu = ctuMap + topLftCtuIdx;
    topRgtCtu = ctuMap + topRgtCtuIdx;
    lftBotCtu = ctuMap + lftBotCtuIdx;
    sliceIdx  = curCtu->sliceIndex;

    if ((puLftPelX < 0) || (puBotPelY >= frameHeight) || ((sliceIdx != lftBotCtu->sliceIndex) && (!enableWpp)))
    {
        secSet->lftBotBlock = &nonAvailBlock;
    }
    else
    {
        secSet->lftBotBlock = lftBotBlock;
    }

    if ((puLftPelX < 0) || ((sliceIdx != lftBCtu->sliceIndex) && (!enableWpp)))
    {
        secSet->lftBBlock = &nonAvailBlock;
        secSet->lftTBlock = &nonAvailBlock;
    }
    else
    {
        secSet->lftBBlock = lftBBlock;
        secSet->lftTBlock = curBlock - 1;
    }

    if ((puTopPelY < 0) || (puRgtPelX >= frameWidth) || ((sliceIdx != topRgtCtu->sliceIndex) && (!enableWpp)))
    {
        secSet->topRgtBlock = &nonAvailBlock;
    }
    else
    {
        secSet->topRgtBlock = topRgtBlock;
    }

    if ((puTopPelY < 0) || ((sliceIdx != topRCtu->sliceIndex) && (!enableWpp)))
    {
        secSet->topRBlock = &nonAvailBlock;
        secSet->topLBlock = &nonAvailBlock;
    }
    else
    {
        secSet->topRBlock = topRBlock;
        secSet->topLBlock = curBlock - blockSetWidth;
    }

    if ((puTopPelY < 0) || (puLftPelX < 0) || ((sliceIdx != topLftCtu->sliceIndex) && (!enableWpp)))
    {
        secSet->topLftBlock = &nonAvailBlock;
    }
    else
    {
        secSet->topLftBlock = topLftBlock;
    }

    if (seqSet->config.enableWpp)
    {
        if (puRgtPelX >= (SINT32)(curCtu->ctuPelX + curCtu->width))
        {
            secSet->topRgtBlock = &nonAvailBlock;
        }

        if (puBotPelY >= (SINT32)(curCtu->ctuPelY + curCtu->height))
        {
            secSet->lftBotBlock = &nonAvailBlock;
        }
    }

    secSet->curBlock = curBlock;

}

