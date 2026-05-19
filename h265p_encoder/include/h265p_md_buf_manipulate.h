/***************************************************************************//**
*
* @file          h265p_md_buf_manipulate.h
* @brief         This file declare mode decsion buffer manipulation subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_md_buf_manipulate_h_
#define _h265p_md_buf_manipulate_h_

void Xin265pSortMdBufSad (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum);

void Xin265pSortMdBufSse (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum);

void Xin265pFindHighestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **highestBuf);

void Xin265pFindLowestSadBuf (
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum,
    xin_fast_md_buf **lowestBuf);

#endif

