/***************************************************************************//**
*
* @file          h265p_encode_sb.h
* @brief         This file contains super block level subroutine declare.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_encode_sb_h_
#define _h265p_encode_sb_h_

void Xin265pReadInputSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pReadInputSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pEncodeSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pWriteSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pComputeBsSb (
    xin_sec_struct *secSet);

void Xin265pDeblockSbHor (
    xin_pic_struct *picSet,
    xin_sb_struct  *sb);

void Xin265pDeblockSbVer (
    xin_pic_struct *picSet,
    xin_sb_struct  *sb);

#endif
