/***************************************************************************//**
*
* @file          h265p_loop_filter.h
* @brief         This file declare AV1 loop filter subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_loop_filter_h_
#define _h265p_loop_filter_h_

void Xin265pLpfVert4 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfVert6 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8   *blimit,
    UINT8   *limit,
    UINT8   *thresh);

void Xin265pLpfVert8 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfVert14 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz4 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz6 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz8 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz14 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

#endif

