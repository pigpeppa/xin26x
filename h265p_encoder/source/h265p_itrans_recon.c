/***************************************************************************//**
 *
 * @file          h265p_itrans_recon.c
 * @brief         Reconstruct a MB for intra and inter reference.
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
#include "xin26x_logger.h"
#include "h26x_block_utility.h"
#include "h265p_idct_add.h"

void Xin265pReconMb (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    xin_tu_struct   *tuY;
    xin_tu_struct   *tuU;
    xin_tu_struct   *tuV;
    xin_full_md_buf *fullBuf;
    xin_fast_md_buf *fastBuf;
    xin_func_struct *funcSet;
    PIXEL           *reconSb[PLANE_NUM];
    UINT32          tuIdx;
    UINT32          tuNum;
    UINT32          mbOffX;
    UINT32          mbOffY;
    UINT32          offsetX;
    UINT32          offsetY;
    intptr_t        coefStride;
    intptr_t        predStride;
    intptr_t        reconStride;

    fullBuf = mb->bestBuf;
    fastBuf = fullBuf->fastBuf;
    funcSet = secSet->funcSet;

    reconSb[PLANE_LUMA]     = secSet->reconSb[PLANE_LUMA];
    reconSb[PLANE_CHROMA_U] = secSet->reconSb[PLANE_CHROMA_U];
    reconSb[PLANE_CHROMA_V] = secSet->reconSb[PLANE_CHROMA_V];

    tuY    = mb->tu[PLANE_LUMA];
    tuNum  = mb->tuNum;
    mbOffX = mb->offX[PLANE_LUMA];
    mbOffY = mb->offY[PLANE_LUMA];

    coefStride  = fullBuf->coefStride[PLANE_LUMA];
    reconStride = secSet->reconStride[PLANE_LUMA];
    predStride  = fastBuf->predStride[PLANE_LUMA];

    for (tuIdx = 0; tuIdx < tuNum; tuIdx++, tuY++)
    {
        offsetX = mbOffX + tuY->offsetX;
        offsetY = mbOffY + tuY->offsetY;

        if (tuY->eob)
        {
            Xin265pIDctAdd (
                secSet,
                fullBuf->rCoefBuf[PLANE_LUMA] + offsetX + offsetY*coefStride,
                coefStride,
                fastBuf->predBuf[PLANE_LUMA] + offsetX + offsetY*predStride,
                predStride,
                reconSb[PLANE_LUMA] + offsetX + offsetY*reconStride,
                reconStride,
                tuY->txType,
                tuY->txSize);
        }
        else
        {
            funcSet->pfXinBlockCopy[tuY->lgWidth] (
                fastBuf->predBuf[PLANE_LUMA] + offsetX + offsetY*predStride,
                predStride,
                reconSb[PLANE_LUMA] + offsetX + offsetY*reconStride,
                reconStride,
                tuY->width,
                tuY->height);
        }

    }

    tuNum = (mb->blockSize == XIN_BLOCK_128X128) ? mb->tuNum : 1;
    tuU   = mb->tu[PLANE_CHROMA_U];
    tuV   = mb->tu[PLANE_CHROMA_V];

    coefStride  = fullBuf->coefStride[PLANE_CHROMA];
    reconStride = secSet->reconStride[PLANE_CHROMA];
    predStride  = fastBuf->predStride[PLANE_CHROMA];
    mbOffX      = mb->offX[PLANE_CHROMA];
    mbOffY      = mb->offY[PLANE_CHROMA];

    for (tuIdx = 0; tuIdx < tuNum; tuIdx++, tuU++, tuV++)
    {
        offsetX = mbOffX + tuU->offsetX;
        offsetY = mbOffY + tuU->offsetY;

        if (tuU->eob)
        {
            Xin265pIDctAdd (
                secSet,
                fullBuf->rCoefBuf[PLANE_CHROMA_U] + offsetX + offsetY*coefStride,
                coefStride,
                fastBuf->predBuf[PLANE_CHROMA_U] + offsetX + offsetY*predStride,
                predStride,
                reconSb[PLANE_CHROMA_U] + offsetX + offsetY*reconStride,
                reconStride,
                tuU->txType,
                tuU->txSize);
        }
        else
        {
            funcSet->pfXinBlockCopy[tuU->lgWidth] (
                fastBuf->predBuf[PLANE_CHROMA_U] + offsetX + offsetY*predStride,
                predStride,
                reconSb[PLANE_CHROMA_U] + offsetX + offsetY*reconStride,
                reconStride,
                tuU->width,
                tuU->height);
        }

        offsetX = mbOffX + tuV->offsetX;
        offsetY = mbOffY + tuV->offsetY;

        if (tuV->eob)
        {
            Xin265pIDctAdd (
                secSet,
                fullBuf->rCoefBuf[PLANE_CHROMA_V] + offsetX + offsetY*coefStride,
                coefStride,
                fastBuf->predBuf[PLANE_CHROMA_V] + offsetX + offsetY*predStride,
                predStride,
                reconSb[PLANE_CHROMA_V] + offsetX + offsetY*reconStride,
                reconStride,
                tuV->txType,
                tuV->txSize);
        }
        else
        {
            funcSet->pfXinBlockCopy[tuV->lgWidth] (
                fastBuf->predBuf[PLANE_CHROMA_V] + offsetX + offsetY*predStride,
                predStride,
                reconSb[PLANE_CHROMA_V] + offsetX + offsetY*reconStride,
                reconStride,
                tuV->width,
                tuV->height);
        }

    }

}

