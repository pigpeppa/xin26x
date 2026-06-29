/***************************************************************************//**
 *
 * @file          h266_inter_prediction.c
 * @brief         h.266 motion compensation wrapper.
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
#include "h266_motion_comp.h"
#include "h266_inter_pred_context.h"
#include "basic_macro.h"
#include "h26x_block_utility.h"
#include "memory.h"
#include "h26x_compute_dist.h"
#include "h26x_common_data.h"
#include "h266_get_neighbour_mv.h"
#include "h266_common_data.h"
#include "h266_func_struct.h"

static const xin_mv_s searchMv[25] =
{
    {-2, -2},
    {-1, -2},
    { 0, -2},
    { 1, -2},
    { 2, -2},
    {-2, -1},
    {-1, -1},
    { 0, -1},
    { 1, -1},
    { 2, -1},
    {-2,  0},
    {-1,  0},
    { 0,  0},
    { 1,  0},
    { 2,  0},
    {-2,  1},
    {-1,  1},
    { 0,  1},
    { 1,  1},
    { 2,  1},
    {-2,  2},
    {-1,  2},
    { 0,  2},
    { 1,  2},
    { 2,  2}
};


void Xin266LumaInterpolationS16 (
    const PIXEL     *src,
    intptr_t        srcStride,
    SINT16          *dst,
    intptr_t        dstStride,
    SINT32          frac,
    UINT32          lgWidth,
    UINT32          height,
    BOOL            useAltHpelIf)
{
    UINT32  width;

    width = 1 << lgWidth;

    if (!frac)
    {
        Xin266InterpCopyU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            useAltHpelIf);
    }
    else if (!(frac & XIN_MV_FRAC_MASK))
    {
        Xin266LumaInterpVetU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            useAltHpelIf);
    }
    else if (!(frac & (XIN_MV_FRAC_MASK<<XIN_MV_FRAC_BITS)))
    {
        Xin266LumaInterpHorU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            useAltHpelIf);
    }
    else
    {
        Xin266LumaInterpHorVetU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            useAltHpelIf);
    }

}

void Xin266LumaInterpolation (
    xin_func_struct *funcSet,
    const PIXEL     *src,
    intptr_t        srcStride,
    PIXEL           *dst,
    intptr_t        dstStride,
    SINT32          frac,
    UINT32          lgWidth,
    UINT32          height,
    BOOL            useAltHpelIf)
{
    UINT32  width;
    SINT32  fracX;
    SINT32  fracY;

    fracX = frac & XIN_MV_FRAC_MASK;
    fracY = (frac >> XIN_MV_FRAC_BITS) & XIN_MV_FRAC_MASK;
    width = 1 << lgWidth;

    funcSet->pfXinLumaInterp[!!fracX][!!fracY][lgWidth] (
        src,
        srcStride,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}

void Xin266ChromaInterpolation (
    const PIXEL     *src,
    intptr_t        srcStride,
    PIXEL           *dst,
    intptr_t        dstStride,
    SINT32          frac,
    UINT32          lgWidth,
    UINT32          height)
{
    UINT32 width;

    width = 1 << lgWidth;

    if (!frac)
    {
        Xin266InterpCopy (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }
    else if (!(frac & (XIN_MV_UV_FRAC_MASK<<XIN_MV_UV_FRAC_BITS)))
    {
        Xin266ChromaInterpHor (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }
    else if (!(frac & XIN_MV_UV_FRAC_MASK))
    {
        Xin266ChromaInterpVet (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }
    else
    {
        Xin266ChromaInterpHorVet (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }

}

void Xin266ChromaInterpolationS16 (
    const PIXEL     *src,
    intptr_t        srcStride,
    SINT16          *dst,
    intptr_t        dstStride,
    SINT32          frac,
    UINT32          lgWidth,
    UINT32          height)
{
    UINT32 width;

    width = 1 << lgWidth;

    if (!frac)
    {
        Xin266InterpCopyU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }
    else if (!(frac & (XIN_MV_UV_FRAC_MASK<<XIN_MV_UV_FRAC_BITS)))
    {
        Xin266ChromaInterpHorU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }
    else if (!(frac & XIN_MV_UV_FRAC_MASK))
    {
        Xin266ChromaInterpVetU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }
    else
    {
        Xin266ChromaInterpHorVetU8S16 (
            src,
            srcStride,
            dst,
            dstStride,
            frac,
            width,
            height,
            FALSE);
    }

}

void Xin266LumaInterPred (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *pred,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT32         refIdx,
    SINT32         listIdx,
    BOOL           useAltHpelIf)
{
    xin_pic_struct  *picSet;
    xin_func_struct *funcSet;
    xin_ref_picture *pictureRead;
    PIXEL           *ref;
    xin_cu_struct   *cu;
    intptr_t        refStride;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          frac;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          mvX;
    SINT32          mvY;

    cu          = secSet->cu;
    picSet      = secSet->picSet;
    funcSet     = secSet->funcSet;
    pictureRead = picSet->pictureRead[listIdx][refIdx];
    ref         = pictureRead->refBuf[PLANE_LUMA];
    refStride   = pictureRead->refStride[PLANE_LUMA];
    mvX         = mv->mv.mv32X;
    mvY         = mv->mv.mv32Y;
    mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
    mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
    refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
    refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
    fracX       = (refOffX & XIN_MV_FRAC_MASK);
    fracY       = (refOffY & XIN_MV_FRAC_MASK);
    frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
    refOffX     = refOffX>>XIN_MV_FRAC_BITS;
    refOffY     = refOffY>>XIN_MV_FRAC_BITS;

    funcSet->pfXinLumaInterp[!!fracX][!!fracY][pu->lgWidth] (
        ref + refStride*refOffY + refOffX,
        refStride,
        pred,
        predStride,
        frac,
        pu->width,
        pu->height,
        useAltHpelIf);

}

void Xin266ChromaInterPred (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *predU,
    PIXEL          *predV,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT32         refIdx,
    SINT32         listIdx)
{
    xin_pic_struct  *picSet;
    xin_func_struct *funcSet;
    xin_ref_picture *pictureRead;
    PIXEL           *refU;
    PIXEL           *refV;
    xin_cu_struct   *cu;
    intptr_t        refStride;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          frac;
    SINT32          mvX;
    SINT32          mvY;

    cu          = secSet->cu;
    picSet      = secSet->picSet;
    funcSet     = secSet->funcSet;
    pictureRead = picSet->pictureRead[listIdx][refIdx];
    refU        = pictureRead->refBuf[PLANE_CHROMA_U];
    refV        = pictureRead->refBuf[PLANE_CHROMA_V];
    refStride   = pictureRead->refStride[PLANE_CHROMA];
    mvX         = mv->mv.mv32X;
    mvY         = mv->mv.mv32Y;
    mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
    mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
    refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
    refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
    fracX       = (refOffX & XIN_MV_FRAC_MASK);
    fracY       = (refOffY & XIN_MV_FRAC_MASK);
    frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
    refOffX     = refOffX >> XIN_MV_UV_FRAC_BITS;
    refOffY     = refOffY >> XIN_MV_UV_FRAC_BITS;

    funcSet->pfXinChromaInterp[!!fracX][!!fracY][pu->lgWidth-1] (
        refU + (refStride*refOffY + refOffX),
        refStride,
        predU,
        predStride,
        frac,
        pu->width>>1,
        pu->height>>1,
        FALSE);

    funcSet->pfXinChromaInterp[!!fracX][!!fracY][pu->lgWidth-1] (
        refV + (refStride*refOffY + refOffX),
        refStride,
        predV,
        predStride,
        frac,
        pu->width>>1,
        pu->height>>1,
        FALSE);

}

static UINT32 Xin266GetBcwWeight (
    UINT32 bcwIdx,
    UINT32 listIdx)
{
    UINT32  bcwWeight;

    bcwWeight = listIdx == XIN_LIST_0 ? XIN_BCW_WGT_BASE - bcwWeights[bcwIdx] : bcwWeights[bcwIdx];

    return bcwWeight;
}

void Xin266LumaMotionComp (
    xin_sec_struct *secSet,
    SINT32         width,
    SINT32         height,
    PIXEL          *pred,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    UINT8          bcwIdx,
    UINT32         filterIndex)
{
    xin_pic_struct  *picSet;
    xin_func_struct *funcSet;
    xin_ref_picture *pictureRead;
    PIXEL           *ref0;
    PIXEL           *ref1;
    xin_cu_struct   *cu;
    intptr_t        refStride;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          frac;
    SINT32          mvX;
    SINT32          mvY;
    SINT16          *predBuf0;
    SINT16          *predBuf1;
    SINT32          refIdx0;
    SINT32          refIdx1;
    SINT32          lgWidth;

    cu       = secSet->cu;
    picSet   = secSet->picSet;
    predBuf0 = (SINT16 *)secSet->tempBuffer;
    predBuf1 = predBuf0 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    refIdx0  = refIdx[0];
    refIdx1  = refIdx[1];
    funcSet  = secSet->funcSet;
    lgWidth  = calcLog2[width];

    if ((refIdx0 >= 0) && (refIdx1 >= 0))
    {
        pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
        ref0        = pictureRead->refBuf[PLANE_LUMA];
        refStride   = pictureRead->refStride[0];
        mvX         = mv[0].mv.mv32X;
        mvY         = mv[0].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_FRAC_MASK);
        frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
        refOffX     = refOffX>>XIN_MV_FRAC_BITS;
        refOffY     = refOffY>>XIN_MV_FRAC_BITS;

        funcSet->pfXinLumaInterpS16[!!fracX][!!fracY][lgWidth] (
            ref0 + refStride*refOffY + refOffX,
            refStride,
            predBuf0,
            XIN_MAX_CU_SIZE,
            frac,
            width,
            height,
            filterIndex);

        pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
        ref1        = pictureRead->refBuf[PLANE_LUMA];
        refStride   = pictureRead->refStride[0];
        mvX         = mv[1].mv.mv32X;
        mvY         = mv[1].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_FRAC_MASK);
        frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
        refOffX     = refOffX>>XIN_MV_FRAC_BITS;
        refOffY     = refOffY>>XIN_MV_FRAC_BITS;

        funcSet->pfXinLumaInterpS16[!!fracX][!!fracY][lgWidth] (
            ref1 + refStride*refOffY + refOffX,
            refStride,
            predBuf1,
            XIN_MAX_CU_SIZE,
            frac,
            width,
            height,
            filterIndex);

        if (bcwIdx == XIN_BCW_DEFAULT)
        {
            funcSet->pfXinInterpAvg[lgWidth] (
                predBuf0,
                XIN_MAX_CU_SIZE,
                predBuf1,
                XIN_MAX_CU_SIZE,
                pred,
                predStride,
                width,
                height);
        }
        else
        {
            funcSet->pfXinInterpWeight[lgWidth] (
                predBuf0,
                XIN_MAX_CU_SIZE,
                predBuf1,
                XIN_MAX_CU_SIZE,
                pred,
                predStride,
                width,
                height,
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_0),
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_1));
        }

    }
    else if (refIdx0 >= 0)
    {
        pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
        ref0        = pictureRead->refBuf[PLANE_LUMA];
        refStride   = pictureRead->refStride[0];
        mvX         = mv[0].mv.mv32X;
        mvY         = mv[0].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_FRAC_MASK);
        frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
        refOffX     = refOffX>>XIN_MV_FRAC_BITS;
        refOffY     = refOffY>>XIN_MV_FRAC_BITS;

        funcSet->pfXinLumaInterp[!!fracX][!!fracY][lgWidth] (
            ref0 + refStride*refOffY + refOffX,
            refStride,
            pred,
            predStride,
            frac,
            width,
            height,
            filterIndex);

    }
    else
    {
        pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
        ref1        = pictureRead->refBuf[PLANE_LUMA];
        refStride   = pictureRead->refStride[0];
        mvX         = mv[1].mv.mv32X;
        mvY         = mv[1].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_FRAC_MASK);
        frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
        refOffX     = refOffX>>XIN_MV_FRAC_BITS;
        refOffY     = refOffY>>XIN_MV_FRAC_BITS;

        funcSet->pfXinLumaInterp[!!fracX][!!fracY][lgWidth] (
            ref1 + refStride*refOffY + refOffX,
            refStride,
            pred,
            predStride,
            frac,
            width,
            height,
            filterIndex);

    }

}

void Xin266ChromaMotionComp (
    xin_sec_struct *secSet,
    SINT32         width,
    SINT32         height,
    PIXEL          *predU,
    PIXEL          *predV,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    UINT8          bcwIdx)
{
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureRead;
    xin_func_struct *funcSet;
    PIXEL           *refU0;
    PIXEL           *refU1;
    PIXEL           *refV0;
    PIXEL           *refV1;
    xin_cu_struct   *cu;
    intptr_t        refStride;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          frac;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          mvX;
    SINT32          mvY;
    SINT32          refIdx0;
    SINT32          refIdx1;
    SINT16          *predBufU0;
    SINT16          *predBufU1;
    SINT16          *predBufV0;
    SINT16          *predBufV1;
    SINT32          lgWidth;

    cu         = secSet->cu;
    picSet     = secSet->picSet;
    funcSet    = secSet->funcSet;
    refIdx0    = refIdx[0];
    refIdx1    = refIdx[1];
    predBufU0  = (SINT16 *)secSet->tempBuffer;
    predBufU1  = predBufU0 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    predBufV0  = predBufU1 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    predBufV1  = predBufV0 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    lgWidth    = calcLog2[width];

    if ((refIdx0 >= 0) && (refIdx1 >= 0))
    {
        pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
        refU0       = pictureRead->refBuf[PLANE_CHROMA_U];
        refV0       = pictureRead->refBuf[PLANE_CHROMA_V];
        refStride   = pictureRead->refStride[PLANE_CHROMA];
        mvX         = mv[0].mv.mv32X;
        mvY         = mv[0].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_UV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_UV_FRAC_MASK);
        frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
        refOffX     = refOffX >> XIN_MV_UV_FRAC_BITS;
        refOffY     = refOffY >> XIN_MV_UV_FRAC_BITS;

        funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgWidth - 1] (
            refU0 + (refStride*refOffY + refOffX),
            refStride,
            predBufU0,
            XIN_MAX_CU_SIZE,
            frac,
            width>>1,
            height>>1,
            FALSE);

        funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgWidth - 1] (
            refV0 + (refStride*refOffY + refOffX),
            refStride,
            predBufV0,
            XIN_MAX_CU_SIZE,
            frac,
            width>>1,
            height>>1,
            FALSE);

        pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
        refU1       = pictureRead->refBuf[PLANE_CHROMA_U];
        refV1       = pictureRead->refBuf[PLANE_CHROMA_V];
        refStride   = pictureRead->refStride[PLANE_CHROMA];
        mvX         = mv[1].mv.mv32X;
        mvY         = mv[1].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_UV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_UV_FRAC_MASK);
        frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
        refOffX     = refOffX >> XIN_MV_UV_FRAC_BITS;
        refOffY     = refOffY >> XIN_MV_UV_FRAC_BITS;

        funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgWidth - 1] (
            refU1 + (refStride*refOffY + refOffX),
            refStride,
            predBufU1,
            XIN_MAX_CU_SIZE,
            frac,
            width>>1,
            height>>1,
            FALSE);

        funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgWidth - 1] (
            refV1 + (refStride*refOffY + refOffX),
            refStride,
            predBufV1,
            XIN_MAX_CU_SIZE,
            frac,
            width>>1,
            height>>1,
            FALSE);

        if (bcwIdx == XIN_BCW_DEFAULT)
        {
            funcSet->pfXinInterpAvg[lgWidth-1] (
                predBufU0,
                XIN_MAX_CU_SIZE,
                predBufU1,
                XIN_MAX_CU_SIZE,
                predU,
                predStride,
                width>>1,
                height>>1);

            funcSet->pfXinInterpAvg[lgWidth-1] (
                predBufV0,
                XIN_MAX_CU_SIZE,
                predBufV1,
                XIN_MAX_CU_SIZE,
                predV,
                predStride,
                width>>1,
                height>>1);

        }
        else
        {
            funcSet->pfXinInterpWeight[lgWidth-1] (
                predBufU0,
                XIN_MAX_CU_SIZE,
                predBufU1,
                XIN_MAX_CU_SIZE,
                predU,
                predStride,
                width>>1,
                height>>1,
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_0),
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_1));

            funcSet->pfXinInterpWeight[lgWidth-1] (
                predBufV0,
                XIN_MAX_CU_SIZE,
                predBufV1,
                XIN_MAX_CU_SIZE,
                predV,
                predStride,
                width>>1,
                height>>1,
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_0),
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_1));

        }

    }
    else if (refIdx0 >= 0)
    {
        pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
        refU0       = pictureRead->refBuf[PLANE_CHROMA_U];
        refV0       = pictureRead->refBuf[PLANE_CHROMA_V];
        refStride   = pictureRead->refStride[PLANE_CHROMA];
        mvX         = mv[0].mv.mv32X;
        mvY         = mv[0].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_UV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_UV_FRAC_MASK);
        frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
        refOffX     = refOffX >> XIN_MV_UV_FRAC_BITS;
        refOffY     = refOffY >> XIN_MV_UV_FRAC_BITS;

        funcSet->pfXinChromaInterp[!!fracX][!!fracY][lgWidth-1] (
            refU0 + (refStride*refOffY + refOffX),
            refStride,
            predU,
            predStride,
            frac,
            width>>1,
            height>>1,
            FALSE);

        funcSet->pfXinChromaInterp[!!fracX][!!fracY][lgWidth-1] (
            refV0 + (refStride*refOffY + refOffX),
            refStride,
            predV,
            predStride,
            frac,
            width>>1,
            height>>1,
            FALSE);

    }
    else
    {
        pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
        refU1       = pictureRead->refBuf[PLANE_CHROMA_U];
        refV1       = pictureRead->refBuf[PLANE_CHROMA_V];
        refStride   = pictureRead->refStride[PLANE_CHROMA];
        mvX         = mv[1].mv.mv32X;
        mvY         = mv[1].mv.mv32Y;
        mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
        mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
        refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
        refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
        fracX       = (refOffX & XIN_MV_UV_FRAC_MASK);
        fracY       = (refOffY & XIN_MV_UV_FRAC_MASK);
        frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
        refOffX     = refOffX >> XIN_MV_UV_FRAC_BITS;
        refOffY     = refOffY >> XIN_MV_UV_FRAC_BITS;

        funcSet->pfXinChromaInterp[!!fracX][!!fracY][lgWidth-1] (
            refU1 + (refStride*refOffY + refOffX),
            refStride,
            predU,
            predStride,
            frac,
            width>>1,
            height>>1,
            FALSE);

        funcSet->pfXinChromaInterp[!!fracX][!!fracY][lgWidth-1] (
            refV1 + (refStride*refOffY + refOffX),
            refStride,
            predV,
            predStride,
            frac,
            width>>1,
            height>>1,
            FALSE);

    }

}

static SINT32 Xin266DivForMaxQ7 (
    SINT64 N,
    SINT64 D)
{
    SINT32 sign;
    SINT32 q;

    sign = 0;

    if (N < 0)
    {
        sign = 1;
        N    = -N;
    }

    q = 0;
    D = D << 3;

    if (N >= D)
    {
        N -= D;
        q++;
    }

    q = q << 1;
    D = D >> 1;

    if (N >= D)
    {
        N -= D;
        q++;
    }

    q = q << 1;

    if (N >= (D >> 1))
    {
        q++;
    }

    if (sign)
    {
        return -q;
    }
    else
    {
        return q;
    }

}

static void Xin266SubPelErrorSrfc (
    SINT64     *sadBuf,
    xin_mv32_u *deltaMv)
{
    SINT32  hv;
    SINT32  mvSubPelLvl;
    SINT64  numerator;
    SINT64  denominator;

    mvSubPelLvl = 4;

    for (hv = 0; hv < 2; hv++)
    {
        numerator   = (SINT64)((sadBuf[hv+1] - sadBuf[hv+3]) << mvSubPelLvl);
        denominator = (SINT64)(sadBuf[hv+1] + sadBuf[hv+3] - (sadBuf[0] << 1));

        if (0 != denominator)
        {
            if ((sadBuf[hv+1] != sadBuf[0]) && (sadBuf[hv+3] != sadBuf[0]))
            {
                deltaMv->s32x2[hv] = Xin266DivForMaxQ7 (numerator, denominator);
            }
            else
            {
                deltaMv->s32x2[hv] = (sadBuf[hv+1] == sadBuf[0]) ? -8 : 8;
            }
        }
    }

}

void Xin266SubPelRefineDmvr (
    BOOL     non0Cost,
    xin_mv_u *totalDelMv,
    UINT32   *sadArray)
{
    SINT32     sadStride;
    SINT64     sadBuf[5];
    xin_mv32_u deltaMv32;

    sadStride = 2*DMVR_NUM_ITERATION + 1;

    if (non0Cost && (XIN_ABS(totalDelMv->mv.mvX) != (2 << XIN_MV_FRAC_BITS)) && (XIN_ABS(totalDelMv->mv.mvY) != (2 << XIN_MV_FRAC_BITS)))
    {
        deltaMv32.s64x1 = 0;

        sadBuf[0] = sadArray[0];
        sadBuf[1] = sadArray[-1];
        sadBuf[2] = sadArray[-sadStride];
        sadBuf[3] = sadArray[1];
        sadBuf[4] = sadArray[sadStride];

        Xin266SubPelErrorSrfc (sadBuf, &deltaMv32);

        totalDelMv->mv.mvX += (SINT16)deltaMv32.mv.mv32X;
        totalDelMv->mv.mvY += (SINT16)deltaMv32.mv.mv32Y;
    }

}

void Xin266DeriveMvFromDmvr (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    xin_mv_s       *mvdL0SubPu)
{
    xin_func_struct *funcSet;
    xin_cu_struct   *cu;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureRead;
    PIXEL           *ref;
    SINT16          *predBuf0;
    SINT16          *predBuf1;
    SINT16          *predCentre0;
    SINT16          *predCentre1;
    SINT32          width;
    SINT32          height;
    UINT32          subHeight;
    UINT32          subWidth;
    UINT32          lgSubWidth;
    intptr_t        refStride;
    SINT32          mvX;
    SINT32          mvY;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          frac;
    SINT32          padSize;
    SINT32          refIdx0;
    SINT32          refIdx1;
    SINT32          yIdx;
    SINT32          xIdx;
    UINT32          sadArray[(2*DMVR_NUM_ITERATION + 1)*(2*DMVR_NUM_ITERATION+1)];
    UINT32          *sadBuf;
    SINT32          sadOffset;
    UINT32          minCost;
    UINT32          sadCost;
    BOOL            non0Cost;
    xin_mv_u        totalDelMv;
    xin_mv_u        deltaMv;
    SINT32          iter;
    SINT32          nIdx;
    SINT32          mvIdx;

    funcSet    = secSet->funcSet;
    cu         = secSet->cu;
    picSet     = secSet->picSet;
    width      = pu->width;
    height     = pu->height;
    subHeight  = XIN_MIN (height, DMVR_MAX_SUB_SIZE);
    subWidth   = XIN_MIN (width, DMVR_MAX_SUB_SIZE);
    lgSubWidth = calcLog2[subWidth];
    padSize    = DMVR_NUM_ITERATION<<1;
    refIdx0    = refIdx[0];
    refIdx1    = refIdx[1];
    predBuf0   = (SINT16 *)secSet->tempBuffer;
    predBuf1   = predBuf0 + BILI_BUF_STRIDE*BILI_BUF_STRIDE;
    mvIdx      = 0;

    /*L0 MC for refinement*/
    pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
    ref         = pictureRead->refBuf[PLANE_LUMA];
    refStride   = pictureRead->refStride[0];
    mvX         = mv[0].mv.mv32X;
    mvY         = mv[0].mv.mv32Y;
    mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
    mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
    mvX         = mvX - (DMVR_NUM_ITERATION << XIN_MV_FRAC_BITS);
    mvY         = mvY - (DMVR_NUM_ITERATION << XIN_MV_FRAC_BITS);
    refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
    refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
    fracX       = (refOffX & XIN_MV_FRAC_MASK);
    fracY       = (refOffY & XIN_MV_FRAC_MASK);
    frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
    refOffX     = refOffX>>XIN_MV_FRAC_BITS;
    refOffY     = refOffY>>XIN_MV_FRAC_BITS;

    funcSet->pfXinBiliInterp[!!fracX][!!fracY][pu->lgWidth] (
        ref + refStride*refOffY + refOffX,
        refStride,
        predBuf0,
        BILI_BUF_STRIDE,
        frac,
        width,
        height + padSize);

    /*L1 MC for refinement*/
    pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
    ref         = pictureRead->refBuf[PLANE_LUMA];
    refStride   = pictureRead->refStride[PLANE_LUMA];
    mvX         = mv[1].mv.mv32X;
    mvY         = mv[1].mv.mv32Y;
    mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
    mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
    mvX         = mvX - (DMVR_NUM_ITERATION << XIN_MV_FRAC_BITS);
    mvY         = mvY - (DMVR_NUM_ITERATION << XIN_MV_FRAC_BITS);
    refOffX     = (cu->cuPelX << XIN_MV_FRAC_BITS) + mvX;
    refOffY     = (cu->cuPelY << XIN_MV_FRAC_BITS) + mvY;
    fracX       = (refOffX & XIN_MV_FRAC_MASK);
    fracY       = (refOffY & XIN_MV_FRAC_MASK);
    frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
    refOffX     = refOffX>>XIN_MV_FRAC_BITS;
    refOffY     = refOffY>>XIN_MV_FRAC_BITS;

    funcSet->pfXinBiliInterp[!!fracX][!!fracY][pu->lgWidth] (
        ref + refStride*refOffY + refOffX,
        refStride,
        predBuf1,
        BILI_BUF_STRIDE,
        frac,
        width,
        height + padSize);

    for (yIdx = 0; yIdx < height; yIdx += subHeight)
    {
        for (xIdx = 0; xIdx < width; xIdx += subWidth)
        {
            minCost  = XIN_MAX_U32_COST;
            non0Cost = TRUE;
            sadBuf   = &sadArray[((2 * DMVR_NUM_ITERATION + 1)*(2 * DMVR_NUM_ITERATION + 1)) >> 1];

            totalDelMv.s32x1 = 0;
            deltaMv.s32x1    = 0;

            predCentre0 = predBuf0 + (DMVR_NUM_ITERATION + yIdx)*BILI_BUF_STRIDE + xIdx + DMVR_NUM_ITERATION;
            predCentre1 = predBuf1 + (DMVR_NUM_ITERATION + yIdx)*BILI_BUF_STRIDE + xIdx + DMVR_NUM_ITERATION;

            memset (sadArray, 0xff, sizeof(sadArray));

            for (iter = 0; iter < 1; iter++)
            {
                funcSet->pfXinComputeSadS16[lgSubWidth] (
                    predCentre0,
                    BILI_BUF_STRIDE,
                    predCentre1,
                    BILI_BUF_STRIDE,
                    subWidth,
                    subHeight,
                    1,
                    &minCost);

                minCost = minCost >> 1;
                minCost = minCost - (minCost >> 2);

                if (minCost < (subHeight*subWidth))
                {
                    non0Cost = FALSE;
                    break;
                }

                sadBuf[0] = minCost;

                if (!minCost)
                {
                    non0Cost = FALSE;
                    break;
                }

                // xBIPMVRefine
                deltaMv.s32x1 = 0;

                for (nIdx = 0; nIdx < 25; nIdx++)
                {
                    sadOffset = searchMv[nIdx].mvY * (2*DMVR_NUM_ITERATION+1) + searchMv[nIdx].mvX;

                    if (sadBuf[sadOffset] == XIN_MAX_U32)
                    {
                        funcSet->pfXinComputeSadS16[lgSubWidth] (
                            predCentre0 + BILI_BUF_STRIDE*searchMv[nIdx].mvY + searchMv[nIdx].mvX,
                            BILI_BUF_STRIDE,
                            predCentre1 - BILI_BUF_STRIDE*searchMv[nIdx].mvY - searchMv[nIdx].mvX,
                            BILI_BUF_STRIDE,
                            subWidth,
                            subHeight,
                            1,
                            &sadCost);

                        sadCost           = sadCost >> 1;
                        sadBuf[sadOffset] = sadCost;
                    }

                    if (sadBuf[sadOffset] < minCost)
                    {
                        minCost    = sadBuf[sadOffset];
                        deltaMv.mv = searchMv[nIdx];
                    }

                }

                if (deltaMv.s32x1 == 0)
                {
                    break;
                }

                totalDelMv.mv.mvX += deltaMv.mv.mvX;
                totalDelMv.mv.mvY += deltaMv.mv.mvY;

                sadBuf += (deltaMv.mv.mvY*(2*DMVR_NUM_ITERATION + 1) + deltaMv.mv.mvX);

            }

            totalDelMv.mv.mvX = totalDelMv.mv.mvX << XIN_MV_FRAC_BITS;
            totalDelMv.mv.mvY = totalDelMv.mv.mvY << XIN_MV_FRAC_BITS;

            Xin266SubPelRefineDmvr (
                non0Cost,
                &totalDelMv,
                sadBuf);

            mvdL0SubPu[mvIdx] = totalDelMv.mv;

            mvIdx++;

        }

    }

}

