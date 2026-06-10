/***************************************************************************//**
 *
 * @file          h265p_write_header.h
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
#ifndef _h265p_write_header_h_
#define _h265p_write_header_h_

void Xin265pWriteSeqHeader (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet);

void Xin265pGenIvfFileHeader (
    xin_seq_struct *seqSet,
    UINT8          *ivfFileHeader);

void Xin265pGenIvfFrameHeader (
    xin_seq_struct *seqSet,
    UINT8          *ivfFrameHeader,
    UINT32         frameSize,
    UINT64         poc);

void Xin265pWriteObuHeader (
    xin_bs_struct *bitstream,
    UINT32        obuType);

void Xin265pWriteSliceHeader (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet);

void Xin265pGenObuHeaderAndUleb (
    UINT8   *obuValue,
    UINT8   *ulebValue,
    UINT8   *ulebLength,
    UINT32  obuType,
    UINT32  obuSize);

#endif

