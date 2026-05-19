/***************************************************************************//**
*
* @file          h265p_bit_stream.h
* @brief         This file declare Av1 low level bit stream manipulation functions.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_bit_stream_h_
#define _h265p_bit_stream_h_

void Xin265pWriteOneBit (
    xin_bs_struct *bitstream,
    UINT32        bit);

void Xin265pWriteBits (
    xin_bs_struct *bitstream,
    UINT32        bits,
    UINT32        length);

void Xin265pWriteSu (
    xin_bs_struct *bitstream,
    SINT32        bits,
    UINT32        length);

void Xin265pWriteUvlc (
    xin_bs_struct *bitstream,
    UINT32        bits);

void Xin265pWriteSvlc (
    xin_bs_struct *bitstream,
    SINT32        bits);

void Xin265pWriteFlush (
    xin_bs_struct *bitstream);

void Xin265pBitstreamSize (
    xin_bs_struct *bitstream,
    UINT32        *bitstreamSize);

void Xin265pOutputBitToLinearBuffer (
    xin_bs_struct *bitstream,
    UINT8         obuHeader,
    UINT8         *ulebValue,
    UINT32        ulebLength,
    xin_lb_struct *linearBuffer);

void Xin265pWriteByteAlign (
    xin_bs_struct *bitstream);

void Xin265pInitBitstream (
    xin_bs_struct *bitstream);

#endif