void Xin266LumaMotionCompDmvr (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *pred,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    xin_mv_s       *mvdL0SubPu,
    BOOL           useAltHpelIf)
{
    xin_func_struct *funcSet;
    xin_cu_struct   *cu;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureRead;
    PIXEL           *ref;
    SINT16          *predBuf0;
    SINT16          *predBuf1;
    SINT32          width;
    SINT32          height;
    SINT32          subHeight;
    SINT32          subWidth;
    SINT32          lgSubWidth;
    intptr_t        refStride;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          frac;
    SINT32          refIdx0;
    SINT32          refIdx1;
    SINT32          yIdx;
    SINT32          xIdx;
    xin_mv32_s      mv0;
    xin_mv32_s      mv1;
    SINT32          mvX;
    SINT32          mvY;
    SINT32          delIntMvX;
    SINT32          delIntMvY;
    SINT32          mvIdx;
    PIXEL           pad[DMVR_PAD_BUF_STRIDE*DMVR_PAD_BUF_STRIDE];
    PIXEL           *padBuf;

    funcSet    = secSet->funcSet;
    cu         = secSet->cu;
    picSet     = secSet->picSet;
    width      = pu->width;
    height     = pu->height;
    subHeight  = XIN_MIN (height, DMVR_MAX_SUB_SIZE);
    subWidth   = XIN_MIN (width, DMVR_MAX_SUB_SIZE);
    lgSubWidth = calcLog2[subWidth];
    refIdx0    = refIdx[0];
    refIdx1    = refIdx[1];
    predBuf0   = (SINT16 *)secSet->tempBuffer;
    predBuf1   = predBuf0 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    mvIdx      = 0;
    padBuf     = pad + 8*DMVR_PAD_BUF_STRIDE;

    Xin266DeriveMvFromDmvr (
        secSet,
        pu,
        mv,
        refIdx,
        mvdL0SubPu);

    for (yIdx = 0; yIdx < height; yIdx += subHeight)
    {
        for (xIdx = 0; xIdx < width; xIdx += subWidth)
        {
            mv0.mv32X = mv[0].mv.mv32X + mvdL0SubPu[mvIdx].mvX;
            mv0.mv32Y = mv[0].mv.mv32Y + mvdL0SubPu[mvIdx].mvY;

            mv1.mv32X = mv[1].mv.mv32X - mvdL0SubPu[mvIdx].mvX;
            mv1.mv32Y = mv[1].mv.mv32Y - mvdL0SubPu[mvIdx].mvY;

            delIntMvX = (mv0.mv32X >> XIN_MV_FRAC_BITS) - (mv[0].mv.mv32X >> XIN_MV_FRAC_BITS);
            delIntMvY = (mv0.mv32Y >> XIN_MV_FRAC_BITS) - (mv[0].mv.mv32Y >> XIN_MV_FRAC_BITS);

            if (delIntMvX || delIntMvY)
            {
                pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
                ref         = pictureRead->refBuf[PLANE_LUMA];
                refStride   = pictureRead->refStride[0];
                mvX         = mv[0].mv.mv32X;
                mvY         = mv[0].mv.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                refOffX     = refOffX>>XIN_MV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_FRAC_BITS;

                funcSet->pfXinCopyAndPad (
                    ref + refStride*refOffY + refOffX,
                    refStride,
                    padBuf,
                    DMVR_PAD_BUF_STRIDE,
                    subWidth,
                    subHeight);

                fracX = (mv0.mv32X & XIN_MV_FRAC_MASK);
                fracY = (mv0.mv32Y & XIN_MV_FRAC_MASK);
                frac  = (fracY << XIN_MV_FRAC_BITS) + fracX;

                funcSet->pfXinLumaInterpS16[!!fracX][!!fracY][lgSubWidth] (
                    padBuf + delIntMvY*DMVR_PAD_BUF_STRIDE + delIntMvX,
                    DMVR_PAD_BUF_STRIDE,
                    predBuf0,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth,
                    subHeight,
                    useAltHpelIf);

            }
            else
            {
                pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
                ref         = pictureRead->refBuf[PLANE_LUMA];
                refStride   = pictureRead->refStride[PLANE_LUMA];
                mvX         = mv0.mv32X;
                mvY         = mv0.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                fracX       = (refOffX & XIN_MV_FRAC_MASK);
                fracY       = (refOffY & XIN_MV_FRAC_MASK);
                frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
                refOffX     = refOffX>>XIN_MV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_FRAC_BITS;

                funcSet->pfXinLumaInterpS16[!!fracX][!!fracY][lgSubWidth](
                    ref + refStride*refOffY + refOffX,
                    refStride,
                    predBuf0,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth,
                    subHeight,
                    useAltHpelIf);

            }

            delIntMvX = (mv1.mv32X >> XIN_MV_FRAC_BITS) - (mv[1].mv.mv32X >> XIN_MV_FRAC_BITS);
            delIntMvY = (mv1.mv32Y >> XIN_MV_FRAC_BITS) - (mv[1].mv.mv32Y >> XIN_MV_FRAC_BITS);

            if (delIntMvX || delIntMvY)
            {
                pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
                ref         = pictureRead->refBuf[PLANE_LUMA];
                refStride   = pictureRead->refStride[0];
                mvX         = mv[1].mv.mv32X;
                mvY         = mv[1].mv.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                refOffX     = refOffX>>XIN_MV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_FRAC_BITS;

                funcSet->pfXinCopyAndPad (
                    ref + refStride*refOffY + refOffX,
                    refStride,
                    padBuf,
                    DMVR_PAD_BUF_STRIDE,
                    subWidth,
                    subHeight);

                fracX = (mv1.mv32X & XIN_MV_FRAC_MASK);
                fracY = (mv1.mv32Y & XIN_MV_FRAC_MASK);
                frac  = (fracY << XIN_MV_FRAC_BITS) + fracX;

                funcSet->pfXinLumaInterpS16[!!fracX][!!fracY][lgSubWidth] (
                    padBuf + delIntMvY*DMVR_PAD_BUF_STRIDE + delIntMvX,
                    DMVR_PAD_BUF_STRIDE,
                    predBuf1,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth,
                    subHeight,
                    useAltHpelIf);

            }
            else
            {
                pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
                ref         = pictureRead->refBuf[PLANE_LUMA];
                refStride   = pictureRead->refStride[PLANE_LUMA];
                mvX         = mv1.mv32X;
                mvY         = mv1.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                fracX       = (refOffX & XIN_MV_FRAC_MASK);
                fracY       = (refOffY & XIN_MV_FRAC_MASK);
                frac        = (fracY << XIN_MV_FRAC_BITS) + fracX;
                refOffX     = refOffX>>XIN_MV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_FRAC_BITS;

                funcSet->pfXinLumaInterpS16[!!fracX][!!fracY][lgSubWidth](
                    ref + refStride*refOffY + refOffX,
                    refStride,
                    predBuf1,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth,
                    subHeight,
                    useAltHpelIf);

            }

            funcSet->pfXinInterpAvg[lgSubWidth] (
                predBuf0,
                XIN_MAX_CU_SIZE,
                predBuf1,
                XIN_MAX_CU_SIZE,
                pred + yIdx*predStride + xIdx,
                predStride,
                subWidth,
                subHeight);

            mvIdx++;

        }

    }

}

