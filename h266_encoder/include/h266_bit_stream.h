/***************************************************************************//**
 *
 * @file          h266_bit_stream.h
 * @brief         This file declares h266 low-level bit stream
 *                manipulation subroutines.
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
#ifndef _h266_bit_stream_h_
#define _h266_bit_stream_h_

void Xin266WriteOneBit (
    xin_bs_struct *bitstream,
    UINT32        bit);

void Xin266WriteBits (
    xin_bs_struct *bitstream,
    UINT32        bits,
    UINT32        length);

void Xin266WriteUvlc (
    xin_bs_struct *bitstream,
    UINT32        bits);

void Xin266WriteSvlc (
    xin_bs_struct *bitstream,
    SINT32        bits);

void Xin266WriteFlush (
    xin_bs_struct *bitstream);

void Xin266BitstreamSize (
    xin_bs_struct *bitstream,
    UINT32        *bitstreamSize);

void Xin266OutputBitToLinearBuffer (
    xin_bs_struct *bitstream,
    xin_lb_struct *linearBuffer,
    BOOL          addStartCode);

void Xin266InitBitstream (
    xin_bs_struct *bitstream);

#endif
