/***************************************************************************//**
*
* @file          h265p_func_struct.h
* @brief         This file contins av1 SIMD fucntion definition.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_func_struct_h_
#define _h265p_func_struct_h_

#include "h265p_constant.h"
#include "h265p_md_buffer_struct.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_trans_context.h"

typedef void (*XinBlockCopy) (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

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
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref0,
    UINT8    *ref1,
    UINT8    *ref2,
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
    UINT32      height);

typedef void (*XinMotionInterpS16) (
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

typedef void (*XinQuantInvQuant) (
    COEFF    *qCoeff,
    SINT32   *tCoeff,
    SINT32   *rCoeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    SINT32   logScale,
    SINT32   *qAdd,
    SINT32   *qMult,
    SINT32   *qzBin,
    SINT32   *qShift,
    SINT32   *iqMult,
    UINT32   *nonZeroCount);

typedef void(*XinQuantInvQuantArd) (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    SINT16   *adjBias,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   *qOffset,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

typedef void (*XinSubFDct) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    COEFF    *output,
    intptr_t outputStride,
    COEFF    *tempBuffer);

typedef void (*XinInterpHalfPel) (
    UINT8       *ref,
    intptr_t    refStride,
    UINT32      width,
    UINT32      height,
    UINT8       *intergPel,
    UINT8       *halfPelH,
    UINT8       *halfPelV,
    UINT8       *halfPelHv,
    intptr_t    interpStride);

typedef void (*XinComputeSsd) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

typedef void (*XinComputeSsdFd) (
    SINT32   *tCoeff,
    intptr_t tCoeffStride,
    SINT32   *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    SINT32   txSize,
    UINT64   *ssd);

typedef void (*XinInvTrans2d) (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

typedef void(*XinIntraPredDc) (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

typedef void (*XinIntraPredPlanar) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    UINT32   lgWidth);

typedef void (*XinIntraPredAngular) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   dirMode,
    BOOL     bFilter,
    UINT32   lgWidth);

typedef void (*XinLumaLoopFilter) (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta,
    BOOL     isVert);

typedef void (*XinComputeBlockSsd) (
    SINT16   *tCoeff,
    intptr_t coeffStride,
    UINT32   *ssd,
    UINT64   *totalSsd);

typedef void (*XinEstimateTuCoeff) (
    xin_full_md_buf *mdBuf,
    UINT32          partIdx,
    UINT8           *context,
    xin_tu_struct   *tu,
    BOOL            tranSkipEnabled,
    BOOL            sbhOn,
    UINT32          compIdx,
    UINT64          *bitNum);

typedef void (*XinDownscale2x2) (
    UINT8       *pu8InputCorner,
    intptr_t    s32InputStride,
    BOOL        bHaveInputRowAbove,
    BOOL        bHaveInputRowBelow,
    UINT8       *pu8OutputCorner,
    intptr_t    s32OutputStride,
    UINT32      u32OutputWidth,
    UINT32      u32OutputHeight);

typedef void (*XinHierMotionSearch) (
    UINT8    *input1,
    intptr_t input1Stride,
    UINT8    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

typedef void (*XinComputeHada8x8) (
    UINT8    *input,
    intptr_t inputStride,
    UINT32   *satd);

typedef void (*XinContructBiMeInput) (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *pred,
    intptr_t predStride,
    UINT8    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

typedef void (*Xin1DMotionSearch) (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
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
    UINT16   *coeffSign,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qShift);

typedef void (*XinBlockSub) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    SINT16   *diff,
    intptr_t diffStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinBlockRecon) (
    SINT16   *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinForTrans2d) (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer);

typedef void (*XinIntraPredDrZ2) (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy);

typedef void (*XinIntraPredDrZ1) (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

typedef void (*XinIntraPred) (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

typedef void (*XinpTxInitLevel) (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

typedef struct xin_func_struct
{
    XinBlockCopy         pfXinBlockCopy[XIN_BLOCK_NUM];
    XinBlockCopy         pfXinPictureCopy;
    XinComputeDist       pfXinComputeSad[XIN_BLOCK_NUM];
    XinComputeDist       pfXinComputeSatd[XIN_BLOCK_NUM];
    XinComputeDist       pfXinComputeDist[XIN_BLOCK_NUM];
    XinComputeAvgDist    pfXinComputeAvgSad[XIN_BLOCK_NUM];
    XinComputeAvgDist    pfXinComputeAvgSatd[XIN_BLOCK_NUM];
    XinComputeAvgDist    pfXinComputeAvgDist[XIN_BLOCK_NUM];
    XinComputeSadx8      pfXinComputeSadx8[XIN_BLOCK_NUM];
    XinComputeSadx5      pfXinComputeSadx5[XIN_BLOCK_NUM];
    XinComputeSadx3      pfXinComputeSadx3[XIN_BLOCK_NUM];
    XinComputeAvgSadx8   pfXinComputeAvgSadx8[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinInterpCopy[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinChromaInterpHor[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinChromaInterpVet[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinChromaInterpHorVet[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinLumaInterpHor[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinLumaInterpVet[XIN_BLOCK_NUM];
    XinMotionInterp      pfXinLumaInterpHorVet[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinInterpCopyS16[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinChromaInterpHorS16[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinChromaInterpVetS16[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinChromaInterpHorVetS16[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinLumaInterpHorS16[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinLumaInterpVetS16[XIN_BLOCK_NUM];
    XinMotionInterpS16   pfXinLumaInterpHorVetS16[XIN_BLOCK_NUM];
    XinMotionInterpAvg   pfXinInterpAvg[XIN_BLOCK_NUM];
    XinQuantInvQuant     pfXinQuantInvQuant[XIN_BLOCK_NUM];
    XinBlockSub          pfXinBlockSub[XIN_BLOCK_NUM];
    XinForTrans2d        pfXinForTrans2d[XIN_TX_SIZE_NUM];
    XinInvTrans2d        pfXinInvTrans2d[XIN_TX_SIZE_NUM];
    XinComputeSsd        pfXinComputeSsd[XIN_BLOCK_NUM];
    XinComputeSsdFd      pfXinComputeSsdFd[XIN_BLOCK_NUM];
    XinIntraPredDrZ2     pfXinIntraPredDrZ2[XIN_BLOCK_NUM];
    XinIntraPredDrZ1     pfXinIntraPredDrZ1[XIN_BLOCK_NUM];
    XinIntraPred         pfXinIntraPredVer[XIN_BLOCK_NUM];
    XinIntraPred         pfXinIntraPredHor[XIN_BLOCK_NUM];
    XinIntraPred         pfXinIntraPredPaeth[XIN_BLOCK_NUM];
    XinIntraPred         pfXinIntraPredSmV[XIN_BLOCK_NUM];
    XinIntraPred         pfXinIntraPredSmH[XIN_BLOCK_NUM];
    XinIntraPred         pfXinIntraPredSm[XIN_BLOCK_NUM];
    XinBlockRecon        pfXinBlockRecon[XIN_BLOCK_NUM];
    XinLumaLoopFilter    pfXinLumaLoopFilter;
    XinInterpHalfPel     pfXinInterpoHalfPel;
    XinComputeBlockSsd   pfXinComputeBlockSsd;
    XinEstimateTuCoeff   pfXinEstimateTuCoeff;
    XinDownscale2x2      pfXinDownscale2x2;
    XinHierMotionSearch  pfXinMotionSearchHier2[XIN_BLOCK_NUM];
    XinHierMotionSearch  pfXinMotionSearchHier1[XIN_BLOCK_NUM];
    XinContructBiMeInput pfXinConstructBiMeInput[XIN_BLOCK_NUM];
    Xin1DMotionSearch    pfXinHorMotionSearch[XIN_BLOCK_NUM];
    Xin1DMotionSearch    pfXinVerMotionSearch[XIN_BLOCK_NUM];
    XinpTxInitLevel      pfXinTxInitLevel[XIN_BLOCK_NUM];
    XinComputeHada8x8    pfXinComputeHada8x8;
    XinGetBlockDeltaU    pfXinGetBlockDeltaU;
}xin_func_struct;

void Xin265pFuncInit (
    xin_func_struct *funcSet,
    UINT32          cpuFeature);

#endif