void Xin266ChromaMotionCompDmvr (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *predU,
    PIXEL          *predV,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    xin_mv_s       *mvdL0SubPu)
{
    xin_func_struct *funcSet;
    xin_cu_struct   *cu;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureRead;
    PIXEL           *refU;
    PIXEL           *refV;
    SINT16          *predUBuf0;
    SINT16          *predUBuf1;
    SINT16          *predVBuf0;
    SINT16          *predVBuf1;
    SINT32          width;
    SINT32          height;
    SINT32          subHeight;
    SINT32          subWidth;
    SINT32          lgSubWidth;
    intptr_t        refStride;
    SINT32          refOffX;
    SINT32          refOffY;
    SINT32          fracX;
    SINT32          fracY;
    SINT32          frac;
    SINT32          refIdx0;
    SINT32          refIdx1;
    SINT32          yIdx;
    SINT32          xIdx;
    xin_mv32_s      mv0;
    xin_mv32_s      mv1;
    SINT32          mvIdx;
    SINT32          delIntMvX;
    SINT32          delIntMvY;
    SINT32          mvX;
    SINT32          mvY;
    PIXEL           padU[DMVR_PAD_BUF_STRIDE*DMVR_PAD_BUF_STRIDE];
    PIXEL           padV[DMVR_PAD_BUF_STRIDE*DMVR_PAD_BUF_STRIDE];
    PIXEL           *padUBuf;
    PIXEL           *padVBuf;

    funcSet    = secSet->funcSet;
    cu         = secSet->cu;
    picSet     = secSet->picSet;
    width      = pu->width;
    height     = pu->height;
    subHeight  = XIN_MIN (height, DMVR_MAX_SUB_SIZE);
    subWidth   = XIN_MIN (width, DMVR_MAX_SUB_SIZE);
    lgSubWidth = calcLog2[subWidth];
    refIdx0    = refIdx[0];
    refIdx1    = refIdx[1];
    predUBuf0  = (SINT16 *)secSet->tempBuffer;
    predUBuf1  = predUBuf0 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    predVBuf0  = predUBuf1 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    predVBuf1  = predVBuf0 + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE;
    padUBuf    = padU + 8*DMVR_PAD_BUF_STRIDE;
    padVBuf    = padV + 8*DMVR_PAD_BUF_STRIDE;
    mvIdx      = 0;

    for (yIdx = 0; yIdx < height; yIdx += subHeight)
    {
        for (xIdx = 0; xIdx < width; xIdx += subWidth)
        {
            mv0.mv32X = mv[0].mv.mv32X + mvdL0SubPu[mvIdx].mvX;
            mv0.mv32Y = mv[0].mv.mv32Y + mvdL0SubPu[mvIdx].mvY;

            mv1.mv32X = mv[1].mv.mv32X - mvdL0SubPu[mvIdx].mvX;
            mv1.mv32Y = mv[1].mv.mv32Y - mvdL0SubPu[mvIdx].mvY;

            delIntMvX = (mv0.mv32X >> XIN_MV_UV_FRAC_BITS) - (mv[0].mv.mv32X >> XIN_MV_UV_FRAC_BITS);
            delIntMvY = (mv0.mv32Y >> XIN_MV_UV_FRAC_BITS) - (mv[0].mv.mv32Y >> XIN_MV_UV_FRAC_BITS);

            if (delIntMvX || delIntMvY)
            {
                pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
                refU        = pictureRead->refBuf[PLANE_CHROMA_U];
                refV        = pictureRead->refBuf[PLANE_CHROMA_V];
                refStride   = pictureRead->refStride[PLANE_CHROMA];
                mvX         = mv[0].mv.mv32X;
                mvY         = mv[0].mv.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                refOffX     = refOffX>>XIN_MV_UV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_UV_FRAC_BITS;

                funcSet->pfXinCopyAndPadUv (
                    refU + refStride*refOffY + refOffX,
                    refV + refStride*refOffY + refOffX,
                    refStride,
                    padUBuf,
                    padVBuf,
                    DMVR_PAD_BUF_STRIDE,
                    subWidth>>1,
                    subHeight>>1);

                fracX = (mv0.mv32X & XIN_MV_UV_FRAC_MASK);
                fracY = (mv0.mv32Y & XIN_MV_UV_FRAC_MASK);
                frac  = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1] (
                    padUBuf + delIntMvY*DMVR_PAD_BUF_STRIDE + delIntMvX,
                    DMVR_PAD_BUF_STRIDE,
                    predUBuf0,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1] (
                    padVBuf + delIntMvY*DMVR_PAD_BUF_STRIDE + delIntMvX,
                    DMVR_PAD_BUF_STRIDE,
                    predVBuf0,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

            }
            else
            {
                pictureRead = picSet->pictureRead[XIN_LIST_0][refIdx0];
                refU        = pictureRead->refBuf[PLANE_CHROMA_U];
                refV        = pictureRead->refBuf[PLANE_CHROMA_V];
                refStride   = pictureRead->refStride[PLANE_CHROMA];
                mvX         = mv0.mv32X;
                mvY         = mv0.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                fracX       = (refOffX & XIN_MV_UV_FRAC_MASK);
                fracY       = (refOffY & XIN_MV_UV_FRAC_MASK);
                frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
                refOffX     = refOffX>>XIN_MV_UV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_UV_FRAC_BITS;

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1](
                    refU + refStride*refOffY + refOffX,
                    refStride,
                    predUBuf0,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1](
                    refV + refStride*refOffY + refOffX,
                    refStride,
                    predVBuf0,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

            }

            delIntMvX = (mv1.mv32X >> XIN_MV_UV_FRAC_BITS) - (mv[1].mv.mv32X >> XIN_MV_UV_FRAC_BITS);
            delIntMvY = (mv1.mv32Y >> XIN_MV_UV_FRAC_BITS) - (mv[1].mv.mv32Y >> XIN_MV_UV_FRAC_BITS);

            if (delIntMvX || delIntMvY)
            {
                pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
                refU        = pictureRead->refBuf[PLANE_CHROMA_U];
                refV        = pictureRead->refBuf[PLANE_CHROMA_V];
                refStride   = pictureRead->refStride[PLANE_CHROMA];
                mvX         = mv[1].mv.mv32X;
                mvY         = mv[1].mv.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                refOffX     = refOffX>>XIN_MV_UV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_UV_FRAC_BITS;

                funcSet->pfXinCopyAndPadUv (
                    refU + refStride*refOffY + refOffX,
                    refV + refStride*refOffY + refOffX,
                    refStride,
                    padUBuf,
                    padVBuf,
                    DMVR_PAD_BUF_STRIDE,
                    subWidth>>1,
                    subHeight>>1);

                fracX = (mv1.mv32X & XIN_MV_UV_FRAC_MASK);
                fracY = (mv1.mv32Y & XIN_MV_UV_FRAC_MASK);
                frac  = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1](
                    padUBuf + delIntMvY*DMVR_PAD_BUF_STRIDE + delIntMvX,
                    DMVR_PAD_BUF_STRIDE,
                    predUBuf1,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1](
                    padVBuf + delIntMvY*DMVR_PAD_BUF_STRIDE + delIntMvX,
                    DMVR_PAD_BUF_STRIDE,
                    predVBuf1,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

            }
            else
            {
                pictureRead = picSet->pictureRead[XIN_LIST_1][refIdx1];
                refU        = pictureRead->refBuf[PLANE_CHROMA_U];
                refV        = pictureRead->refBuf[PLANE_CHROMA_V];
                refStride   = pictureRead->refStride[PLANE_CHROMA];
                mvX         = mv1.mv32X;
                mvY         = mv1.mv32Y;
                mvX         = XIN_CLIP (mvX, secSet->minMv.mv.mvX<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvX<<XIN_MV_FRAC_BITS);
                mvY         = XIN_CLIP (mvY, secSet->minMv.mv.mvY<<XIN_MV_FRAC_BITS, secSet->maxMv.mv.mvY<<XIN_MV_FRAC_BITS);
                refOffX     = ((cu->cuPelX + xIdx) << XIN_MV_FRAC_BITS) + mvX;
                refOffY     = ((cu->cuPelY + yIdx) << XIN_MV_FRAC_BITS) + mvY;
                fracX       = (refOffX & XIN_MV_UV_FRAC_MASK);
                fracY       = (refOffY & XIN_MV_UV_FRAC_MASK);
                frac        = (fracY << XIN_MV_UV_FRAC_BITS) + fracX;
                refOffX     = refOffX>>XIN_MV_UV_FRAC_BITS;
                refOffY     = refOffY>>XIN_MV_UV_FRAC_BITS;

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1](
                    refU + refStride*refOffY + refOffX,
                    refStride,
                    predUBuf1,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

                funcSet->pfXinChromaInterpS16[!!fracX][!!fracY][lgSubWidth-1](
                    refV + refStride*refOffY + refOffX,
                    refStride,
                    predVBuf1,
                    XIN_MAX_CU_SIZE,
                    frac,
                    subWidth>>1,
                    subHeight>>1,
                    FALSE);

            }

            funcSet->pfXinInterpAvg[lgSubWidth-1](
                predUBuf0,
                XIN_MAX_CU_SIZE,
                predUBuf1,
                XIN_MAX_CU_SIZE,
                predU + ((yIdx*predStride + xIdx)>>1),
                predStride,
                subWidth>>1,
                subHeight>>1);

            funcSet->pfXinInterpAvg[lgSubWidth-1](
                predVBuf0,
                XIN_MAX_CU_SIZE,
                predVBuf1,
                XIN_MAX_CU_SIZE,
                predV + ((yIdx*predStride + xIdx)>>1),
                predStride,
                subWidth>>1,
                subHeight>>1);

            mvIdx++;

        }

    }

}

