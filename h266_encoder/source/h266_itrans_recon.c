/***************************************************************************//**
 *
 * @file          h266_itrans_recon.c
 * @brief         Reconstruct a CU for intra and inter reference.
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
#include "h266_idct_add.h"
#include "h26x_block_utility.h"
#include "h266_func_struct.h"

void Xin266ReconCu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_tu_struct   *tu;
    xin_func_struct *funcSet;
    xin_full_md_buf *fullBuf;
    xin_fast_md_buf *fastBuf;
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    PIXEL           *reconCu[3];
    UINT32          tuIdx;
    UINT32          tuNum;
    UINT32          mtsIdxY;
    UINT32          mtsIdxU;
    UINT32          mtsIdxV;
    UINT32          cuOffX;
    UINT32          cuOffY;
    UINT32          offsetX;
    UINT32          offsetY;
    intptr_t        lumaStride;
    intptr_t        chromaStride;
    intptr_t        reconYStride;
    intptr_t        reconUvStride;
    UINT32          width;
    UINT32          height;
    BOOL            isIntra;

    tuNum   = cu->tuNum;
    cuOffX  = cu->offX;
    cuOffY  = cu->offY;
    fastBuf = cu->bestBuf;
    fullBuf = fastBuf->fullBuf;
    picSet  = secSet->picSet;
    funcSet = secSet->funcSet;
    seqSet  = secSet->seqSet;
    isIntra = fastBuf->type == XIN_INTRA_MODE;

    lumaStride    = fullBuf->coeffStride[PLANE_LUMA];
    chromaStride  = fullBuf->coeffStride[PLANE_CHROMA];
    reconYStride  = secSet->reconYStride;
    reconUvStride = secSet->reconUvStride;
    pictureWrite  = picSet->pictureWrite;

    reconCu[PLANE_LUMA]     = pictureWrite->refBuf[PLANE_LUMA] + cu->cuPelX + cu->cuPelY*secSet->reconYStride;
    reconCu[PLANE_CHROMA_U] = pictureWrite->refBuf[PLANE_CHROMA_U] + ((cu->cuPelX + cu->cuPelY*secSet->reconUvStride) >> 1);
    reconCu[PLANE_CHROMA_V] = pictureWrite->refBuf[PLANE_CHROMA_V] + ((cu->cuPelX + cu->cuPelY*secSet->reconUvStride) >> 1);

    if (cu->treeMask & XIN_CU_TREE_L_MASK)
    {
        if ((seqSet->config.enableCclm) && (cu->type == XIN_INTRA_MODE))
        {
            for (tuIdx = 0; tuIdx < tuNum; tuIdx++)
            {
                tu      = cu->tu[tuIdx];
                offsetX = cuOffX + tu->offsetX;
                offsetY = cuOffY + tu->offsetY;
                mtsIdxY = tu->mtsIdx[PLANE_LUMA];
                width   = 1 << tu->lgWidth[PLANE_LUMA];
                height  = 1 << tu->lgHeight[PLANE_LUMA];

                funcSet->pfXinBlockCopy[tu->lgWidth[0]] (
                    fullBuf->reconBuf[mtsIdxY][PLANE_LUMA] + offsetX + offsetY*lumaStride,
                    lumaStride,
                    reconCu[PLANE_LUMA] + tu->offsetX + tu->offsetY*reconYStride,
                    reconYStride,
                    width,
                    height);

            }

        }
        else
        {
            for (tuIdx = 0; tuIdx < tuNum; tuIdx++)
            {
                tu      = cu->tu[tuIdx];
                offsetX = cuOffX + tu->offsetX;
                offsetY = cuOffY + tu->offsetY;
                mtsIdxY = tu->mtsIdx[PLANE_LUMA];
                width   = 1 << tu->lgWidth[PLANE_LUMA];
                height  = 1 << tu->lgHeight[PLANE_LUMA];

                if (tu->yCbf)
                {
                    Xin266IDctAdd (
                        secSet,
                        fullBuf->rCoefBuf[mtsIdxY][PLANE_LUMA] + offsetX + offsetY*lumaStride,
                        lumaStride,
                        fastBuf->predBuf[PLANE_LUMA] + offsetX + offsetY*fastBuf->lumaStride,
                        fastBuf->lumaStride,
                        reconCu[PLANE_LUMA] + tu->offsetX + tu->offsetY*reconYStride,
                        reconYStride,
                        isIntra,
                        PLANE_LUMA,
                        mtsIdxY,
                        width,
                        height);
                }
                else
                {
                    funcSet->pfXinBlockCopy[tu->lgWidth[0]] (
                        fastBuf->predBuf[PLANE_LUMA] + offsetX + offsetY*fastBuf->lumaStride,
                        fastBuf->lumaStride,
                        reconCu[PLANE_LUMA] + tu->offsetX + tu->offsetY*reconYStride,
                        reconYStride,
                        width,
                        height);
                }

            }

        }

    }

    if (cu->treeMask & XIN_CU_TREE_C_MASK)
    {
        for (tuIdx = 0; tuIdx < tuNum; tuIdx++)
        {
            tu      = cu->tu[tuIdx];
            offsetX = cuOffX + tu->offsetX;
            offsetY = cuOffY + tu->offsetY;
            mtsIdxU = tu->mtsIdx[PLANE_CHROMA_U];
            mtsIdxV = tu->mtsIdx[PLANE_CHROMA_V];
            width   = 1 << tu->lgWidth[PLANE_CHROMA];
            height  = 1 << tu->lgHeight[PLANE_CHROMA];

            if (tu->uCbf)
            {
                Xin266IDctAdd (
                    secSet,
                    fullBuf->rCoefBuf[mtsIdxU][PLANE_CHROMA_U] + (offsetX + offsetY*chromaStride)/2,
                    chromaStride,
                    fastBuf->predBuf[PLANE_CHROMA_U] + (offsetX + offsetY*fastBuf->chromaStride)/2,
                    fastBuf->chromaStride,
                    reconCu[PLANE_CHROMA_U] + (tu->offsetX + tu->offsetY*reconUvStride)/2,
                    reconUvStride,
                    isIntra,
                    PLANE_CHROMA_U,
                    mtsIdxU,
                    width,
                    height);
            }
            else
            {
                funcSet->pfXinBlockCopy[tu->lgWidth[1]] (
                    fastBuf->predBuf[PLANE_CHROMA_U] + (offsetX + offsetY*fastBuf->chromaStride)/2,
                    fastBuf->chromaStride,
                    reconCu[PLANE_CHROMA_U] + (tu->offsetX + tu->offsetY*reconUvStride)/2,
                    reconUvStride,
                    width,
                    height);
            }

            if (tu->vCbf)
            {
                Xin266IDctAdd (
                    secSet,
                    fullBuf->rCoefBuf[mtsIdxV][PLANE_CHROMA_V] + (offsetX + offsetY*chromaStride)/2,
                    chromaStride,
                    fastBuf->predBuf[PLANE_CHROMA_V] + (offsetX + offsetY*fastBuf->chromaStride)/2,
                    fastBuf->chromaStride,
                    reconCu[PLANE_CHROMA_V] + (tu->offsetX + tu->offsetY*reconUvStride)/2,
                    reconUvStride,
                    isIntra,
                    PLANE_CHROMA_V,
                    mtsIdxV,
                    width,
                    height);
            }
            else
            {
                funcSet->pfXinBlockCopy[tu->lgWidth[1]] (
                    fastBuf->predBuf[PLANE_CHROMA_V] + (offsetX + offsetY*fastBuf->chromaStride)/2,
                    fastBuf->chromaStride,
                    reconCu[PLANE_CHROMA_V] + (tu->offsetX + tu->offsetY*reconUvStride)/2,
                    reconUvStride,
                    width,
                    height);
            }

        }

    }

}

void Xin266ReconTu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    UINT32          planeIdx)
{
    xin_tu_struct   *tu;
    xin_func_struct *funcSet;
    xin_full_md_buf *fullBuf;
    UINT32          tuIdx;
    UINT32          cuOffX;
    UINT32          cuOffY;
    UINT32          offsetX;
    UINT32          offsetY;
    intptr_t        fullStride;
    intptr_t        fastStride;
    UINT32          width;
    UINT32          height;
    UINT32          lgWidth;
    UINT32          compType;
    BOOL            isIntra;

    cuOffX     = cu->offX;
    cuOffY     = cu->offY;
    fullBuf    = fastBuf->fullBuf;
    funcSet    = secSet->funcSet;
    compType   = (planeIdx > PLANE_LUMA);
    fullStride = fullBuf->coeffStride[compType];
    fastStride = (planeIdx > PLANE_LUMA) ? fastBuf->chromaStride : fastBuf->lumaStride;
    isIntra    = fastBuf->type == XIN_INTRA_MODE;

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        tu      = cu->tu[tuIdx];
        offsetX = cuOffX + tu->offsetX;
        offsetY = cuOffY + tu->offsetY;
        lgWidth = tu->lgWidth[compType];
        width   = 1 << lgWidth;
        height  = 1 << tu->lgHeight[compType];

        if ((fullBuf->yuvCbf[mtsIdx][planeIdx] >> tuIdx) & 0x01)
        {
            Xin266IDctAdd (
                secSet,
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + ((offsetX + offsetY*fullStride) >> compType),
                fullStride,
                fastBuf->predBuf[planeIdx] + ((offsetX + offsetY*fastStride) >> compType),
                fastStride,
                fullBuf->reconBuf[mtsIdx][planeIdx] + ((offsetX + offsetY*fullStride) >> compType),
                fullStride,
                isIntra,
                planeIdx,
                mtsIdx,
                width,
                height);
        }
        else
        {
            funcSet->pfXinBlockCopy[lgWidth] (
                fastBuf->predBuf[planeIdx] + ((offsetX + offsetY*fastStride) >> compType),
                fastStride,
                fullBuf->reconBuf[mtsIdx][planeIdx] + ((offsetX + offsetY*fullStride) >> compType),
                fullStride,
                width,
                height);
        }

    }

}



