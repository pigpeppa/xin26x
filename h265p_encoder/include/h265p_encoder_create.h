/***************************************************************************//**
*
* @file          h265p_encoder_create.c
* @brief         This file declare encoder creation subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_encoder_create_h_
#define _h265p_encoder_create_h_

SINT32 Xin265pSeqCreate (
    xin_seq_struct **dblSeqSet,
    xin26x_params  *config);

void Xin265pSeqDelete (
    xin_seq_struct *seqSet);

SINT32 Xin265pPicCreate (
    xin_pic_struct **dblPicSet,
    xin_seq_struct *seqSet);

void Xin265pPicDelete (
    xin_pic_struct *picSet);

SINT32 Xin265pSecCreate (
    xin_sec_struct **dblSecSet,
    xin_seq_struct *seqSet);

void Xin265pSecDelete (
    xin_sec_struct *secSet,
    xin_seq_struct *seqSet);

SINT32 Xin265pConstructRefPicBuf (
    xin_seq_struct *seqSet);


#endif