void Xin266LumaMotionCompSubBlock (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *pred,
    intptr_t         predStride,
    xin_mv32_u       subBlockMv[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    SINT8            refIdx[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    UINT8            bcwIdx)
{

    SINT32      subHeight;
    SINT32      subWidth;
    SINT32      height;
    SINT32      width;
    SINT32      xIdx, yIdx;
    xin_mv32_u  subMv[XIN_LIST_NUM];
    SINT32      blockIdx;

    height     = pu->height;
    width      = pu->width;
    subHeight  = XIN_MIN (height, XIN_ATMVP_SUB_BLOCK_SIZE);
    subWidth   = XIN_MIN (width, XIN_ATMVP_SUB_BLOCK_SIZE);
    blockIdx   = 0;

    for (yIdx = 0; yIdx < height; yIdx += subHeight)
    {
        for (xIdx = 0; xIdx < width;)
        {
            // Check if we can combine two blocks to one block
            if ((xIdx + subWidth*2 <= width) &&
                    (subBlockMv[blockIdx][XIN_LIST_0].s64x1 == subBlockMv[blockIdx+1][XIN_LIST_0].s64x1)
                    && (subBlockMv[blockIdx][XIN_LIST_1].s64x1 == subBlockMv[blockIdx+1][XIN_LIST_1].s64x1)
                    && (refIdx[blockIdx][XIN_LIST_0] == refIdx[blockIdx+1][XIN_LIST_0])
                    && (refIdx[blockIdx][XIN_LIST_1] == refIdx[blockIdx+1][XIN_LIST_1]))
            {
                subMv[XIN_LIST_0].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_0].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                subMv[XIN_LIST_1].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_1].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                Xin266LumaMotionComp (
                    secSet,
                    subWidth*2,
                    subHeight,
                    pred + xIdx + yIdx*predStride,
                    predStride,
                    subMv,
                    refIdx[blockIdx],
                    bcwIdx,
                    XIN_INTERP_DEF_FILTER);

                blockIdx += 2;
                xIdx     += 2*subWidth;

            }
            else
            {
                subMv[XIN_LIST_0].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_0].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                subMv[XIN_LIST_1].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_1].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                Xin266LumaMotionComp (
                    secSet,
                    subWidth,
                    subHeight,
                    pred + xIdx + yIdx*predStride,
                    predStride,
                    subMv,
                    refIdx[blockIdx],
                    bcwIdx,
                    XIN_INTERP_DEF_FILTER);

                blockIdx += 1;
                xIdx     += subWidth;
            }

        }

    }

}

