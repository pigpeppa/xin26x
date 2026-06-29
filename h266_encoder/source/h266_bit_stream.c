/***************************************************************************//**
 *
 * @file          h266_bit_stream.c
 * @brief         h266 low-level bit stream manipulation functions.
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
#include "xin_typedef.h"
#include "h266_bit_stream_struct.h"
#include "xin_video_common.h"

// Add a single bit (flag) to the bitstream.
void Xin266WriteOneBit (
    xin_bs_struct *bitstream,
    UINT32        bit)
{
    UINT32  code;

    bitstream->bitCount += 1;
    
    // Append the bit to the current byte without completing it.
    if (bitstream->bitsLeft > 1)
    {
        bitstream->bitsLeft--;
        bitstream->cur[0] |= bit << bitstream->bitsLeft;

        return;
    }

    // Complete and write the current byte.
    code = bitstream->cur[0] | bit;
    *bitstream->cur++ = (UINT8)code;
    bitstream->cur[0] = 0;

    bitstream->bitsLeft = 8;

}

// Add bits to the bitstream.
void Xin266WriteBits (
    xin_bs_struct *bitstream,
    UINT32        bits,
    UINT32        length)
{
    UINT32 code;

    bitstream->bitCount += length;
    
    // Append the bits to the current byte without completing it.
    if ((SINT32)length < bitstream->bitsLeft)
    {
        bitstream->bitsLeft -= length;
        bitstream->cur[0]   |= bits << bitstream->bitsLeft;
        
        return;
    }

    // Complete and write the current byte.
    length -= bitstream->bitsLeft;
    code    = bitstream->cur[0]|(bits>>length);

    *bitstream->cur++ = (UINT8)code;

    // Write the next complete bytes.
    while (length >= 8)
    {
        length -= 8;
        code    = (bits>>length)&0xff;

        *bitstream->cur++ = (UINT8)code;
    }

    // Begin the last partial byte.
    bitstream->bitsLeft = 8 - length;
    bitstream->cur[0]   = (bits&((1 << length) - 1)) << bitstream->bitsLeft;

}

// Add a uvlc code to the bitstream.
void Xin266WriteUvlc (
    xin_bs_struct *bitstream,
    UINT32        bits)
{
    UINT32  length;
    UINT32  tempBits;

    tempBits = ++bits;
    length   = 1;

    while (1 != tempBits)
    {
        tempBits >>= 1;
        length    += 2;
    }

    Xin266WriteBits (
        bitstream,
        bits,
        length);

}

// Add a svlc code to the bitstream.
void Xin266WriteSvlc (
    xin_bs_struct *bitstream,
    SINT32        bits)
{
    UINT32  code;

    code = (bits <= 0) ? -bits<<1 : (bits<<1) - 1;

    Xin266WriteUvlc (
        bitstream,
        code);
}

// Write a stop bit, byte alignment and flush the bit buffer
void Xin266WriteFlush (
    xin_bs_struct *bitstream)
{
    Xin266WriteOneBit (
        bitstream,
        1);

    if (bitstream->bitsLeft & 0x7)
    {
        Xin266WriteBits(
            bitstream,
            0,
            bitstream->bitsLeft);
    }
}

// Calculate bit stream size.
void Xin266BitstreamSize (
    xin_bs_struct *bitstream,
    UINT32        *bitstreamSize)
{
    UINT32  trailZeros;
    UINT32  readIndex;
    UINT32  writeIndex;
    UINT8   *read;
    UINT32  writeCount;

    read       = bitstream->base;
    trailZeros = 0;
    readIndex  = 0;
    writeIndex = 0;
    writeCount = (UINT32)(bitstream->cur - bitstream->base);

    while (readIndex < writeCount)
    {
        if ((trailZeros == 2) && ((read[readIndex] & 0xfc) == 0))
        {
            writeIndex++;
            trailZeros = 0;
        }

        writeIndex++;
        trailZeros = (read[readIndex]) ? 0 : trailZeros+1;
        readIndex++;
    }

    *bitstreamSize = writeIndex;

}

// Output bit to linear buffer and add emualtion byte if necessary
void Xin266OutputBitToLinearBuffer (
    xin_bs_struct *bitstream,
    xin_lb_struct *linearBuffer,
    BOOL          addStartCode)
{
    UINT8   *write;
    UINT8   *read;
    UINT32  writeIndex;
    UINT32  readIndex;
    UINT32  writeCount;
    UINT32  trailZeros;

    trailZeros = 0;
    writeIndex = 0;
    readIndex  = 0;
    write      = ((UINT8 *)linearBuffer->base) + linearBuffer->index;
    read       = bitstream->base;
    writeCount = (UINT32)(bitstream->cur - bitstream->base);
    
    if (addStartCode)
    {
        write[writeIndex++] = 0;
        write[writeIndex++] = 0;
        write[writeIndex++] = 0;
        write[writeIndex++] = 1;

        linearBuffer->index += 4;
    }

    while ((readIndex < writeCount) && (linearBuffer->index < (linearBuffer->size - 5)))
    {
        if ((trailZeros == 2) && ((read[readIndex] & 0xfc) == 0))
        {
            trailZeros          = 0;
            write[writeIndex++] = 3;
            linearBuffer->index++;   
        }

        write[writeIndex++] = read[readIndex];
        linearBuffer->index++;
        
        trailZeros = (read[readIndex]) ? 0 : trailZeros+1;
        readIndex++;
    }
    
}

// Prepare to write bits into bit stream buffer
void Xin266InitBitstream (
    xin_bs_struct *bitstream)
{
    bitstream->bitsLeft = 8;
    bitstream->bitCount = 0;
    bitstream->base[0]  = 0;
    bitstream->cur      = bitstream->base;
}

