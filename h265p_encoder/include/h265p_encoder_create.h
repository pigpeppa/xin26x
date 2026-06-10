/***************************************************************************//**
 *
 * @file          h265p_encoder_create.h
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

