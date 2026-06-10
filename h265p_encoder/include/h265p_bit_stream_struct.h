/***************************************************************************//**
 *
 * @file          h265p_bit_stream_struct.h
 * @brief         This file contains low-level bit stream data structure.
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
#ifndef _h265p_bit_stream_struct_h_
#define _h265p_bit_stream_struct_h_

// Object used to write and escape the VLC bitstream.
typedef struct xin_bs_struct
{
    // Pointer to the first byte of the buffer. The buffer is assumed to be
    // large enough to write all the data.
    UINT8  *base;

    // Buffer size
    UINT32 size;

    // Pointer to the current byte of the buffer. The data is written in byte
    // chunks.
    UINT8  *cur;

    // Bit number has been written into buffer
    UINT32 bitCount;

    // Number of bits required to complete the current byte.
    SINT32 bitsLeft;

} xin_bs_struct;

#endif

