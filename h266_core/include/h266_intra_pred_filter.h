/***************************************************************************//**
 *
 * @file          h266_intra_pred_filter.h
 * @brief         This file declares h266 intra prediction filter subroutines.
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

#ifndef _h266_intra_pred_filter_h_
#define _h266_intra_pred_filter_h_

void Xin266IntraPredDc (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredDc4xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredDc8xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredDc16xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredDc32xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredDc64xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredPlanar (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredPlanar4xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredPlanar_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredPlanarGt8xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor4xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor8xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor16xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor32xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor64xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredHor64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer4xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer32xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer64xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266IntraPredVer64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

void Xin266ApplyPDPCHor (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCHor4xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCHor8xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCHor16xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCHor32xH_AVX2 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCHor64xH_AVX2 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCVer (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCVer4xH_SSE4 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCVer8xH_SSE4 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPCVer16xH_SSE4 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC4xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC8xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC16xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC32xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC64xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC16xH_AVX2 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC32xH_AVX2 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266ApplyPDPC64xH_AVX2 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

void Xin266LinearTransform (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   scale,
    SINT32   offset,
    SINT32   shift,
    UINT32   width,
    UINT32   height);

void Xin266LinearTransform8xH_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   scale,
    SINT32   offset,
    SINT32   shift,
    UINT32   width,
    UINT32   height);

void Xin266LinearTransformGt8xH_AVX2 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   scale,
    SINT32   offset,
    SINT32   shift,
    UINT32   width,
    UINT32   height);

void Xin266ApplyAngPDPCHor (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCVert (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCHor4xH_SSE4 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCHor8xH_SSE4 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCHor16xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCHorGt16xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCVert4xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCVert8xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266ApplyAngPDPCVertGt8xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

void Xin266LumaIntraFilter (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266LumaIntraFilterGt8xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266LumaIntraFilter8xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266LumaIntraFilterGt16xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266ChromaIntraFilter (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266ChromaIntraFilter4xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266ChromaIntraFilterGt4xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266ChromaIntraFilterGt8xH_AVX2 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

void Xin266ExtendIntraRef_AVX2 (
    PIXEL  *refMain,
    PIXEL  *refSide,
    SINT32 intraPredAngleMode,
    SINT32 width,
    SINT32 height,
    BOOL   isModeVer);

void Xin266ExtendIntraRef (
    PIXEL  *refMain,
    PIXEL  *refSide,
    SINT32 intraPredAngleMode,
    SINT32 width,
    SINT32 height,
    BOOL   isModeVer);

#endif
