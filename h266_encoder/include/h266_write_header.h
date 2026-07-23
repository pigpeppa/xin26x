/***************************************************************************//**
 *
 * @file          h266_write_header.h
 * @brief         This file declares header write subroutines.
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
#ifndef _h266_write_header_h_
#define _h266_write_header_h_

void Xin266WriteVps (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet);

void Xin266WriteSps (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet);

void Xin266WritePps (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet);

void Xin266WriteApsAlf (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet);

void Xin266WriteApsLmcs (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet);

void Xin266WriteSliceHeader (
    UINT32         firstCtuAddr,
    SINT32         sliceHeaderQp,
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet);

#endif

