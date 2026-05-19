/***************************************************************************//**
*
* @file          h265_encode_init.c
* @brief         This file declare frame, section, super block and
*                block level initialization subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_enc_init_h_
#define _h265p_enc_init_h_

void Xin265pMbUpdateMi (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb);

void Xin265pMbInit (
    xin_sec_struct *secSet,
    xin_mb_struct  *parentMb,
    UINT32         partType,
    UINT32         partIdx);

void Xin265pSectionInit (
    xin_sec_struct *secSet);

void Xin265pSbInit (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb);

void Xin265pMbPostInit (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb);

void Xin265pFrameInit (
    xin_pic_struct *picSet,
    xin_input_picture *inputPicture);

void Xin265pEncodeSection (
    xin_sec_struct *secSet);

void Xin265pFramePostInit (
    xin_pic_struct *picSet);

void Xin265pLfCdefRestSection (
    xin_sec_struct * secSet);

#endif