void Xin266LumaMotionCompAffine (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *pred,
    intptr_t         predStride,
    SINT8            refIdx[XIN_LIST_NUM],
    xin_mv32_u       *affineMv[XIN_LIST_NUM],
    UINT8            bcwIdx)
{
    SINT32      subHeight;
    SINT32      subWidth;
    SINT32      height;
    SINT32      width;
    SINT32      xIdx, yIdx;
    xin_mv32_u  subMv[XIN_LIST_NUM];
    SINT32      blockIdx;

    height     = pu->height;
    width      = pu->width;
    subHeight  = XIN_MIN (height, XIN_AFFINE_SUB_BLOCK_SIZE);
    subWidth   = XIN_MIN (width, XIN_AFFINE_SUB_BLOCK_SIZE);
    blockIdx   = 0;

    for (yIdx = 0; yIdx < height; yIdx += subHeight)
    {
        for (xIdx = 0; xIdx < width;)
        {
            // Check if we can combine two blocks to one block
            if ((xIdx + subWidth*2 <= width) &&
                    ((refIdx[XIN_LIST_0] < 0) || (affineMv[XIN_LIST_0][blockIdx].s64x1 == affineMv[XIN_LIST_0][blockIdx+1].s64x1))
                    && ((refIdx[XIN_LIST_1] < 0) || (affineMv[XIN_LIST_1][blockIdx].s64x1 == affineMv[XIN_LIST_1][blockIdx+1].s64x1)))
            {
                subMv[XIN_LIST_0].s32x2[0] = affineMv[XIN_LIST_0][blockIdx].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_0].s32x2[1] = affineMv[XIN_LIST_0][blockIdx].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                subMv[XIN_LIST_1].s32x2[0] = affineMv[XIN_LIST_1][blockIdx].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_1].s32x2[1] = affineMv[XIN_LIST_1][blockIdx].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                Xin266LumaMotionComp (
                    secSet,
                    subWidth*2,
                    subHeight,
                    pred + xIdx + yIdx*predStride,
                    predStride,
                    subMv,
                    refIdx,
                    bcwIdx,
                    XIN_INTERP_4x4_FILTER);

                blockIdx += 2;
                xIdx     += subWidth*2;

            }
            else
            {
                subMv[XIN_LIST_0].s32x2[0] = affineMv[XIN_LIST_0][blockIdx].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_0].s32x2[1] = affineMv[XIN_LIST_0][blockIdx].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                subMv[XIN_LIST_1].s32x2[0] = affineMv[XIN_LIST_1][blockIdx].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_1].s32x2[1] = affineMv[XIN_LIST_1][blockIdx].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                Xin266LumaMotionComp (
                    secSet,
                    subWidth,
                    subHeight,
                    pred + xIdx + yIdx*predStride,
                    predStride,
                    subMv,
                    refIdx,
                    bcwIdx,
                    XIN_INTERP_4x4_FILTER);

                blockIdx += 1;
                xIdx     += subWidth;

            }

        }

    }

}

