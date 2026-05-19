/***************************************************************************//**
*
* @file          h265p_intra_prediction.h
* @brief         This declare av1 intra prediction subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_intra_prediction_h_
#define _h265p_intra_prediction_h_

void Xin265pIntraPred (
    xin_sec_struct *secSet,
    UINT32         planeIdx,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         mode,
    SINT32         deltaAngle);

BOOL Xin265pFilterIntraAllowed (
    BOOL    enableFilterIntra,
    UINT32  blockSize);

void Xin265pGetIntraAvail (
    xin_mb_struct  *mb,
    UINT32         lgMiSize,
    intptr_t       miStride);

void Xin265pExtractIntraNBChroma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb);

void Xin265pExtractIntraNBLuma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb);


#endif

