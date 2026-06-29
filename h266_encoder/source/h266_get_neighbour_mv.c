/***************************************************************************//**
 *
 * @file          h266_get_neighbour_mv.c
 * @brief         Get merge or skip motion vector candidates.
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
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_constant.h"
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
#include "basic_macro.h"
#include "h266_md_buf_manipulate.h"
#include "h266_intra_prediction.h"
#include "video_macro.h"
#include "h26x_common_data.h"

#define XIN_MV_MANTISSA_BIT_NUM     6
#define XIN_MV_EXPONENT_BIT_NUM     4
#define XIN_MV_MANTISSA_UPPER_LIMIT ((1 << (XIN_MV_MANTISSA_BIT_NUM - 1)) - 1)
#define XIN_MV_MANTISSA_LIMIT       (1 << (XIN_MV_MANTISSA_BIT_NUM - 1))
#define XIN_MV_EXPONENT_MASK        ((1 << XIN_MV_EXPONENT_BIT_NUM) - 1)

static SINT32 affineModel[6][4] =
{
    { 0, 1, 2 },          // 0:  LT, RT, LB
    { 0, 1, 3 },          // 1:  LT, RT, RB
    { 0, 2, 3 },          // 2:  LT, LB, RB
    { 1, 2, 3 },          // 3:  RT, LB, RB
    { 0, 1 },             // 4:  LT, RT
    { 0, 2 },             // 5:  LT, LB
};

static SINT32 affineVerNum[6] =
{
    3, 3, 3, 3, 2, 2
};

extern const SINT32 amvrPrecision[4];

void Xin266AddMvpCandHmvpLut (
    xin_sec_struct   *secSet,
    xin_neighbour_mv *neighbourMv)
{
    SINT32           candIdx;
    SINT32           sameIdx;
    xin_neighbour_mv *hmvpLut;

    sameIdx = secSet->hmvpNum;
    hmvpLut = secSet->hmvpLut;

    for (candIdx = 0; candIdx < secSet->hmvpNum; candIdx++)
    {
        if (!IS_MV_DIFF_B(hmvpLut + candIdx, neighbourMv))
        {
            sameIdx = candIdx;

            break;
        }
    }

    sameIdx = (secSet->hmvpNum == XIN_MAX_HMVP_CAND_NUM) && (sameIdx == secSet->hmvpNum) ? 0 : sameIdx;

    if (sameIdx < secSet->hmvpNum)
    {
        for (candIdx = sameIdx; candIdx < secSet->hmvpNum - 1; candIdx++)
        {
            hmvpLut[candIdx] = hmvpLut[candIdx + 1];
        }

        secSet->hmvpNum--;
    }

    hmvpLut[secSet->hmvpNum] = *neighbourMv;
    secSet->hmvpNum++;

}

static void Xin266GetHMvpForMergeP (
    xin_sec_struct   *secSet,
    xin_block_struct *topBlock,
    xin_block_struct *lftBlock,
    xin_neighbour_mv *mergeCand,
    UINT32           *mvNum)
{
    xin_seq_struct   *seqSet;
    SINT32           hmvpNum;
    SINT32           candIdx;
    SINT32           maxCand1;
    SINT32           mrgIdx;
    xin_neighbour_mv *hmvpLut;
    SINT32           mvIdx;

    seqSet   = secSet->seqSet;
    hmvpNum  = secSet->hmvpNum;
    hmvpLut  = secSet->hmvpLut;
    mvIdx    = *mvNum;
    maxCand1 = seqSet->config.maxMergeCand - 1;

    for (mrgIdx = 1; mrgIdx <= hmvpNum; mrgIdx++)
    {
        candIdx = hmvpNum - mrgIdx;

        if ((mrgIdx > 2) ||
                (((lftBlock->type == XIN_INVALID_MODE) ||
                  IS_MV_DIFF_P (lftBlock, hmvpLut + candIdx)) &&
                 ((topBlock->type == XIN_INVALID_MODE) ||
                  IS_MV_DIFF_P (topBlock, hmvpLut + candIdx))))
        {
            mergeCand[mvIdx] = hmvpLut[candIdx];
            mvIdx++;
        }

        if (maxCand1 == mvIdx)
        {
            break;
        }
    }

    *mvNum = mvIdx;

}

static void Xin266GetHMvpForMergeB (
    xin_sec_struct   *secSet,
    xin_block_struct *topBlock,
    xin_block_struct *lftBlock,
    xin_neighbour_mv *mergeCand,
    UINT32           *mvNum)
{
    xin_seq_struct   *seqSet;
    SINT32           hmvpNum;
    SINT32           candIdx;
    SINT32           maxCand1;
    SINT32           mrgIdx;
    xin_neighbour_mv *hmvpLut;
    SINT32           mvIdx;

    seqSet   = secSet->seqSet;
    hmvpNum  = secSet->hmvpNum;
    hmvpLut  = secSet->hmvpLut;
    mvIdx    = *mvNum;
    maxCand1 = seqSet->config.maxMergeCand - 1;

    for (mrgIdx = 1; mrgIdx <= hmvpNum; mrgIdx++)
    {
        candIdx = hmvpNum - mrgIdx;

        if ((mrgIdx > 2) ||
                (((lftBlock->type == XIN_INVALID_MODE) ||
                  IS_MV_DIFF_B (lftBlock, hmvpLut + candIdx)) &&
                 ((topBlock->type == XIN_INVALID_MODE) ||
                  IS_MV_DIFF_B (topBlock, hmvpLut + candIdx))))
        {
            mergeCand[mvIdx] = hmvpLut[candIdx];
            mvIdx++;
        }

        if (maxCand1 == mvIdx)
        {
            break;
        }
    }

    *mvNum = mvIdx;

}

static void RoundMv (
    xin_mv32_u *inputMv0,
    xin_mv32_u *inputMv1,
    xin_mv32_u *outputMv)
{
    SINT32 sumMvX;
    SINT32 sumMvY;

    sumMvX = inputMv0->s32x2[0] + inputMv1->s32x2[0];
    sumMvY = inputMv0->s32x2[1] + inputMv1->s32x2[1];

    sumMvX = (sumMvX + 1 - (sumMvX >= 0)) >> 1;
    sumMvY = (sumMvY + 1 - (sumMvY >= 0)) >> 1;

    outputMv->s32x2[0] = sumMvX;
    outputMv->s32x2[1] = sumMvY;
}

void Xin266ChangeMv32Prec (
    xin_mv32_u *inputMv,
    SINT32     inputPrec,
    SINT32     outputPrec)
{
    SINT32 shift;
    SINT32 offset;
    SINT32 mvX;
    SINT32 mvY;

    shift = outputPrec - inputPrec;
    mvX   = inputMv->s32x2[0];
    mvY   = inputMv->s32x2[1];

    if (shift >= 0)
    {
        mvX <<= shift;
        mvY <<= shift;
    }
    else
    {
        shift  = -shift;
        offset = 1 << (shift - 1);

        mvX = mvX >= 0 ? (mvX + offset - 1) >> shift : (mvX + offset) >> shift;
        mvY = mvY >= 0 ? (mvY + offset - 1) >> shift : (mvY + offset) >> shift;
    }

    inputMv->s32x2[0] = mvX;
    inputMv->s32x2[1] = mvY;

}

void Xin266ChangeMvPrec (
    xin_mv_u *inputMv,
    SINT32   inputPrec,
    SINT32   outputPrec)
{
    SINT32 shift;
    SINT32 offset;
    SINT32 mvX;
    SINT32 mvY;

    shift = outputPrec - inputPrec;
    mvX   = inputMv->s16x2[0];
    mvY   = inputMv->s16x2[1];

    if (shift >= 0)
    {
        mvX <<= shift;
        mvY <<= shift;
    }
    else
    {
        shift  = -shift;
        offset = 1 << (shift - 1);

        mvX = mvX >= 0 ? (mvX + offset - 1) >> shift : (mvX + offset) >> shift;
        mvY = mvY >= 0 ? (mvY + offset - 1) >> shift : (mvY + offset) >> shift;
    }

    inputMv->s16x2[0] = (SINT16)mvX;
    inputMv->s16x2[1] = (SINT16)mvY;

}

static void Xin266RoundPrecInternal2Amvr (
    xin_mv32_u *inputMv,
    SINT32     mvPrecIdx)
{
    Xin266ChangeMv32Prec (
        inputMv,
        XIN_MV_PREC_INTERNAL,
        mvPrecIdx);

    Xin266ChangeMv32Prec (
        inputMv,
        mvPrecIdx,
        XIN_MV_PREC_INTERNAL);
}

void Xin266ChangeMv2Me (
    xin_mv32_u *inputMv,
    xin_mv_u   *ouputMv)
{
    SINT32 shift;
    SINT32 offset;
    SINT32 mvX;
    SINT32 mvY;

    shift = XIN_ME_PREC_INTERNAL - XIN_MV_PREC_INTERNAL;
    mvX   = inputMv->s32x2[0];
    mvY   = inputMv->s32x2[1];

    if (shift >= 0)
    {
        mvX <<= shift;
        mvY <<= shift;
    }
    else
    {
        shift  = -shift;
        offset = 1 << (shift - 1);

        mvX = mvX >= 0 ? (mvX + offset - 1) >> shift : (mvX + offset) >> shift;
        mvY = mvY >= 0 ? (mvY + offset - 1) >> shift : (mvY + offset) >> shift;
    }

    ouputMv->s16x2[0] = (SINT16)mvX;
    ouputMv->s16x2[1] = (SINT16)mvY;

}

static void RoundMvComp (
    xin_mv32_u *inputMv)
{
    SINT32  compIdx;
    SINT32  mvComp;
    SINT32  sign;
    SINT32  firstSet;
    SINT32  scale;
    SINT32  exponent;
    SINT32  mantissa;
    SINT32  round;
    SINT32  mvFloat;

    for (compIdx = 0; compIdx < 2; compIdx++)
    {
        mvComp = inputMv->s32x2[compIdx];

        if (mvComp)
        {
            sign = mvComp >> 31;

            BIT_SCAN_REVERSE_32 ((mvComp^sign) | XIN_MV_MANTISSA_UPPER_LIMIT, firstSet);

            scale = firstSet - (XIN_MV_MANTISSA_BIT_NUM - 1);

            if (scale >= 0)
            {
                round    = (1 << scale) >> 1;
                exponent = scale + ((((mvComp + round) >> scale) ^ sign) >> (XIN_MV_MANTISSA_BIT_NUM - 1));
                mantissa = (((mvComp + round) >> scale) & XIN_MV_MANTISSA_UPPER_LIMIT) | (sign << (XIN_MV_MANTISSA_BIT_NUM - 1));
            }
            else
            {
                exponent = 0;
                mantissa = mvComp;
            }

            mvFloat  = exponent | (mantissa << XIN_MV_EXPONENT_BIT_NUM);
            exponent = mvFloat & XIN_MV_EXPONENT_MASK;
            mantissa = mvFloat >> XIN_MV_EXPONENT_BIT_NUM;

            inputMv->s32x2[compIdx] = (exponent == 0) ? (SINT32)mantissa : (SINT32)((mantissa ^ XIN_MV_MANTISSA_LIMIT) << (exponent - 1));

        }

    }

}

static void ScaleMv (
    xin_mv32_u *mv,
    SINT32     curPOC,
    SINT32     curRefPOC,
    SINT32     colPOC,
    SINT32     colRefPOC)
{
    SINT32 diffPocD;
    SINT32 diffPocB;
    SINT32 scale;
    SINT32 mult;

    diffPocD = colPOC - colRefPOC;
    diffPocB = curPOC - curRefPOC;

    if (diffPocD != diffPocB)
    {
        diffPocB = XIN_CLIP(diffPocB, -128, 127);
        diffPocD = XIN_CLIP(diffPocD, -128, 127);
        mult     = (0x4000 + XIN_ABS(diffPocD >> 1))/diffPocD;
        scale    = XIN_CLIP((diffPocB*mult + 32) >> 6, -4096, 4095);

        mv->mv.mv32X = XIN_CLIP ((scale*mv->mv.mv32X + 127 + (scale*mv->mv.mv32X < 0)) >> 8, XIN_MIN_MV, XIN_MAX_MV);
        mv->mv.mv32Y = XIN_CLIP ((scale*mv->mv.mv32Y + 127 + (scale*mv->mv.mv32Y < 0)) >> 8, XIN_MIN_MV, XIN_MAX_MV);
    }

}

static void GetDirectMvp (
    xin_ref_picture  *pictureWrite,
    xin_mv32_u       *mv,
    xin_block_struct *nbBlock,
    SINT32           listIdx,
    SINT32           refIdx,
    BOOL             *mvpAvail)
{
    SINT32 curRefPoc;
    SINT32 nbRefIdx;
    SINT32 nbRefPoc;

    curRefPoc = pictureWrite->refFramePoc[listIdx][refIdx];
    nbRefIdx  = nbBlock->refIdx[listIdx];
    nbRefPoc  = (nbRefIdx != -1) ? pictureWrite->refFramePoc[listIdx][nbRefIdx] : nbRefIdx;

    if (curRefPoc == nbRefPoc)
    {
        mv->s64x1 = nbBlock->mv[listIdx].s64x1;
        *mvpAvail = TRUE;
    }
    else
    {
        nbRefIdx  = nbBlock->refIdx[!listIdx];
        nbRefPoc  = (nbRefIdx != -1) ? pictureWrite->refFramePoc[!listIdx][nbRefIdx] : -1;

        if (curRefPoc == nbRefPoc)
        {
            mv->s64x1 = nbBlock->mv[!listIdx].s64x1;
            *mvpAvail = TRUE;
        }
    }

}

static void Xin266AddDirectMvpCand (
    xin_ref_picture  *pictureWrite,
    xin_block_struct *block[3],
    UINT32           blockNum,
    xin_mv32_u       *mvp,
    UINT32           availField,
    SINT32           listIdx,
    SINT32           refIdx,
    BOOL             *outMvpAvail)
{
    BOOL    mvpAvail;
    UINT32  blockIdx;
    UINT32  availMask;

    mvpAvail = FALSE;

    for (blockIdx = 0; blockIdx < blockNum; blockIdx++)
    {
        availMask = 1 << blockIdx;

        if (availField & availMask)
        {
            GetDirectMvp (
                pictureWrite,
                mvp,
                block[blockIdx],
                listIdx,
                refIdx,
                &mvpAvail);

            if (mvpAvail)
            {
                break;
            }
        }
    }

    *outMvpAvail = mvpAvail;

}


void Xin266GetTMvpForMerge (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *neighbourMv)
{
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    UINT32           frameWidth;
    UINT32           frameHeight;
    UINT32           blockSetWidth;
    UINT32           blockIdx;
    UINT32           offsetX;
    UINT32           offsetY;
    UINT32           middleX;
    UINT32           middleY;
    UINT32           mostRgtX;
    UINT32           mostBotY;
    UINT32           tmvpPos;
    xin_ref_picture  *pictureWrite;
    xin_ref_picture  *pictureRead;
    UINT32           colRefList;

    seqSet        = secSet->seqSet;
    picSet        = secSet->picSet;
    frameWidth    = seqSet->frameWidth;
    frameHeight   = seqSet->frameHeight;
    offsetX       = pu->width;
    offsetY       = pu->height;
    mostRgtX      = secSet->cu->cuPelX + offsetX;
    mostBotY      = secSet->cu->cuPelY + offsetY;
    middleX       = mostRgtX - pu->width/2;
    middleY       = mostBotY - pu->height/2;
    pictureWrite  = picSet->pictureWrite;
    pictureRead   = picSet->pictureRead[(pictureWrite->frameType == XIN_B_FRAME) ? 1-pictureWrite->colFromL0Flag : 0][0];
    blockIdx      = 0;
    blockSetWidth = pictureRead->blockSetWidth;

    if ((mostRgtX >= frameWidth) || (mostBotY >= frameHeight) || ((offsetY + secSet->cu->offY) >= seqSet->ctuSize))
    {
        tmvpPos = XIN_TMVP_COL_CENTRE;
    }
    else
    {
        mostRgtX = (mostRgtX >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;
        mostBotY = (mostBotY >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;

        PEL_XY_TO_BLOCK_INDEX(mostRgtX, mostBotY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

        tmvpPos = (pictureRead->blockSetMap[blockIdx].type <= XIN_INTER_MODE) ? XIN_TMVP_COL_BOT_RGT : XIN_TMVP_COL_CENTRE;
    }

    neighbourMv[0].useAltHpelIf = FALSE;

    if (tmvpPos == XIN_TMVP_COL_BOT_RGT)
    {
        colRefList = pictureWrite->checkLDC ? XIN_LIST_0 : pictureWrite->colFromL0Flag;

        if (pictureRead->blockSetMap[blockIdx].refIdx[colRefList] >= 0)
        {
            neighbourMv[0].mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
            neighbourMv[0].refIdx[XIN_LIST_0] = 0;
        }
        else
        {
            colRefList                        = !colRefList;
            neighbourMv[0].mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
            neighbourMv[0].refIdx[XIN_LIST_0] = 0;
        }

        RoundMvComp (
            &neighbourMv[0].mv[XIN_LIST_0]);

        ScaleMv (
            &(neighbourMv->mv[XIN_LIST_0]),
            pictureWrite->framePoc,
            pictureWrite->refFramePoc[XIN_LIST_0][0],
            pictureRead->framePoc,
            pictureRead->refFramePoc[colRefList][pictureRead->blockSetMap[blockIdx].refIdx[colRefList]]);

    }
    else
    {
        middleX = (middleX >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;
        middleY = (middleY >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;

        PEL_XY_TO_BLOCK_INDEX(middleX, middleY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

        if (pictureRead->blockSetMap[blockIdx].type <= XIN_INTER_MODE)
        {
            colRefList = pictureWrite->checkLDC ? XIN_LIST_0 : pictureWrite->colFromL0Flag;

            if (pictureRead->blockSetMap[blockIdx].refIdx[colRefList] >= 0)
            {
                neighbourMv[0].mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                neighbourMv[0].refIdx[XIN_LIST_0] = 0;
            }
            else
            {
                colRefList                        = !colRefList;
                neighbourMv[0].mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                neighbourMv[0].refIdx[XIN_LIST_0] = 0;
            }

            RoundMvComp (
                &neighbourMv[0].mv[XIN_LIST_0]);

            ScaleMv (
                &(neighbourMv->mv[XIN_LIST_0]),
                pictureWrite->framePoc,
                pictureWrite->refFramePoc[XIN_LIST_0][0],
                pictureRead->framePoc,
                pictureRead->refFramePoc[colRefList][pictureRead->blockSetMap[blockIdx].refIdx[colRefList]]);

        }
        else
        {
            neighbourMv->refIdx[XIN_LIST_0] = -1;
        }

    }

    if (pictureWrite->frameType == XIN_B_FRAME)
    {
        if (tmvpPos == XIN_TMVP_COL_BOT_RGT)
        {
            colRefList = pictureWrite->checkLDC ? XIN_LIST_1 : pictureWrite->colFromL0Flag;

            if (pictureRead->blockSetMap[blockIdx].refIdx[colRefList] >= 0)
            {
                neighbourMv[0].mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                neighbourMv[0].refIdx[XIN_LIST_1] = 0;
            }
            else
            {
                colRefList                        = !colRefList;
                neighbourMv[0].mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                neighbourMv[0].refIdx[XIN_LIST_1] = 0;
            }

            RoundMvComp (
                &neighbourMv[0].mv[XIN_LIST_1]);

            ScaleMv (
                &(neighbourMv->mv[XIN_LIST_1]),
                pictureWrite->framePoc,
                pictureWrite->refFramePoc[XIN_LIST_1][0],
                pictureRead->framePoc,
                pictureRead->refFramePoc[colRefList][pictureRead->blockSetMap[blockIdx].refIdx[colRefList]]);

        }
        else
        {
            middleX = (middleX >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;
            middleY = (middleY >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;

            PEL_XY_TO_BLOCK_INDEX(middleX, middleY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

            if (pictureRead->blockSetMap[blockIdx].type <= XIN_INTER_MODE)
            {
                colRefList = pictureWrite->checkLDC ? XIN_LIST_1 : pictureWrite->colFromL0Flag;

                if (pictureRead->blockSetMap[blockIdx].refIdx[colRefList] >= 0)
                {
                    neighbourMv[0].mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                    neighbourMv[0].refIdx[XIN_LIST_1] = 0;
                }
                else
                {
                    colRefList                        = !colRefList;
                    neighbourMv[0].mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                    neighbourMv[0].refIdx[XIN_LIST_1] = 0;
                }

                RoundMvComp(
                    &neighbourMv[0].mv[XIN_LIST_1]);

                ScaleMv (
                    &(neighbourMv->mv[XIN_LIST_1]),
                    pictureWrite->framePoc,
                    pictureWrite->refFramePoc[XIN_LIST_1][0],
                    pictureRead->framePoc,
                    pictureRead->refFramePoc[colRefList][pictureRead->blockSetMap[blockIdx].refIdx[colRefList]]);

            }
            else
            {
                neighbourMv->refIdx[XIN_LIST_1] = -1;
            }

        }

    }

    neighbourMv->bcwIdx = XIN_BCW_DEFAULT;

}

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

void Xin266FillMergeCandP (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *mergeCand,
    UINT32           *mvNum)
{
    xin_seq_struct   *seqSet;
    UINT32           numMvs;
    xin_block_struct *lftBBlock;
    xin_block_struct *topRBlock;
    xin_block_struct *topRgtBlock;
    xin_block_struct *lftBotBlock;
    xin_block_struct *topLftBlock;
    SINT32           refIdxI;
    SINT32           refIdxJ;

    seqSet       = secSet->seqSet;
    numMvs       = 0;
    lftBBlock    = secSet->lftBBlock;
    topRBlock    = secSet->topRBlock;
    topRgtBlock  = secSet->topRgtBlock;
    lftBotBlock  = secSet->lftBotBlock;
    topLftBlock  = secSet->topLftBlock;

    // Top
    if (topRBlock->type <= XIN_INTER_MODE)
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = topRBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = topRBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].useAltHpelIf       = topRBlock->imvIdx == XIN_IMV_HPEL;
        numMvs++;
    }

    // Left
    if ((lftBBlock->type <= XIN_INTER_MODE) && ((topRBlock->type > XIN_INTER_MODE) ||
            IS_MV_DIFF_P(topRBlock, lftBBlock)))
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = lftBBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = lftBBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].useAltHpelIf       = lftBBlock->imvIdx == XIN_IMV_HPEL;
        numMvs++;
    }

    // Top Right
    if ((topRgtBlock->type <= XIN_INTER_MODE) && ((topRBlock->type > XIN_INTER_MODE) ||
            IS_MV_DIFF_P(topRgtBlock, topRBlock)))
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = topRgtBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = topRgtBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].useAltHpelIf       = topRgtBlock->imvIdx == XIN_IMV_HPEL;
        numMvs++;
    }

    // Left Bottom
    if ((lftBotBlock->type <= XIN_INTER_MODE) && ((lftBBlock->type > XIN_INTER_MODE) ||
            IS_MV_DIFF_P(lftBotBlock, lftBBlock)))
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = lftBotBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = lftBotBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].useAltHpelIf       = lftBotBlock->imvIdx == XIN_IMV_HPEL;
        numMvs++;
    }

    // Top Left
    if (numMvs < 4)
    {
        if ((topLftBlock->type <= XIN_INTER_MODE) &&
                ((lftBBlock->type > XIN_INTER_MODE) ||
                 IS_MV_DIFF_P(topLftBlock, lftBBlock)) &&
                ((topRBlock->type > XIN_INTER_MODE) ||
                 IS_MV_DIFF_P(topLftBlock, topRBlock)))
        {
            mergeCand[numMvs].mv[XIN_LIST_0]     = topLftBlock->mv[XIN_LIST_0];
            mergeCand[numMvs].refIdx[XIN_LIST_0] = topLftBlock->refIdx[XIN_LIST_0];
            mergeCand[numMvs].useAltHpelIf       = topLftBlock->imvIdx == XIN_IMV_HPEL;
            numMvs++;
        }
    }

    if (numMvs >= seqSet->config.maxMergeCand)
    {
        *mvNum = numMvs;

        return;
    }

    if ((seqSet->config.enableTMvp) && ((pu->width + pu->height) > 12))
    {
        Xin266GetTMvpForMerge (
            secSet,
            pu,
            mergeCand+numMvs);

        if ((mergeCand[numMvs].refIdx[0] >= 0) || (mergeCand[numMvs].refIdx[1] >= 0))
        {
            numMvs++;
        }
    }

    if (numMvs >= seqSet->config.maxMergeCand)
    {
        *mvNum = numMvs;

        return;
    }

    if (numMvs != seqSet->config.maxMergeCand - 1)
    {
        Xin266GetHMvpForMergeP (
            secSet,
            topRBlock,
            lftBBlock,
            mergeCand,
            &numMvs);
    }

    if ((numMvs > 1) && (numMvs < seqSet->config.maxMergeCand))
    {
        refIdxI = mergeCand[0].refIdx[0];
        refIdxJ = mergeCand[1].refIdx[0];

        if ((refIdxI != -1) && (refIdxJ != -1))
        {
            RoundMv (
                &mergeCand[0].mv[XIN_LIST_0],
                &mergeCand[1].mv[XIN_LIST_0],
                &mergeCand[numMvs].mv[XIN_LIST_0]);

            mergeCand[numMvs].refIdx[XIN_LIST_0] = (SINT8)refIdxI;
            mergeCand[numMvs].useAltHpelIf       = (mergeCand[0].useAltHpelIf == mergeCand[1].useAltHpelIf) ? mergeCand[0].useAltHpelIf : FALSE;

            numMvs++;
        }
    }

    if (numMvs < seqSet->config.maxMergeCand)
    {
        mergeCand[numMvs].mv[XIN_LIST_0].s64x1 = 0;
        mergeCand[numMvs].refIdx[XIN_LIST_0]   = 0;
        mergeCand[numMvs].useAltHpelIf         = FALSE;
    }

    *mvNum = numMvs;

}

void Xin266FillMergeCandB (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *mergeCand,
    UINT32           *mvNum)
{
    xin_seq_struct   *seqSet;
    UINT32           numMvs;
    xin_block_struct *lftBBlock;
    xin_block_struct *topRBlock;
    xin_block_struct *topRgtBlock;
    xin_block_struct *lftBotBlock;
    xin_block_struct *topLftBlock;
    SINT32           refIdxI;
    SINT32           refIdxJ;
    UINT32           listIdx;

    seqSet       = secSet->seqSet;
    numMvs       = 0;
    lftBBlock    = secSet->lftBBlock;
    topRBlock    = secSet->topRBlock;
    topRgtBlock  = secSet->topRgtBlock;
    lftBotBlock  = secSet->lftBotBlock;
    topLftBlock  = secSet->topLftBlock;

    // Top
    if (topRBlock->type <= XIN_INTER_MODE)
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = topRBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = topRBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].mv[XIN_LIST_1]     = topRBlock->mv[XIN_LIST_1];
        mergeCand[numMvs].refIdx[XIN_LIST_1] = topRBlock->refIdx[XIN_LIST_1];
        mergeCand[numMvs].useAltHpelIf       = topRBlock->imvIdx == XIN_IMV_HPEL;
        mergeCand[numMvs].bcwIdx             = (topRBlock->refIdx[XIN_LIST_0] >= 0 && topRBlock->refIdx[XIN_LIST_1] >= 0) ? topRBlock->bcwIdx : XIN_BCW_DEFAULT;
        numMvs++;
    }

    // Left
    if ((lftBBlock->type <= XIN_INTER_MODE) && ((topRBlock->type > XIN_INTER_MODE) ||
            IS_MV_DIFF_B(topRBlock, lftBBlock)))
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = lftBBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = lftBBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].mv[XIN_LIST_1]     = lftBBlock->mv[XIN_LIST_1];
        mergeCand[numMvs].refIdx[XIN_LIST_1] = lftBBlock->refIdx[XIN_LIST_1];
        mergeCand[numMvs].useAltHpelIf       = lftBBlock->imvIdx == XIN_IMV_HPEL;
        mergeCand[numMvs].bcwIdx             = (lftBBlock->refIdx[XIN_LIST_0] >= 0 && lftBBlock->refIdx[XIN_LIST_1] >= 0) ? lftBBlock->bcwIdx : XIN_BCW_DEFAULT;
        numMvs++;
    }

    // Top Right
    if ((topRgtBlock->type <= XIN_INTER_MODE) && ((topRBlock->type > XIN_INTER_MODE) ||
            IS_MV_DIFF_B(topRgtBlock, topRBlock)))
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = topRgtBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = topRgtBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].mv[XIN_LIST_1]     = topRgtBlock->mv[XIN_LIST_1];
        mergeCand[numMvs].refIdx[XIN_LIST_1] = topRgtBlock->refIdx[XIN_LIST_1];
        mergeCand[numMvs].useAltHpelIf       = topRgtBlock->imvIdx == XIN_IMV_HPEL;
        mergeCand[numMvs].bcwIdx             = (topRgtBlock->refIdx[XIN_LIST_0] >= 0 && topRgtBlock->refIdx[XIN_LIST_1] >= 0) ? topRgtBlock->bcwIdx : XIN_BCW_DEFAULT;
        numMvs++;
    }

    // Left Bottom
    if ((lftBotBlock->type <= XIN_INTER_MODE) && ((lftBBlock->type > XIN_INTER_MODE) ||
            IS_MV_DIFF_B(lftBotBlock, lftBBlock)))
    {
        mergeCand[numMvs].mv[XIN_LIST_0]     = lftBotBlock->mv[XIN_LIST_0];
        mergeCand[numMvs].refIdx[XIN_LIST_0] = lftBotBlock->refIdx[XIN_LIST_0];
        mergeCand[numMvs].mv[XIN_LIST_1]     = lftBotBlock->mv[XIN_LIST_1];
        mergeCand[numMvs].refIdx[XIN_LIST_1] = lftBotBlock->refIdx[XIN_LIST_1];
        mergeCand[numMvs].useAltHpelIf       = lftBotBlock->imvIdx == XIN_IMV_HPEL;
        mergeCand[numMvs].bcwIdx             = (lftBotBlock->refIdx[XIN_LIST_0] >= 0 && lftBotBlock->refIdx[XIN_LIST_1] >= 0) ? lftBotBlock->bcwIdx : XIN_BCW_DEFAULT;
        numMvs++;
    }

    // Top Left
    if (numMvs < 4)
    {
        if ((topLftBlock->type <= XIN_INTER_MODE) &&
                ((lftBBlock->type > XIN_INTER_MODE) ||
                 IS_MV_DIFF_B(topLftBlock, lftBBlock)) &&
                ((topRBlock->type > XIN_INTER_MODE) ||
                 IS_MV_DIFF_B(topLftBlock, topRBlock)))
        {
            mergeCand[numMvs].mv[XIN_LIST_0]     = topLftBlock->mv[XIN_LIST_0];
            mergeCand[numMvs].refIdx[XIN_LIST_0] = topLftBlock->refIdx[XIN_LIST_0];
            mergeCand[numMvs].mv[XIN_LIST_1]     = topLftBlock->mv[XIN_LIST_1];
            mergeCand[numMvs].refIdx[XIN_LIST_1] = topLftBlock->refIdx[XIN_LIST_1];
            mergeCand[numMvs].useAltHpelIf       = topLftBlock->imvIdx == XIN_IMV_HPEL;
            mergeCand[numMvs].bcwIdx             = (topLftBlock->refIdx[XIN_LIST_0] >= 0 && topLftBlock->refIdx[XIN_LIST_1] >= 0) ? topLftBlock->bcwIdx : XIN_BCW_DEFAULT;
            numMvs++;
        }
    }

    if (numMvs >= seqSet->config.maxMergeCand)
    {
        *mvNum = numMvs;

        return;
    }

    if ((seqSet->config.enableTMvp) && ((pu->width + pu->height) > 12))
    {
        Xin266GetTMvpForMerge (
            secSet,
            pu,
            mergeCand+numMvs);

        if ((mergeCand[numMvs].refIdx[0] >= 0) || (mergeCand[numMvs].refIdx[1] >= 0))
        {
            numMvs++;

            if (numMvs >= seqSet->config.maxMergeCand)
            {
                *mvNum = numMvs;

                return;
            }
        }
    }

    if (numMvs != seqSet->config.maxMergeCand - 1)
    {
        Xin266GetHMvpForMergeB (
            secSet,
            topRBlock,
            lftBBlock,
            mergeCand,
            &numMvs);
    }

    if ((numMvs > 1) && (numMvs < seqSet->config.maxMergeCand))
    {
        mergeCand[numMvs].refIdx[XIN_LIST_0] = -1;
        mergeCand[numMvs].refIdx[XIN_LIST_1] = -1;
        mergeCand[numMvs].bcwIdx             = XIN_BCW_DEFAULT;

        for (listIdx = 0; listIdx < XIN_LIST_NUM; listIdx++)
        {
            refIdxI = mergeCand[0].refIdx[listIdx];
            refIdxJ = mergeCand[1].refIdx[listIdx];

            if ((refIdxI != -1) && (refIdxJ != -1))
            {
                RoundMv (
                    &mergeCand[0].mv[listIdx],
                    &mergeCand[1].mv[listIdx],
                    &mergeCand[numMvs].mv[listIdx]);

                mergeCand[numMvs].refIdx[listIdx] = (SINT8)refIdxI;
            }
            else if (refIdxI != -1)
            {
                mergeCand[numMvs].mv[listIdx].s64x1 = mergeCand[0].mv[listIdx].s64x1;
                mergeCand[numMvs].refIdx[listIdx]   = (SINT8)refIdxI;
            }
            else if (refIdxJ != -1)
            {
                mergeCand[numMvs].mv[listIdx].s64x1 = mergeCand[1].mv[listIdx].s64x1;
                mergeCand[numMvs].refIdx[listIdx]   = (SINT8)refIdxJ;
            }

        }

        if ((mergeCand[numMvs].refIdx[XIN_LIST_0] != -1) || (mergeCand[numMvs].refIdx[XIN_LIST_1] != -1))
        {
            mergeCand[numMvs].useAltHpelIf = (mergeCand[0].useAltHpelIf == mergeCand[1].useAltHpelIf) ? mergeCand[0].useAltHpelIf : FALSE;
            numMvs++;
        }

    }

    if (numMvs >= seqSet->config.maxMergeCand)
    {
        *mvNum = numMvs;
    }
    else
    {
        mergeCand[numMvs].mv[XIN_LIST_0].s64x1 = 0;
        mergeCand[numMvs].refIdx[XIN_LIST_0]   = 0;
        mergeCand[numMvs].mv[XIN_LIST_1].s64x1 = 0;
        mergeCand[numMvs].refIdx[XIN_LIST_1]   = 0;
        mergeCand[numMvs].useAltHpelIf         = FALSE;
        mergeCand[numMvs].bcwIdx               = XIN_BCW_DEFAULT;
        numMvs++;

        *mvNum = numMvs;
    }

}

void Xin266ClipColPos (
    SINT32         *posX,
    SINT32         *posY,
    SINT32         ctuPelX,
    SINT32         ctuPelY,
    xin_seq_struct *seqSet)
{
    SINT32    horMax;
    SINT32    horMin;
    SINT32    verMax;
    SINT32    verMin;
    SINT32    ctuSize;

    ctuSize = seqSet->ctuSize;
    horMax  = XIN_MIN ((SINT32)seqSet->frameWidth - 1, ctuPelX + ctuSize + 3);
    horMin  = XIN_MAX (0, ctuPelX);
    verMax  = XIN_MIN ((SINT32)seqSet->frameHeight - 1, ctuPelY + ctuSize - 1);
    verMin  = XIN_MAX (0, ctuPelY);

    *posX = XIN_MIN (horMax, XIN_MAX (horMin, *posX));
    *posY = XIN_MIN (verMax, XIN_MAX (verMin, *posY));

}

void Xin266FillSbtMvpMergeCand (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_neighbour_mv *affineMv,
    BOOL             *sbTmvpAvail)
{
    xin_seq_struct   *seqSet;
    xin_block_struct *lftBBlock;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;
    xin_ref_picture  *pictureRead;
    xin_mode_struct  *modeCtrl;
    xin_mv32_u       ctMv;
    xin_neighbour_mv lftMv;
    UINT32           colRefList;
    SINT32           colRefPoc;
    SINT32           middleX;
    SINT32           middleY;
    SINT32           subBlockX;
    SINT32           subBlockY;
    UINT32           blockSetWidth;
    UINT32           blockIdx;
    UINT32           subBlockIdx;
    xin_neighbour_mv defaultMv;
    SINT32           colIdx, rowIdx;
    UINT32           candNum;
    xin_ctu_struct   *ctu;
    xin_neighbour_mv neighbourMv;
    UINT32           mask;

    seqSet        = secSet->seqSet;
    picSet        = secSet->picSet;
    ctu           = secSet->ctu;
    modeCtrl      = secSet->cu->modeCtrl;
    pictureWrite  = picSet->pictureWrite;
    colRefList    = (pictureWrite->frameType == XIN_B_FRAME) ? 1 - pictureWrite->colFromL0Flag : 0;
    pictureRead   = picSet->pictureRead[colRefList][0];
    colRefPoc     = pictureRead->framePoc;
    ctMv.s64x1    = 0;
    candNum       = 0;
    blockSetWidth = pictureRead->blockSetWidth;
    subBlockIdx   = 0;
    mask          = 0xFFFFFFF8;

    if (seqSet->config.enableTMvp && seqSet->config.enableSbtMvp)
    {
        lftBBlock = secSet->lftBBlock;

        if (lftBBlock->type <= XIN_INTER_MODE)
        {
            lftMv.mv[XIN_LIST_0]     = lftBBlock->mv[XIN_LIST_0];
            lftMv.refIdx[XIN_LIST_0] = lftBBlock->refIdx[XIN_LIST_0];
            lftMv.mv[XIN_LIST_1]     = lftBBlock->mv[XIN_LIST_1];
            lftMv.refIdx[XIN_LIST_1] = lftBBlock->refIdx[XIN_LIST_1];

            if ((lftMv.refIdx[XIN_LIST_0] >= 0) && (colRefPoc == picSet->pictureRead[XIN_LIST_0][lftMv.refIdx[XIN_LIST_0]]->framePoc))
            {
                ctMv = lftMv.mv[XIN_LIST_0];
            }
            else if ((lftMv.refIdx[XIN_LIST_1] >= 0) && (colRefPoc == picSet->pictureRead[XIN_LIST_1][lftMv.refIdx[XIN_LIST_1]]->framePoc))
            {
                ctMv = lftMv.mv[XIN_LIST_1];
            }
        }

        ctMv.s32x2[0] = (ctMv.s32x2[0] >= 0) ? (ctMv.s32x2[0] + 8 - 1) >> 4 : (ctMv.s32x2[0] + 8) >> 4;
        ctMv.s32x2[1] = (ctMv.s32x2[1] >= 0) ? (ctMv.s32x2[1] + 8 - 1) >> 4 : (ctMv.s32x2[1] + 8) >> 4;

        middleX = secSet->cu->cuPelX + (pu->width >> 1) + ctMv.s32x2[0];
        middleY = secSet->cu->cuPelY + (pu->height >> 1) + ctMv.s32x2[1];

        // clipColPos
        Xin266ClipColPos (&middleX, &middleY, ctu->ctuPelX, ctu->ctuPelY, seqSet);

        middleX = middleX & mask;
        middleY = middleY & mask;

        PEL_XY_TO_BLOCK_INDEX (middleX, middleY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

        if (pictureRead->blockSetMap[blockIdx].type <= XIN_INTER_MODE)
        {
            defaultMv.refIdx[XIN_LIST_0] = -1;
            defaultMv.refIdx[XIN_LIST_1] = -1;

            if (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0] >= 0)
            {
                defaultMv.mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_0];
                defaultMv.refIdx[XIN_LIST_0] = 0;

                RoundMvComp (
                    &defaultMv.mv[XIN_LIST_0]);

                ScaleMv (
                    &(defaultMv.mv[XIN_LIST_0]),
                    pictureWrite->framePoc,
                    pictureWrite->refFramePoc[XIN_LIST_0][0],
                    pictureRead->framePoc,
                    pictureRead->refFramePoc[XIN_LIST_0][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0]]);
            }
            else if ((pictureWrite->checkLDC) && (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1] >= 0))
            {
                defaultMv.mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_1];
                defaultMv.refIdx[XIN_LIST_0] = 0;

                RoundMvComp (
                    &defaultMv.mv[XIN_LIST_0]);

                ScaleMv (
                    &(defaultMv.mv[XIN_LIST_0]),
                    pictureWrite->framePoc,
                    pictureWrite->refFramePoc[XIN_LIST_0][0],
                    pictureRead->framePoc,
                    pictureRead->refFramePoc[XIN_LIST_1][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1]]);
            }

            if (pictureWrite->frameType == XIN_B_FRAME)
            {
                if (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1] >= 0)
                {

                    defaultMv.mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_1];
                    defaultMv.refIdx[XIN_LIST_1] = 0;

                    RoundMvComp (
                        &defaultMv.mv[XIN_LIST_1]);

                    ScaleMv (
                        &(defaultMv.mv[XIN_LIST_1]),
                        pictureWrite->framePoc,
                        pictureWrite->refFramePoc[XIN_LIST_1][0],
                        pictureRead->framePoc,
                        pictureRead->refFramePoc[XIN_LIST_1][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1]]);
                }
                else if ((pictureWrite->checkLDC) && (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0] >= 0))
                {
                    defaultMv.mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_0];
                    defaultMv.refIdx[XIN_LIST_1] = 0;

                    RoundMvComp (
                        &defaultMv.mv[XIN_LIST_1]);

                    ScaleMv (
                        &(defaultMv.mv[XIN_LIST_1]),
                        pictureWrite->framePoc,
                        pictureWrite->refFramePoc[XIN_LIST_1][0],
                        pictureRead->framePoc,
                        pictureRead->refFramePoc[XIN_LIST_0][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0]]);
                }
            }

            *affineMv = defaultMv;

            for (rowIdx = 0; rowIdx < (SINT32)pu->height; rowIdx += XIN_ATMVP_SUB_BLOCK_SIZE)
            {
                for (colIdx = 0; colIdx < (SINT32)pu->width; colIdx += XIN_ATMVP_SUB_BLOCK_SIZE)
                {
                    subBlockX = secSet->cu->cuPelX + colIdx + ctMv.s32x2[0] + (XIN_ATMVP_SUB_BLOCK_SIZE >> 1);
                    subBlockY = secSet->cu->cuPelY + rowIdx + ctMv.s32x2[1] + (XIN_ATMVP_SUB_BLOCK_SIZE >> 1);

                    Xin266ClipColPos (&subBlockX, &subBlockY, ctu->ctuPelX, ctu->ctuPelY, seqSet);

                    subBlockX = subBlockX & mask;
                    subBlockY = subBlockY & mask;

                    PEL_XY_TO_BLOCK_INDEX (subBlockX, subBlockY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

                    neighbourMv.mv[XIN_LIST_0].s64x1 = 0;
                    neighbourMv.mv[XIN_LIST_1].s64x1 = 0;

                    neighbourMv.refIdx[XIN_LIST_0] = -1;
                    neighbourMv.refIdx[XIN_LIST_1] = -1;

                    if (pictureRead->blockSetMap[blockIdx].type <= XIN_INTER_MODE)
                    {
                        if (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0] >= 0)
                        {
                            neighbourMv.mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_0];
                            neighbourMv.refIdx[XIN_LIST_0] = 0;

                            RoundMvComp (
                                &neighbourMv.mv[XIN_LIST_0]);

                            ScaleMv (
                                &(neighbourMv.mv[XIN_LIST_0]),
                                pictureWrite->framePoc,
                                pictureWrite->refFramePoc[XIN_LIST_0][0],
                                pictureRead->framePoc,
                                pictureRead->refFramePoc[XIN_LIST_0][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0]]);
                        }
                        else if ((pictureWrite->checkLDC) && (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1] >= 0))
                        {
                            neighbourMv.mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_1];
                            neighbourMv.refIdx[XIN_LIST_0] = 0;

                            RoundMvComp (
                                &neighbourMv.mv[XIN_LIST_0]);

                            ScaleMv (
                                &(neighbourMv.mv[XIN_LIST_0]),
                                pictureWrite->framePoc,
                                pictureWrite->refFramePoc[XIN_LIST_0][0],
                                pictureRead->framePoc,
                                pictureRead->refFramePoc[XIN_LIST_1][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1]]);
                        }

                        if (pictureWrite->frameType == XIN_B_FRAME)
                        {
                            if (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1] >= 0)
                            {
                                neighbourMv.mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_1];
                                neighbourMv.refIdx[XIN_LIST_1] = 0;

                                RoundMvComp (
                                    &neighbourMv.mv[XIN_LIST_1]);

                                ScaleMv (
                                    &(neighbourMv.mv[XIN_LIST_1]),
                                    pictureWrite->framePoc,
                                    pictureWrite->refFramePoc[XIN_LIST_1][0],
                                    pictureRead->framePoc,
                                    pictureRead->refFramePoc[XIN_LIST_1][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_1]]);
                            }
                            else if ((pictureWrite->checkLDC) && (pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0] >= 0))
                            {
                                neighbourMv.mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[XIN_LIST_0];
                                neighbourMv.refIdx[XIN_LIST_1] = 0;

                                RoundMvComp (
                                    &neighbourMv.mv[XIN_LIST_1]);

                                ScaleMv (
                                    &(neighbourMv.mv[XIN_LIST_1]),
                                    pictureWrite->framePoc,
                                    pictureWrite->refFramePoc[XIN_LIST_1][0],
                                    pictureRead->framePoc,
                                    pictureRead->refFramePoc[XIN_LIST_0][pictureRead->blockSetMap[blockIdx].refIdx[XIN_LIST_0]]);
                            }

                        }

                        modeCtrl->subMv[subBlockIdx][0].s64x1 = neighbourMv.mv[0].s64x1;
                        modeCtrl->subMv[subBlockIdx][1].s64x1 = neighbourMv.mv[1].s64x1;
                        modeCtrl->subRefIdx[subBlockIdx][0]   = neighbourMv.refIdx[0];
                        modeCtrl->subRefIdx[subBlockIdx][1]   = neighbourMv.refIdx[1];

                    }
                    else
                    {
                        modeCtrl->subMv[subBlockIdx][0].s64x1 = defaultMv.mv[0].s64x1;
                        modeCtrl->subMv[subBlockIdx][1].s64x1 = defaultMv.mv[1].s64x1;
                        modeCtrl->subRefIdx[subBlockIdx][0]   = defaultMv.refIdx[0];
                        modeCtrl->subRefIdx[subBlockIdx][1]   = defaultMv.refIdx[1];
                    }

                    subBlockIdx++;

                }
            }

            candNum++;

        }

    }

    *sbTmvpAvail = candNum != 0;

}

static BOOL Xin266IsSbVectorSpreadOverLimit (
    SINT32 a,
    SINT32 b,
    SINT32 c,
    SINT32 d,
    SINT8  refIdx[2])
{
    SINT32 s4;
    SINT32 filterTap;
    SINT32 refWidth;
    SINT32 refHeight;

    s4        = (4 << 11);
    filterTap = 6;

    if ((refIdx[0] >= 0) && (refIdx[1] >= 0))
    {
        refWidth  = XIN_MAX(XIN_MAX(0, 4 * a + s4), XIN_MAX(4 * c, 4 * a + 4 * c + s4)) - XIN_MIN(XIN_MIN(0, 4 * a + s4), XIN_MIN(4 * c, 4 * a + 4 * c + s4));
        refHeight = XIN_MAX(XIN_MAX(0, 4 * b), XIN_MAX(4 * d + s4, 4 * b + 4 * d + s4)) - XIN_MIN(XIN_MIN(0, 4 * b), XIN_MIN(4 * d + s4, 4 * b + 4 * d + s4));

        refWidth  = (refWidth >> 11) + filterTap + 3;
        refHeight = (refHeight >> 11) + filterTap + 3;

        if (refWidth * refHeight > (filterTap + 9) * (filterTap + 9))
        {
            return TRUE;
        }
    }
    else
    {
        refWidth  = XIN_MAX(0, 4 * a + s4) - XIN_MIN(0, 4 * a + s4);
        refHeight = XIN_MAX(0, 4 * b) - XIN_MIN(0, 4 * b);

        refWidth  = (refWidth >> 11) + filterTap + 3;
        refHeight = (refHeight >> 11) + filterTap + 3;

        if (refWidth * refHeight > (filterTap + 9) * (filterTap + 5))
        {
            return TRUE;
        }

        refWidth  = XIN_MAX(0, 4 * c) - XIN_MIN(0, 4 * c);
        refHeight = XIN_MAX(0, 4 * d + s4) - XIN_MIN(0, 4 * d + s4);

        refWidth  = (refWidth >> 11) + filterTap + 3;
        refHeight = (refHeight >> 11) + filterTap + 3;

        if (refWidth * refHeight > (filterTap + 5) * (filterTap + 9))
        {
            return TRUE;
        }

    }

    return FALSE;

}

void Xin266RoundAffineMv (
    SINT32 *mvx,
    SINT32 *mvy,
    SINT32 nShift)
{
    SINT32 nOffset;

    nOffset = 1 << (nShift - 1);
    *mvx    = (*mvx + nOffset - (*mvx >= 0)) >> nShift;
    *mvy    = (*mvy + nOffset - (*mvy >= 0)) >> nShift;
}

void Xin266FillAffineMotionVector (
    xin_pu_struct   *pu,
    xin_mv32_u      affineMv[XIN_LIST_NUM][XIN_MAX_AFFINE_CPMV_NUM],
    SINT8           refIdx[XIN_LIST_NUM],
    UINT32          affineType,
    UINT32          listIdx,
    xin_mv32_u      *subBlockMv)
{
    SINT32     lgWidth;
    SINT32     lgHeight;
    SINT32     shift;
    BOOL       sameMv;
    SINT32     deltaMvHorX;
    SINT32     deltaMvHorY;
    SINT32     deltaMvVerX;
    SINT32     deltaMvVerY;
    SINT32     rowIdx;
    SINT32     colIdx;
    SINT32     blockInWidth;
    SINT32     blockInHeight;
    SINT32     mvScaleHor;
    SINT32     mvScaleVer;
    BOOL       sbMvSpreadOverLimit;
    SINT32     mvScaleTmpHor;
    SINT32     mvScaleTmpVer;
    SINT32     sbSize;
    SINT32     halfSbSize;
    xin_mv32_u sbMv;
    xin_mv32_u affLT;
    xin_mv32_u affRT;
    xin_mv32_u affLB;

    sameMv        = FALSE;
    affLT         = affineMv[listIdx][0];
    affRT         = affineMv[listIdx][1];
    affLB         = affineMv[listIdx][2];
    lgWidth       = pu->lgWidth;
    lgHeight      = pu->lgHeight;
    deltaMvHorX   = 0;
    deltaMvHorY   = 0;
    deltaMvVerX   = 0;
    deltaMvVerY   = 0;
    blockInWidth  = (1 << lgWidth) / XIN_AFFINE_SUB_BLOCK_SIZE;
    blockInHeight = (1 << lgHeight) / XIN_AFFINE_SUB_BLOCK_SIZE;
    sbSize        = XIN_AFFINE_SUB_BLOCK_SIZE;
    halfSbSize    = XIN_AFFINE_SUB_BLOCK_SIZE >> 1;
    shift         = XIN_MAX_CU_DEPTH;

    if (affLT.s64x1 == affRT.s64x1)
    {
        if (affLT.s64x1 == affLB.s64x1)
        {
            sameMv = TRUE;
        }
    }

    if (!sameMv)
    {
        deltaMvHorX = (affRT.s32x2[0] - affLT.s32x2[0]) * (1 << (shift - lgWidth));
        deltaMvHorY = (affRT.s32x2[1] - affLT.s32x2[1]) * (1 << (shift - lgWidth));

        if (affineType == XIN_AFFINE_MODEL_6PARAM)
        {
            deltaMvVerX = (affLB.s32x2[0] - affLT.s32x2[0]) * (1 << (shift - lgHeight));
            deltaMvVerY = (affLB.s32x2[1] - affLT.s32x2[1]) * (1 << (shift - lgHeight));
        }
        else
        {
            deltaMvVerX = -deltaMvHorY;
            deltaMvVerY =  deltaMvHorX;
        }
    }

    mvScaleHor          = affLT.s32x2[0] * (1<< shift);
    mvScaleVer          = affLT.s32x2[1] * (1<< shift);
    sbMvSpreadOverLimit = Xin266IsSbVectorSpreadOverLimit (deltaMvHorX, deltaMvHorY, deltaMvVerX, deltaMvVerY, refIdx);

    for (rowIdx = 0; rowIdx < blockInHeight; rowIdx++)
    {
        for (colIdx = 0; colIdx < blockInWidth; colIdx++)
        {
            if (sameMv)
            {
                mvScaleTmpHor = mvScaleHor;
                mvScaleTmpVer = mvScaleVer;
            }
            else
            {
                if (!sbMvSpreadOverLimit)
                {
                    mvScaleTmpHor = mvScaleHor + deltaMvHorX * (halfSbSize + colIdx*sbSize) + deltaMvVerX * (halfSbSize + rowIdx*sbSize);
                    mvScaleTmpVer = mvScaleVer + deltaMvHorY * (halfSbSize + colIdx*sbSize) + deltaMvVerY * (halfSbSize + rowIdx*sbSize);

                }
                else
                {
                    mvScaleTmpHor = mvScaleHor + deltaMvHorX * ((1 << lgWidth) >> 1) + deltaMvVerX * ((1 << lgHeight) >> 1);
                    mvScaleTmpVer = mvScaleVer + deltaMvHorY * ((1 << lgWidth) >> 1) + deltaMvVerY * ((1 << lgHeight) >> 1);
                }
            }

            Xin266RoundAffineMv (
                &mvScaleTmpHor,
                &mvScaleTmpVer,
                shift);

            sbMv.mv.mv32X = XIN_CLIP (mvScaleTmpHor, -(1 << 17), (1 << 17) - 1);
            sbMv.mv.mv32Y = XIN_CLIP (mvScaleTmpVer, -(1 << 17), (1 << 17) - 1);

            subBlockMv[rowIdx*blockInWidth+colIdx] = sbMv;

        }

    }

}

void Xin266InheritedAffineMv (
    xin_sec_struct   *secSet,
    xin_block_struct *neighBlock,
    UINT32           affineType,
    UINT32           listIdx,
    xin_mv32_u       rcMv[3])
{
    xin_pic_struct   *picSet;
    xin_seq_struct   *seqSet;
    xin_ref_picture  *pictureWrite;
    xin_cu_struct    *cu;
    xin_affine_mv    *affineMvMap;
    SINT32           posNeiX;
    SINT32           posNeiY;
    SINT32           posCurX;
    SINT32           posCurY;
    SINT32           blockSize;
    BOOL             isTopCtuBoundary;

    SINT32           neiW;
    SINT32           curW;
    SINT32           neiH;
    SINT32           curH;

    SINT32           posRBX;
    SINT32           posRBY;
    SINT32           posLBX;
    SINT32           posLBY;
    UINT32           posRBIdx;
    UINT32           posLBIdx;
    UINT32           blockSetWidth;
    SINT32           iDMvHorX;
    SINT32           iDMvHorY;
    SINT32           iDMvVerX;
    SINT32           iDMvVerY;
    SINT32           shift;
    intptr_t         affineMvStride;
    xin_mv32_u       mvLT, mvRT, mvLB;

    cu      = secSet->cu;
    posNeiX = neighBlock->cuPelX;
    posNeiY = neighBlock->cuPelY;
    posCurX = cu->cuPelX;
    posCurY = cu->cuPelY;
    shift   = XIN_MAX_CU_DEPTH;

    picSet         = secSet->picSet;
    seqSet         = secSet->seqSet;
    blockSize      = seqSet->blockSize;
    affineMvStride = picSet->affineMvStride;
    affineMvMap    = picSet->affineMvMap;
    affineMvMap   += (posNeiY/blockSize)*affineMvStride + posNeiX/blockSize;
    pictureWrite   = picSet->pictureWrite;
    blockSetWidth  = pictureWrite->blockSetWidth;

    neiW = neighBlock->width;
    curW = cu->width;
    neiH = neighBlock->height;
    curH = cu->height;

    mvLT = affineMvMap->mv[listIdx][0];
    mvRT = affineMvMap->mv[listIdx][1];
    mvLB = affineMvMap->mv[listIdx][2];

    isTopCtuBoundary = FALSE;

    if ((posNeiY + neiH) % seqSet->ctuSize == 0 && (posNeiY + neiH) == posCurY)
    {
        // use bottom-left and bottom-right sub-block MVs for inheritance
        posRBX = posNeiX + neiW - 1;
        posRBY = posNeiY + neiH - 1;
        posLBX = posNeiX;
        posLBY = posNeiY + neiH - 1;

        PEL_XY_TO_BLOCK_INDEX (posRBX, posRBY, posRBIdx, blockSetWidth, seqSet->lgBlockSize);
        PEL_XY_TO_BLOCK_INDEX (posLBX, posLBY, posLBIdx, blockSetWidth, seqSet->lgBlockSize);

        mvRT = pictureWrite->blockSetMap[posRBIdx].mv[listIdx];
        mvLT = pictureWrite->blockSetMap[posLBIdx].mv[listIdx];

        posNeiY += neiH;

        isTopCtuBoundary = TRUE;
    }

    iDMvHorX = (mvRT.s32x2[0] - mvLT.s32x2[0]) * (1 << (shift - calcLog2[neiW]));
    iDMvHorY = (mvRT.s32x2[1] - mvLT.s32x2[1]) * (1 << (shift - calcLog2[neiW]));

    if (neighBlock->affineType == XIN_AFFINE_MODEL_6PARAM && !isTopCtuBoundary)
    {
        iDMvVerX = (mvLB.s32x2[0] - mvLT.s32x2[0]) * (1 << (shift - calcLog2[neiH]));
        iDMvVerY = (mvLB.s32x2[1] - mvLT.s32x2[1]) * (1 << (shift - calcLog2[neiH]));
    }
    else
    {
        iDMvVerX = -iDMvHorY;
        iDMvVerY =  iDMvHorX;
    }

    int iMvScaleHor = mvLT.s32x2[0] * (1<< shift);
    int iMvScaleVer = mvLT.s32x2[1] * (1 << shift);
    int horTmp, verTmp;

    // v0
    horTmp = iMvScaleHor + iDMvHorX * (posCurX - posNeiX) + iDMvVerX * (posCurY - posNeiY);
    verTmp = iMvScaleVer + iDMvHorY * (posCurX - posNeiX) + iDMvVerY * (posCurY - posNeiY);

    Xin266RoundAffineMv (
        &horTmp,
        &verTmp,
        shift);

    rcMv[0].mv.mv32X = XIN_CLIP (horTmp, -(1 << 17), (1 << 17) - 1);
    rcMv[0].mv.mv32Y = XIN_CLIP (verTmp, -(1 << 17), (1 << 17) - 1);

    // v1
    horTmp = iMvScaleHor + iDMvHorX * (posCurX + curW - posNeiX) + iDMvVerX * (posCurY - posNeiY);
    verTmp = iMvScaleVer + iDMvHorY * (posCurX + curW - posNeiX) + iDMvVerY * (posCurY - posNeiY);

    Xin266RoundAffineMv (
        &horTmp,
        &verTmp,
        shift);

    rcMv[1].mv.mv32X = XIN_CLIP (horTmp, -(1 << 17), (1 << 17) - 1);
    rcMv[1].mv.mv32Y = XIN_CLIP (verTmp, -(1 << 17), (1 << 17) - 1);

    // v2
    if (affineType == XIN_AFFINE_MODEL_6PARAM)
    {
        horTmp = iMvScaleHor + iDMvHorX * (posCurX - posNeiX) + iDMvVerX * (posCurY + curH - posNeiY);
        verTmp = iMvScaleVer + iDMvHorY * (posCurX - posNeiX) + iDMvVerY * (posCurY + curH - posNeiY);

        Xin266RoundAffineMv (
            &horTmp,
            &verTmp,
            shift);

        rcMv[2].mv.mv32X = XIN_CLIP (horTmp, -(1 << 17), (1 << 17) - 1);
        rcMv[2].mv.mv32Y = XIN_CLIP (verTmp, -(1 << 17), (1 << 17) - 1);
    }

}

void Xin266FillInheritedAffineBlock (
    xin_sec_struct   *secSet,
    xin_block_struct *neighBlock[5],
    UINT32           *blockNum)
{
    xin_block_struct *lftBotBlock;
    xin_block_struct *lftBBlock;
    xin_block_struct *topRgtBlock;
    xin_block_struct *topRBlock;
    xin_block_struct *topLftBlock;
    UINT32           blockIdx;

    blockIdx    = 0;
    lftBotBlock = secSet->lftBotBlock;
    lftBBlock   = secSet->lftBBlock;
    topRgtBlock = secSet->topRgtBlock;
    topRBlock   = secSet->topRBlock;
    topLftBlock = secSet->topLftBlock;

    if ((lftBotBlock->affine > 0) && (lftBotBlock->affineType != XIN_AFFINE_SBTMVP))
    {
        neighBlock[blockIdx] = lftBotBlock;
        blockIdx++;
    }
    else if ((lftBBlock->affine > 0) && (lftBBlock->affineType != XIN_AFFINE_SBTMVP))
    {
        neighBlock[blockIdx] = lftBBlock;
        blockIdx++;
    }

    if ((topRgtBlock->affine > 0) && (topRgtBlock->affineType != XIN_AFFINE_SBTMVP))
    {
        neighBlock[blockIdx] = topRgtBlock;
        blockIdx++;
    }
    else if ((topRBlock->affine > 0) && (topRBlock->affineType != XIN_AFFINE_SBTMVP))
    {
        neighBlock[blockIdx] = topRBlock;
        blockIdx++;
    }
    else if ((topLftBlock->affine > 0) && (topLftBlock->affineType != XIN_AFFINE_SBTMVP))
    {
        neighBlock[blockIdx] = topLftBlock;
        blockIdx++;
    }

    *blockNum = blockIdx;

}

void Xin266FillConstructedAffineCand (
    xin_sec_struct   *secSet,
    BOOL             *isAvailable,
    xin_neighbour_mv *neighMv,
    UINT8            neighBcw[2])
{
    xin_seq_struct   *seqSet;
    xin_cu_struct    *cu;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;
    xin_ref_picture  *pictureRead;
    xin_block_struct *topLftBlock;
    xin_block_struct *lftTBlock;
    xin_block_struct *topLBlock;
    xin_block_struct *topRBlock;
    xin_block_struct *topRgtBlock;
    xin_block_struct *lftBBlock;
    xin_block_struct *lftBotBlock;
    SINT32           mostRgtX;
    SINT32           frameWidth;
    SINT32           mostBotY;
    SINT32           frameHeight;
    SINT32           offsetY;
    SINT32           offsetX;
    UINT32           blockIdx;
    UINT32           blockSetWidth;
    UINT32           colRefList;

    seqSet      = secSet->seqSet;
    picSet      = secSet->picSet;
    cu          = secSet->cu;
    topLftBlock = secSet->topLftBlock;
    lftTBlock   = secSet->lftTBlock;
    topLBlock   = secSet->topLBlock;
    topRBlock   = secSet->topRBlock;
    topRgtBlock = secSet->topRgtBlock;
    lftBBlock   = secSet->lftBBlock;
    lftBotBlock = secSet->lftBotBlock;
    frameWidth  = seqSet->frameWidth;
    frameHeight = seqSet->frameHeight;

    isAvailable[0] = FALSE;
    isAvailable[1] = FALSE;
    isAvailable[2] = FALSE;
    isAvailable[3] = FALSE;

    if (topLftBlock->type <= XIN_INTER_MODE)
    {
        neighMv[0].mv[0].s64x1 = topLftBlock->mv[0].s64x1;
        neighMv[0].mv[1].s64x1 = topLftBlock->mv[1].s64x1;
        neighMv[0].refIdx[0]   = topLftBlock->refIdx[0];
        neighMv[0].refIdx[1]   = topLftBlock->refIdx[1];

        neighBcw[0]    = topLftBlock->bcwIdx;
        isAvailable[0] = TRUE;
    }
    else if (topLBlock->type <= XIN_INTER_MODE)
    {
        neighMv[0].mv[0].s64x1 = topLBlock->mv[0].s64x1;
        neighMv[0].mv[1].s64x1 = topLBlock->mv[1].s64x1;
        neighMv[0].refIdx[0]   = topLBlock->refIdx[0];
        neighMv[0].refIdx[1]   = topLBlock->refIdx[1];

        neighBcw[0]    = topLBlock->bcwIdx;
        isAvailable[0] = TRUE;
    }
    else if (lftTBlock->type <= XIN_INTER_MODE)
    {
        neighMv[0].mv[0].s64x1 = lftTBlock->mv[0].s64x1;
        neighMv[0].mv[1].s64x1 = lftTBlock->mv[1].s64x1;
        neighMv[0].refIdx[0]   = lftTBlock->refIdx[0];
        neighMv[0].refIdx[1]   = lftTBlock->refIdx[1];
        
        neighBcw[0]    = lftTBlock->bcwIdx;
        isAvailable[0] = TRUE;
    }

    if (topRBlock->type <= XIN_INTER_MODE)
    {
        neighMv[1].mv[0].s64x1 = topRBlock->mv[0].s64x1;
        neighMv[1].mv[1].s64x1 = topRBlock->mv[1].s64x1;
        neighMv[1].refIdx[0]   = topRBlock->refIdx[0];
        neighMv[1].refIdx[1]   = topRBlock->refIdx[1];

        neighBcw[1]    = topRBlock->bcwIdx;
        isAvailable[1] = TRUE;
    }
    else if (topRgtBlock->type <= XIN_INTER_MODE)
    {
        neighMv[1].mv[0].s64x1 = topRgtBlock->mv[0].s64x1;
        neighMv[1].mv[1].s64x1 = topRgtBlock->mv[1].s64x1;
        neighMv[1].refIdx[0]   = topRgtBlock->refIdx[0];
        neighMv[1].refIdx[1]   = topRgtBlock->refIdx[1];
        
        neighBcw[1]    = topRgtBlock->bcwIdx;
        isAvailable[1] = TRUE;
    }

    if (lftBBlock->type <= XIN_INTER_MODE)
    {
        neighMv[2].mv[0].s64x1 = lftBBlock->mv[0].s64x1;
        neighMv[2].mv[1].s64x1 = lftBBlock->mv[1].s64x1;
        neighMv[2].refIdx[0]   = lftBBlock->refIdx[0];
        neighMv[2].refIdx[1]   = lftBBlock->refIdx[1];
        
        isAvailable[2] = TRUE;
    }
    else if (lftBotBlock->type <= XIN_INTER_MODE)
    {
        neighMv[2].mv[0].s64x1 = lftBotBlock->mv[0].s64x1;
        neighMv[2].mv[1].s64x1 = lftBotBlock->mv[1].s64x1;
        neighMv[2].refIdx[0]   = lftBotBlock->refIdx[0];
        neighMv[2].refIdx[1]   = lftBotBlock->refIdx[1];
        
        isAvailable[2] = TRUE;
    }

    // control point: RB
    if (seqSet->config.enableTMvp)
    {
        offsetX       = cu->width;
        offsetY       = cu->height;
        mostRgtX      = cu->cuPelX + offsetX;
        mostBotY      = cu->cuPelY + offsetY;
        pictureWrite  = picSet->pictureWrite;
        pictureRead   = picSet->pictureRead[(pictureWrite->frameType == XIN_B_FRAME) ? 1-pictureWrite->colFromL0Flag : 0][0];
        blockIdx      = 0;
        blockSetWidth = pictureRead->blockSetWidth;

        if ((mostRgtX >= frameWidth) || (mostBotY >= frameHeight) || ((offsetY + cu->offY) >= seqSet->ctuSize))
        {

        }
        else
        {
            mostRgtX = (mostRgtX >> seqSet->lgBlockSize) << seqSet->lgBlockSize;
            mostBotY = (mostBotY >> seqSet->lgBlockSize) << seqSet->lgBlockSize;

            PEL_XY_TO_BLOCK_INDEX(mostRgtX, mostBotY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

            if (pictureRead->blockSetMap[blockIdx].type <= XIN_INTER_MODE)
            {
                colRefList = pictureWrite->checkLDC ? XIN_LIST_0 : pictureWrite->colFromL0Flag;

                if (pictureRead->blockSetMap[blockIdx].refIdx[colRefList] >= 0)
                {
                    neighMv[3].mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                    neighMv[3].refIdx[XIN_LIST_0] = 0;
                }
                else
                {
                    colRefList                        = !colRefList;
                    neighMv[3].mv[XIN_LIST_0]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                    neighMv[3].refIdx[XIN_LIST_0] = 0;
                }

                RoundMvComp (
                    &neighMv[3].mv[XIN_LIST_0]);

                ScaleMv (
                    &(neighMv[3].mv[XIN_LIST_0]),
                    pictureWrite->framePoc,
                    pictureWrite->refFramePoc[XIN_LIST_0][0],
                    pictureRead->framePoc,
                    pictureRead->refFramePoc[colRefList][pictureRead->blockSetMap[blockIdx].refIdx[colRefList]]);

                if (pictureWrite->frameType == XIN_B_FRAME)
                {
                    colRefList = pictureWrite->checkLDC ? XIN_LIST_1 : pictureWrite->colFromL0Flag;

                    if (pictureRead->blockSetMap[blockIdx].refIdx[colRefList] >= 0)
                    {
                        neighMv[3].mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                        neighMv[3].refIdx[XIN_LIST_1] = 0;
                    }
                    else
                    {
                        colRefList                    = !colRefList;
                        neighMv[3].mv[XIN_LIST_1]     = pictureRead->blockSetMap[blockIdx].mv[colRefList];
                        neighMv[3].refIdx[XIN_LIST_1] = 0;
                    }

                    RoundMvComp (
                        &neighMv[3].mv[XIN_LIST_1]);

                    ScaleMv (
                        &(neighMv[3].mv[XIN_LIST_1]),
                        pictureWrite->framePoc,
                        pictureWrite->refFramePoc[XIN_LIST_1][0],
                        pictureRead->framePoc,
                        pictureRead->refFramePoc[colRefList][pictureRead->blockSetMap[blockIdx].refIdx[colRefList]]);

                }

                isAvailable[3] = TRUE;

            }

        }

    }

}

void Xin266GetAffineControlPointCand (
    xin_sec_struct   *secSet,
    xin_neighbour_mv neighMv[4],
    BOOL             isAvailable[4],
    SINT32           verIdx[4],
    UINT8            bcwIdx,
    SINT32           modelIdx,
    SINT32           verNum,
    xin_affine_cpmv  *affineMv,
    UINT32           *numMv)
{
    xin_cu_struct *cu;
    SINT32        lgWidth;
    SINT32        lgHeight;
    SINT32        shift;
    SINT32        shiftHtoW;
    SINT32        idx0;
    SINT32        idx1;
    SINT32        idx2;
    SINT32        idx;
    SINT32        listIdx;
    SINT32        refDir;
    xin_mv32_u    cMv[2][4];
    SINT32        mvX, mvY;
    UINT32        affineType;
    SINT32        refIdx[2];

    cu         = secSet->cu;
    lgWidth    = cu->lgWidth;
    lgHeight   = cu->lgHeight;
    shift      = XIN_MAX_CU_DEPTH;
    shiftHtoW  = shift + lgWidth - lgHeight;
    refIdx[0]  = -1;
    refIdx[1]  = -1;
    affineType = (verNum == 2) ? XIN_AFFINE_MODEL_4PARAM : XIN_AFFINE_MODEL_6PARAM;
    refDir     = 0;

    if (verNum == 2)
    {
        idx0 = verIdx[0];
        idx1 = verIdx[1];

        if (!isAvailable[idx0] || !isAvailable[idx1])
        {
            return;
        }

        for (listIdx = 0; listIdx < 2; listIdx++)
        {
            if (neighMv[idx0].refIdx[listIdx] >= 0 && neighMv[idx1].refIdx[listIdx] >= 0)
            {
                // check same refidx and different mv
                if (neighMv[idx0].refIdx[listIdx] == neighMv[idx1].refIdx[listIdx])
                {
                    refDir         |= (listIdx + 1);
                    refIdx[listIdx] = neighMv[idx0].refIdx[listIdx];
                }
            }
        }

    }
    else if (verNum == 3)
    {
        idx0 = verIdx[0];
        idx1 = verIdx[1];
        idx2 = verIdx[2];

        if (!isAvailable[idx0] || !isAvailable[idx1] || !isAvailable[idx2])
        {
            return;
        }

        for (listIdx = 0; listIdx < 2; listIdx++)
        {
            if (neighMv[idx0].refIdx[listIdx] >= 0 && neighMv[idx1].refIdx[listIdx] >= 0 && neighMv[idx2].refIdx[listIdx] >= 0)
            {
                // check same refidx and different mv
                if (neighMv[idx0].refIdx[listIdx] == neighMv[idx1].refIdx[listIdx] && neighMv[idx0].refIdx[listIdx] == neighMv[idx2].refIdx[listIdx])
                {
                    refDir         |= (listIdx + 1);
                    refIdx[listIdx] = neighMv[idx0].refIdx[listIdx];
                }
            }
        }
    }

    if (refDir == 0)
    {
        return;
    }

    for (listIdx = 0; listIdx < 2; listIdx++)
    {
        if (refDir & (listIdx + 1))
        {
            for (idx = 0; idx < verNum; idx++)
            {
                cMv[listIdx][verIdx[idx]] = neighMv[verIdx[idx]].mv[listIdx];
            }

            // convert to LT, RT[, [LB]]
            switch (modelIdx)
            {
            case 0: // 0 : LT, RT, LB
                break;

            case 1: // 1 : LT, RT, RB
                mvX = cMv[listIdx][3].mv.mv32X + cMv[listIdx][0].mv.mv32X - cMv[listIdx][1].mv.mv32X;
                mvY = cMv[listIdx][3].mv.mv32Y + cMv[listIdx][0].mv.mv32Y - cMv[listIdx][1].mv.mv32Y;

                cMv[listIdx][2].mv.mv32X = XIN_CLIP (mvX, -(1 << 17), (1 << 17) - 1);
                cMv[listIdx][2].mv.mv32Y = XIN_CLIP (mvY, -(1 << 17), (1 << 17) - 1);

                break;

            case 2: // 2 : LT, LB, RB
                mvX = cMv[listIdx][3].mv.mv32X + cMv[listIdx][0].mv.mv32X - cMv[listIdx][2].mv.mv32X;
                mvY = cMv[listIdx][3].mv.mv32Y + cMv[listIdx][0].mv.mv32Y - cMv[listIdx][2].mv.mv32Y;

                cMv[listIdx][1].mv.mv32X = XIN_CLIP (mvX, -(1 << 17), (1 << 17) - 1);
                cMv[listIdx][1].mv.mv32Y = XIN_CLIP (mvY, -(1 << 17), (1 << 17) - 1);

                break;

            case 3: // 3 : RT, LB, RB
                mvX = cMv[listIdx][1].mv.mv32X + cMv[listIdx][2].mv.mv32X - cMv[listIdx][3].mv.mv32X;
                mvY = cMv[listIdx][1].mv.mv32Y + cMv[listIdx][2].mv.mv32Y - cMv[listIdx][3].mv.mv32Y;

                cMv[listIdx][0].mv.mv32X = XIN_CLIP (mvX, -(1 << 17), (1 << 17) - 1);
                cMv[listIdx][0].mv.mv32Y = XIN_CLIP (mvY, -(1 << 17), (1 << 17) - 1);

                break;

            case 4: // 4 : LT, RT

                break;

            case 5: // 5 : LT, LB
                mvX = (cMv[listIdx][0].mv.mv32X *(1<< shift)) + ((cMv[listIdx][2].mv.mv32Y - cMv[listIdx][0].mv.mv32Y) * (1<< shiftHtoW));
                mvY = (cMv[listIdx][0].mv.mv32Y *(1<< shift)) - ((cMv[listIdx][2].mv.mv32X - cMv[listIdx][0].mv.mv32X) * (1<< shiftHtoW));

                Xin266RoundAffineMv (
                    &mvX,
                    &mvY,
                    shift);

                cMv[listIdx][1].mv.mv32X = XIN_CLIP (mvX, -(1 << 17), (1 << 17) - 1);
                cMv[listIdx][1].mv.mv32Y = XIN_CLIP (mvY, -(1 << 17), (1 << 17) - 1);

                break;

            default:

                _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid model index!");
                break;

            }

        }
        else
        {
            for (idx = 0; idx < 4; idx++)
            {
                cMv[listIdx][idx].s64x1 = 0;
            }
        }

    }

    for (idx = 0; idx < 3; idx++)
    {
        affineMv->mv[0][idx].s64x1 = cMv[0][idx].s64x1;
        affineMv->mv[1][idx].s64x1 = cMv[1][idx].s64x1;
    }

    affineMv->refIdx[0]  = (SINT8)refIdx[0];
    affineMv->refIdx[1]  = (SINT8)refIdx[1];
    affineMv->affineType = (UINT8)affineType;
    affineMv->bcwIdx     = (refIdx[0] >= 0 && refIdx[1] >= 0) ? bcwIdx : XIN_BCW_DEFAULT;

    (*numMv)++;

}

void Xin266GetAffineMergeCand (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    xin_affine_cpmv  *affMergeCand,
    UINT32           *mvNum)
{
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;
    xin_ref_picture  *pictureRead;
    xin_neighbour_mv sbtMvp;
    UINT32           colRefList;
    UINT32           maxAffineMergeCand;
    UINT32           mergeIdx;
    UINT32           mvIdx;
    UINT32           numMvs;
    BOOL             sbTmvpAvail;
    BOOL             enableSbTmvp;
    xin_block_struct *neighBlock[5];
    UINT32           blockNum;
    UINT32           blockIdx;
    xin_mv32_u       affineMv[XIN_LIST_NUM][3];
    xin_neighbour_mv neighMv[4];
    UINT8            neighBcw[2];
    BOOL             isAvailable[4];
    SINT32           modelIdx;
    SINT32           startIdx;

    seqSet             = secSet->seqSet;
    maxAffineMergeCand = seqSet->config.maxAffineMergeCand;
    picSet             = secSet->picSet;
    pictureWrite       = picSet->pictureWrite;
    colRefList         = (pictureWrite->frameType == XIN_B_FRAME) ? 1 - pictureWrite->colFromL0Flag : 0;
    pictureRead        = picSet->pictureRead[colRefList][0];
    enableSbTmvp       = seqSet->config.enableSbtMvp && pictureRead->frameType < XIN_I_FRAME;
    numMvs             = 0;
    neighBcw[0]        = XIN_BCW_DEFAULT;
    neighBcw[1]        = XIN_BCW_DEFAULT;

    for (mergeIdx = 0; mergeIdx < maxAffineMergeCand; mergeIdx++)
    {
        for (mvIdx = 0; mvIdx < XIN_MAX_AFFINE_CPMV_NUM; mvIdx++)
        {
            affMergeCand[mergeIdx].mv[0][mvIdx].s64x1 = 0;
            affMergeCand[mergeIdx].mv[1][mvIdx].s64x1 = 0;
        }

        affMergeCand[mergeIdx].refIdx[0]  = -1;
        affMergeCand[mergeIdx].refIdx[1]  = -1;
        affMergeCand[mergeIdx].affineType = 0;
        affMergeCand[mergeIdx].bcwIdx     = XIN_BCW_DEFAULT;
    }

    if (enableSbTmvp)
    {
        Xin266FillSbtMvpMergeCand (
            secSet,
            pu,
            &sbtMvp,
            &sbTmvpAvail);

        if (sbTmvpAvail)
        {
            for (mvIdx = 0; mvIdx < 3; mvIdx++)
            {
                affMergeCand[numMvs].mv[0][mvIdx] = sbtMvp.mv[0];
                affMergeCand[numMvs].mv[1][mvIdx] = sbtMvp.mv[1];
            }

            affMergeCand[numMvs].affineType = XIN_AFFINE_SBTMVP;

            numMvs++;

            // early termination
            if (maxAffineMergeCand == numMvs)
            {
                *mvNum = numMvs;

                return;
            }

        }

    }

    if (seqSet->config.enableAffine)
    {
        ///> Start: inherited affine candidates
        Xin266FillInheritedAffineBlock (
            secSet,
            neighBlock,
            &blockNum);

        for (blockIdx = 0; blockIdx < blockNum; blockIdx++)
        {
            if (neighBlock[blockIdx]->refIdx[0] >= 0)
            {
                Xin266InheritedAffineMv (
                    secSet,
                    neighBlock[blockIdx],
                    neighBlock[blockIdx]->affineType,
                    XIN_LIST_0,
                    affineMv[XIN_LIST_0]);
            }

            if (pictureWrite->frameType == XIN_B_FRAME)
            {
                if (neighBlock[blockIdx]->refIdx[1] >= 0)
                {
                    Xin266InheritedAffineMv (
                        secSet,
                        neighBlock[blockIdx],
                        neighBlock[blockIdx]->affineType,
                        XIN_LIST_1,
                        affineMv[XIN_LIST_1]);
                }
            }

            for (mvIdx = 0; mvIdx < 3; mvIdx++)
            {
                affMergeCand[numMvs].mv[0][mvIdx].s64x1 = affineMv[XIN_LIST_0][mvIdx].s64x1;
                affMergeCand[numMvs].mv[1][mvIdx].s64x1 = affineMv[XIN_LIST_1][mvIdx].s64x1;
            }

            affMergeCand[numMvs].refIdx[0]  = neighBlock[blockIdx]->refIdx[XIN_LIST_0];
            affMergeCand[numMvs].refIdx[1]  = neighBlock[blockIdx]->refIdx[XIN_LIST_1];
            affMergeCand[numMvs].affineType = neighBlock[blockIdx]->affineType;
            affMergeCand[numMvs].bcwIdx     = neighBlock[blockIdx]->bcwIdx;

            numMvs++;

            if (maxAffineMergeCand == numMvs)
            {
                *mvNum = numMvs;

                return;
            }

        }
        ///> End: inherited affine candidates

        ///> Start: Constructed affine candidates
        Xin266FillConstructedAffineCand (
            secSet,
            isAvailable,
            neighMv,
            neighBcw);

        //-------------------  insert model  -------------------//
        startIdx = seqSet->config.affineType ? 0 : 4;

        for (modelIdx = startIdx; modelIdx < 6; modelIdx++)
        {
            Xin266GetAffineControlPointCand (
                secSet,
                neighMv,
                isAvailable,
                affineModel[modelIdx],
                (modelIdx == 3) ? neighBcw[1] : neighBcw[0],
                modelIdx,
                affineVerNum[modelIdx],
                affMergeCand + numMvs,
                &numMvs);

            // early termination
            if (maxAffineMergeCand == numMvs)
            {
                *mvNum = numMvs;

                return;
            }

        }
        ///> End: Constructed affine candidates

    }

    *mvNum = numMvs;

    ///> zero padding
    while (numMvs < maxAffineMergeCand)
    {
        for (mvIdx = 0; mvIdx < 3; mvIdx++)
        {
            affMergeCand[numMvs].mv[0][mvIdx].s64x1 = 0;
            affMergeCand[numMvs].mv[1][mvIdx].s64x1 = 0;
        }

        affMergeCand[numMvs].refIdx[0] = 0;
        affMergeCand[numMvs].refIdx[1] = (pictureWrite->frameType == XIN_B_FRAME) ? 0 : -1;

        numMvs++;
    }

}

void Xin266GetTMvpForAmvp (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    SINT32           listIdx,
    SINT32           refIdx,
    xin_mv32_u       *outputMv)
{
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    UINT32           frameWidth;
    UINT32           frameHeight;
    UINT32           blockSetWidth;
    UINT32           blockIdx;
    UINT32           offsetX;
    UINT32           offsetY;
    UINT32           middleX;
    UINT32           middleY;
    UINT32           mostRgtX;
    UINT32           mostBotY;
    UINT32           tmvpPos;
    xin_ref_picture  *pictureWrite;
    xin_ref_picture  *pictureRead;
    xin_block_struct *blockSetMap;
    SINT32           colRefList;

    seqSet        = secSet->seqSet;
    picSet        = secSet->picSet;
    frameWidth    = seqSet->frameWidth;
    frameHeight   = seqSet->frameHeight;
    offsetX       = pu->width;
    offsetY       = pu->height;
    mostRgtX      = secSet->cu->cuPelX + offsetX;
    mostBotY      = secSet->cu->cuPelY + offsetY;
    middleX       = mostRgtX - pu->width/2;
    middleY       = mostBotY - pu->height/2;
    pictureWrite  = picSet->pictureWrite;
    pictureRead   = picSet->pictureRead[(pictureWrite->frameType == XIN_B_FRAME) ? 1-pictureWrite->colFromL0Flag : 0][0];
    blockIdx      = 0;
    blockSetWidth = pictureRead->blockSetWidth;
    blockSetMap   = pictureRead->blockSetMap;

    if ((mostRgtX >= frameWidth) || (mostBotY >= frameHeight) || ((offsetY + secSet->cu->offY) >= seqSet->ctuSize))
    {
        tmvpPos = XIN_TMVP_COL_CENTRE;
    }
    else
    {
        mostRgtX = (mostRgtX >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;
        mostBotY = (mostBotY >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;

        PEL_XY_TO_BLOCK_INDEX(mostRgtX, mostBotY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

        tmvpPos = (blockSetMap[blockIdx].type <= XIN_INTER_MODE) ? XIN_TMVP_COL_BOT_RGT : XIN_TMVP_COL_CENTRE;
    }

    if (tmvpPos == XIN_TMVP_COL_BOT_RGT)
    {
        colRefList = pictureWrite->checkLDC ? listIdx : pictureWrite->colFromL0Flag;
        colRefList = (blockSetMap[blockIdx].refIdx[colRefList] >= 0) ? colRefList : !colRefList;
        outputMv->s64x1 = blockSetMap[blockIdx].mv[colRefList].s64x1;

        RoundMvComp (
            outputMv);

        ScaleMv (
            outputMv,
            pictureWrite->framePoc,
            pictureWrite->refFramePoc[listIdx][refIdx],
            pictureRead->framePoc,
            pictureRead->refFramePoc[colRefList][blockSetMap[blockIdx].refIdx[colRefList]]);

    }
    else
    {
        middleX = (middleX >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;
        middleY = (middleY >> XIN_LG_TMP_UNIT_SIZE) << XIN_LG_TMP_UNIT_SIZE;

        PEL_XY_TO_BLOCK_INDEX(middleX, middleY, blockIdx, blockSetWidth, seqSet->lgBlockSize);

        if (blockSetMap[blockIdx].type <= XIN_INTER_MODE)
        {
            colRefList = pictureWrite->checkLDC ? listIdx : pictureWrite->colFromL0Flag;
            colRefList = (blockSetMap[blockIdx].refIdx[colRefList] >= 0) ? colRefList : !colRefList;

            outputMv->s64x1 = blockSetMap[blockIdx].mv[colRefList].s64x1;

            RoundMvComp (
                outputMv);

            ScaleMv (
                outputMv,
                pictureWrite->framePoc,
                pictureWrite->refFramePoc[listIdx][refIdx],
                pictureRead->framePoc,
                pictureRead->refFramePoc[colRefList][blockSetMap[blockIdx].refIdx[colRefList]]);

        }
        else
        {
            outputMv->s64x1 = 0x8000000080000000;
        }

    }

}

static void Xin266GetHMvpForAmvp (
    xin_sec_struct   *secSet,
    SINT32           listIdx,
    SINT32           refIdx,
    xin_mv32_u       *mvpList,
    UINT32           *mvNum)
{
    SINT32           mrgIdx;
    UINT32           mvIdx;
    SINT32           hmvpNum;
    xin_neighbour_mv *hmvpLut;
    xin_neighbour_mv *neighbourMv;
    SINT32           list2ndIdx;
    SINT32           refListIdx;
    SINT32           neibRefIdx;
    SINT32           predIdx;
    SINT32           currRefPOC;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;

    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    currRefPOC   = pictureWrite->refFramePoc[listIdx][refIdx];
    hmvpNum      = secSet->hmvpNum;
    hmvpNum      = XIN_MIN (XIN_MAX_HMVP_AVMP_CAND_NUM, hmvpNum);
    hmvpLut      = secSet->hmvpLut;
    list2ndIdx   = (listIdx == XIN_LIST_0) ? XIN_LIST_1 : XIN_LIST_0;
    mvIdx        = *mvNum;

    for (mrgIdx = 1; mrgIdx <= hmvpNum; mrgIdx++)
    {
        neighbourMv = &hmvpLut[mrgIdx - 1];

        for (predIdx = 0; predIdx < 2; predIdx++)
        {
            refListIdx = (predIdx == 0) ? listIdx : list2ndIdx;
            neibRefIdx = neighbourMv->refIdx[refListIdx];

            if ((neibRefIdx >= 0) && (currRefPOC == pictureWrite->refFramePoc[refListIdx][neibRefIdx]))
            {
                mvpList[mvIdx] = neighbourMv->mv[refListIdx];
                mvIdx++;

                if (mvIdx >= XIN_MAX_AMVP_CAND_NUM)
                {
                    *mvNum = mvIdx;

                    return;
                }
            }
        }

    }

    *mvNum = mvIdx;

}

void Xin266FillAmvpCand (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    SINT32         listIdx,
    SINT32         refIdx,
    SINT32         imvIdx,
    xin_mv32_u     *amvpCand,
    UINT32         *validMvs)
{
    UINT32           numMvs;
    UINT32           mvIdx;
    xin_ref_picture  *pictureWrite;
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    xin_block_struct *lftBlock[2];
    xin_block_struct *topBlock[3];
    UINT32           topAvailField;
    UINT32           lftAvailField;
    xin_mv32_u       tempMv;
    BOOL             lftMvpAvail;
    BOOL             topMvpAvail;
    xin_mv32_u       mvpList[4];
    SINT32           mvPrecIdx;

    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    pictureWrite = picSet->pictureWrite;
    lftBlock[0]  = secSet->lftBotBlock;
    lftBlock[1]  = secSet->lftBBlock;
    topBlock[0]  = secSet->topRgtBlock;
    topBlock[1]  = secSet->topRBlock;
    topBlock[2]  = secSet->topLftBlock;
    lftMvpAvail  = FALSE;
    topMvpAvail  = FALSE;
    numMvs       = 0;
    mvPrecIdx    = amvrPrecision[imvIdx];

    lftAvailField = (lftBlock[0]->type <= XIN_INTER_MODE) + ((lftBlock[1]->type <= XIN_INTER_MODE) << 1);
    topAvailField = (topBlock[0]->type <= XIN_INTER_MODE) + ((topBlock[1]->type <= XIN_INTER_MODE) << 1) + ((topBlock[2]->type <= XIN_INTER_MODE) << 2);

    Xin266AddDirectMvpCand (
        pictureWrite,
        lftBlock,
        2,
        &tempMv,
        lftAvailField,
        listIdx,
        refIdx,
        &lftMvpAvail);

    if (lftMvpAvail)
    {
        mvpList[numMvs].s64x1 = tempMv.s64x1;
        numMvs++;
    }

    Xin266AddDirectMvpCand (
        pictureWrite,
        topBlock,
        3,
        &tempMv,
        topAvailField,
        listIdx,
        refIdx,
        &topMvpAvail);

    if (topMvpAvail)
    {
        mvpList[numMvs].s64x1 = tempMv.s64x1;
        numMvs++;
    }

    if ((numMvs == 2) && (mvpList[0].s64x1 == mvpList[1].s64x1))
    {
        numMvs = 1;
    }

    if ((seqSet->config.enableTMvp) && (numMvs < 2))
    {
        Xin266GetTMvpForAmvp (
            secSet,
            pu,
            listIdx,
            refIdx,
            mvpList + numMvs);

        if (mvpList[numMvs].s64x1 != 0x8000000080000000)
        {
            numMvs++;
        }
    }

    if (numMvs < XIN_MAX_AMVP_CAND_NUM)
    {
        Xin266GetHMvpForAmvp (
            secSet,
            listIdx,
            refIdx,
            mvpList,
            &numMvs);
    }

    if ((numMvs < 1) || (numMvs == 1 && mvpList[0].s64x1 != 0))
    {
        mvpList[numMvs].s64x1 = 0;
        numMvs++;
    }

    if ((numMvs >= XIN_MAX_AMVP_CAND_NUM) && (mvpList[0].s64x1 != mvpList[1].s64x1))
    {
        *validMvs         = XIN_MAX_AMVP_CAND_NUM;
        amvpCand[0].s64x1 = mvpList[0].s64x1;
        amvpCand[1].s64x1 = mvpList[1].s64x1;
    }
    else
    {
        *validMvs         = 1;
        amvpCand[0].s64x1 = mvpList[0].s64x1;
    }

    for (mvIdx = 0; mvIdx < *validMvs; mvIdx++)
    {
        Xin266ChangeMv32Prec (
            amvpCand + mvIdx,
            XIN_MV_PREC_INTERNAL,
            mvPrecIdx);

        Xin266ChangeMv32Prec (
            amvpCand + mvIdx,
            mvPrecIdx,
            XIN_MV_PREC_INTERNAL);
    }

}

static void GetDirectAffineMvp (
    xin_sec_struct   *secSet,
    xin_mv32_u       mv[3],
    SINT32           affineType,
    xin_block_struct *nbBlock,
    SINT32           listIdx,
    SINT32           refIdx,
    BOOL             *mvpAvail)
{
    SINT32          curRefPoc;
    SINT32          nbRefIdx;
    SINT32          nbRefPoc;
    xin_ref_picture *pictureWrite;
    xin_pic_struct  *picSet;

    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    curRefPoc    = pictureWrite->refFramePoc[listIdx][refIdx];
    nbRefIdx     = nbBlock->refIdx[listIdx];
    nbRefPoc     = (nbRefIdx != -1) ? pictureWrite->refFramePoc[listIdx][nbRefIdx] : nbRefIdx;

    if (curRefPoc == nbRefPoc)
    {
        Xin266InheritedAffineMv (
            secSet,
            nbBlock,
            affineType,
            listIdx,
            mv);

        *mvpAvail = TRUE;
    }
    else
    {
        listIdx   = !listIdx;
        nbRefIdx  = nbBlock->refIdx[listIdx];
        nbRefPoc  = (nbRefIdx != -1) ? pictureWrite->refFramePoc[listIdx][nbRefIdx] : -1;

        if (curRefPoc == nbRefPoc)
        {
            Xin266InheritedAffineMv (
                secSet,
                nbBlock,
                affineType,
                listIdx,
                mv);

            *mvpAvail = TRUE;
        }
    }

    return;

}

void Xin266AddDirectAffineMvpCandLeft (
    xin_sec_struct   *secSet,
    xin_block_struct *lftBlock[2],
    xin_mv32_u       mvp[3],
    UINT32           affineType,
    UINT32           lftAvailField,
    SINT32           listIdx,
    SINT32           refIdx,
    BOOL             *lftMvpAvail)
{
    BOOL mvpAvail;

    mvpAvail = FALSE;

    if (lftAvailField & 1)
    {
        GetDirectAffineMvp (
            secSet,
            mvp,
            affineType,
            lftBlock[0],
            listIdx,
            refIdx,
            &mvpAvail);

    }

    if ((!mvpAvail) && (lftAvailField & 2))
    {
        GetDirectAffineMvp (
            secSet,
            mvp,
            affineType,
            lftBlock[1],
            listIdx,
            refIdx,
            &mvpAvail);
    }

    *lftMvpAvail = mvpAvail;

}

void Xin266FillAffineAmvpCand (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    SINT32         affineType,
    SINT32         listIdx,
    SINT32         refIdx,
    xin_mv32_u     amvpCand[3][3],
    UINT32         *validMvs)
{
    xin_block_struct *lftBlock[2];
    xin_block_struct *topBlock[3];
    xin_block_struct *topLftBlock[3];
    BOOL             lftMvpAvail;
    BOOL             topMvpAvail;
    BOOL             topLftMvpAvail;
    xin_seq_struct   *seqSet;
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;
    UINT32           lftAvailField;
    UINT32           topAvailField;
    UINT32           topLftAvailField;
    xin_mv32_u       tempMv[3];
    UINT32           numMvs;
    SINT32           cornerMVPattern;

    lftBlock[0]  = secSet->lftBotBlock;
    lftBlock[1]  = secSet->lftBBlock;
    topBlock[0]  = secSet->topRgtBlock;
    topBlock[1]  = secSet->topRBlock;
    topBlock[2]  = secSet->topLftBlock;
    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    pictureWrite = picSet->pictureWrite;
    numMvs       = 0;

    lftAvailField   = ((lftBlock[0]->type <= XIN_INTER_MODE) && (lftBlock[0]->affine)) + (((lftBlock[1]->type <= XIN_INTER_MODE) && (lftBlock[1]->affine)) << 1);
    topAvailField   = ((topBlock[0]->type <= XIN_INTER_MODE) && (topBlock[0]->affine)) + (((topBlock[1]->type <= XIN_INTER_MODE) && (topBlock[1]->affine)) << 1) + (((topBlock[2]->type <= XIN_INTER_MODE) && (topBlock[2]->affine)) << 2);
    cornerMVPattern = 0;

    // check left neighbor
    Xin266AddDirectAffineMvpCandLeft (
        secSet,
        lftBlock,
        tempMv,
        affineType,
        lftAvailField,
        listIdx,
        refIdx,
        &lftMvpAvail);

    if (lftMvpAvail)
    {
        amvpCand[numMvs][0].s64x1 = tempMv[0].s64x1;
        amvpCand[numMvs][1].s64x1 = tempMv[1].s64x1;
        amvpCand[numMvs][2].s64x1 = tempMv[2].s64x1;

        numMvs++;
    }

    // check above neighbor
    Xin266AddDirectAffineMvpCandLeft (
        secSet,
        topBlock,
        tempMv,
        affineType,
        topAvailField,
        listIdx,
        refIdx,
        &topMvpAvail);

    if (topMvpAvail)
    {
        amvpCand[numMvs][0].s64x1 = tempMv[0].s64x1;
        amvpCand[numMvs][1].s64x1 = tempMv[1].s64x1;
        amvpCand[numMvs][2].s64x1 = tempMv[2].s64x1;

        numMvs++;
    }

    if (numMvs >= XIN_MAX_AMVP_CAND_NUM)
    {
        return;
    }

    topLftBlock[0] = secSet->topLftBlock;
    topLftBlock[1] = secSet->topLBlock;
    topLftBlock[2] = secSet->lftTBlock;

    topLftAvailField = (topLftBlock[0]->type <= XIN_INTER_MODE) + ((topLftBlock[1]->type <= XIN_INTER_MODE) << 1) + ((topLftBlock[2]->type <= XIN_INTER_MODE) << 2);

    Xin266AddDirectMvpCand (
        pictureWrite,
        topLftBlock,
        3,
        &tempMv[0],
        topLftAvailField,
        listIdx,
        refIdx,
        &topLftMvpAvail);

    if ((numMvs < XIN_MAX_AMVP_CAND_NUM) && (seqSet->config.enableTMvp))
    {
        Xin266GetTMvpForAmvp (
            secSet,
            pu,
            listIdx,
            refIdx,
            tempMv);

        if (tempMv[0].s64x1 != 0x8000000080000000)
        {
            amvpCand[numMvs][0].s64x1 = tempMv[0].s64x1;
            amvpCand[numMvs][1].s64x1 = tempMv[0].s64x1;
            amvpCand[numMvs][2].s64x1 = tempMv[0].s64x1;

            numMvs++;
        }

    }

    *validMvs = numMvs;

}


