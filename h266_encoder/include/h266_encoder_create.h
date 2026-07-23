/***************************************************************************//**
 *
 * @file          h266_encoder_create.h
 * @brief         This file declares encoder creation subroutines.
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
#ifndef _h266_encoder_create_h_
#define _h266_encoder_create_h_

SINT32 Xin266SeqCreate (
    xin_seq_struct *seqSet,
    xin26x_params  *config);

void Xin266SeqDelete (
    xin_seq_struct *seqSet);

SINT32 Xin266PicCreate (
    xin_pic_struct **dblPicSet,
    xin_seq_struct *seqSet);

void Xin266PicDelete (
    xin_pic_struct *picSet);

SINT32 Xin266SecCreate (
    xin_sec_struct **dblSecSet,
    xin_seq_struct *seqSet);

void Xin266SecDelete (
    xin_sec_struct *secSet,
    xin_seq_struct *seqSet);

void Xin266ConstructRps (
    xin_seq_struct *seqSet);

SINT32 Xin266ContructScanOrder (
    xin_seq_struct *seqSet);

void Xin266DestructScanOrder (
    xin_seq_struct *seqSet);

SINT32 ConstructRefPicBuf (
    xin_seq_struct *seqSet);

void Xin266DestructRefPicBuf (
    xin_seq_struct *seqSet);

#endif