void Xin266ChromaMotionCompSubBlock (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *predU,
    PIXEL            *predV,
    intptr_t         predStride,
    xin_mv32_u       subBlockMv[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    SINT8            refIdx[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    UINT8            bcwIdx)
{

    SINT32      subHeight;
    SINT32      subWidth;
    SINT32      height;
    SINT32      width;
    SINT32      xIdx, yIdx;
    xin_mv32_u  subMv[XIN_LIST_NUM];
    SINT32      blockIdx;

    height     = pu->height;
    width      = pu->width;
    subHeight  = XIN_MIN (height, XIN_ATMVP_SUB_BLOCK_SIZE);
    subWidth   = XIN_MIN (width, XIN_ATMVP_SUB_BLOCK_SIZE);
    blockIdx   = 0;

    for (yIdx = 0; yIdx < height; yIdx += subHeight)
    {
        for (xIdx = 0; xIdx < width;)
        {
            // Check if we can combine two blocks to one block
            if ((xIdx + subWidth*2 <= width) &&
                    (subBlockMv[blockIdx][XIN_LIST_0].s64x1 == subBlockMv[blockIdx+1][XIN_LIST_0].s64x1)
                    && (subBlockMv[blockIdx][XIN_LIST_1].s64x1 == subBlockMv[blockIdx+1][XIN_LIST_1].s64x1)
                    && (refIdx[blockIdx][XIN_LIST_0] == refIdx[blockIdx+1][XIN_LIST_0])
                    && (refIdx[blockIdx][XIN_LIST_1] == refIdx[blockIdx+1][XIN_LIST_1]))
            {
                subMv[XIN_LIST_0].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_0].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                subMv[XIN_LIST_1].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_1].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                Xin266ChromaMotionComp (
                    secSet,
                    subWidth*2,
                    subHeight,
                    predU + ((xIdx + yIdx*predStride)>>1),
                    predV + ((xIdx + yIdx*predStride)>>1),
                    predStride,
                    subMv,
                    refIdx[blockIdx],
                    bcwIdx);

                blockIdx += 2;
                xIdx     += subWidth*2;

            }
            else
            {
                subMv[XIN_LIST_0].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_0].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_0].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                subMv[XIN_LIST_1].s32x2[0] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[0] + (xIdx<<XIN_MV_FRAC_BITS);
                subMv[XIN_LIST_1].s32x2[1] = subBlockMv[blockIdx][XIN_LIST_1].s32x2[1] + (yIdx<<XIN_MV_FRAC_BITS);

                Xin266ChromaMotionComp (
                    secSet,
                    subWidth,
                    subHeight,
                    predU + ((xIdx + yIdx*predStride)>>1),
                    predV + ((xIdx + yIdx*predStride)>>1),
                    predStride,
                    subMv,
                    refIdx[blockIdx],
                    bcwIdx);

                blockIdx += 1;
                xIdx     += subWidth;

            }

        }

    }

}

