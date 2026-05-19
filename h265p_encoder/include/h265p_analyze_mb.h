/***************************************************************************//**
*
* @file          h265p_analyze_mb.h
* @brief         This file contains block analyze subroutines declare.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_analyze_mb_h_
#define _h265p_analyze_mb_h_

void Xin265pAnalyzeMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb);

void Xin265pAnalyzeIntraMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf **fastBuf,
    UINT32          bufNum,
    xin_fast_md_buf *interMdBuf);

void Xin265pGetBlockAvail (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb);

void Xin265pEncodeIntraMb (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf);

#endif

