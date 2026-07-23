/***************************************************************************//**
 *
 * @file          h266_func_struct.h
 * @brief         This file contains h266 SIMD function definitions.
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
#ifndef _h266_func_struct_h_
#define _h266_func_struct_h_

#include "h266_constant.h"
#include "h266_md_buffer_struct.h"
#include "h266_trans_unit_struct.h"
#include "h266_alf_subroutines.h"

typedef void (*XinBlockCopy) (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinPictureScaleCopy) (
    void     *src,
    intptr_t srcStride,
    void     *dst,
    intptr_t dstStride,
    UINT32   srcBitDepth,
    UINT32   dstBitDepth,
    UINT32   width,
    UINT32   height);

typedef void (*XinBlockAvg) (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

typedef void (*XinComputeDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeAvgDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeWeightDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    SINT32   weightA,
    SINT32   weightB,
    UINT32   *sad);

typedef void (*XinComputeSadx8) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeAvgSadx8) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeSadx5) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    PIXEL    *ref3,
    PIXEL    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeSadx3) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinMotionInterp) (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

typedef void (*XinMotionInterpS16) (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

typedef void (*XinBiliInterp) (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

typedef void (*XinMotionInterpAvg) (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height);

typedef void (*XinMotionInterpWeight) (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height,
    SINT32       weightA,
    SINT32       weightB);

typedef void (*XinQuantInvQuant) (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    intptr_t coeffStride,
    UINT32   width,
    UINT32   height,
    UINT32   cGWidth,
    UINT32   cGHeight,
    SINT32   qMult,
    SINT32   qAdd,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nzCGBitMapRs);

typedef void (*XinFor1dDct) (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

typedef void (*XinFor2dDct2) (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

typedef void (*XinFor2dSkip) (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf);

typedef void (*XinComputeSsd) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

typedef void (*XinComputeSsdFd) (
    COEFF    *tCoef,
    intptr_t tCoefStride,
    COEFF    *rCoef,
    intptr_t rCoefStride,
    UINT32   tuLgSize,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

typedef void  (*XinComputeSsdFdYuv) (
    COEFF    *tCoef[PLANE_NUM],
    intptr_t  tCoefYStride,
    intptr_t  tCoefUvStride,
    COEFF     *rCoef[PLANE_NUM],
    intptr_t  rCoefYStride,
    intptr_t  rCoefUvStride,
    UINT32    tuLgYSize,
    UINT32    tuLgUvSize,
    UINT32    lgWidth,
    UINT32    height,
    UINT32    planeMask,
    UINT64    *ssdY,
    UINT64    *ssdUv);

typedef void (*XinIDctAdd) (
    COEFF    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    COEFF    *tempBuffer);

typedef void (*XinInv2dDct2)(
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

typedef void (*XinInv1dDct) (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

typedef void (*XinIntraLower) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    UINT32   size);

typedef void (*XinIntraPredDc) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

typedef void (*XinIntraPredPlanar) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

typedef void (*XinIntraPredHor) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

typedef void (*XinIntraPredVer) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

typedef void (*XinIntraPredAng) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    BOOL     interpFlag,
    BOOL     applyPDPC,
    SINT32   intraPredAngle,
    SINT32   invAngle,
    SINT32   angularScale,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight);

typedef void (*XinApplyPDPC) (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight);

typedef void (*XinApplyAngPDPC) (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height);

typedef void (*XinSaoStatEo) (
    SINT16    *diff,
    intptr_t  diffStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diffSum,
    UINT16    *count);

typedef void (*XinSaoEo45) (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

typedef void (*XinSaoEo135) (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

typedef void (*XinSaoEo90) (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

typedef void (*XinSaoEo0) (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

typedef void (*XinComputeBlockSsd) (
    COEFF   *tCoeff,
    intptr_t coeffStride,
    UINT32   *ssd,
    UINT64   *totalSsd);

typedef void (*XinEstimateTuCoeff) (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    BOOL            depQuant,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum);

typedef void (*XinDownscale2x2) (
    PIXEL       *pu8InputCorner,
    intptr_t    s32InputStride,
    BOOL        bHaveInputRowAbove,
    BOOL        bHaveInputRowBelow,
    PIXEL       *pu8OutputCorner,
    intptr_t    s32OutputStride,
    UINT32      u32OutputWidth,
    UINT32      u32OutputHeight);

typedef void (*XinHierMotionSearch) (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

typedef void (*XinComputeHada8x8) (
    PIXEL    *input,
    intptr_t inputStride,
    UINT32   *satd);

typedef void (*XinContructBiMeInput) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinConstructWeightBiMeInput) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    SINT32   bcwWeight);

typedef void (*Xin1DMotionSearch) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

typedef void (*XinGetBlockDeltaU) (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *deltaU,
    UINT32   cgWidth,
    UINT32   cgHeight,
    UINT16   *coeffSign,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qShift);

typedef void (*XinUpdateQuantOffset) (
    SINT32  *quantOffset[2][4][3],
    SINT16  *quantBias[2][4][3],
    UINT32  qpPer[2]);

typedef void (*XinComputeVar8x8) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT32   *sse,
    SINT32   *var);

typedef void (*XinBlockSub) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinBlockSubForDct) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    COEFF    *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinBlockAddForDct) (
    COEFF    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinComputeDistS16) (
    SINT16   *input,
    intptr_t inputStride,
    SINT16   *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   subShift,
    UINT32   *sad);

typedef void (*XinCoeffScanCG) (
    COEFF        *coefBuffer,
    intptr_t     coefStride,
    SINT32       cgWidth,
    SINT32       cgHeight,
    xin_scan_pos *scanOrder,
    UINT16       *coefSign,
    UINT16       *gt0Buf);

typedef void (*XinLumaLoopFilter) (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta[2],
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ,
    BOOL     isVert);

typedef void (*XinLinearTransform) (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   scale,
    SINT32   offset,
    SINT32   shift,
    UINT32   width,
    UINT32   height);

typedef void (*XinComputeFrameAct) (
    PIXEL       *input,
    intptr_t    inputStride,
    PIXEL       *ref,
    intptr_t    refStride,
    SINT32      width,
    SINT32      height,
    double      *outputAct);

typedef void (*XinCalcBlockDeltaU) (
    COEFF    *tCoef,
    COEFF    *rCoef,
    COEFF    *qCoef,
    intptr_t coefStride,
    SINT32   iqMult,
    SINT32   iqShift,
    UINT32   cgWidth,
    UINT32   cgHeight,
    SINT32   *distNow,
    SINT32   *distUp,
    SINT32   *distDown,
    UINT16   *coeffSign);

typedef void (*XinCopyAndPad) (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pad,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height);

typedef void (*XinCopyAndPadUv) (
    PIXEL      *refU,
    PIXEL      *refV,
    intptr_t   refStride,
    PIXEL      *padU,
    PIXEL      *padV,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height);

typedef void(*XinPreRdoq) (
    COEFF       *tCoeff,
    COEFF       *rCoeff,
    COEFF       *qCoeff,
    intptr_t    coeffStride,
    UINT32       width,
    UINT32       height,
    UINT32       cGWidth,
    UINT32       cGHeight,
    SINT32       rdoqThrVal,
    xin_scan_pos *scanOrderCG,
    UINT64       *nzCGBitMapRs);

typedef void (*XinInv2dSkip) (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

typedef void(*XinPreDepQuant) (
    COEFF       *tCoeff,
    COEFF       *rCoeff,
    COEFF       *qCoeff,
    intptr_t    coeffStride,
    UINT32       width,
    UINT32       height,
    UINT32       cGWidth,
    UINT32       cGHeight,
    SINT32       rdoqThrVal,
    xin_scan_pos *scanOrderCG,
    xin_scan_pos *scanOrder,
    SINT32       *nzCoeffIdx);

typedef void(*XinComputeGradient) (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height);

typedef void (*XinDepQuant) (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_struct   *tu,
    COEFF           *tCoeff,
    COEFF           *rCoeff,
    COEFF           *qCoeff,
    intptr_t        coeffStride,
    UINT32          qp,
    SINT32          width,
    SINT32          height,
    UINT64          *nzCGBitMapRs,
    UINT32          compId);

typedef void (*XinBlockTranspose) (
    const PIXEL *input,
    intptr_t    inputStride,
    PIXEL       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

typedef void (*XinLumaIntraFilter) (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

typedef void (*XinChromaIntraFilter) (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height);

typedef void (*XinExtendIntraRef) (
    PIXEL  *refMain,
    PIXEL  *refSide,
    SINT32 intraPredAngleMode,
    SINT32 width,
    SINT32 height,
    BOOL   isModeVer);

typedef void (*XinFilterIntraNB) (
    PIXEL   *src,
    PIXEL   *dst,
    UINT32  width,
    UINT32  height,
    SINT32  multiRefIdx);

typedef struct xin_func_struct
{
    XinBlockCopy                pfXinBlockCopy[XIN_BLOCK_NUM];
    XinBlockAvg                 pfXinBlockAvg[XIN_BLOCK_NUM];
    XinBlockCopy                pfXinPictureCopy;
    XinPictureScaleCopy         pfXinPictureScaleCopy;
    XinComputeDist              pfXinComputeSad[XIN_BLOCK_NUM];
    XinComputeDistS16           pfXinComputeSadS16[XIN_BLOCK_NUM];
    XinComputeDist              pfXinComputeSatd[XIN_BLOCK_NUM];
    XinComputeDist              pfXinComputeDist[XIN_BLOCK_NUM];
    XinComputeAvgDist           pfXinComputeAvgSad[XIN_BLOCK_NUM];
    XinComputeAvgDist           pfXinComputeAvgSatd[XIN_BLOCK_NUM];
    XinComputeAvgDist           pfXinComputeAvgDist[XIN_BLOCK_NUM];
    XinComputeWeightDist        pfXinComputeWeightSad[XIN_BLOCK_NUM];
    XinComputeWeightDist        pfXinComputeWeightSatd[XIN_BLOCK_NUM];
    XinComputeWeightDist        pfXinComputeWeightDist[XIN_BLOCK_NUM];
    XinComputeSadx8             pfXinComputeSadx8[XIN_BLOCK_NUM];
    XinComputeSadx5             pfXinComputeSadx5[XIN_BLOCK_NUM];
    XinComputeSadx3             pfXinComputeSadx3[XIN_BLOCK_NUM];
    XinComputeAvgSadx8          pfXinComputeAvgSadx8[XIN_BLOCK_NUM];
    XinMotionInterp             pfXinLumaInterp[2][2][XIN_BLOCK_NUM];
    XinMotionInterp             pfXinChromaInterp[2][2][XIN_BLOCK_NUM];
    XinBiliInterp               pfXinBiliInterp[2][2][XIN_BLOCK_NUM];
    XinMotionInterpS16          pfXinLumaInterpS16[2][2][XIN_BLOCK_NUM];
    XinMotionInterpS16          pfXinChromaInterpS16[2][2][XIN_BLOCK_NUM];
    XinMotionInterpAvg          pfXinInterpAvg[XIN_BLOCK_NUM];
    XinMotionInterpWeight       pfXinInterpWeight[XIN_BLOCK_NUM];
    XinQuantInvQuant            pfXinQuantInvQuant[XIN_BLOCK_NUM];
    XinFor2dDct2                pfXinFor2dDct2[XIN_BLOCK_NUM][XIN_BLOCK_NUM];
    XinFor2dSkip                pfXinFor2dSkip[XIN_BLOCK_NUM];
    XinInv2dSkip                pfXinInv2dSkip[XIN_BLOCK_NUM];
    XinFor1dDct                 pfXinFor1dDct[XIN_TRANS_TYPE][XIN_BLOCK_NUM];
    XinIDctAdd                  pfXinIDctAdd[XIN_BLOCK_NUM];
    XinIDctAdd                  pfXinIDctAddDc[XIN_BLOCK_NUM];
    XinInv2dDct2                pfXinInv2dDct2[XIN_BLOCK_NUM][XIN_BLOCK_NUM];
    XinInv1dDct                 pfXinInv1dDct[XIN_TRANS_TYPE][XIN_BLOCK_NUM];
    XinComputeSsd               pfXinComputeSsd[XIN_BLOCK_NUM];
    XinComputeSsdFd             pfXinComputeSsdFd[XIN_BLOCK_NUM];
    XinLinearTransform          pfXinLinearTransform[XIN_BLOCK_NUM];
    XinIntraPredDc              pfXinIntraPredDc[XIN_BLOCK_NUM];
    XinIntraPredPlanar          pfXinIntraPredPlanar[XIN_BLOCK_NUM];
    XinIntraPredHor             pfXinIntraPredHor[XIN_BLOCK_NUM];
    XinIntraPredVer             pfXinIntraPredVer[XIN_BLOCK_NUM];
    XinApplyPDPC                pfXinApplyPDPC[XIN_BLOCK_NUM];
    XinApplyPDPC                pfXinApplyHorPDPC[XIN_BLOCK_NUM];
    XinApplyPDPC                pfXinApplyVerPDPC[XIN_BLOCK_NUM];
    XinApplyAngPDPC             pfXinApplyAngPDPC[2][XIN_BLOCK_NUM];
    XinSaoStatEo                pfXinSaoStatEo[XIN_BLOCK_NUM];
    XinCoeffScanCG              pfXinCoeffScanCG[2];
    XinSaoEo0                   pfXinSaoEo0[2];
    XinSaoEo45                  pfXinSaoEo45[2];
    XinSaoEo90                  pfXinSaoEo90[2];
    XinSaoEo135                 pfXinSaoEo135[2];
    XinBlockSub                 pfXinBlockSub[XIN_BLOCK_NUM];
    XinBlockSubForDct           pfXinBlockSubForDct[XIN_BLOCK_NUM];
    XinBlockAddForDct           pfXinBlockAddForDct[XIN_BLOCK_NUM];
    XinLumaIntraFilter          pfXinLumaIntraFilter[XIN_BLOCK_NUM];
    XinChromaIntraFilter        pfXinChromaIntraFilter[XIN_BLOCK_NUM];
    XinComputeBlockSsd          pfXinComputeBlockSsd;
    XinEstimateTuCoeff          pfXinEstimateTuCoeff;
    XinDownscale2x2             pfXinDownscale2x2;
    XinContructBiMeInput        pfXinConstructBiMeInput[XIN_BLOCK_NUM];
    XinConstructWeightBiMeInput pfXinConstructWeightBiMeInput[XIN_BLOCK_NUM];
    XinComputeHada8x8           pfXinComputeHada8x8;
    XinGetBlockDeltaU           pfXinGetBlockDeltaU[2];
    XinCalcBlockDeltaU          pfXinCalcBlockDeltaU[2];
    XinUpdateQuantOffset        pfXinUpdateQuantOffset;
    XinComputeVar8x8            pfXinComputeVar8x8;
    XinComputeDist              pfXinLaComputeDist[XIN_BLOCK_NUM];
    XinCopyAndPad               pfXinCopyAndPad;
    XinCopyAndPadUv             pfXinCopyAndPadUv;
    XinComputeGradient          pfXinComputeGradient[XIN_BLOCK_NUM];
    XinLumaLoopFilter           pfXinLumaLoopFilter;
    XinComputeFrameAct          pfXinComputeFrameAct;
    XinPreRdoq                  pfXinPreRdoq;
    XinPreDepQuant              pfXinPreDepQuant;
    XinDepQuant                 pfXinDepQuant;
    XinBlockTranspose           pfXinBlockTranspose;
    XinExtendIntraRef           pfXinExtendIntraRef;
    XinFilterIntraNB            pfXinFilterIntraNB;
} xin_func_struct;

void Xin266FuncInit (
    xin_func_struct *funcSet,
    UINT32          cpuFeature);

#endif


