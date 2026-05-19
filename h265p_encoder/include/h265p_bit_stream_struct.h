/***************************************************************************//**
*
* @file          h265p_bit_stream_struct.h
* @brief         This file contains low level bit stream data structure.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
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

