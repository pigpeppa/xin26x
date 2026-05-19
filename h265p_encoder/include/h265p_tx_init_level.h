/***************************************************************************//**
*
* @file          h265p_tx_init_level.h
* @brief         This file declare tx level initialization subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_tx_init_level_h_
#define _h265p_tx_init_level_h_

void Xin265pTxInitLevel (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel4xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel8xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel16xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel32xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pTxInitLevel64xH_SSE4 (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

#endif

