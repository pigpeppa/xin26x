/***************************************************************************//**
*
* @file          h265p_bit_stream.c
* @brief         Av1 low level bit stream manipulation functions
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "h265p_bit_stream_struct.h"
#include "xin_video_common.h"
#include "memory.h"

// Add a single bit (flag) to the bitstream.
void Xin265pWriteOneBit (
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
void Xin265pWriteBits (
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

// Add a su code to the bitstream.
void Xin265pWriteSu (
    xin_bs_struct *bitstream,
    SINT32        bits,
    UINT32        length)
{
    UINT32  code;

    code = bits & ((1 << (length + 1)) - 1);

    Xin265pWriteBits (
        bitstream,
        code,
        bits+1);
}

// Add a uvlc code to the bitstream.
void Xin265pWriteUvlc (
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

    Xin265pWriteBits (
        bitstream,
        bits,
        length);

}

// Add a svlc code to the bitstream.
void Xin265pWriteSvlc (
    xin_bs_struct *bitstream,
    SINT32        bits)
{
    UINT32  code;

    code = (bits <= 0) ? -bits<<1 : (bits<<1) - 1;

    Xin265pWriteUvlc (
        bitstream,
        code);
}

void Xin265pWriteByteAlign (
    xin_bs_struct *bitstream)
{
    if (bitstream->bitsLeft & 0x7)
    {
        Xin265pWriteBits(
            bitstream,
            0,
            bitstream->bitsLeft);
    }
}

// Write a stop bit, byte alignment and flush the bit buffer
void Xin265pWriteFlush (
    xin_bs_struct *bitstream)
{
    Xin265pWriteOneBit (
        bitstream,
        1);

    if (bitstream->bitsLeft & 0x7)
    {
        Xin265pWriteBits(
            bitstream,
            0,
            bitstream->bitsLeft);
    }
}

// Calculate bit stream size.
void Xin265pBitstreamSize (
    xin_bs_struct *bitstream,
    UINT32        *bitstreamSize)
{
    UINT32  writeCount;

    writeCount = (UINT32)(bitstream->cur - bitstream->base);

    *bitstreamSize = writeCount;
}

// Output bit to linear buffer and add emualtion byte if necessary
void Xin265pOutputBitToLinearBuffer (
    xin_bs_struct *bitstream,
    UINT8         obuHeader,
    UINT8         *ulebValue,
    UINT32        ulebLength,
    xin_lb_struct *linearBuffer)
{
    UINT8   *write;
    UINT8   *read;
    UINT32  writeCount;
    UINT32  idx;

    write = ((UINT8 *)linearBuffer->base) + linearBuffer->index;

    if (ulebValue != NULL)
    {
        // Write obu header
        *write++ = obuHeader;
        linearBuffer->index++;

        // Write uleb
        for (idx = 0; idx < ulebLength; idx++)
        {
            *write++ = ulebValue[idx];
            linearBuffer->index++;
        }
    }

    if (bitstream != NULL)
    {
        read       = bitstream->base;
        writeCount = (UINT32)(bitstream->cur - bitstream->base);

        memcpy (write, read, writeCount);

        linearBuffer->index += writeCount;
    }

}

// Prepare to write bits into bit stream buffer
void Xin265pInitBitstream (
    xin_bs_struct *bitstream)
{
    bitstream->bitsLeft = 8;
    bitstream->bitCount = 0;
    bitstream->base[0]  = 0;
    bitstream->cur      = bitstream->base;
}

