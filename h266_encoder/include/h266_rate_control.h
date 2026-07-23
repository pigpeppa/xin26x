/***************************************************************************//**
 *
 * @file          h266_rate_control.h
 * @brief         This file declares subroutines related to rate control.
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
#ifndef _h266_rate_control_h_
#define _h266_rate_control_h_

void Xin266RcCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266RcUpdateCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266RcPic (
    xin_pic_struct *picSet);

void Xin266RcSkipDec (
    xin_pic_struct *picSet);

void Xin266RcGop (
    xin_pic_struct *picSet,
    UINT32         gopSize);

SINT32 Xin266RcCreate (
    xin_seq_struct  *seqSet);

void Xin266RcDelete (
    xin_seq_struct  *seqSet);

void Xin266RcUpdatePic (
    xin_pic_struct *picSet,
    SINT32         outputBits,
    SINT32         headerBits);

void Xin266RcUpdateSkip (
    xin_pic_struct *picSet);

#endif