void Xin266ChromaMotionCompAffine (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *predU,
    PIXEL            *predV,
    intptr_t         predStride,
    SINT8            refIdx[XIN_LIST_NUM],
    xin_mv32_u       *affineMv[XIN_LIST_NUM],
    UINT8            bcwIdx)
{

    SINT32      subHeight;
    SINT32      subWidth;
    SINT32      height;
    SINT32      width;
    SINT32      xIdx, yIdx;
    xin_mv32_u  subMv[XIN_LIST_NUM];
    SINT32      blockIdx;
    SINT32      widthInBlock;

    height       = pu->height;
    width        = pu->width;
    subHeight    = XIN_MIN (height, XIN_AFFINE_SUB_BLOCK_SIZE);
    subWidth     = XIN_MIN (width, XIN_AFFINE_SUB_BLOCK_SIZE);
    blockIdx     = 0;
    widthInBlock = width / subWidth;

    for (yIdx = 0; yIdx < height; yIdx += subHeight*2)
    {
        for (xIdx = 0; xIdx < width; xIdx += subWidth*2)
        {
            subMv[XIN_LIST_0].s32x2[0] = affineMv[XIN_LIST_0][blockIdx].s32x2[0] + affineMv[XIN_LIST_0][blockIdx + widthInBlock + 1].s32x2[0];
            subMv[XIN_LIST_0].s32x2[1] = affineMv[XIN_LIST_0][blockIdx].s32x2[1] + affineMv[XIN_LIST_0][blockIdx + widthInBlock + 1].s32x2[1];

            subMv[XIN_LIST_1].s32x2[0] = affineMv[XIN_LIST_1][blockIdx].s32x2[0] + affineMv[XIN_LIST_1][blockIdx + widthInBlock + 1].s32x2[0];
            subMv[XIN_LIST_1].s32x2[1] = affineMv[XIN_LIST_1][blockIdx].s32x2[1] + affineMv[XIN_LIST_1][blockIdx + widthInBlock + 1].s32x2[1];

            Xin266RoundAffineMv (
                &subMv[XIN_LIST_0].s32x2[0],
                &subMv[XIN_LIST_0].s32x2[1],
                1);

            Xin266RoundAffineMv (
                &subMv[XIN_LIST_1].s32x2[0],
                &subMv[XIN_LIST_1].s32x2[1],
                1);

            subMv[XIN_LIST_0].s32x2[0] += (xIdx<<XIN_MV_FRAC_BITS);
            subMv[XIN_LIST_0].s32x2[1] += (yIdx<<XIN_MV_FRAC_BITS);

            subMv[XIN_LIST_1].s32x2[0] += (xIdx<<XIN_MV_FRAC_BITS);
            subMv[XIN_LIST_1].s32x2[1] += (yIdx<<XIN_MV_FRAC_BITS);

            Xin266ChromaMotionComp (
                secSet,
                subWidth*2,
                subHeight*2,
                predU + ((xIdx + yIdx*predStride)>>1),
                predV + ((xIdx + yIdx*predStride)>>1),
                predStride,
                subMv,
                refIdx,
                bcwIdx);

            blockIdx += 2;

        }

        blockIdx += widthInBlock;

    }

}

