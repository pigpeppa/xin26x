/***************************************************************************//**
 *
 * @file          h266_sao_rdo.c
 * @brief         Get SAO statistics (RDO).
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
#include "memory.h"
#include "h26x_sao_context.h"
#include "basic_macro.h"
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
#include "h266_entropy_manipulate.h"
#include "h266_sao_rdo.h"
#include "h26x_block_utility.h"
#include "h26x_sao.h"
#include "h266_func_struct.h"

#define EST_SAO_DIST(COUNT, OFFSET, DIFF)  ((COUNT)*(OFFSET) - (DIFF)*2)*(OFFSET)

static inline SINT64 CalcSaoDist(
    SINT8  *offset,
    SINT32 *diff,
    UINT16 *count,
    UINT32 saoType)
{
    UINT32 classes;
    UINT32 classIdx;
    SINT64 dist;

    dist    = 0;
    classes = (saoType == XIN_SAO_BO) ? 4 : XIN_NUM_SAO_EO_CLASS;

    for (classIdx = 0; classIdx < classes; classIdx++)
    {
        dist += EST_SAO_DIST(count[classIdx], offset[classIdx], diff[classIdx]);
    }

    return dist;

}

static void estIterOffset (
    UINT32  saoType,
    SINT64  lambda,
    SINT32  count,
    SINT8   *offset,
    SINT32  diff,
    SINT32  maxSaoOffset,
    SINT64  *bestCost)
{
    SINT32  iterOffset;
    UINT32  rate;
    SINT64  cost;
    SINT64  bCost;
    SINT32  bOffset;

    iterOffset = *offset;
    bOffset    = 0;

    // Assuming sending quantized value 0 results in zero offset and sending the value zero needs 1 bit.
    // entropy coder can be used to measure the exact rate here.
    bCost = CALC_SSE_COST(lambda, 1<<XIN_RATE_FRACTION);

    while (iterOffset != 0)
    {
        // Calculate the bits required for signalling the offset
        rate = (saoType == XIN_SAO_BO) ? (XIN_ABS(iterOffset) + 2) : (XIN_ABS(iterOffset) + 1);

        if (XIN_ABS(iterOffset) == maxSaoOffset)
        {
            rate--;
        }

        // Do the dequntization before distorion calculation
        cost = EST_SAO_DIST (count, iterOffset, diff);
        cost = (cost << XIN_COST_FRACTION) + CALC_SSE_COST(lambda, rate<<XIN_RATE_FRACTION);

        if (cost < bCost)
        {
            bCost   = cost;
            bOffset = iterOffset;
        }

        iterOffset = (iterOffset > 0) ? (iterOffset - 1) : (iterOffset + 1);

    }

    *offset   = (SINT8)bOffset;
    *bestCost = bCost;

}


void Xin266LumaSaoRdo (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu,
    UINT32         saoMask,
    SINT64         *bestCost)
{
    SINT32         classIdx;
    UINT16         *boCount;
    SINT32         *boDiff;
    UINT16         *eoCount;
    SINT32         *eoDiff;
    SINT8          eoOffset[XIN_NUM_SAO_EO_CLASS];
    SINT64         eoCost[XIN_NUM_SAO_EO_CLASS];
    SINT8          boOffset[XIN_NUM_SAO_BO_CLASS];
    SINT64         boCost[XIN_NUM_SAO_BO_CLASS];
    UINT32         bandIdx;
    UINT32         bestBand;
    SINT64         tempCost;
    SINT64         minBoCost;
    SINT64         minEoCost;
    SINT64         baseCost;
    SINT64         lambda;
    UINT32         minEoType;
    UINT32         boRate;
    UINT32         eoRate;
    UINT32         saoTypeRate;
    UINT32         eoType;
    SINT32         offset;
    SINT32         maxSaoOffset;
    xin_prob_model *context;
    xin_seq_struct *seqSet;

    seqSet       = secSet->seqSet;
    context      = secSet->cabacSet->context;
    minEoCost    = XIN_MAX_U64_COST;
    minBoCost    = XIN_MAX_U64_COST;
    lambda       = secSet->sseLambda[PLANE_LUMA];
    bestBand     = 0;
    minEoType    = 0;
    maxSaoOffset = (1 << (seqSet->config.internalBitDepth - XIN_NUM_SAO_BO_CLASSES_LOG2)) - 1;

    Xin266EstimateSaoType (
        context,
        -1,
        &saoTypeRate);

    baseCost  = CALC_SSE_COST(lambda, saoTypeRate);

    // Bo
    if (saoMask & (1 << XIN_SAO_BO))
    {
        boCount   = secSet->boCount[PLANE_LUMA];
        boDiff    = secSet->boDiff[PLANE_LUMA];

        for (classIdx = 0; classIdx < XIN_NUM_SAO_BO_CLASS; classIdx++)
        {
            if (boCount[classIdx] == 0)
            {
                boOffset[classIdx] = 0;
                boCost[classIdx]   = CALC_SSE_COST(lambda, 1<<XIN_RATE_FRACTION);

                continue;
            }

            offset = XIN_SIGNED_ROUND_DIV (boDiff[classIdx], boCount[classIdx]);

            boOffset[classIdx] = (SINT8)XIN_CLIP (offset, -maxSaoOffset, maxSaoOffset);

            estIterOffset (
                XIN_SAO_BO,
                lambda,
                boCount[classIdx],
                boOffset + classIdx,
                boDiff[classIdx],
                maxSaoOffset,
                boCost + classIdx);

        }

        for (bandIdx = 0; bandIdx < XIN_NUM_SAO_BO_CLASS - 4 + 1; bandIdx++)
        {
            tempCost  = boCost[bandIdx    ];
            tempCost += boCost[bandIdx + 1];
            tempCost += boCost[bandIdx + 2];
            tempCost += boCost[bandIdx + 3];

            if(tempCost < minBoCost)
            {
                minBoCost = tempCost;
                bestBand  = bandIdx;
            }
        }

        minBoCost = CalcSaoDist (boOffset + bestBand, boDiff + bestBand, boCount + bestBand, XIN_SAO_BO);

        Xin266EstiamteSaoBoOffsets (
            boOffset + bestBand,
            &boRate);

        Xin266EstimateSaoType (
            context,
            XIN_SAO_BO,
            &saoTypeRate);

        minBoCost <<= XIN_COST_FRACTION;
        minBoCost  += CALC_SSE_COST(lambda, boRate + saoTypeRate);

    }

    // Eo
    for (eoType = 0; eoType < XIN_NUM_SAO_EO; eoType++)
    {
        if (saoMask & (1 << eoType))
        {
            eoCount = secSet->eoCount[PLANE_LUMA][eoType];
            eoDiff  = secSet->eoDiff[PLANE_LUMA][eoType];

            for (classIdx = 0; classIdx < XIN_NUM_SAO_EO_CLASS; classIdx++)
            {
                if(eoCount[classIdx] == 0)
                {
                    eoOffset[classIdx] = 0;
                    continue;
                }

                offset = XIN_SIGNED_ROUND_DIV (eoDiff[classIdx], eoCount[classIdx]);

                eoOffset[classIdx] = (SINT8)XIN_CLIP (offset, -maxSaoOffset, maxSaoOffset);

                if(classIdx == XIN_SAO_EO_CLASS_FV && eoOffset[classIdx] < 0)
                {
                    eoOffset[classIdx] = 0;
                }

                if(classIdx == XIN_SAO_EO_CLASS_HV && eoOffset[classIdx] < 0)
                {
                    eoOffset[classIdx] = 0;
                }

                if(classIdx == XIN_SAO_EO_CLASS_HP && eoOffset[classIdx] > 0)
                {
                    eoOffset[classIdx] = 0;
                }

                if(classIdx == XIN_SAO_EO_CLASS_FP && eoOffset[classIdx] > 0)
                {
                    eoOffset[classIdx] = 0;
                }

                if (eoOffset[classIdx])
                {
                    estIterOffset (
                        eoType,
                        lambda,
                        eoCount[classIdx],
                        eoOffset + classIdx,
                        eoDiff[classIdx],
                        maxSaoOffset,
                        eoCost + classIdx);
                }

            }

            tempCost = CalcSaoDist (eoOffset, eoDiff, eoCount, eoType);

            Xin266EstiamteSaoEoOffsets (
                eoOffset,
                &eoRate);

            Xin266EstimateSaoType (
                context,
                eoType,
                &saoTypeRate);

            tempCost <<= XIN_COST_FRACTION;
            tempCost  += CALC_SSE_COST(lambda, eoRate + saoTypeRate);
            tempCost  += CALC_SSE_COST(lambda, 2<<15);

            if (tempCost < minEoCost)
            {
                minEoCost = tempCost;
                minEoType = eoType;
            }

        }

    }

    if ((minBoCost < baseCost) || (minEoCost < baseCost))
    {
        if (minEoCost < minBoCost)
        {
            *bestCost = minEoCost;

            ctu->saoType[PLANE_LUMA]      = minEoType;
            ctu->saoOffset[PLANE_LUMA][0] = eoOffset[0];
            ctu->saoOffset[PLANE_LUMA][1] = eoOffset[1];
            ctu->saoOffset[PLANE_LUMA][2] = eoOffset[2];
            ctu->saoOffset[PLANE_LUMA][3] = eoOffset[3];
            ctu->saoOffset[PLANE_LUMA][4] = eoOffset[4];
        }
        else
        {
            *bestCost = minBoCost;

            ctu->saoType[PLANE_LUMA]      = XIN_SAO_BO;
            ctu->saoOffset[PLANE_LUMA][0] = boOffset[bestBand    ];
            ctu->saoOffset[PLANE_LUMA][1] = boOffset[bestBand + 1];
            ctu->saoOffset[PLANE_LUMA][2] = boOffset[bestBand + 2];
            ctu->saoOffset[PLANE_LUMA][3] = boOffset[bestBand + 3];
            ctu->saoOffset[PLANE_LUMA][4] = 0;
            ctu->saoBandPos[PLANE_LUMA]   = bestBand;
        }

    }
    else
    {
        *bestCost = baseCost;
        ctu->saoType[PLANE_LUMA] = -1;
    }
}

void Xin266ChromaSaoRdo (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu,
    UINT32         saoMask,
    SINT64         *bestCost)
{
    SINT32         classIdx;
    UINT16         *boCount;
    SINT32         *boDiff;
    SINT8          boOffset[2][XIN_NUM_SAO_BO_CLASS];
    SINT64         boCost[XIN_NUM_SAO_BO_CLASS];
    UINT32         bandIdx;
    UINT32         bestBand;
    UINT32         boBand[2];
    UINT16         *eoCount;
    SINT32         *eoDiff;
    SINT8          eoOffset[2][XIN_NUM_SAO_EO_CLASS];
    SINT64         eoCost[XIN_NUM_SAO_EO_CLASS];
    SINT64         tempCost;
    SINT64         tempBestCost;
    SINT64         minEoCost;
    SINT64         minBoCost;
    SINT64         baseCost;
    SINT64         lambda;
    UINT32         minEoType;
    UINT32         eoRate[2];
    UINT32         boRate[2];
    UINT32         saoTypeRate;
    UINT32         eoType;
    UINT32         planeIdx;
    SINT32         offset;
    SINT32         maxSaoOffset;
    xin_prob_model *context;
    xin_seq_struct *seqSet;

    seqSet       = secSet->seqSet;
    context      = secSet->cabacSet->context;
    minEoCost    = XIN_MAX_U64_COST;
    minBoCost    = XIN_MAX_U64_COST;
    lambda       = secSet->sseLambda[PLANE_CHROMA];
    minEoType    = 0;
    maxSaoOffset = (1 << (seqSet->config.internalBitDepth - XIN_NUM_SAO_BO_CLASSES_LOG2)) - 1;

    Xin266EstimateSaoType (
        context,
        -1,
        &saoTypeRate);

    baseCost = CALC_SSE_COST(lambda, saoTypeRate);

    // Bo
    if (saoMask & (1 << XIN_SAO_BO))
    {
        minBoCost = 0;

        for (planeIdx = 0; planeIdx < 2; planeIdx++)
        {
            boCount      = secSet->boCount[planeIdx+1];
            boDiff       = secSet->boDiff[planeIdx+1];
            tempBestCost = XIN_MAX_U64_COST;
            bestBand     = 0;

            for (classIdx = 0; classIdx < XIN_NUM_SAO_BO_CLASS; classIdx++)
            {
                if (boCount[classIdx] == 0)
                {
                    boOffset[planeIdx][classIdx] = 0;

                    boCost[classIdx] = CALC_SSE_COST(lambda, 1<<XIN_RATE_FRACTION);

                    continue;
                }

                offset = XIN_SIGNED_ROUND_DIV (boDiff[classIdx], boCount[classIdx]);

                boOffset[planeIdx][classIdx] = (SINT8)XIN_CLIP (offset, -maxSaoOffset, maxSaoOffset);

                estIterOffset (
                    XIN_SAO_BO,
                    lambda,
                    boCount[classIdx],
                    &(boOffset[planeIdx][classIdx]),
                    boDiff[classIdx],
                    maxSaoOffset,
                    boCost + classIdx);

            }

            for (bandIdx = 0; bandIdx < XIN_NUM_SAO_BO_CLASS - 4 + 1; bandIdx++)
            {
                tempCost  = boCost[bandIdx + 0];
                tempCost += boCost[bandIdx + 1];
                tempCost += boCost[bandIdx + 2];
                tempCost += boCost[bandIdx + 3];

                if(tempCost < tempBestCost)
                {
                    tempBestCost = tempCost;
                    bestBand     = bandIdx;
                }
            }

            boBand[planeIdx] = bestBand;
            minBoCost       += CalcSaoDist (&(boOffset[planeIdx][bestBand]), boDiff+bestBand, boCount+bestBand, XIN_SAO_BO);

            Xin266EstiamteSaoBoOffsets (
                &(boOffset[planeIdx][bestBand]),
                boRate + planeIdx);

        }

        Xin266EstimateSaoType (
            context,
            XIN_SAO_BO,
            &saoTypeRate);

        minBoCost <<= XIN_COST_FRACTION;
        minBoCost  += CALC_SSE_COST(lambda, boRate[0] + boRate[1] + saoTypeRate);

    }

    // Eo
    for (eoType = 0; eoType < XIN_NUM_SAO_EO; eoType++)
    {
        if (saoMask & (1 << eoType))
        {
            tempCost = 0;

            for (planeIdx = 0; planeIdx < 2; planeIdx++)
            {
                eoCount = secSet->eoCount[planeIdx+1][eoType];
                eoDiff  = secSet->eoDiff[planeIdx+1][eoType];

                for (classIdx = 0; classIdx < XIN_NUM_SAO_EO_CLASS; classIdx++)
                {
                    if(eoCount[classIdx] == 0)
                    {
                        eoOffset[planeIdx][classIdx] = 0;
                        continue;
                    }

                    offset = XIN_SIGNED_ROUND_DIV (eoDiff[classIdx], eoCount[classIdx]);

                    eoOffset[planeIdx][classIdx] = (SINT8)XIN_CLIP (offset, -maxSaoOffset, maxSaoOffset);

                    if((classIdx == XIN_SAO_EO_CLASS_FV) && (eoOffset[planeIdx][classIdx] < 0))
                    {
                        eoOffset[planeIdx][classIdx] = 0;
                    }

                    if((classIdx == XIN_SAO_EO_CLASS_HV) && (eoOffset[planeIdx][classIdx] < 0))
                    {
                        eoOffset[planeIdx][classIdx] = 0;
                    }

                    if((classIdx == XIN_SAO_EO_CLASS_HP) && (eoOffset[planeIdx][classIdx] > 0))
                    {
                        eoOffset[planeIdx][classIdx] = 0;
                    }

                    if((classIdx == XIN_SAO_EO_CLASS_FP) && (eoOffset[planeIdx][classIdx] > 0))
                    {
                        eoOffset[planeIdx][classIdx] = 0;
                    }

                    if (eoOffset[planeIdx][classIdx])
                    {
                        estIterOffset (
                            eoType,
                            lambda,
                            eoCount[classIdx],
                            eoOffset[planeIdx] + classIdx,
                            eoDiff[classIdx],
                            maxSaoOffset,
                            eoCost + classIdx);
                    }

                }

                tempCost += CalcSaoDist(eoOffset[planeIdx], eoDiff, eoCount, eoType);

                Xin266EstiamteSaoEoOffsets (
                    eoOffset[planeIdx],
                    eoRate+planeIdx);

            }

            Xin266EstimateSaoType (
                context,
                eoType,
                &saoTypeRate);

            tempCost <<= XIN_COST_FRACTION;
            tempCost  += CALC_SSE_COST(lambda, eoRate[0] + eoRate[1] + saoTypeRate);
            tempCost  += CALC_SSE_COST(lambda, 2<<15);

            if (tempCost < minEoCost)
            {
                minEoCost = tempCost;
                minEoType = eoType;
            }

        }

    }

    if ((minBoCost < baseCost) || (minEoCost < baseCost))
    {
        if (minEoCost < minBoCost)
        {
            *bestCost = minEoCost;

            ctu->saoType[PLANE_CHROMA] = minEoType;

            ctu->saoOffset[PLANE_CHROMA_U][0] = eoOffset[0][0];
            ctu->saoOffset[PLANE_CHROMA_U][1] = eoOffset[0][1];
            ctu->saoOffset[PLANE_CHROMA_U][2] = eoOffset[0][2];
            ctu->saoOffset[PLANE_CHROMA_U][3] = eoOffset[0][3];
            ctu->saoOffset[PLANE_CHROMA_U][4] = eoOffset[0][4];

            ctu->saoOffset[PLANE_CHROMA_V][0] = eoOffset[1][0];
            ctu->saoOffset[PLANE_CHROMA_V][1] = eoOffset[1][1];
            ctu->saoOffset[PLANE_CHROMA_V][2] = eoOffset[1][2];
            ctu->saoOffset[PLANE_CHROMA_V][3] = eoOffset[1][3];
            ctu->saoOffset[PLANE_CHROMA_V][4] = eoOffset[1][4];
        }
        else
        {
            *bestCost = minBoCost;

            ctu->saoType[PLANE_CHROMA] = XIN_SAO_BO;

            bestBand = boBand[0];
            ctu->saoOffset[PLANE_CHROMA_U][0] = boOffset[0][bestBand + 0];
            ctu->saoOffset[PLANE_CHROMA_U][1] = boOffset[0][bestBand + 1];
            ctu->saoOffset[PLANE_CHROMA_U][2] = boOffset[0][bestBand + 2];
            ctu->saoOffset[PLANE_CHROMA_U][3] = boOffset[0][bestBand + 3];
            ctu->saoOffset[PLANE_CHROMA_U][4] = 0;
            ctu->saoBandPos[PLANE_CHROMA_U]   = bestBand;

            bestBand = boBand[1];
            ctu->saoOffset[PLANE_CHROMA_V][0] = boOffset[1][bestBand + 0];
            ctu->saoOffset[PLANE_CHROMA_V][1] = boOffset[1][bestBand + 1];
            ctu->saoOffset[PLANE_CHROMA_V][2] = boOffset[1][bestBand + 2];
            ctu->saoOffset[PLANE_CHROMA_V][3] = boOffset[1][bestBand + 3];
            ctu->saoOffset[PLANE_CHROMA_V][4] = 0;
            ctu->saoBandPos[PLANE_CHROMA_V]   = bestBand;
        }

    }
    else
    {
        *bestCost = baseCost;

        ctu->saoType[PLANE_CHROMA] = -1;
    }

}

void Xin266MergeSaoRdo (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu,
    SINT64         bestCost)
{
    xin_ctu_struct *refCtu;
    SINT8          *offest;
    SINT32         *diff;
    UINT16         *count;
    SINT32         saoType;
    xin_prob_model *context;
    SINT64         lumaDist;
    SINT64         chromaDist;
    xin_ctu_struct *mergeList[2];
    UINT32         mergeIdx;
    SINT64         newCost;
    SINT64         mergeCost[2];
    UINT32         mergeOffRate;
    UINT32         mergeOnRate;
    UINT64         lambda;
    UINT32         bandPos;

    mergeList[0] = secSet->lftCtu;
    mergeList[1] = secSet->topCtu;
    mergeCost[0] = XIN_MAX_U64_COST;
    mergeCost[1] = XIN_MAX_U64_COST;
    lambda       = secSet->sseLambda[PLANE_LUMA];
    context      = secSet->cabacSet->context;

    Xin266EstimateSaoMerge (
        context,
        FALSE,
        &mergeOffRate);

    Xin266EstimateSaoMerge (
        context,
        TRUE,
        &mergeOnRate);

    mergeOffRate = mergeOffRate*((secSet->lftCtu != NULL) + (secSet->topCtu != NULL));
    newCost      = CALC_SSE_COST(lambda, mergeOffRate) + bestCost;

    for (mergeIdx = 0; mergeIdx < 2; mergeIdx++)
    {
        refCtu = mergeList[mergeIdx];

        lumaDist   = 0;
        chromaDist = 0;

        if (refCtu)
        {
            saoType = refCtu->saoType[PLANE_LUMA];

            if (saoType >= 0)
            {
                offest  = refCtu->saoOffset[PLANE_LUMA];

                if (saoType == XIN_SAO_BO)
                {
                    bandPos  = refCtu->saoBandPos[PLANE_LUMA];
                    diff     = secSet->boDiff[PLANE_LUMA];
                    count    = secSet->boCount[PLANE_LUMA];
                    lumaDist = CalcSaoDist(offest, diff + bandPos, count + bandPos, saoType);
                }
                else
                {
                    diff     = secSet->eoDiff[PLANE_LUMA][saoType];
                    count    = secSet->eoCount[PLANE_LUMA][saoType];
                    lumaDist = CalcSaoDist(offest, diff, count, saoType);
                }
            }

            saoType = refCtu->saoType[PLANE_CHROMA];

            if (saoType >= 0)
            {
                if (saoType == XIN_SAO_BO)
                {
                    offest      = refCtu->saoOffset[PLANE_CHROMA_U];
                    bandPos     = refCtu->saoBandPos[PLANE_CHROMA_U];
                    diff        = secSet->boDiff[PLANE_CHROMA_U];
                    count       = secSet->boCount[PLANE_CHROMA_U];
                    chromaDist  = CalcSaoDist(offest, diff + bandPos, count + bandPos, saoType);

                    offest      = refCtu->saoOffset[PLANE_CHROMA_V];
                    bandPos     = refCtu->saoBandPos[PLANE_CHROMA_V];
                    diff        = secSet->boDiff[PLANE_CHROMA_V];
                    count       = secSet->boCount[PLANE_CHROMA_V];
                    chromaDist += CalcSaoDist(offest, diff + bandPos, count + bandPos, saoType);
                }
                else
                {
                    offest      = refCtu->saoOffset[PLANE_CHROMA_U];
                    diff        = secSet->eoDiff[PLANE_CHROMA_U][saoType];
                    count       = secSet->eoCount[PLANE_CHROMA_U][saoType];
                    chromaDist  = CalcSaoDist(offest, diff, count, saoType);

                    offest      = refCtu->saoOffset[PLANE_CHROMA_V];
                    diff        = secSet->eoDiff[PLANE_CHROMA_V][saoType];
                    count       = secSet->eoCount[PLANE_CHROMA_V][saoType];
                    chromaDist += CalcSaoDist(offest, diff, count, saoType);
                }

            }

            mergeCost[mergeIdx]  = (lumaDist + chromaDist) << XIN_COST_FRACTION;
            mergeCost[mergeIdx] += CALC_SSE_COST(lambda, mergeOnRate);

        }

    }

    if ((mergeCost[0] < newCost) || (mergeCost[1] < newCost))
    {
        if (mergeCost[0] <= mergeCost[1])
        {
            refCtu = secSet->lftCtu;

            ctu->saoMergeLftFlag = TRUE;
            ctu->saoMergeTopFlag = FALSE;
        }
        else
        {
            refCtu = secSet->topCtu;

            ctu->saoMergeLftFlag = FALSE;
            ctu->saoMergeTopFlag = TRUE;
        }

        ctu->saoType[PLANE_LUMA]   = refCtu->saoType[PLANE_LUMA];
        ctu->saoType[PLANE_CHROMA] = refCtu->saoType[PLANE_CHROMA];

        ctu->saoBandPos[PLANE_LUMA]     = refCtu->saoBandPos[PLANE_LUMA];
        ctu->saoBandPos[PLANE_CHROMA_U] = refCtu->saoBandPos[PLANE_CHROMA_U];
        ctu->saoBandPos[PLANE_CHROMA_V] = refCtu->saoBandPos[PLANE_CHROMA_V];

        memcpy (ctu->saoOffset[PLANE_LUMA], refCtu->saoOffset[PLANE_LUMA], sizeof(SINT8)*PLANE_NUM*XIN_NUM_SAO_EO_CLASS);

    }
    else
    {
        ctu->saoMergeLftFlag = FALSE;
        ctu->saoMergeTopFlag = FALSE;
    }

}

void Xin266SaoRdoCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_ctu_struct  *lftCtu;
    xin_ctu_struct  *topCtu;
    UINT32          eoType;
    SINT64          lumaCost;
    SINT64          chromaCost;
    SINT32          lftSaoType;
    SINT32          topSaoType;
    BOOL            lumaSaoMask;
    BOOL            chromaSaoMask;
    SINT16          *diffY;
    SINT16          *diffU;
    SINT16          *diffV;

    seqSet  = secSet->seqSet;
    funcSet = secSet->funcSet;
    lftCtu  = secSet->lftCtu;
    topCtu  = secSet->topCtu;
    diffY   = (SINT16 *)secSet->tempBuffer;
    diffU   = (SINT16 *)secSet->tempBuffer;
    diffV   = (SINT16 *)secSet->tempBuffer + XIN_MAX_CTU_SIZE*XIN_MAX_CTU_SIZE;

    if (!seqSet->config.enableSao)
    {
        return;
    }

    if ((!(ctu->ctuAddr % XIN_SAO_GT_PERIOD)) || (!seqSet->config.fastSao))
    {
        lumaSaoMask   = XIN_SAO_LUMA_FULL_MASK;
        chromaSaoMask = XIN_SAO_CHROMA_FULL_MASK;
    }
    else
    {
        lftSaoType = (lftCtu == NULL) ? -1 : lftCtu->saoType[PLANE_LUMA];
        topSaoType = (topCtu == NULL) ? -1 : topCtu->saoType[PLANE_LUMA];

        if (((lftSaoType < 0) || (topSaoType < 0)) && (ctu->vaildCuCount != 1))
        {
            lumaSaoMask  = XIN_SAO_LUMA_EO_MASK;
            lumaSaoMask |= (lftSaoType >= 0) ? 1 << lftSaoType : 0;
            lumaSaoMask |= (topSaoType >= 0) ? 1 << topSaoType : 0;
        }
        else
        {
            lumaSaoMask  = (lftSaoType >= 0) ? 1 << lftSaoType : 0;
            lumaSaoMask |= (topSaoType >= 0) ? 1 << topSaoType : 0;
        }

        lftSaoType = (lftCtu == NULL) ? -1 : lftCtu->saoType[PLANE_CHROMA];
        topSaoType = (topCtu == NULL) ? -1 : topCtu->saoType[PLANE_CHROMA];

        if (((lftSaoType < 0) || (topSaoType < 0)) && (ctu->vaildCuCount != 1))
        {
            chromaSaoMask  = XIN_SAO_CHROMA_EO_MASK;
            chromaSaoMask |= (lftSaoType >= 0) ? 1 << lftSaoType : 0;
            chromaSaoMask |= (topSaoType >= 0) ? 1 << topSaoType : 0;
        }
        else
        {
            chromaSaoMask  = (lftSaoType >= 0) ? 1 << lftSaoType : 0;
            chromaSaoMask |= (topSaoType >= 0) ? 1 << topSaoType : 0;
        }

    }


    if (lumaSaoMask & XIN_SAO_LUMA_FULL_MASK)
    {
        funcSet->pfXinBlockSub[ctu->lgWidth] (
            secSet->inputCtu[PLANE_LUMA],
            secSet->inputYStride,
            secSet->reconCtu[PLANE_LUMA],
            secSet->reconYStride,
            diffY,
            XIN_MAX_CTU_SIZE,
            ctu->width,
            ctu->height);
    }

    if (lumaSaoMask & (1 << XIN_SAO_BO))
    {
        Xin26xSaoStatBo (
            diffY,
            XIN_MAX_CTU_SIZE,
            secSet->reconCtu[PLANE_LUMA],
            secSet->reconYStride,
            ctu->width,
            ctu->height,
            seqSet->config.internalBitDepth,
            secSet->boDiff[PLANE_LUMA],
            secSet->boCount[PLANE_LUMA]);
    }

    for (eoType = 0; eoType < XIN_NUM_SAO_EO; eoType++)
    {
        if (lumaSaoMask & (1 << eoType))
        {
            funcSet->pfXinSaoStatEo[ctu->lgWidth] (
                diffY,
                XIN_MAX_CTU_SIZE,
                secSet->reconCtu[PLANE_LUMA],
                secSet->reconYStride,
                eoType,
                ctu->width,
                ctu->height,
                secSet->eoDiff[PLANE_LUMA][eoType],
                secSet->eoCount[PLANE_LUMA][eoType]);
        }

    }

    Xin266LumaSaoRdo (
        secSet,
        ctu,
        lumaSaoMask,
        &lumaCost);

    if (chromaSaoMask & XIN_SAO_CHROMA_FULL_MASK)
    {
        funcSet->pfXinBlockSub[ctu->lgWidth-1](
            secSet->inputCtu[PLANE_CHROMA_U],
            secSet->inputUvStride,
            secSet->reconCtu[PLANE_CHROMA_U],
            secSet->reconUvStride,
            diffU,
            XIN_MAX_CTU_SIZE,
            ctu->width/2,
            ctu->height/2);

        funcSet->pfXinBlockSub[ctu->lgWidth-1](
            secSet->inputCtu[PLANE_CHROMA_V],
            secSet->inputUvStride,
            secSet->reconCtu[PLANE_CHROMA_V],
            secSet->reconUvStride,
            diffV,
            XIN_MAX_CTU_SIZE,
            ctu->width/2,
            ctu->height/2);

    }

    if (chromaSaoMask & (1 << XIN_SAO_BO))
    {
        Xin26xSaoStatBo (
            diffU,
            XIN_MAX_CTU_SIZE,
            secSet->reconCtu[PLANE_CHROMA_U],
            secSet->reconUvStride,
            ctu->width/2,
            ctu->height/2,
            seqSet->config.internalBitDepth,
            secSet->boDiff[PLANE_CHROMA_U],
            secSet->boCount[PLANE_CHROMA_U]);

        Xin26xSaoStatBo (
            diffV,
            XIN_MAX_CTU_SIZE,
            secSet->reconCtu[PLANE_CHROMA_V],
            secSet->reconUvStride,
            ctu->width/2,
            ctu->height/2,
            seqSet->config.internalBitDepth,
            secSet->boDiff[PLANE_CHROMA_V],
            secSet->boCount[PLANE_CHROMA_V]);

    }

    for (eoType = 0; eoType < XIN_NUM_SAO_EO; eoType++)
    {
        if (chromaSaoMask & (1 << eoType))
        {
            funcSet->pfXinSaoStatEo[ctu->lgWidth-1] (
                diffU,
                XIN_MAX_CTU_SIZE,
                secSet->reconCtu[PLANE_CHROMA_U],
                secSet->reconUvStride,
                eoType,
                ctu->width>>1,
                ctu->height>>1,
                secSet->eoDiff[PLANE_CHROMA_U][eoType],
                secSet->eoCount[PLANE_CHROMA_U][eoType]);

            funcSet->pfXinSaoStatEo[ctu->lgWidth-1] (
                diffV,
                XIN_MAX_CTU_SIZE,
                secSet->reconCtu[PLANE_CHROMA_V],
                secSet->reconUvStride,
                eoType,
                ctu->width>>1,
                ctu->height>>1,
                secSet->eoDiff[PLANE_CHROMA_V][eoType],
                secSet->eoCount[PLANE_CHROMA_V][eoType]);

        }

    }

    Xin266ChromaSaoRdo (
        secSet,
        ctu,
        chromaSaoMask,
        &chromaCost);

    Xin266MergeSaoRdo (
        secSet,
        ctu,
        lumaCost + chromaCost);

}