void Xin266ChromaCompensation (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_pu_struct   *pu;
    xin_mode_struct *modeCtrl;
    intptr_t        predUvPos;

    pu        = &cu->pu;
    predUvPos = (cu->offX + cu->offY*fastBuf->chromaStride) >> 1;
    modeCtrl  = cu->modeCtrl;

    if (fastBuf->mvRefine)
    {
        Xin266ChromaMotionCompDmvr (
            secSet,
            pu,
            fastBuf->predBuf[PLANE_CHROMA_U] + predUvPos,
            fastBuf->predBuf[PLANE_CHROMA_V] + predUvPos,
            fastBuf->chromaStride,
            fastBuf->mv,
            fastBuf->refIdx,
            fastBuf->mvdL0SubPu);
    }
    else if (fastBuf->affine && (fastBuf->affineType != XIN_AFFINE_SBTMVP))
    {
        Xin266ChromaMotionCompAffine (
            secSet,
            pu,
            fastBuf->predBuf[PLANE_CHROMA_U] + predUvPos,
            fastBuf->predBuf[PLANE_CHROMA_V] + predUvPos,
            fastBuf->chromaStride,
            fastBuf->refIdx,
            secSet->affMvBuf[fastBuf->mergeIndex],
            (UINT8)fastBuf->bcwIdx);

    }
    else if (fastBuf->affine)
    {
        Xin266ChromaMotionCompSubBlock (
            secSet,
            pu,
            fastBuf->predBuf[PLANE_CHROMA_U] + predUvPos,
            fastBuf->predBuf[PLANE_CHROMA_V] + predUvPos,
            fastBuf->chromaStride,
            modeCtrl->subMv,
            modeCtrl->subRefIdx,
            (UINT8)fastBuf->bcwIdx);
    }
    else
    {
        Xin266ChromaMotionComp (
            secSet,
            pu->width,
            pu->height,
            fastBuf->predBuf[PLANE_CHROMA_U] + predUvPos,
            fastBuf->predBuf[PLANE_CHROMA_V] + predUvPos,
            fastBuf->chromaStride,
            fastBuf->mv,
            fastBuf->refIdx,
            (UINT8)fastBuf->bcwIdx);
    }

    fastBuf->didChromaMc = TRUE;

}

