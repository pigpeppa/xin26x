/***************************************************************************//**
 *
 * @file          h266_entropy_write.c
 * @brief         h266 syntax and coefficient writing.
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
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h26x_me_struct.h"
#include "h266_section_struct.h"
#include "video_macro.h"
#include "h266_bit_stream.h"
#include "h266_enc_init.h"
#include "h266_scan_order.h"
#include "h266_get_neighbour_mv.h"
#include "assert.h"

#define ENTROPY_WRITE_UPDATE(x)         {int a = XIN_ABS(x); sumAbs += XIN_MIN(4+(a&1), a); numPos+=!!a;}
#define ENTROPY_WRITE_UPDATE_TS(x)      {int a = XIN_ABS(x); numPos+=!!a;}
#define COEF_REMAIN_BIN_REDUCTION       5 ///< indicates the level at which the VLC transitions from Golomb-Rice to TU+EG(k)
#define CU_DQP_TU_CMAX                  5
#define CU_DQP_EGK                      0
#define MAX_TU_LEVEL_CTX_CODED_BIN      28

const UINT32 cabacbinFracBits[256][2] =
{
    { 0x0005c, 0x48000 }, { 0x00116, 0x3b520 }, { 0x001d0, 0x356cb }, { 0x0028b, 0x318a9 },
    { 0x00346, 0x2ea40 }, { 0x00403, 0x2c531 }, { 0x004c0, 0x2a658 }, { 0x0057e, 0x28beb },
    { 0x0063c, 0x274ce }, { 0x006fc, 0x26044 }, { 0x007bc, 0x24dc9 }, { 0x0087d, 0x23cfc },
    { 0x0093f, 0x22d96 }, { 0x00a01, 0x21f60 }, { 0x00ac4, 0x2122e }, { 0x00b89, 0x205dd },
    { 0x00c4e, 0x1fa51 }, { 0x00d13, 0x1ef74 }, { 0x00dda, 0x1e531 }, { 0x00ea2, 0x1db78 },
    { 0x00f6a, 0x1d23c }, { 0x01033, 0x1c970 }, { 0x010fd, 0x1c10b }, { 0x011c8, 0x1b903 },
    { 0x01294, 0x1b151 }, { 0x01360, 0x1a9ee }, { 0x0142e, 0x1a2d4 }, { 0x014fc, 0x19bfc },
    { 0x015cc, 0x19564 }, { 0x0169c, 0x18f06 }, { 0x0176d, 0x188de }, { 0x0183f, 0x182e8 },
    { 0x01912, 0x17d23 }, { 0x019e6, 0x1778a }, { 0x01abb, 0x1721c }, { 0x01b91, 0x16cd5 },
    { 0x01c68, 0x167b4 }, { 0x01d40, 0x162b6 }, { 0x01e19, 0x15dda }, { 0x01ef3, 0x1591e },
    { 0x01fcd, 0x15480 }, { 0x020a9, 0x14fff }, { 0x02186, 0x14b99 }, { 0x02264, 0x1474e },
    { 0x02343, 0x1431b }, { 0x02423, 0x13f01 }, { 0x02504, 0x13afd }, { 0x025e6, 0x1370f },
    { 0x026ca, 0x13336 }, { 0x027ae, 0x12f71 }, { 0x02894, 0x12bc0 }, { 0x0297a, 0x12821 },
    { 0x02a62, 0x12494 }, { 0x02b4b, 0x12118 }, { 0x02c35, 0x11dac }, { 0x02d20, 0x11a51 },
    { 0x02e0c, 0x11704 }, { 0x02efa, 0x113c7 }, { 0x02fe9, 0x11098 }, { 0x030d9, 0x10d77 },
    { 0x031ca, 0x10a63 }, { 0x032bc, 0x1075c }, { 0x033b0, 0x10461 }, { 0x034a5, 0x10173 },
    { 0x0359b, 0x0fe90 }, { 0x03693, 0x0fbb9 }, { 0x0378c, 0x0f8ed }, { 0x03886, 0x0f62b },
    { 0x03981, 0x0f374 }, { 0x03a7e, 0x0f0c7 }, { 0x03b7c, 0x0ee23 }, { 0x03c7c, 0x0eb89 },
    { 0x03d7d, 0x0e8f9 }, { 0x03e7f, 0x0e671 }, { 0x03f83, 0x0e3f2 }, { 0x04088, 0x0e17c },
    { 0x0418e, 0x0df0e }, { 0x04297, 0x0dca8 }, { 0x043a0, 0x0da4a }, { 0x044ab, 0x0d7f3 },
    { 0x045b8, 0x0d5a5 }, { 0x046c6, 0x0d35d }, { 0x047d6, 0x0d11c }, { 0x048e7, 0x0cee3 },
    { 0x049fa, 0x0ccb0 }, { 0x04b0e, 0x0ca84 }, { 0x04c24, 0x0c85e }, { 0x04d3c, 0x0c63f },
    { 0x04e55, 0x0c426 }, { 0x04f71, 0x0c212 }, { 0x0508d, 0x0c005 }, { 0x051ac, 0x0bdfe },
    { 0x052cc, 0x0bbfc }, { 0x053ee, 0x0b9ff }, { 0x05512, 0x0b808 }, { 0x05638, 0x0b617 },
    { 0x0575f, 0x0b42a }, { 0x05888, 0x0b243 }, { 0x059b4, 0x0b061 }, { 0x05ae1, 0x0ae83 },
    { 0x05c10, 0x0acaa }, { 0x05d41, 0x0aad6 }, { 0x05e74, 0x0a907 }, { 0x05fa9, 0x0a73c },
    { 0x060e0, 0x0a575 }, { 0x06219, 0x0a3b3 }, { 0x06354, 0x0a1f5 }, { 0x06491, 0x0a03b },
    { 0x065d1, 0x09e85 }, { 0x06712, 0x09cd4 }, { 0x06856, 0x09b26 }, { 0x0699c, 0x0997c },
    { 0x06ae4, 0x097d6 }, { 0x06c2f, 0x09634 }, { 0x06d7c, 0x09495 }, { 0x06ecb, 0x092fa },
    { 0x0701d, 0x09162 }, { 0x07171, 0x08fce }, { 0x072c7, 0x08e3e }, { 0x07421, 0x08cb0 },
    { 0x0757c, 0x08b26 }, { 0x076da, 0x089a0 }, { 0x0783b, 0x0881c }, { 0x0799f, 0x0869c },
    { 0x07b05, 0x0851f }, { 0x07c6e, 0x083a4 }, { 0x07dd9, 0x0822d }, { 0x07f48, 0x080b9 },
    { 0x080b9, 0x07f48 }, { 0x0822d, 0x07dd9 }, { 0x083a4, 0x07c6e }, { 0x0851f, 0x07b05 },
    { 0x0869c, 0x0799f }, { 0x0881c, 0x0783b }, { 0x089a0, 0x076da }, { 0x08b26, 0x0757c },
    { 0x08cb0, 0x07421 }, { 0x08e3e, 0x072c7 }, { 0x08fce, 0x07171 }, { 0x09162, 0x0701d },
    { 0x092fa, 0x06ecb }, { 0x09495, 0x06d7c }, { 0x09634, 0x06c2f }, { 0x097d6, 0x06ae4 },
    { 0x0997c, 0x0699c }, { 0x09b26, 0x06856 }, { 0x09cd4, 0x06712 }, { 0x09e85, 0x065d1 },
    { 0x0a03b, 0x06491 }, { 0x0a1f5, 0x06354 }, { 0x0a3b3, 0x06219 }, { 0x0a575, 0x060e0 },
    { 0x0a73c, 0x05fa9 }, { 0x0a907, 0x05e74 }, { 0x0aad6, 0x05d41 }, { 0x0acaa, 0x05c10 },
    { 0x0ae83, 0x05ae1 }, { 0x0b061, 0x059b4 }, { 0x0b243, 0x05888 }, { 0x0b42a, 0x0575f },
    { 0x0b617, 0x05638 }, { 0x0b808, 0x05512 }, { 0x0b9ff, 0x053ee }, { 0x0bbfc, 0x052cc },
    { 0x0bdfe, 0x051ac }, { 0x0c005, 0x0508d }, { 0x0c212, 0x04f71 }, { 0x0c426, 0x04e55 },
    { 0x0c63f, 0x04d3c }, { 0x0c85e, 0x04c24 }, { 0x0ca84, 0x04b0e }, { 0x0ccb0, 0x049fa },
    { 0x0cee3, 0x048e7 }, { 0x0d11c, 0x047d6 }, { 0x0d35d, 0x046c6 }, { 0x0d5a5, 0x045b8 },
    { 0x0d7f3, 0x044ab }, { 0x0da4a, 0x043a0 }, { 0x0dca8, 0x04297 }, { 0x0df0e, 0x0418e },
    { 0x0e17c, 0x04088 }, { 0x0e3f2, 0x03f83 }, { 0x0e671, 0x03e7f }, { 0x0e8f9, 0x03d7d },
    { 0x0eb89, 0x03c7c }, { 0x0ee23, 0x03b7c }, { 0x0f0c7, 0x03a7e }, { 0x0f374, 0x03981 },
    { 0x0f62b, 0x03886 }, { 0x0f8ed, 0x0378c }, { 0x0fbb9, 0x03693 }, { 0x0fe90, 0x0359b },
    { 0x10173, 0x034a5 }, { 0x10461, 0x033b0 }, { 0x1075c, 0x032bc }, { 0x10a63, 0x031ca },
    { 0x10d77, 0x030d9 }, { 0x11098, 0x02fe9 }, { 0x113c7, 0x02efa }, { 0x11704, 0x02e0c },
    { 0x11a51, 0x02d20 }, { 0x11dac, 0x02c35 }, { 0x12118, 0x02b4b }, { 0x12494, 0x02a62 },
    { 0x12821, 0x0297a }, { 0x12bc0, 0x02894 }, { 0x12f71, 0x027ae }, { 0x13336, 0x026ca },
    { 0x1370f, 0x025e6 }, { 0x13afd, 0x02504 }, { 0x13f01, 0x02423 }, { 0x1431b, 0x02343 },
    { 0x1474e, 0x02264 }, { 0x14b99, 0x02186 }, { 0x14fff, 0x020a9 }, { 0x15480, 0x01fcd },
    { 0x1591e, 0x01ef3 }, { 0x15dda, 0x01e19 }, { 0x162b6, 0x01d40 }, { 0x167b4, 0x01c68 },
    { 0x16cd5, 0x01b91 }, { 0x1721c, 0x01abb }, { 0x1778a, 0x019e6 }, { 0x17d23, 0x01912 },
    { 0x182e8, 0x0183f }, { 0x188de, 0x0176d }, { 0x18f06, 0x0169c }, { 0x19564, 0x015cc },
    { 0x19bfc, 0x014fc }, { 0x1a2d4, 0x0142e }, { 0x1a9ee, 0x01360 }, { 0x1b151, 0x01294 },
    { 0x1b903, 0x011c8 }, { 0x1c10b, 0x010fd }, { 0x1c970, 0x01033 }, { 0x1d23c, 0x00f6a },
    { 0x1db78, 0x00ea2 }, { 0x1e531, 0x00dda }, { 0x1ef74, 0x00d13 }, { 0x1fa51, 0x00c4e },
    { 0x205dd, 0x00b89 }, { 0x2122e, 0x00ac4 }, { 0x21f60, 0x00a01 }, { 0x22d96, 0x0093f },
    { 0x23cfc, 0x0087d }, { 0x24dc9, 0x007bc }, { 0x26044, 0x006fc }, { 0x274ce, 0x0063c },
    { 0x28beb, 0x0057e }, { 0x2a658, 0x004c0 }, { 0x2c531, 0x00403 }, { 0x2ea40, 0x00346 },
    { 0x318a9, 0x0028b }, { 0x356cb, 0x001d0 }, { 0x3b520, 0x00116 }, { 0x48000, 0x0005c },
};

const SINT32 amvrPrecision[4] =
{
    XIN_MV_PREC_QUARTER, XIN_MV_PREC_INT, XIN_MV_PREC_4PEL, XIN_MV_PREC_HALF
};

static const UINT32 bitShiftTable[] =
{
    6,  5,  4,  4,
    3,  3,  3,  3,
    2,  2,  2,  2,
    2,  2,  2,  2,
    1,  1,  1,  1,
    1,  1,  1,  1,
    1,  1,  1,  1,
    1,  1,  1,  1,
    0,  0,  0,  0,
    0,  0,  0,  0,
    0,  0,  0,  0,
    0,  0,  0,  0,
    0,  0,  0,  0,
    0,  0,  0,  0,
    0,  0,  0,  0,
    0,  0,  0,  0,
};

const UINT32 lastSigXYGroupIdx[] =
{
    0,  1,  2,  3,  4,  4,  5,  5,
    6,  6,  6,  6,  7,  7,  7,  7,
    8,  8,  8,  8,  8,  8,  8,  8,
    9,  9,  9,  9,  9,  9,  9,  9,
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11
};

static const UINT32 prefixCtx[8] =
{
    0, 0, 0, 3, 6, 10, 15, 21
};

static const UINT32 lastSigXYMinInGroup[] =
{
    0, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96
};

const UINT64 rgtGrpSigMask[6] =
{
    0x00,
    0x00,
    0x00,
    0x5555555555555555,
    0x7777777777777777,
    0x7F7F7F7F7F7F7F7F,
};

const UINT64 lftGrpSigMask[6] =
{
    0x00,
    0x00,
    0x00,
    0xAAAAAAAAAAAAAAAA,
    0xEEEEEEEEEEEEEEEE,
    0xF7F7F7F7F7F7F7F7,
};

const UINT32 goRiceParsCoeff[32] =
{
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3
};

const UINT8 tbMax[257] =
{
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8
};

static const UINT32 cbfCtxOffset[3] =
{
    XIN_CO_QT_CBF_Y,
    XIN_CO_QT_CBF_U,
    XIN_CO_QT_CBF_V
};

const UINT32 bcwCodingOrder[XIN_BCW_NUM] =
{
    4, 2, 0, 1, 3
};

static UINT8 Xin266GetLps (
    UINT32         range,
    xin_prob_model *context)
{
    UINT8 state;

    state = (context->state[0] + context->state[1]) >> 8;
    state = (state & 0x80) ? (state ^ 0xff) : state;
    state = (UINT8)((state >> 2)*(range >> 5) >> 1) + 4;

    return state;
}

static UINT8 Xin266GetMps (
    xin_prob_model *context)
{
    UINT8 state;

    state = (context->state[0] + context->state[1]) >> 8;
    state = state >> 7;

    return state;
}

static void Xin266StateUpdate (
    xin_prob_model *context,
    UINT32         bin)
{
    SINT32 rate0;
    SINT32 rate1;

    rate0 = context->rate >> 4;
    rate1 = context->rate & 15;

    context->state[0] -= (context->state[0] >> rate0) & XIN_CABAC_STATE_MASK_0;
    context->state[1] -= (context->state[1] >> rate1) & XIN_CABAC_STATE_MASK_1;

    if (bin)
    {
        context->state[0] += (0x7fffu >> rate0) & XIN_CABAC_STATE_MASK_0;
        context->state[1] += (0x7fffu >> rate1) & XIN_CABAC_STATE_MASK_1;
    }
}

static void Xin266CabacWriteOut (
    xin_cabac_struct *cabac)
{
    UINT32        leadByte;
    UINT32        carry;
    UINT32        tmpByte;
    xin_bs_struct *bitstream;

    if (cabac->bitsLeft < 12)
    {
        leadByte         = cabac->low >> (24 - cabac->bitsLeft);
        cabac->bitsLeft += 8;
        cabac->low      &= 0xffffffffu >> cabac->bitsLeft;

        if (leadByte == 0xff)
        {
            cabac->numBufferedBytes++;
        }
        else
        {
            if (cabac->numBufferedBytes > 0)
            {
                bitstream = &(cabac->bitstream);
                carry     = leadByte >> 8;
                tmpByte   = cabac->bufferedByte + carry;

                bitstream->bitCount += 8;
                *(bitstream->cur++)  = (UINT8)tmpByte;
                cabac->bufferedByte  = leadByte & 0xff;

                while ( cabac->numBufferedBytes > 1 )
                {
                    cabac->numBufferedBytes--;

                    *(bitstream->cur++)  = (0xff + carry) & 0xff;
                    bitstream->bitCount += 8;
                }
            }
            else
            {
                cabac->numBufferedBytes = 1;
                cabac->bufferedByte     = leadByte;
            }

        }

    }

}

static void Xin266EncodeBin (
    xin_cabac_struct *cabac,
    UINT32           binValue,
    xin_prob_model   *context)
{
    UINT32  lps;
    UINT32  shiftBits;
    UINT32  mps;

    // Get the context and LPS value.
    lps = Xin266GetLps (cabac->range, context);
    mps = Xin266GetMps (context);

    cabac->range -= lps;

    if (binValue != mps)
    {
        shiftBits = bitShiftTable[lps >> 3];

        cabac->bitsLeft -= shiftBits;
        cabac->low      += cabac->range;
        cabac->low     <<= shiftBits;
        cabac->range     = lps << shiftBits;

        Xin266CabacWriteOut (
            cabac);
    }
    else if (cabac->range < 256)
    {
        cabac->bitsLeft -= 1;
        cabac->low     <<= 1;
        cabac->range   <<= 1;

        Xin266CabacWriteOut (
            cabac);
    }

    Xin266StateUpdate (
        context,
        binValue);

}

static void Xin266EncodeBinEP (
    xin_cabac_struct *cabac,
    UINT32           binValue)
{
    cabac->low <<= 1;

    if (binValue)
    {
        cabac->low += cabac->range;
    }

    cabac->bitsLeft--;

    Xin266CabacWriteOut (
        cabac);
}

static void Xin266EncodeBinsEP (
    xin_cabac_struct *cabac,
    UINT32           binValues,
    UINT32           numBins)
{
    UINT32  pattern;

    while (numBins > 8)
    {
        numBins -= 8;
        pattern  = binValues >> numBins;

        cabac->low <<= 8;
        cabac->low += cabac->range*pattern;
        binValues  -= pattern << numBins;

        cabac->bitsLeft -= 8;

        Xin266CabacWriteOut (
            cabac);
    }

    cabac->low <<= numBins;
    cabac->low  += cabac->range*binValues;

    cabac->bitsLeft -= numBins;

    Xin266CabacWriteOut (
        cabac);

}

static void Xin266EncodeEGKBin (
    xin_cabac_struct *cabac,
    UINT32           value,
    UINT32           k)
{

    UINT32 bins;
    SINT32 numBins;

    bins    = 0;
    numBins = 0;

    while (value >= (UINT32)(1<<k))
    {
        bins = (bins << 1) + 1;
        numBins++;
        value -= 1 << k;
        k++;
    }

    bins = (bins << 1);
    numBins++;

    bins     = (bins << k) | value;
    numBins += k;

    Xin266EncodeBinsEP (
        cabac,
        bins,
        numBins);

}

void Xin266EncodeTerminate (
    xin_cabac_struct *cabac,
    UINT32           binValue)
{
    UINT32 shiftNum;

    cabac->range -= 2;

    if (binValue)
    {
        cabac->low  += cabac->range;
        cabac->range = 2;
        shiftNum     = 7;
    }
    else if (cabac->range >= 256)
    {
        return;
    }
    else
    {
        shiftNum = 1;
    }

    cabac->low       = cabac->low << shiftNum;
    cabac->range     = cabac->range << shiftNum;
    cabac->bitsLeft -= shiftNum;

    Xin266CabacWriteOut (
        cabac);

}

void Xin266EncodeFinish (
    xin_cabac_struct *cabac)
{
    xin_bs_struct *bitstream;

    bitstream = &cabac->bitstream;

    if (cabac->low >> (32 - cabac->bitsLeft))
    {
        bitstream->cur[0] = (cabac->bufferedByte + 1) & 0xff;
        bitstream->cur++;

        while (cabac->numBufferedBytes > 1)
        {
            bitstream->cur[0] = 0;
            bitstream->cur++;
            cabac->numBufferedBytes--;
        }

        cabac->low -= 1 << (32-cabac->bitsLeft);
    }
    else
    {
        if (cabac->numBufferedBytes > 0)
        {
            bitstream->cur[0] = (cabac->bufferedByte) & 0xff;
            bitstream->cur++;
        }

        while ( cabac->numBufferedBytes > 1 )
        {
            bitstream->cur[0] = 0xff;
            bitstream->cur++;
            cabac->numBufferedBytes--;
        }
    }

    bitstream->cur[0] = 0;

    Xin266WriteBits (
        &(cabac->bitstream),
        cabac->low >> 8,
        24 - cabac->bitsLeft);

    Xin266WriteFlush (
        &(cabac->bitstream));

}

static void Xin266EncodeRemAbsEP (
    xin_cabac_context *cabacSet,
    UINT32            symbolValue,
    UINT32            rice,
    UINT32            cutOff,
    UINT32            maxLog2TrDynRange)
{
    UINT32 length;
    UINT32 bitMask;
    UINT32 prefixLength;
    UINT32 suffixLength;
    UINT32 prefix;
    UINT32 suffix;
    UINT32 codeValue;
    UINT32 threshold;

    threshold  = cutOff << rice;

    if (symbolValue < threshold)
    {
        bitMask = (1 << rice) - 1;
        length  = (symbolValue >> rice) + 1;

        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            (1 << length) - 2,
            length);

        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            symbolValue & bitMask,
            rice);
    }
    else
    {
        length       = 32 - cutOff - maxLog2TrDynRange;
        prefixLength = 0;
        codeValue    = (symbolValue >> rice) - cutOff;

        if (codeValue >= (UINT32)((1 << length) - 1))
        {
            prefixLength = length;
            suffixLength = maxLog2TrDynRange;
        }
        else
        {
            while (codeValue > (UINT32)((2 << prefixLength) - 2))
            {
                prefixLength++;
            }

            suffixLength = prefixLength + rice + 1;
        }

        length  = prefixLength + cutOff;
        bitMask = (1 << rice) - 1;
        prefix  = (1 << length) - 1;
        suffix  = ((codeValue - ((1 << prefixLength) - 1)) << rice) | (symbolValue & bitMask);

        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            prefix,
            length);

        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            suffix,
            suffixLength);

    }

}

static void Xin266WriteTruncBinCode (
    xin_cabac_struct *cabacSet,
    SINT32           symbol,
    SINT32           maxSymbol)
{
    SINT32 thresh;
    SINT32 threshVal;
    SINT32 val;
    SINT32 b;

    if (maxSymbol > 256)
    {
        threshVal = 1 << 8;
        thresh    = 8;

        while (threshVal <= maxSymbol)
        {
            thresh++;
            threshVal <<= 1;
        }

        thresh--;
    }
    else
    {
        thresh = tbMax[maxSymbol];
    }

    val = 1 << thresh;
    b   = maxSymbol - val;

    if (symbol < val - b)
    {
        Xin266EncodeBinsEP (
            cabacSet,
            symbol,
            thresh);
    }
    else
    {
        symbol += val - b;

        Xin266EncodeBinsEP (
            cabacSet,
            symbol,
            thresh + 1);
    }

}

static void Xin266EncodeMaxUvlc (
    xin_cabac_struct *cabac,
    UINT32            value,
    UINT32            maxSymbol)
{
    UINT32 idx;

    if (value == 0)
    {
        Xin266EncodeBinEP (
            cabac,
            0);
    }
    else
    {
        Xin266EncodeBinEP (
            cabac,
            1);

        for (idx = 0; idx < value - 1; idx++)
        {
            Xin266EncodeBinEP (
                cabac,
                1);
        }

        if (maxSymbol > value)
        {
            Xin266EncodeBinEP (
                cabac,
                0);
        }
    }

}

static void Xin266WriteSaoOffsets (
    xin_cabac_context *cabacSet,
    xin_ctu_struct    *ctu,
    UINT32            bitDepth,
    UINT32            planeIdx)
{
    SINT32  saoType;
    SINT8   *offset;
    SINT32  maxSaoOffset;

    offset       = ctu->saoOffset[planeIdx];
    maxSaoOffset = (1 << (bitDepth - XIN_NUM_SAO_BO_CLASSES_LOG2)) - 1;

    if (planeIdx != PLANE_CHROMA_V)
    {
        saoType = ctu->saoType[planeIdx];

        Xin266EncodeBin (
            &(cabacSet->cabac),
            saoType >= 0,
            cabacSet->context + XIN_CO_SAO_TYPE_IDX);

        if (saoType >= 0)
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                (saoType < XIN_SAO_BO) ? 1 : 0);
        }

    }
    else
    {
        saoType = ctu->saoType[1];
    }

    if (saoType >= 0)
    {
        if (saoType == XIN_SAO_BO)
        {
            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                XIN_ABS(offset[0]),
                maxSaoOffset);

            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                XIN_ABS(offset[1]),
                maxSaoOffset);

            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                XIN_ABS(offset[2]),
                maxSaoOffset);

            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                XIN_ABS(offset[3]),
                maxSaoOffset);

            if (offset[0] != 0)
            {
                Xin266EncodeBinEP (
                    &(cabacSet->cabac),
                    (offset[0] < 0)? 1 : 0);
            }

            if (offset[1] != 0)
            {
                Xin266EncodeBinEP (
                    &(cabacSet->cabac),
                    (offset[1] < 0)? 1 : 0);
            }

            if (offset[2] != 0)
            {
                Xin266EncodeBinEP (
                    &(cabacSet->cabac),
                    (offset[2] < 0)? 1 : 0);
            }

            if (offset[3] != 0)
            {
                Xin266EncodeBinEP (
                    &(cabacSet->cabac),
                    (offset[3] < 0)? 1 : 0);
            }

            // code band position
            Xin266EncodeBinsEP (
                &(cabacSet->cabac),
                ctu->saoBandPos[planeIdx],
                5);

        }
        else
        {
            // code the offset values
            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                offset[0],
                maxSaoOffset);

            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                offset[1],
                maxSaoOffset);

            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                (UINT32)-offset[3],
                maxSaoOffset);

            Xin266EncodeMaxUvlc (
                &(cabacSet->cabac),
                (UINT32)-offset[4],
                maxSaoOffset);

            // Derive EO type from sao type
            // 0: EO - 0 degrees
            // 1: EO - 90 degrees
            // 2: EO - 135 degrees
            // 3: EO - 45 degrees
            if (planeIdx != PLANE_CHROMA_V)
            {
                Xin266EncodeBinsEP (
                    &(cabacSet->cabac),
                    saoType,
                    2);
            }

        }
    }

}

static void Xin266WriteSaoMerge(
    xin_cabac_context *cabacSet,
    BOOL              mergeFlag)
{
    Xin266EncodeBin (
        &(cabacSet->cabac),
        mergeFlag,
        cabacSet->context + XIN_CO_SAO_MERGE_FLAG);
}

void Xin266WriteSaoParam (
    xin_cabac_context *cabacSet,
    BOOL              saoEnabled,
    UINT32            bitDepth,
    xin_ctu_struct    *ctu)
{
    if (!saoEnabled)
    {
        return;
    }

    if (ctu->availField & XIN_LFT_CTU_AVAIL)
    {
        Xin266WriteSaoMerge (
            cabacSet,
            ctu->saoMergeLftFlag);
    }
    else
    {
        ctu->saoMergeLftFlag = 0;
    }

    if (ctu->saoMergeLftFlag == 0)
    {
        if (ctu->availField & XIN_TOP_CTU_AVAIL)
        {
            Xin266WriteSaoMerge (
                cabacSet,
                ctu->saoMergeTopFlag);
        }
        else
        {
            ctu->saoMergeTopFlag = 0;
        }
    }

    if ((ctu->saoMergeLftFlag == 0) && (ctu->saoMergeTopFlag == 0))
    {
        // Luma
        if (saoEnabled)
        {
            Xin266WriteSaoOffsets (
                cabacSet,
                ctu,
                bitDepth,
                PLANE_LUMA);

            Xin266WriteSaoOffsets (
                cabacSet,
                ctu,
                bitDepth,
                PLANE_CHROMA_U);

            Xin266WriteSaoOffsets (
                cabacSet,
                ctu,
                bitDepth,
                PLANE_CHROMA_V);

        }

    }

}

static void XinGetAlfCtuFlagCtxIdx (
    UINT8   *ctuEnableFlag,
    UINT32  ctuAvialField,
    SINT32  frameWidthInCtu,
    UINT32  *ctxInc)
{

    *ctxInc = 0;

    if (ctuAvialField & XIN_LFT_CTU_AVAIL)
    {
        *ctxInc += ctuEnableFlag[-1];
    }

    if (ctuAvialField & XIN_TOP_CTU_AVAIL)
    {
        *ctxInc += ctuEnableFlag[-frameWidthInCtu];
    }

}

static void Xin266WriteAlfCtbFlag (
    xin_cabac_context *cabacSet,
    xin_alf_struct    *alfSet,
    UINT32            ctuIdx,
    UINT32            planeIdx)
{
    UINT32 ctxInc;

    XinGetAlfCtuFlagCtxIdx (
        alfSet->ctuEnableFlag[planeIdx] + ctuIdx,
        alfSet->ctu[ctuIdx].availField,
        alfSet->frameWidthInCtu,
        &ctxInc);

    Xin266EncodeBin (
        &(cabacSet->cabac),
        alfSet->ctuEnableFlag[planeIdx][ctuIdx],
        cabacSet->context + planeIdx*3 + XIN_CO_CTB_ALF_FLAG + ctxInc);
}

static void Xin266WriteAlfUseApsFlag (
    xin_cabac_context *cabacSet,
    BOOL              useApsFlag)
{
    Xin266EncodeBin (
        &(cabacSet->cabac),
        useApsFlag,
        cabacSet->context + XIN_CO_USE_TEMPO_FLT);
}

void Xin266WriteAlfCtuAlt (
    xin_cabac_context *cabacSet,
    SINT32            numAlts,
    SINT32            ctbAlfAlt,
    SINT32            compIdx)
{
    SINT32  idx;

    for (idx = 0; idx < ctbAlfAlt; ++idx)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            1,
            cabacSet->context + XIN_CO_CTB_ALF_ALT + compIdx - 1);
    }

    if (ctbAlfAlt < numAlts - 1)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            0,
            cabacSet->context + XIN_CO_CTB_ALF_ALT + compIdx - 1);
    }

}

static void XinGetCcAlfFltCtrlIdcCtxIdx (
    UINT8   *filterControlIdc,
    UINT32  ctuAvialField,
    SINT32  frameWidthInCtu,
    SINT32  compId,
    UINT32  *ctxInc)
{

    *ctxInc = 0;

    if (ctuAvialField & XIN_LFT_CTU_AVAIL)
    {
        *ctxInc += ( filterControlIdc[- 1]) ? 1 : 0;
    }

    if (ctuAvialField & XIN_TOP_CTU_AVAIL)
    {
        *ctxInc += (filterControlIdc[-frameWidthInCtu]) ? 1 : 0;
    }

    *ctxInc += ( compId == PLANE_CHROMA_V ) ? 3 : 0;

}

void Xin266WriteCcAlfFltCtrlIdc (
    xin_cabac_context *cabacSet,
    SINT32            ctxInc,
    UINT32            idcVal,
    UINT32            filterCount)
{

    Xin266EncodeBin (
        &(cabacSet->cabac),
        idcVal != 0,
        cabacSet->context + XIN_CO_CC_ALF_FLT_CTRL_FLAG + ctxInc); // ON/OFF flag is context coded

    if (idcVal > 0)
    {
        int val = (idcVal - 1);

        while ( val )
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                val != 0);

            val--;
        }

        if ( idcVal < filterCount )
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                0);
        }
    }

}

void Xin266WriteAlfParam (
    xin_cabac_context *cabacSet,
    xin_alf_struct    *alfSet,
    BOOL              alfEnabled,
    UINT32            ctuIdx)
{
    BOOL    useApsFlag;
    SINT32  compId;
    UINT32  ctxInc;
    BOOL    ccAlfEnabled;

    if (!alfEnabled)
    {
        return;
    }

    if (alfSet->alfEnabled[PLANE_LUMA])
    {
        Xin266WriteAlfCtbFlag (
            cabacSet,
            alfSet,
            ctuIdx,
            PLANE_LUMA);

        if (alfSet->ctuEnableFlag[PLANE_LUMA][ctuIdx])
        {
            useApsFlag = alfSet->alfCtbFilterSetIndex[ctuIdx] == XIN_ALF_FIXED_FLT_SET_NUM;

            if (alfSet->apsNum)
            {
                Xin266WriteAlfUseApsFlag (
                    cabacSet,
                    useApsFlag);
            }

            if (!useApsFlag)
            {
                Xin266WriteTruncBinCode (
                    &cabacSet->cabac,
                    alfSet->alfCtbFilterSetIndex[ctuIdx],
                    XIN_ALF_FIXED_FLT_SET_NUM);
            }

        }

    }

    if (alfSet->alfEnabled[PLANE_CHROMA_U])
    {
        Xin266WriteAlfCtbFlag (
            cabacSet,
            alfSet,
            ctuIdx,
            PLANE_CHROMA_U);

        if (alfSet->ctuEnableFlag[PLANE_CHROMA_U][ctuIdx])
        {
            Xin266WriteAlfCtuAlt (
                cabacSet,
                alfSet->alfAps[0].alfParam.numAltChroma,
                alfSet->ctuAlternative[PLANE_CHROMA_U][ctuIdx],
                PLANE_CHROMA_U);
        }
    }

    if (alfSet->alfEnabled[PLANE_CHROMA_V])
    {
        Xin266WriteAlfCtbFlag (
            cabacSet,
            alfSet,
            ctuIdx,
            PLANE_CHROMA_V);

        if (alfSet->ctuEnableFlag[PLANE_CHROMA_V][ctuIdx])
        {
            Xin266WriteAlfCtuAlt (
                cabacSet,
                alfSet->alfAps[0].alfParam.numAltChroma,
                alfSet->ctuAlternative[PLANE_CHROMA_V][ctuIdx],
                PLANE_CHROMA_V);
        }
    }

    for (compId = 1; compId < PLANE_NUM; compId++)
    {
        ccAlfEnabled = compId == PLANE_CHROMA_U ? alfSet->ccAlfCbEnabled : alfSet->ccAlfCrEnabled;

        if (ccAlfEnabled)
        {
            XinGetCcAlfFltCtrlIdcCtxIdx(
                alfSet->ccAlfFilterControl[compId - 1] + ctuIdx,
                alfSet->ctu[ctuIdx].availField,
                alfSet->frameWidthInCtu,
                compId,
                &ctxInc);

            Xin266WriteCcAlfFltCtrlIdc(
                cabacSet,
                ctxInc,
                alfSet->ccAlfFilterControl[compId - 1][ctuIdx],
                alfSet->alfAps[0].ccAlfParam.ccAlfFilterCount[compId - 1]);
        }
    }

}

static void Xin266WriteSplitType (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu,
    UINT32            splitType)
{
    UINT32  canQt;
    UINT32  canBtt;
    UINT32  canSplit;
    UINT32  canNoSplit;
    UINT32  canHor;
    UINT32  canVer;
    BOOL    isVer;
    BOOL    is12;
    UINT32  can14;
    UINT32  can12;

    canSplit   = cu->canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT | XIN_CAN_QUAD_SPLIT);
    canNoSplit = cu->canSplit & XIN_CAN_NO_SPLIT;

    if (canNoSplit && canSplit)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            splitType != XIN_CU_NO_SPLIT,
            cabacSet->context + XIN_CO_SPLIT_FLAG + cu->splitCtx);
    }

    if (splitType == XIN_CU_NO_SPLIT)
    {
        return;
    }

    canQt  = canSplit & XIN_CAN_QUAD_SPLIT;
    canBtt = canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT);

    if (canQt && canBtt)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            splitType == XIN_CU_QUAD_SPLIT,
            cabacSet->context + XIN_CO_SPLIT_QT_FLAG + cu->qtCtx);
    }

    if (splitType == XIN_CU_QUAD_SPLIT)
    {
        return;
    }

    canHor = canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_TRIH_SPLIT);
    canVer = canSplit & (XIN_CAN_VERT_SPLIT | XIN_CAN_TRIV_SPLIT);
    isVer  = splitType == XIN_CU_VERT_SPLIT || splitType == XIN_CU_TRIV_SPLIT;

    if (canVer && canHor)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            isVer,
            cabacSet->context + XIN_CO_SPLIT_HV_FLAG + cu->hvCtx);
    }

    can14 = isVer ? canSplit & XIN_CAN_TRIV_SPLIT : canSplit & XIN_CAN_TRIH_SPLIT;
    can12 = isVer ? canSplit & XIN_CAN_VERT_SPLIT : canSplit & XIN_CAN_HORZ_SPLIT;
    is12  = isVer ? (splitType == XIN_CU_VERT_SPLIT) : (splitType == XIN_CU_HORZ_SPLIT);

    if (can12 && can14)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            is12,
            cabacSet->context + XIN_CO_SPLIT_12_FLAG + (isVer ? cu->verBtCtx : cu->horBtCtx));
    }

}

static void Xin266WriteSkipFlag (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu,
    UINT32            skipFlag)
{
    if ((cu->width == 4) && (cu->height == 4))
    {
        return;
    }

    Xin266EncodeBin (
        &(cabacSet->cabac),
        skipFlag,
        cabacSet->context + XIN_CO_SKIP_FLAG + cu->skipContext);
}

static void Xin266WritePredMode (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu,
    UINT32            predMode)
{
    Xin266EncodeBin (
        &(cabacSet->cabac),
        predMode,
        cabacSet->context + XIN_CO_PRED_MODE + cu->predModeContext);
}

static void Xin266WriteMergeFlag (
    xin_cabac_context *cabacSet,
    UINT32            mergeFlag)
{
    Xin266EncodeBin (
        &(cabacSet->cabac),
        mergeFlag,
        cabacSet->context + XIN_CO_MERGE_FLAG);
}

static void Xin266WriteSubblockMergeFlag (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu,
    UINT32            subMergeFlag)
{

    Xin266EncodeBin (
        &(cabacSet->cabac),
        subMergeFlag,
        cabacSet->context + cu->affineContext + XIN_CO_SUBBLOCK_MERGE_FLAG);
}

void Xin266WriteMergeData (
    xin_cabac_context *cabacSet,
    xin_pu_struct     *pu,
    UINT32            maxMergeCand)
{
    UINT32  mergeIndex;
    UINT32  mergeMask;

    mergeIndex = pu->mergeIndex;

    if (maxMergeCand > 1)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            mergeIndex != 0,
            cabacSet->context + XIN_CO_MERGE_IDX);

        if (mergeIndex != 0)
        {
            mergeMask = (1 << mergeIndex) - 2;

            if (mergeIndex + 1 == maxMergeCand)
            {
                mergeMask = mergeMask >> 1;
            }

            Xin266EncodeBinsEP (
                &cabacSet->cabac,
                mergeMask,
                mergeIndex - (mergeIndex + 1 == maxMergeCand));
        }

    }

}

static void Xin266WriteMvd (
    xin_cabac_context *cabacSet,
    xin_mv32_u        *mvd,
    UINT8             imvIdx)
{
    SINT32     mvdX;
    SINT32     mvdY;
    UINT32     xSign;
    UINT32     ySign;

    UINT32     absMvdX;
    UINT32     absMvdY;

    UINT32     mvdXneq0;
    UINT32     mvdYneq0;

    UINT32     absmvdXgt1;
    UINT32     absmvdYgt1;

    Xin266ChangeMv32Prec (
        mvd,
        XIN_MV_PREC_INTERNAL,
        amvrPrecision[imvIdx]);

    mvdX = mvd->mv.mv32X;
    mvdY = mvd->mv.mv32Y;

    xSign = (0 > mvdX);
    ySign = (0 > mvdY);

    absMvdX = XIN_ABS(mvdX);
    absMvdY = XIN_ABS(mvdY);

    mvdXneq0 = (mvdX != 0);
    mvdYneq0 = (mvdY != 0);

    absmvdXgt1 = (absMvdX > 1);
    absmvdYgt1 = (absMvdY > 1);

    Xin266EncodeBin (
        &cabacSet->cabac,
        mvdXneq0,
        cabacSet->context + XIN_CO_MVD);

    Xin266EncodeBin (
        &cabacSet->cabac,
        mvdYneq0,
        cabacSet->context + XIN_CO_MVD);

    if (mvdXneq0)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            absmvdXgt1,
            cabacSet->context + XIN_CO_MVD + 1);
    }

    if (mvdYneq0)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            absmvdYgt1,
            cabacSet->context + XIN_CO_MVD + 1);
    }

    switch (mvdXneq0)
    {
    case 1:
        switch (absmvdXgt1)
        {
        case 1:
            Xin266EncodeEGKBin(
                &cabacSet->cabac,
                (SINT32)absMvdX - 2,
                1);
        case 0:
            // Sign of mvdX
            Xin266EncodeBinEP (
                &cabacSet->cabac,
                xSign);

        default:
            break;
        }

    case 0:
    default:
        break;

    }

    switch (mvdYneq0)
    {
    case 1:
        switch (absmvdYgt1)
        {
        case 1:
            Xin266EncodeEGKBin (
                &cabacSet->cabac,
                (SINT32)absMvdY - 2,
                1);
        case 0:
            // Sign of mvdY
            Xin266EncodeBinEP (
                &cabacSet->cabac,
                ySign);
        default:
            break;
        }

    case 0:
    default:
        break;

    }

}

static void Xin266WriteAffineMvd (
    xin_cabac_context *cabacSet,
    SINT32            listIdx,
    xin_cu_struct     *cu)
{
    UINT32          mvIdx;
    UINT32          mvNum;
    xin_mode_struct *modeCtrl;
    xin_pu_struct   *pu;
    xin_mv32_u      mvd[3];
    UINT8           imvIdx;

    modeCtrl = cu->modeCtrl;
    pu       = &cu->pu;
    mvNum    = pu->affineType ? 3 : 2;
    imvIdx   = pu->imvIdx;

    for (mvIdx = 0; mvIdx < mvNum; mvIdx++)
    {
        mvd[mvIdx].mv.mv32X = modeCtrl->affineMv[listIdx][mvIdx].mv.mv32X - modeCtrl->affinePredMv[listIdx][mvIdx].mv.mv32X;
        mvd[mvIdx].mv.mv32Y = modeCtrl->affineMv[listIdx][mvIdx].mv.mv32Y - modeCtrl->affinePredMv[listIdx][mvIdx].mv.mv32Y;

        if (mvIdx)
        {
            mvd[mvIdx].mv.mv32X = mvd[mvIdx].mv.mv32X - mvd[0].mv.mv32X;
            mvd[mvIdx].mv.mv32Y = mvd[mvIdx].mv.mv32Y - mvd[0].mv.mv32Y;
        }

        Xin266WriteMvd (
            cabacSet,
            mvd + mvIdx,
            imvIdx);
    }

}

static void Xin266WriteNormalMvd (
    xin_cabac_context *cabacSet,
    SINT32            listIdx,
    xin_cu_struct     *cu)
{
    xin_pu_struct   *pu;
    xin_mv32_u      mvd;
    UINT8           imvIdx;

    pu     = &cu->pu;
    imvIdx = pu->imvIdx;

    mvd.mv.mv32X = pu->mv[listIdx].mv.mv32X - pu->predMv[listIdx].mv.mv32X;
    mvd.mv.mv32Y = pu->mv[listIdx].mv.mv32Y - pu->predMv[listIdx].mv.mv32Y;

    Xin266WriteMvd (
        cabacSet,
        &mvd,
        imvIdx);

}

static inline void Xin266FindLastSigPos (
    UINT16       *sigCoefMap,
    UINT32       lgCgWidth,
    UINT32       lgCgHeight,
    UINT64       sigGrpMap,
    xin_scan_pos *scanOrder,
    xin_scan_pos *scanOrderCg,
    SINT32       *lastSigPosX,
    SINT32       *lastSigPosY,
    SINT32       *scanPosLast)
{
    UINT32  blockIdx;
    UINT32  coeffIdx;
    UINT32  blockX;
    UINT32  blockY;
    UINT16  sigMap;

    BIT_SCAN_REVERSE_64(sigGrpMap, blockIdx);

    blockX = scanOrderCg[blockIdx].posX;
    blockY = scanOrderCg[blockIdx].posY;
    sigMap = *(sigCoefMap + blockIdx);

    BIT_SCAN_REVERSE_32(sigMap, coeffIdx);

    *lastSigPosX = scanOrder[coeffIdx].posX;
    *lastSigPosY = scanOrder[coeffIdx].posY;

    *lastSigPosX += blockX << lgCgWidth;
    *lastSigPosY += blockY << lgCgHeight;
    *scanPosLast  = coeffIdx + (blockIdx << (lgCgWidth + lgCgHeight));

}

void Xin266GetSigCxtIdx (
    COEFF    *qCoeff,
    intptr_t stride,
    UINT32   compType,
    SINT32   posX,
    SINT32   posY,
    SINT32   width,
    SINT32   height,
    SINT32   *sumAbs1,
    UINT32   *sigCxtIdx,
    SINT32   state)
{
    SINT32   numPos;
    SINT32   sumAbs;
    COEFF    *coeff;
    SINT32   diag;
    SINT32   ctxOffset;

    numPos = 0;
    sumAbs = 0;
    diag   = posX + posY;
    coeff  = qCoeff + posX + posY*stride;
    state  = XIN_MAX (0, state - 1);

    if (posX < width - 1)
    {
        ENTROPY_WRITE_UPDATE (coeff[1]);

        if (posX < width - 2)
        {
            ENTROPY_WRITE_UPDATE (coeff[2]);
        }

        if (posY < height - 1)
        {
            ENTROPY_WRITE_UPDATE (coeff[stride + 1]);
        }
    }

    if (posY < height - 1)
    {
        ENTROPY_WRITE_UPDATE (coeff[stride]);

        if (posY < height - 2)
        {
            ENTROPY_WRITE_UPDATE (coeff[stride << 1]);
        }
    }

    ctxOffset = XIN_MIN ((sumAbs+1) >> 1, 3) + (diag < 2 ? 4 : 0);

    if (compType == PLANE_LUMA)
    {
        ctxOffset += diag < 5 ? 4 : 0;
    }


    *sumAbs1   = sumAbs - numPos;
    ctxOffset += compType ? state*XIN_NUM_SIG_FLAG_STATE_CHROMA_CTX : state*XIN_NUM_SIG_FLAG_STATE_LUMA_CTX;
    *sigCxtIdx = ctxOffset;

}

void Xin266GetSigCxtIdxTs (
    COEFF    *qCoeff,
    intptr_t stride,
    SINT32   posX,
    SINT32   posY,
    UINT32   *sigCxtIdx)
{
    SINT32   numPos;
    COEFF    *coeff;

    numPos = 0;
    coeff  = qCoeff + posX + posY*stride;

    if (posX > 0)
    {
        ENTROPY_WRITE_UPDATE_TS (coeff[-1]);
    }

    if (posY > 0)
    {
        ENTROPY_WRITE_UPDATE_TS (coeff[-stride]);
    }

    *sigCxtIdx = numPos;

}

void Xin266SignCtxIdTs (
    COEFF    *qCoeff,
    intptr_t stride,
    SINT32   posX,
    SINT32   posY,
    UINT32   *signCxtIdx)
{
    COEFF    *coeff;
    SINT32  signRgt;
    SINT32  signDwn;
    UINT32  signCtx;

    coeff   = qCoeff + posX + posY*stride;
    signRgt = 0;
    signDwn = 0;

    if (posX > 0)
    {
        signRgt = coeff[-1];
    }

    if (posY > 0)
    {
        signDwn = coeff[-stride];
    }

    if ((signRgt == 0 && signDwn == 0) || ((signRgt*signDwn) < 0))
    {
        signCtx = 0;
    }
    else if (signRgt >= 0 && signDwn >= 0)
    {
        signCtx = 1;
    }
    else
    {
        signCtx = 2;
    }

    *signCxtIdx = signCtx;

}

void Xin266GetGtxCxtIdx (
    SINT32        compType,
    SINT32        coeffDiag,
    UINT32        sumAbs1,
    UINT32        *gtxCxtIdx)
{
    UINT32 ctxOffset;

    ctxOffset = 0;

    if (coeffDiag != -1)
    {
        ctxOffset  = XIN_MIN (sumAbs1, 4) + 1;
        ctxOffset += (!coeffDiag ? (compType == PLANE_LUMA ? 15 : 5) : compType == PLANE_LUMA ? coeffDiag < 3 ? 10 : ( coeffDiag < 10 ? 5 : 0 ) : 0 );
    }

    *gtxCxtIdx = ctxOffset;

}

static void Xin266EncodeLastSigXY(
    xin_cabac_context  *cabacSet,
    UINT32             lastSigXPos,
    UINT32             lastSigYPos,
    const UINT32       lgWidth,
    const UINT32       lgHeight,
    UINT32             compType)
{
    UINT32 xGroupIdx;
    UINT32 yGroupIdx;
    UINT32 maxGrpIdxX;
    UINT32 maxGrpIdxY;
    SINT32 shiftX;
    SINT32 shiftY;
    UINT32 cxtIdx;
    SINT32 index, groupCount;
    UINT32 cxtOffX;
    UINT32 cxtOffY;

    if (compType == PLANE_LUMA)
    {
        shiftX = (lgWidth + 1) >> 2;
        shiftY = (lgHeight + 1) >> 2;

        cxtOffX = prefixCtx[lgWidth] + XIN_CO_LAST_X_LUMA;
        cxtOffY = prefixCtx[lgHeight] + XIN_CO_LAST_Y_LUMA;
    }
    else
    {
        shiftX = XIN_CLIP ((1 << lgWidth)  >> 3, 0, 2);
        shiftY = XIN_CLIP ((1 << lgHeight) >> 3, 0, 2);

        cxtOffX = XIN_CO_LAST_X_CHROMA;
        cxtOffY = XIN_CO_LAST_Y_CHROMA;
    }

    maxGrpIdxX = lastSigXYGroupIdx[XIN_MIN(1 << lgWidth, 32) - 1];
    maxGrpIdxY = lastSigXYGroupIdx[XIN_MIN(1 << lgHeight, 32) - 1];
    xGroupIdx  = lastSigXYGroupIdx[lastSigXPos];
    yGroupIdx  = lastSigXYGroupIdx[lastSigYPos];

    // X position
    for (cxtIdx = 0; cxtIdx < xGroupIdx; cxtIdx++)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            1,
            cabacSet->context + cxtOffX + (cxtIdx>>shiftX));
    }

    if (xGroupIdx < maxGrpIdxX)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            0,
            cabacSet->context + cxtOffX + (cxtIdx >> shiftX));
    }

    // Y position
    for (cxtIdx = 0; cxtIdx < yGroupIdx; cxtIdx++)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            1,
            cabacSet->context + cxtOffY + (cxtIdx >> shiftY));
    }

    if (yGroupIdx < maxGrpIdxY)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            0,
            cabacSet->context + cxtOffY + (cxtIdx >> shiftY));
    }

    if (xGroupIdx > 3)
    {
        groupCount  = (xGroupIdx - 2 ) >> 1;
        lastSigXPos = lastSigXPos - lastSigXYMinInGroup[xGroupIdx];

        for (index = groupCount - 1; index >= 0; index--)
        {

            Xin266EncodeBinEP(
                &(cabacSet->cabac),
                (lastSigXPos >> index) & 1);
        }
    }

    if (yGroupIdx > 3)
    {
        groupCount  = (yGroupIdx - 2) >> 1;
        lastSigYPos = lastSigYPos - lastSigXYMinInGroup[yGroupIdx];

        for (index = groupCount - 1; index >= 0; index--)
        {
            Xin266EncodeBinEP(
                &(cabacSet->cabac),
                (lastSigYPos >> index) & 1);
        }
    }

}

void Xin266GetAbsSum (
    COEFF    *qCoeff,
    SINT32   posX,
    SINT32   posY,
    SINT32   width,
    SINT32   height,
    intptr_t stride,
    SINT32   baseLevel,
    UINT32   *absSum)
{
    COEFF   *coeff;
    SINT32  sum;

    sum   = 0;
    coeff = qCoeff + posX + posY*stride;

    if (posX < width - 1)
    {
        sum += XIN_ABS (coeff[1]);

        if (posX < width - 2)
        {
            sum += XIN_ABS (coeff[2]);
        }

        if (posY < height - 1)
        {
            sum += XIN_ABS (coeff[stride + 1]);
        }
    }

    if (posY < height - 1)
    {
        sum += XIN_ABS (coeff[stride]);

        if (posY < height - 2)
        {
            sum += XIN_ABS (coeff[stride << 1]);
        }
    }

    *absSum = XIN_CLIP (sum - 5 * baseLevel, 0, 31);

}

static void Xin266WriteRootCbf (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu)
{
    Xin266EncodeBin (
        &cabacSet->cabac,
        cu->rootCbf,
        cabacSet->context + XIN_CO_QT_ROOT_CBF);
}

static void Xin266WriteGrpCoeff (
    xin_coeff_context *coeffCtx,
    xin_scan_pos      *scanOrder,
    xin_cabac_context *cabacSet,
    COEFF             *coeff,
    intptr_t          coeffStride,
    UINT16            gt0CoefMap,
    SINT32            stateTransTable,
    SINT32            *state)
{
    SINT32       minSubIdx;
    SINT32       firstSigIdx;
    SINT32       scanIdx;
    SINT32       firstPosMode2;
    SINT32       nextSigIdx;
    SINT32       remRegBins;
    COEFF        coeffVal;
    BOOL         isLast;
    BOOL         gt0Flag;
    BOOL         gt1Flag;
    BOOL         gt2Flag;
    SINT32       posX, posY;
    SINT32       innerIdx;
    SINT32       subSetMask;
    SINT32       numNonZero;
    SINT32       inferSigIdx;
    UINT32       sigCtxIdx;
    SINT32       sumAbs1;
    UINT32       compType;
    UINT32       gtxCtxIdx;
    SINT32       remAbsLevel;
    UINT32       signPattern;
    UINT32       ricePar;
    UINT32       sumAll;
    SINT32       absLevel;
    SINT32       remPar;
    SINT32       pos0Par;
    SINT32       firstNZPosInCG;
    SINT32       lastNZPosInCG;
    BOOL         sbhFlag;

    minSubIdx   = coeffCtx->minSubIdx;
    isLast      = (coeffCtx->lastScanIdx >> coeffCtx->lgCGSize) == coeffCtx->subCGIdx;
    firstSigIdx = isLast ? coeffCtx->lastScanIdx : coeffCtx->maxSubIdx;
    nextSigIdx  = firstSigIdx;
    remRegBins  = coeffCtx->regBinLimit;
    subSetMask  = (1 << coeffCtx->lgCGSize) - 1;
    numNonZero  = 0;
    inferSigIdx = !isLast ? (coeffCtx->subCGIdx ? minSubIdx : -1) : nextSigIdx;
    compType    = coeffCtx->planeIdx != PLANE_LUMA;
    signPattern = 0;
    ricePar     = 0;
    sumAbs1     = coeffCtx->sumAbs1;

    if ((!isLast) && (coeffCtx->subCGIdx != 0))
    {
        if (gt0CoefMap)
        {
            Xin266EncodeBin (
                &cabacSet->cabac,
                1,
                cabacSet->context + coeffCtx->sigGrpCtx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA);
        }
        else
        {
            Xin266EncodeBin (
                &cabacSet->cabac,
                0,
                cabacSet->context + coeffCtx->sigGrpCtx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA);

            return;
        }
    }

    for ( ; nextSigIdx >= minSubIdx && remRegBins >= 4; nextSigIdx--)
    {
        innerIdx = nextSigIdx & subSetMask;
        gt0Flag  = (gt0CoefMap >> innerIdx) & 1;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;
        coeffVal = coeff[posY*coeffStride + posX];

        if (numNonZero || nextSigIdx != inferSigIdx)
        {
            Xin266GetSigCxtIdx (
                coeff,
                coeffStride,
                compType,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                &sumAbs1,
                &sigCtxIdx,
                *state);

            Xin266EncodeBin (
                &cabacSet->cabac,
                gt0Flag,
                cabacSet->context + XIN_CO_SIG_FLAG_LUMA + XIN_NUM_SIG_FLAG_LUMA_CTX*compType + sigCtxIdx);

            remRegBins--;
        }
        else if (nextSigIdx != coeffCtx->lastScanIdx)
        {
            Xin266GetSigCxtIdx (
                coeff,
                coeffStride,
                compType,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                &sumAbs1,
                &sigCtxIdx,
                *state);
        }

        if (gt0Flag)
        {
            Xin266GetGtxCxtIdx (
                compType,
                (nextSigIdx == coeffCtx->lastScanIdx) ? -1 : posX + posY,
                sumAbs1,
                &gtxCtxIdx);

            numNonZero++;

            remAbsLevel = XIN_ABS (coeffVal) - 1;

            if (nextSigIdx != coeffCtx->lastScanIdx)
            {
                signPattern <<= 1;
            }

            if (coeffVal < 0)
            {
                signPattern++;
            }

            gt1Flag = !!remAbsLevel;

            Xin266EncodeBin (
                &cabacSet->cabac,
                gt1Flag,
                cabacSet->context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx);

            remRegBins--;

            if (gt1Flag)
            {
                remAbsLevel -= 1;

                Xin266EncodeBin (
                    &cabacSet->cabac,
                    remAbsLevel&1,
                    cabacSet->context + XIN_CO_PAR_FLAG_LUMA + XIN_NUM_PAR_FLAG_LUMA_CTX*compType + gtxCtxIdx);

                remAbsLevel >>= 1;

                remRegBins--;

                gt2Flag = !!remAbsLevel;

                Xin266EncodeBin (
                    &cabacSet->cabac,
                    gt2Flag,
                    cabacSet->context + XIN_CO_GT2_FLAG_LUMA + XIN_NUM_GT2_FLAG_LUMA_CTX*compType + gtxCtxIdx);

                remRegBins--;

            }

        }

        *state = (stateTransTable >> ((*state<<2)+((coeffVal&1)<<1))) & 3;

    }

    firstPosMode2 = nextSigIdx;

    coeffCtx->regBinLimit = remRegBins;

    //===== 2nd PASS: Go-rice codes =====
    for (scanIdx = firstSigIdx; scanIdx > firstPosMode2; scanIdx-- )
    {
        innerIdx = scanIdx & subSetMask;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;
        absLevel = XIN_ABS (coeff[posY*coeffStride+posX]);

        if (absLevel >= 4)
        {
            Xin266GetAbsSum (
                coeff,
                posX,
                posY,
                coeffCtx->width,
                coeffCtx->height,
                coeffStride,
                4,
                &sumAll);

            ricePar = goRiceParsCoeff[sumAll];
            remPar  = (absLevel - 4) >> 1;

            Xin266EncodeRemAbsEP (
                cabacSet,
                remPar,
                ricePar,
                COEF_REMAIN_BIN_REDUCTION,
                15);

        }

    }

    //===== coeff bypass ====
    for (scanIdx = firstPosMode2; scanIdx >= minSubIdx; scanIdx--)
    {
        innerIdx = scanIdx & subSetMask;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;

        Xin266GetAbsSum (
            coeff,
            posX,
            posY,
            coeffCtx->width,
            coeffCtx->height,
            coeffStride,
            0,
            &sumAll);

        coeffVal = coeff[posY*coeffStride+posX];
        absLevel = XIN_ABS (coeffVal);
        ricePar  = goRiceParsCoeff[sumAll];
        pos0Par  = (*state < 2 ? 1 : 2) << ricePar;
        remPar   = (absLevel == 0 ? pos0Par : absLevel <= pos0Par ? absLevel - 1 : absLevel);

        Xin266EncodeRemAbsEP (
            cabacSet,
            remPar,
            ricePar,
            COEF_REMAIN_BIN_REDUCTION,
            15);

        *state = (stateTransTable >> ((*state<<2)+((absLevel&1)<<1))) & 3;

        if (absLevel)
        {
            numNonZero++;

            signPattern <<= 1;

            if (coeffVal < 0)
            {
                signPattern++;
            }
        }

    }

    //===== encode sign's =====
    if (coeffCtx->sbhOn && numNonZero)
    {
        BIT_SCAN_FORWARD_32 (gt0CoefMap, firstNZPosInCG);
        BIT_SCAN_REVERSE_32 (gt0CoefMap, lastNZPosInCG);

        sbhFlag = lastNZPosInCG - firstNZPosInCG >= XIN_SBH_THRESHOLD;

        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            signPattern>>sbhFlag,
            numNonZero-sbhFlag);
    }
    else
    {
        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            signPattern,
            numNonZero);
    }

}

void Xin266GetNeighCoeff (
    COEFF    *qCoeff,
    intptr_t stride,
    SINT32   posX,
    SINT32   posY,
    COEFF    *coeffRgt,
    COEFF    *coeffDwn)
{
    COEFF   *coeff;

    coeff     = qCoeff + posX + posY*stride;
    *coeffRgt = 0;
    *coeffDwn = 0;

    if (posX > 0)
    {
        *coeffRgt = coeff[-1];
    }

    if (posY > 0)
    {
        *coeffDwn = coeff[-stride];
    }

}

void Xin266DeriveModCoeff (
    COEFF  rgtCoeff,
    COEFF  dwnCoeff,
    COEFF  absCoeff,
    BOOL   bdPcm,
    SINT32 *modAbsCoeff)
{
    SINT32 absMod;
    SINT32 absDwn;
    SINT32 absRgt;
    SINT32 pred;

    if (absCoeff == 0)
    {
        *modAbsCoeff = 0;

        return;
    }

    absDwn = XIN_ABS (dwnCoeff);
    absRgt = XIN_ABS (rgtCoeff);
    absMod = absCoeff;

    if (!bdPcm)
    {
        pred = XIN_MAX (absDwn, absRgt);

        if (absCoeff == pred)
        {
            absMod = 1;
        }
        else
        {
            absMod = absCoeff < pred ? absCoeff + 1 : absCoeff;
        }
    }

    *modAbsCoeff = absMod;

}

static void Xin266WriteGrpCoeffTs (
    xin_coeff_context *coeffCtx,
    xin_scan_pos      *scanOrder,
    xin_cabac_context *cabacSet,
    COEFF             *coeff,
    intptr_t          coeffStride,
    UINT16            gt0CoefMap)
{
    SINT32       minSubIdx;
    SINT32       firstSigIdx;
    SINT32       nextSigIdx;
    SINT32       remRegBins;
    COEFF        coeffVal;
    BOOL         isLast;
    BOOL         gt0Flag;
    BOOL         gt1Flag;
    BOOL         gt2Flag;
    SINT32       posX, posY;
    SINT32       innerIdx;
    SINT32       subSetMask;
    SINT32       numNonZero;
    SINT32       inferSigIdx;
    UINT32       sigCtxIdx;
    UINT32       signCtxIdx;
    UINT32       gtxCtxIdx;
    SINT32       remAbsLevel;
    SINT32       absLevel;
    SINT32       remPar;
    COEFF        rgtCoeff;
    COEFF        dwnCoeff;
    SINT32       modAbsCoeff;
    SINT32       lastScanIdx1;
    SINT32       lastScanIdx2;
    SINT32       cutOffVal;
    SINT32       binIdx;

    minSubIdx    = coeffCtx->maxSubIdx;
    isLast       = (coeffCtx->lastScanIdx >> coeffCtx->lgCGSize) == coeffCtx->subCGIdx;
    firstSigIdx  = coeffCtx->minSubIdx;
    nextSigIdx   = firstSigIdx;
    remRegBins   = coeffCtx->regBinLimit;
    subSetMask   = (1 << coeffCtx->lgCGSize) - 1;
    numNonZero   = 0;
    inferSigIdx  = minSubIdx;
    remAbsLevel  = -1;
    lastScanIdx1 = -1;
    lastScanIdx2 = -1;

    if ((!isLast) || (!coeffCtx->onlyLastSigGrp))
    {
        if (gt0CoefMap)
        {
            Xin266EncodeBin (
                &cabacSet->cabac,
                1,
                cabacSet->context + coeffCtx->sigGrpCtx + XIN_CO_TS_SIG_COEFF_GROUP);
        }
        else
        {
            Xin266EncodeBin(
                &cabacSet->cabac,
                0,
                cabacSet->context + coeffCtx->sigGrpCtx + XIN_CO_TS_SIG_COEFF_GROUP);

            return;
        }
    }

    for ( ; nextSigIdx <= minSubIdx && remRegBins >= 4; nextSigIdx++)
    {
        innerIdx = nextSigIdx & subSetMask;
        gt0Flag  = (gt0CoefMap >> innerIdx) & 1;
        posX     = scanOrder[innerIdx].posX;
        posY     = scanOrder[innerIdx].posY;
        posX    += coeffCtx->cgPosX;
        posY    += coeffCtx->cgPosY;

        if (numNonZero || nextSigIdx != inferSigIdx)
        {
            Xin266GetSigCxtIdxTs (
                coeff,
                coeffStride,
                posX,
                posY,
                &sigCtxIdx);

            Xin266EncodeBin (
                &cabacSet->cabac,
                gt0Flag,
                cabacSet->context + XIN_CO_TS_SIG_FLAG + sigCtxIdx);

            remRegBins--;
        }

        if (gt0Flag)
        {
            coeffVal = coeff[posY*coeffStride + posX];

            Xin266SignCtxIdTs (
                coeff,
                coeffStride,
                posX,
                posY,
                &signCtxIdx);

            Xin266EncodeBin (
                &cabacSet->cabac,
                coeffVal < 0,
                cabacSet->context + XIN_CO_TS_RESIDUAL_SIGN + signCtxIdx);

            remRegBins--;
            numNonZero++;

            Xin266GetNeighCoeff (
                coeff,
                coeffStride,
                posX,
                posY,
                &rgtCoeff,
                &dwnCoeff);

            Xin266DeriveModCoeff (
                rgtCoeff,
                dwnCoeff,
                XIN_ABS(coeffVal),
                FALSE,
                &modAbsCoeff);

            Xin266GetSigCxtIdxTs (
                coeff,
                coeffStride,
                posX,
                posY,
                &gtxCtxIdx);

            remAbsLevel = modAbsCoeff - 1;
            gt1Flag     = !!remAbsLevel;

            Xin266EncodeBin (
                &cabacSet->cabac,
                gt1Flag,
                cabacSet->context + XIN_CO_TS_GT1_FLAG + gtxCtxIdx);

            remRegBins--;

            if (gt1Flag)
            {
                remAbsLevel -= 1;

                Xin266EncodeBin (
                    &cabacSet->cabac,
                    remAbsLevel&1,
                    cabacSet->context + XIN_CO_TS_PAR_FLAG);

                remRegBins--;
            }

        }

        lastScanIdx1 = nextSigIdx;

    }

    //===== 2nd PASS: Go-rice codes =====
    for (nextSigIdx = firstSigIdx; nextSigIdx <= minSubIdx && remRegBins >= 4; nextSigIdx++)
    {
        innerIdx  = nextSigIdx & subSetMask;
        posX      = scanOrder[innerIdx].posX;
        posY      = scanOrder[innerIdx].posY;
        posX     += coeffCtx->cgPosX;
        posY     += coeffCtx->cgPosY;
        coeffVal  = coeff[posY*coeffStride + posX];
        cutOffVal = 2;

        Xin266GetNeighCoeff (
            coeff,
            coeffStride,
            posX,
            posY,
            &rgtCoeff,
            &dwnCoeff);

        Xin266DeriveModCoeff (
            rgtCoeff,
            dwnCoeff,
            XIN_ABS(coeffVal),
            FALSE,
            &absLevel);

        for (binIdx = 0; binIdx < 4; binIdx++)
        {
            if (absLevel >= cutOffVal)
            {
                gt2Flag = (absLevel >= (cutOffVal + 2));

                Xin266EncodeBin (
                    &cabacSet->cabac,
                    gt2Flag,
                    cabacSet->context + XIN_CO_TS_GTX_FLAG + (cutOffVal >> 1));

                remRegBins--;
            }

            cutOffVal += 2;
        }

        lastScanIdx2 = nextSigIdx;

    }

    coeffCtx->regBinLimit = remRegBins;

    //===== coeff bypass ====
    for (nextSigIdx = firstSigIdx; nextSigIdx <= minSubIdx; nextSigIdx++ )
    {
        innerIdx  = nextSigIdx & subSetMask;
        posX      = scanOrder[innerIdx].posX;
        posY      = scanOrder[innerIdx].posY;
        posX     += coeffCtx->cgPosX;
        posY     += coeffCtx->cgPosY;
        coeffVal  = coeff[posY*coeffStride + posX];
        cutOffVal = (nextSigIdx <= lastScanIdx2 ? 10 : (nextSigIdx <= lastScanIdx1 ? 2 : 0));

        Xin266GetNeighCoeff (
            coeff,
            coeffStride,
            posX,
            posY,
            &rgtCoeff,
            &dwnCoeff);

        Xin266DeriveModCoeff (
            rgtCoeff,
            dwnCoeff,
            XIN_ABS(coeffVal),
            !cutOffVal,
            &absLevel);

        if (absLevel >= cutOffVal)
        {
            remPar = nextSigIdx <= lastScanIdx1 ? (absLevel - cutOffVal) >> 1 : absLevel;

            Xin266EncodeRemAbsEP (
                cabacSet,
                remPar,
                1,
                COEF_REMAIN_BIN_REDUCTION,
                15);

            if (absLevel && nextSigIdx > lastScanIdx1)
            {
                Xin266EncodeBinEP (
                    &(cabacSet->cabac),
                    coeffVal < 0);
            }
        }

    }

}

static void Xin266WriteCoeff (
    xin_cabac_context *cabacSet,
    xin_tu_struct     *tu,
    BOOL              depQuant,
    BOOL              sbhOn,
    UINT32            planeIdx)
{
    xin_scan_pos      *scanOrder;
    xin_scan_pos      *scanOrderCG;
    SINT32            lastSigPosX;
    SINT32            lastSigPosY;
    SINT32            lastBlockIdx;
    SINT32            blockIdx;
    SINT32            sigGrpCtx;
    UINT32            sigRgt;
    UINT32            sigDwn;
    SINT32            scanPosLast;
    UINT32            width, height;
    UINT32            clipWidth, clipHeight;
    UINT32            lgWidth, lgHeight;
    UINT32            clipLgWidth;
    UINT32            cgWidth, cgHeight;
    UINT32            lgCGWidth, lgCGHeight;
    UINT32            widthInCG;
    UINT32            lgCGSize;
    UINT32            compType;
    UINT32            blockPos;
    UINT32            blockX;
    UINT32            blockY;
    UINT64            sigGrpMapEs;
    UINT64            sigGrpMapRs;
    UINT64            sigGrpMapRgt;
    UINT64            sigGrpMapDwn;
    UINT16            *gt0BitMap;
    UINT16            gt0CoefMap;
    SINT32            remRegBins;
    COEFF             *qCoeff;
    intptr_t          stride;
    xin_coeff_context coeffCtx;
    SINT32            stateTab;
    SINT32            state;

    compType    = (planeIdx == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA;
    scanOrderCG = tu->scanOrderCG[compType];
    scanOrder   = tu->scanOrder[compType];
    qCoeff      = tu->qCoeff[planeIdx];
    stride      = tu->coeffStride[compType];
    lgWidth     = tu->lgWidth[compType];
    lgHeight    = tu->lgHeight[compType];
    lgCGWidth   = tu->lgCGWidth[compType];
    lgCGHeight  = tu->lgCGHeight[compType];
    cgWidth     = 1 << lgCGWidth;
    cgHeight    = 1 << lgCGHeight;
    lgCGSize    = lgCGWidth + lgCGHeight;
    width       = 1 << lgWidth;
    height      = 1 << lgHeight;
    gt0BitMap   = tu->gt0BitMap[planeIdx];
    sigGrpMapEs = tu->nzCGMapEs[planeIdx];
    sigGrpMapRs = tu->nzCGMapRs[planeIdx];
    clipHeight  = XIN_MIN (height, 32);
    clipWidth   = XIN_MIN (width, 32);
    remRegBins  = (clipHeight*clipWidth*MAX_TU_LEVEL_CTX_CODED_BIN) >> 4;
    clipLgWidth = XIN_MIN (lgWidth, 5);
    widthInCG   = clipWidth >> lgCGWidth;
    stateTab    = depQuant ? 32040 : 0;
    state       = 0;

    Xin266FindLastSigPos (
        gt0BitMap,
        lgCGWidth,
        lgCGHeight,
        sigGrpMapEs,
        scanOrder,
        scanOrderCG,
        &lastSigPosX,
        &lastSigPosY,
        &scanPosLast);

    sigGrpMapRgt = (sigGrpMapRs >> 1) & rgtGrpSigMask[clipLgWidth];
    sigGrpMapDwn = (sigGrpMapRs >> widthInCG);

    // Encode the position of last significant coefficient
    Xin266EncodeLastSigXY (
        cabacSet,
        lastSigPosX,
        lastSigPosY,
        lgWidth,
        lgHeight,
        compType);

    coeffCtx.planeIdx    = planeIdx;
    coeffCtx.regBinLimit = remRegBins;
    coeffCtx.width       = clipWidth;
    coeffCtx.height      = clipHeight;
    coeffCtx.cgWidth     = cgWidth;
    coeffCtx.cgHeight    = cgHeight;
    coeffCtx.lgCGSize    = lgCGSize;
    coeffCtx.lastScanIdx = scanPosLast;
    coeffCtx.sumAbs1     = -1;
    coeffCtx.sbhOn       = sbhOn;

    lastBlockIdx = scanPosLast >> lgCGSize;

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        sigRgt     = (sigGrpMapRgt >> blockPos) & 1;
        sigDwn     = (sigGrpMapDwn >> blockPos) & 1;
        sigGrpCtx  = sigRgt | sigDwn;
        gt0CoefMap = gt0BitMap[blockIdx];

        coeffCtx.cgPosX    = blockX*cgWidth;
        coeffCtx.cgPosY    = blockY*cgHeight;
        coeffCtx.sigGrpCtx = sigGrpCtx;
        coeffCtx.minSubIdx = blockIdx << lgCGSize;
        coeffCtx.maxSubIdx = ((blockIdx + 1) << lgCGSize) - 1;
        coeffCtx.subCGIdx  = blockIdx;

        // encode significant_coeffgroup_flag
        Xin266WriteGrpCoeff (
            &coeffCtx,
            scanOrder,
            cabacSet,
            qCoeff,
            stride,
            gt0CoefMap,
            stateTab,
            &state);

    }

}

static void Xin266WriteCoeffTs (
    xin_cabac_context *cabacSet,
    xin_tu_struct     *tu,
    BOOL              sbhOn,
    UINT32            planeIdx)
{
    xin_scan_pos      *scanOrder;
    xin_scan_pos      *scanOrderCG;
    SINT32            lastBlockIdx;
    SINT32            blockIdx;
    SINT32            sigGrpCtx;
    UINT32            sigLft;
    UINT32            sigTop;
    SINT32            scanPosLast;
    UINT32            width, height;
    UINT32            lgWidth, lgHeight;
    UINT32            cgWidth, cgHeight;
    UINT32            lgCGWidth, lgCGHeight;
    UINT32            widthInCG;
    UINT32            lgCGSize;
    UINT32            compType;
    UINT32            blockPos;
    UINT32            blockX;
    UINT32            blockY;
    UINT64            sigGrpMapEs;
    UINT64            sigGrpMapRs;
    UINT64            sigGrpMapLft;
    UINT64            sigGrpMapTop;
    UINT16            *gt0BitMap;
    UINT16            gt0CoefMap;
    SINT32            remRegBins;
    COEFF             *qCoeff;
    intptr_t          stride;
    xin_coeff_context coeffCtx;

    compType     = (planeIdx == PLANE_LUMA) ? PLANE_LUMA : PLANE_CHROMA;
    scanOrderCG  = tu->scanOrderCG[compType];
    scanOrder    = tu->scanOrder[compType];
    qCoeff       = tu->qCoeff[planeIdx];
    stride       = tu->coeffStride[compType];
    lgWidth      = tu->lgWidth[compType];
    lgHeight     = tu->lgHeight[compType];
    lgCGWidth    = tu->lgCGWidth[compType];
    lgCGHeight   = tu->lgCGHeight[compType];
    cgWidth      = 1 << lgCGWidth;
    cgHeight     = 1 << lgCGHeight;
    lgCGSize     = lgCGWidth + lgCGHeight;
    width        = 1 << lgWidth;
    height       = 1 << lgHeight;
    gt0BitMap    = tu->gt0BitMap[planeIdx];
    sigGrpMapEs  = tu->nzCGMapEs[planeIdx];
    sigGrpMapRs  = tu->nzCGMapRs[planeIdx];
    remRegBins   = (height*width*7) >> 2;
    widthInCG    = width >> lgCGWidth;
    sigGrpMapLft = (sigGrpMapRs << 1) & lftGrpSigMask[lgWidth];
    sigGrpMapTop = (sigGrpMapRs << widthInCG);
    scanPosLast  = width*height - 1;
    lastBlockIdx = scanPosLast >> lgCGSize;

    coeffCtx.planeIdx       = planeIdx;
    coeffCtx.regBinLimit    = remRegBins;
    coeffCtx.width          = width;
    coeffCtx.height         = height;
    coeffCtx.cgWidth        = cgWidth;
    coeffCtx.cgHeight       = cgHeight;
    coeffCtx.lgCGSize       = lgCGSize;
    coeffCtx.lastScanIdx    = scanPosLast;
    coeffCtx.sumAbs1        = -1;
    coeffCtx.sbhOn          = sbhOn;
    coeffCtx.onlyLastSigGrp = (sigGrpMapEs & (~(1<<lastBlockIdx))) == 0;

    for (blockIdx = 0; blockIdx <= lastBlockIdx; blockIdx++)
    {
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        sigLft     = (sigGrpMapLft >> blockPos) & 1;
        sigTop     = (sigGrpMapTop >> blockPos) & 1;
        sigGrpCtx  = sigLft + sigTop;
        gt0CoefMap = gt0BitMap[blockIdx];

        coeffCtx.cgPosX    = blockX*cgWidth;
        coeffCtx.cgPosY    = blockY*cgHeight;
        coeffCtx.sigGrpCtx = sigGrpCtx;
        coeffCtx.minSubIdx = blockIdx << lgCGSize;
        coeffCtx.maxSubIdx = ((blockIdx + 1) << lgCGSize) - 1;
        coeffCtx.subCGIdx  = blockIdx;

        // encode significant_coeffgroup_flag
        Xin266WriteGrpCoeffTs (
            &coeffCtx,
            scanOrder,
            cabacSet,
            qCoeff,
            stride,
            gt0CoefMap);

    }

}


static void Xin266WriteDeltaQp (
    xin_cabac_context *cabacSet,
    SINT32            deltaQp)
{
    UINT32  absDeltaQp;
    UINT32  tuValue;
    BOOL    codeLast;

    absDeltaQp = XIN_ABS(deltaQp);
    tuValue    = XIN_MIN(absDeltaQp, CU_DQP_TU_CMAX);

    Xin266EncodeBin (
        &cabacSet->cabac,
        tuValue ? 1 : 0,
        cabacSet->context + XIN_CO_DELTA_QP);

    if (tuValue)
    {
        codeLast = (CU_DQP_TU_CMAX > tuValue) ? TRUE : FALSE;

        while (--tuValue)
        {
            Xin266EncodeBin (
                &cabacSet->cabac,
                1,
                cabacSet->context + XIN_CO_DELTA_QP + 1);
        }

        if (codeLast)
        {
            Xin266EncodeBin (
                &cabacSet->cabac,
                0,
                cabacSet->context + XIN_CO_DELTA_QP + 1);
        }
    }

    if ( absDeltaQp >= CU_DQP_TU_CMAX)
    {
        Xin266EncodeEGKBin (
            &cabacSet->cabac,
            absDeltaQp - CU_DQP_TU_CMAX,
            CU_DQP_EGK);
    }

    if ( absDeltaQp > 0)
    {
        Xin266EncodeBinEP (
            &cabacSet->cabac,
            deltaQp > 0 ? 0 : 1);
    }

}

static void Xin266WriteCbf (
    xin_cabac_context *cabacSet,
    UINT8             cbf,
    BOOL              prevCbf,
    UINT32            planeIdx)
{
    UINT32 cbfCtxInc;
    UINT32 cbfCtxOff;

    cbfCtxInc = ((planeIdx == PLANE_CHROMA_V) & (prevCbf != 0)) ? 1 : 0;
    cbfCtxOff = cbfCtxOffset[planeIdx];

    Xin266EncodeBin(
        &(cabacSet->cabac),
        cbf,
        cabacSet->context + cbfCtxOff + cbfCtxInc);
}

static void Xin266WriteTransSkipFlag (
    xin_cabac_context *cabacSet,
    BOOL              transSkipFlag,
    UINT32            planeIdx)
{
    UINT32  transSkipFlagCtx;

    transSkipFlagCtx = (planeIdx == PLANE_LUMA) ? 0 : 1;

    Xin266EncodeBin (
        &(cabacSet->cabac),
        transSkipFlag,
        cabacSet->context + XIN_CO_TRANSFORM_SKIP_FLAG + transSkipFlagCtx);
}

static void Xin266WriteTransUnit (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    xin_tu_struct  *tu)
{
    xin_cabac_context *cabacSet;
    xin_seq_struct    *seqSet;

    cabacSet = secSet->cabacSet;
    seqSet   = secSet->seqSet;

    if (cu->treeMask & XIN_CU_TREE_C_MASK)
    {
        Xin266WriteCbf (
            cabacSet,
            tu->uCbf,
            FALSE,
            PLANE_CHROMA_U);

        Xin266WriteCbf (
            cabacSet,
            tu->vCbf,
            tu->uCbf,
            PLANE_CHROMA_V);
    }

    if (((cu->type == XIN_INTRA_MODE) || (cu->tuNum > 1) || (tu->uCbf || tu->vCbf)) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        Xin266WriteCbf (
            cabacSet,
            tu->yCbf,
            FALSE,
            PLANE_LUMA);
    }

    if ((tu->yCbf || tu->uCbf || tu->vCbf || (cu->height > 64) || (cu->width > 64)) && (secSet->codingDeltaQp) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        cu->qp = secSet->qp;

        Xin266WriteDeltaQp (
            cabacSet,
            cu->qp - secSet->refQp);

        // Update ref qp
        secSet->refQp         = cu->qp;
        secSet->codingDeltaQp = FALSE;
    }

    if ((tu->yCbf) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        if ((tu->lgWidth[PLANE_LUMA] <= seqSet->config.maxTrSkipLgSize) && (tu->lgHeight[PLANE_LUMA] <= seqSet->config.maxTrSkipLgSize))
        {
            Xin266WriteTransSkipFlag (
                cabacSet,
                tu->mtsIdx[PLANE_LUMA] == XIN_MTS_SKIP,
                PLANE_LUMA);
        }

        if (tu->mtsIdx[PLANE_LUMA] == XIN_MTS_SKIP)
        {
            Xin266WriteCoeffTs (
                cabacSet,
                tu,
                seqSet->config.enableSignDataHiding,
                PLANE_LUMA);
        }
        else
        {
            Xin266WriteCoeff (
                cabacSet,
                tu,
                seqSet->config.enableDepQuant,
                seqSet->config.enableSignDataHiding,
                PLANE_LUMA);
        }
    }

    if (cu->treeMask & XIN_CU_TREE_C_MASK)
    {
        if (tu->uCbf)
        {
            if ((tu->lgWidth[PLANE_CHROMA] <= seqSet->config.maxTrSkipLgSize) && (tu->lgHeight[PLANE_CHROMA] <= seqSet->config.maxTrSkipLgSize))
            {
                Xin266WriteTransSkipFlag (
                    cabacSet,
                    tu->mtsIdx[PLANE_CHROMA_U] == XIN_MTS_SKIP,
                    PLANE_CHROMA_U);
            }

            if (tu->mtsIdx[PLANE_CHROMA_U] == XIN_MTS_SKIP)
            {
                Xin266WriteCoeffTs (
                    cabacSet,
                    tu,
                    seqSet->config.enableSignDataHiding,
                    PLANE_CHROMA_U);
            }
            else
            {
                Xin266WriteCoeff (
                    cabacSet,
                    tu,
                    seqSet->config.enableDepQuant,
                    seqSet->config.enableSignDataHiding,
                    PLANE_CHROMA_U);
            }
        }

        if (tu->vCbf)
        {
            if ((tu->lgWidth[PLANE_CHROMA] <= seqSet->config.maxTrSkipLgSize) && (tu->lgHeight[PLANE_CHROMA] <= seqSet->config.maxTrSkipLgSize))
            {
                Xin266WriteTransSkipFlag (
                    cabacSet,
                    tu->mtsIdx[PLANE_CHROMA_V] == XIN_MTS_SKIP,
                    PLANE_CHROMA_V);
            }

            if (tu->mtsIdx[PLANE_CHROMA_V] == XIN_MTS_SKIP)
            {
                Xin266WriteCoeffTs (
                    cabacSet,
                    tu,
                    seqSet->config.enableSignDataHiding,
                    PLANE_CHROMA_V);
            }
            else
            {
                Xin266WriteCoeff (
                    cabacSet,
                    tu,
                    seqSet->config.enableDepQuant,
                    seqSet->config.enableSignDataHiding,
                    PLANE_CHROMA_V);
            }

        }

    }

}

static void Xin266WriteTransTree (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    UINT32        tuIdx;
    xin_tu_struct *tu;

    for (tuIdx = 0; tuIdx < cu->tuNum; tuIdx++)
    {
        tu = cu->tu[tuIdx];

        Xin266WriteTransUnit (
            secSet,
            cu,
            tu);
    }
}

static void Xin266WriteMergeIndex (
    xin_cabac_context *cabacSet,
    xin_pu_struct     *pu,
    UINT32            maxMergeCand)
{
    UINT32  mergeIndex;
    UINT32  mergeMask;

    mergeIndex = pu->mergeIndex;

    if (maxMergeCand > 1)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            mergeIndex != 0,
            cabacSet->context + XIN_CO_MERGE_IDX);

        if (mergeIndex != 0)
        {
            mergeMask = (1 << mergeIndex) - 2;

            if (mergeIndex + 1 == maxMergeCand)
            {
                mergeMask = mergeMask >> 1;
            }

            Xin266EncodeBinsEP (
                &cabacSet->cabac,
                mergeMask,
                mergeIndex - (mergeIndex + 1 == maxMergeCand));
        }

    }

}

static void Xin266WriteAffineMergeIndex (
    xin_cabac_context *cabacSet,
    xin_pu_struct     *pu,
    UINT32            maxMergeCand)
{
    UINT32  mergeIndex;
    UINT32  mergeMask;

    mergeIndex = pu->mergeIndex;

    if (maxMergeCand > 1)
    {
        Xin266EncodeBin (
            &cabacSet->cabac,
            mergeIndex != 0,
            cabacSet->context + XIN_CO_AFF_MERGE_IDX);

        if (mergeIndex != 0)
        {
            mergeMask = (1 << mergeIndex) - 2;

            if (mergeIndex + 1 == maxMergeCand)
            {
                mergeMask = mergeMask >> 1;
            }

            Xin266EncodeBinsEP (
                &cabacSet->cabac,
                mergeMask,
                mergeIndex - (mergeIndex + 1 == maxMergeCand));
        }

    }

}


static void Xin266WriteLumaIntraPredMode (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu)
{
    xin_pu_struct *pu;
    SINT32        modeIdx;
    SINT32        intraMode;
    SINT32        mpmIndex;
    SINT8         *intraMPM;

    pu        = &cu->pu;
    intraMPM  = pu->intraMPM;
    intraMode = pu->intraLumaMode;
    mpmIndex  = XIN_INTRA_MPM_NUM;

    for (modeIdx = 0; modeIdx < XIN_INTRA_MPM_NUM; modeIdx++)
    {
        if (intraMode == intraMPM[modeIdx])
        {
            mpmIndex = modeIdx;

            break;
        }
    }

    Xin266EncodeBin (
        &(cabacSet->cabac),
        (mpmIndex < XIN_INTRA_MPM_NUM),
        cabacSet->context + XIN_CO_INTRA_LUMA_MPM_FLAG);

    if (mpmIndex != XIN_INTRA_MPM_NUM)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            (mpmIndex > 0),
            cabacSet->context + XIN_CO_INTRA_LUMA_PLANAR_FLAG + 1);

        if (mpmIndex)
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                mpmIndex > 1);
        }

        if (mpmIndex > 1)
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                mpmIndex > 2);
        }

        if (mpmIndex > 2)
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                mpmIndex > 3);
        }

        if (mpmIndex > 3)
        {
            Xin266EncodeBinEP (
                &(cabacSet->cabac),
                mpmIndex > 4);
        }

    }
    else
    {
        intraMPM = pu->sortedIntraMPM;

        for (modeIdx = XIN_INTRA_MPM_NUM - 1; modeIdx >= 0; modeIdx--)
        {
            if (intraMode > intraMPM[modeIdx])
            {
                intraMode--;
            }
        }

        Xin266WriteTruncBinCode (
            &(cabacSet->cabac),
            intraMode,
            XIN_INTRA_NUM - XIN_INTRA_MPM_NUM);

    }

}

static void Xin266GetIntraChromaCand (
    UINT32  lumaMode,
    UINT32  *modeList)
{
    SINT32  idx;

    modeList[0] = XIN_PLANAR_IDX;
    modeList[1] = XIN_VER_IDX;
    modeList[2] = XIN_HOR_IDX;
    modeList[3] = XIN_DC_IDX;
    modeList[4] = XIN_LM_CHROMA_IDX;
    modeList[5] = XIN_LM_CHROMA_L_IDX;
    modeList[6] = XIN_LM_CHROMA_T_IDX;
    modeList[7] = XIN_DM_CHROMA_IDX;

    for (idx = 0; idx < 4; idx++)
    {
        if (lumaMode == modeList[idx])
        {
            modeList[idx] = XIN_VDIA_IDX;

            break;
        }
    }

}

static void Xin266WriteChromaIntraPredMode (
    xin_cabac_context *cabacSet,
    BOOL              enableCclm,
    xin_cu_struct     *cu)
{
    UINT32        chromaMode;
    UINT32        intraChromaCand[XIN_INTRA_CHROMA_CAND_NUM];
    xin_pu_struct *pu;
    SINT32        candIdx;

    pu         = &cu->pu;
    chromaMode = pu->intraChromaMode;

    if (enableCclm)
    {
        if ((chromaMode >= XIN_LM_CHROMA_IDX) && (chromaMode <= XIN_LM_CHROMA_T_IDX))
        {
            Xin266EncodeBin (
                &(cabacSet->cabac),
                TRUE,
                cabacSet->context + XIN_CO_CCLM_MODE_FLAG);

            Xin266EncodeBin (
                &(cabacSet->cabac),
                chromaMode != XIN_LM_CHROMA_IDX,
                cabacSet->context + XIN_CO_CCLM_MODE_IDX);

            if (chromaMode != XIN_LM_CHROMA_IDX)
            {
                Xin266EncodeBinEP (
                    &(cabacSet->cabac),
                    chromaMode - XIN_LM_CHROMA_IDX - 1);
            }

            return;

        }
        else
        {
            Xin266EncodeBin (
                &(cabacSet->cabac),
                FALSE,
                cabacSet->context + XIN_CO_CCLM_MODE_FLAG);
        }

    }

    if (chromaMode == XIN_DM_CHROMA_IDX)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            0,
            cabacSet->context + XIN_CO_INTRA_CHROMA_PRED_MODE);
    }
    else
    {
        Xin266GetIntraChromaCand (
            pu->intraLumaMode,
            intraChromaCand);

        for (candIdx = 0; candIdx < XIN_INTRA_CHROMA_CAND_NUM; candIdx++)
        {
            if (chromaMode == intraChromaCand[candIdx])
            {
                break;
            }
        }

        Xin266EncodeBin (
            &(cabacSet->cabac),
            1,
            cabacSet->context + XIN_CO_INTRA_CHROMA_PRED_MODE);

        Xin266EncodeBinsEP (
            &(cabacSet->cabac),
            candIdx,
            2);

    }

}

static void Xin266WriteInterPredIdc (
    xin_cabac_context *cabacSet,
    xin_pu_struct     *pu)
{
    UINT32  interPredIdc;
    UINT32  predIdcCtxInc;

    predIdcCtxInc = 7 - ((pu->lgHeight + pu->lgWidth + 1) >> 1);

    if ((pu->refIdx[XIN_LIST_0] >= 0) && (pu->refIdx[XIN_LIST_1] >= 0))
    {
        interPredIdc = 2;
    }
    else if (pu->refIdx[XIN_LIST_0] >= 0)
    {
        interPredIdc = 0;
    }
    else
    {
        interPredIdc = 1;
    }

    if (interPredIdc == 2)
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            1,
            cabacSet->context + XIN_CO_INTER_DIR + predIdcCtxInc);
    }
    else
    {
        Xin266EncodeBin (
            &(cabacSet->cabac),
            0,
            cabacSet->context + XIN_CO_INTER_DIR + predIdcCtxInc);

        Xin266EncodeBin (
            &(cabacSet->cabac),
            interPredIdc,
            cabacSet->context + XIN_CO_INTER_DIR + 5);

    }

}

static void Xin266WriteRefIdx (
    xin_cabac_context *cabacSet,
    xin_pu_struct     *pu,
    SINT32            listIdx,
    UINT32            refNum)
{
    UINT32  refIdx;
    UINT32  mask;

    refIdx = pu->refIdx[listIdx];

    if (refNum > 1)
    {
        Xin266EncodeBin(
            &(cabacSet->cabac),
            refIdx > 0,
            cabacSet->context + XIN_CO_REF_PIC);

        if ((refIdx > 0) && (refNum > 2))
        {
            refNum -= 2;
            refIdx--;

            Xin266EncodeBin (
                &(cabacSet->cabac),
                refIdx > 0,
                cabacSet->context + XIN_CO_REF_PIC + 1);

            if (refIdx > 0)
            {
                mask   = (1 << refIdx) - 2;
                mask >>= (refIdx == refNum) ? 1 : 0;

                Xin266EncodeBinsEP (
                    &(cabacSet->cabac),
                    mask,
                    refIdx - (refIdx == refNum));
            }

        }

    }

}

static void Xin266WriteMvpIdx (
    xin_cabac_context *cabacSet,
    SINT32            listIdx,
    xin_pu_struct     *pu)
{
    UINT32 mvpIndex = pu->mvpIndex[listIdx] > 0;

    Xin266EncodeBin (
        &cabacSet->cabac,
        mvpIndex,
        cabacSet->context + XIN_CO_MVP_IDX);
}

static void Xin266WriteInterPu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    UINT32         refNum[2],
    BOOL           mvdL1Zero,
    UINT32         frameType)
{
    UINT32            listIdx;
    xin_cabac_context *cabacSet;
    xin_seq_struct    *seqSet;
    xin_pu_struct     *pu;
    UINT32            maxMergeCand;
    UINT32            maxAffineMergeCand;

    seqSet             = secSet->seqSet;
    cabacSet           = secSet->cabacSet;
    pu                 = &cu->pu;
    maxMergeCand       = seqSet->config.maxMergeCand;
    maxAffineMergeCand = seqSet->config.maxAffineMergeCand;

    if (cu->type == XIN_SKIP_MODE)
    {
        assert(pu->mergeFlag);
    }
    else
    {
        Xin266WriteMergeFlag (
            cabacSet,
            pu->mergeFlag);
    }

    if (pu->mergeFlag)
    {
        if (maxAffineMergeCand)
        {
            Xin266WriteSubblockMergeFlag (
                cabacSet,
                cu,
                pu->affine);
        }

        if (pu->affine)
        {
            Xin266WriteAffineMergeIndex (
                cabacSet,
                pu,
                maxAffineMergeCand);
        }
        else
        {
            Xin266WriteMergeIndex (
                cabacSet,
                pu,
                maxMergeCand);
        }

    }
    else
    {
        if (frameType == XIN_B_FRAME)
        {
            Xin266WriteInterPredIdc (
                cabacSet,
                pu);
        }

        if (seqSet->config.enableAffine && cu->width >= 16 && cu->height >= 16)
        {
            // inter_affine_flag
            Xin266EncodeBin (
                &(cabacSet->cabac),
                pu->affine,
                cabacSet->context + XIN_CO_AFFINE_FLAG + cu->affineContext);

            if (seqSet->config.affineType && pu->affine)
            {
                // cu_affine_type_flag
                Xin266EncodeBin (
                    &(cabacSet->cabac),
                    pu->affineType,
                    cabacSet->context + XIN_CO_AFFINE_TYPE + cu->affineContext);
            }
        }

        for (listIdx = XIN_LIST_0; listIdx < XIN_LIST_NUM; listIdx++)
        {
            if (pu->refIdx[listIdx] < 0)
            {
                continue;
            }

            // Reference Index
            Xin266WriteRefIdx (
                cabacSet,
                pu,
                listIdx,
                refNum[listIdx]);

            // Motion Vector Difference
            if ((listIdx == XIN_LIST_1) && (mvdL1Zero) && ((pu->refIdx[XIN_LIST_0] >= 0) && (pu->refIdx[XIN_LIST_1] >= 0)))
            {

            }
            else
            {
                // Motion Vector Difference
                if (pu->affine)
                {
                    Xin266WriteAffineMvd (
                        cabacSet,
                        listIdx,
                        cu);
                }
                else
                {
                    Xin266WriteNormalMvd (
                        cabacSet,
                        listIdx,
                        cu);
                }
            }

            // Motion Vector Prediction Index
            Xin266WriteMvpIdx (
                cabacSet,
                listIdx,
                pu);

        }

    }

}

static BOOL Xin266NonZeroMvd (
    xin_pu_struct *pu,
    BOOL          mvdL1Zero)
{

    if (pu->refIdx[XIN_LIST_0] >= 0)
    {
        if ((pu->mv[XIN_LIST_0].mv.mv32X != pu->predMv[XIN_LIST_0].mv.mv32X) || (pu->mv[XIN_LIST_0].mv.mv32Y != pu->predMv[XIN_LIST_0].mv.mv32Y))
        {
            return TRUE;
        }
    }

    if ((pu->refIdx[XIN_LIST_1] >= 0) && (!mvdL1Zero))
    {
        if ((pu->mv[XIN_LIST_1].mv.mv32X != pu->predMv[XIN_LIST_1].mv.mv32X) || (pu->mv[XIN_LIST_1].mv.mv32Y != pu->predMv[XIN_LIST_1].mv.mv32Y))
        {
            return TRUE;
        }
    }

    return FALSE;

}

static void Xin266WriteImv (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu,
    BOOL              mvdL1Zero)
{
    xin_pu_struct *pu;

    pu = &cu->pu;

    if (pu->affine)
    {
        return;
    }

    if (Xin266NonZeroMvd (pu, mvdL1Zero))
    {
        assert(!pu->mergeFlag);

        // amvr_flag
        Xin266EncodeBin (
            &cabacSet->cabac,
            pu->imvIdx > XIN_IMV_OFF,
            cabacSet->context + XIN_CO_IMV_FLAG);

        if (pu->imvIdx)
        {
            // amvr_precision_idx
            Xin266EncodeBin (
                &cabacSet->cabac,
                pu->imvIdx < XIN_IMV_HPEL,
                cabacSet->context + XIN_CO_IMV_FLAG + 4);

            if (pu->imvIdx < XIN_IMV_HPEL)
            {
                Xin266EncodeBin (
                    &cabacSet->cabac,
                    pu->imvIdx > XIN_IMV_FPEL,
                    cabacSet->context + XIN_CO_IMV_FLAG + 1);

            }
        }
    }

}

static void Xin266WriteBcw (
    xin_cabac_context *cabacSet,
    xin_cu_struct     *cu,
    BOOL              checkLdc)
{
    SINT32  numBcw;
    SINT32  bcwCodingIdx;
    UINT32  prefixBitNum;
    UINT32  step;
    UINT32  idx;
    SINT32  codingIdx;
    UINT32  bcwIdx;

    bcwIdx       = cu->pu.bcwIdx;
    numBcw       = checkLdc ? 5 : 3;
    bcwCodingIdx = bcwCodingOrder[bcwIdx];
    prefixBitNum = numBcw - 2;
    
    Xin266EncodeBin (
        &cabacSet->cabac,
        (bcwCodingIdx == 0 ? 0 : 1),
        cabacSet->context + XIN_CO_BCW_IDX);

    if ((numBcw > 2) && (bcwCodingIdx != 0))
    {   
        step      = 1;
        codingIdx = 1;
        
        for(idx = 0; idx < prefixBitNum; ++idx)
        {
            if (bcwCodingIdx == codingIdx)
            {
                Xin266EncodeBinEP (
                    &cabacSet->cabac, 
                    0);
                
                break;
            }
            else
            {
                Xin266EncodeBinEP (
                    &cabacSet->cabac, 
                    1);
                
                codingIdx += step;
            }
        }
    }

}

void Xin266WriteCu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    BOOL           realEntropy)
{
    xin_pic_struct    *picSet;
    xin_seq_struct    *seqSet;
    xin_ctu_struct    *ctu;
    BOOL              skipFlag;
    xin_cabac_context *cabacSet;
    xin_ref_picture   *pictureWrite;
    BOOL              intraFlag;
    BOOL              is4x4Cu;
    UINT32            frameType;

    skipFlag     = (cu->type == XIN_SKIP_MODE);
    intraFlag    = (cu->type >= XIN_INTRA_MODE);
    ctu          = secSet->ctu;
    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    pictureWrite = picSet->pictureWrite;
    frameType    = pictureWrite->frameType;
    cabacSet     = secSet->cabacSet;
    secSet->qp   = (realEntropy && picSet->offlineMode) ? ctu->ctuQp : secSet->qp;
    cu->qp       = secSet->refQp;
    is4x4Cu      = (cu->height == XIN_MIN_CU_SIZE) && (cu->width == XIN_MIN_CU_SIZE);

    // Write skip flag, if frame type is not I frame
    if ((frameType < XIN_I_FRAME) && (!is4x4Cu) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        Xin266WriteSkipFlag (
            cabacSet,
            cu,
            skipFlag);
    }

    if (skipFlag)
    {
        Xin266WriteInterPu (
            secSet,
            cu,
            pictureWrite->numOfRefs,
            pictureWrite->mvdL1Zero,
            frameType);
    }
    else
    {
        // Prediction mode and partitioning data
        if ((frameType < XIN_I_FRAME) && (!is4x4Cu) && (cu->treeMask & XIN_CU_TREE_L_MASK))
        {
            Xin266WritePredMode (
                cabacSet,
                cu,
                intraFlag);
        }

        if (intraFlag)
        {
            if (cu->treeMask & XIN_CU_TREE_L_MASK)
            {
                Xin266WriteLumaIntraPredMode (
                    cabacSet,
                    cu);
            }

            if (cu->treeMask & XIN_CU_TREE_C_MASK)
            {
                Xin266WriteChromaIntraPredMode (
                    cabacSet,
                    seqSet->config.enableCclm,
                    cu);
            }

            Xin266WriteTransTree (
                secSet,
                cu);

        }
        else
        {
            Xin266WriteInterPu (
                secSet,
                cu,
                pictureWrite->numOfRefs,
                pictureWrite->mvdL1Zero,
                frameType);

            if ((seqSet->config.enableAmvr) && (!cu->pu.mergeFlag))
            {
                Xin266WriteImv (
                    cabacSet,
                    cu,
                    pictureWrite->mvdL1Zero);
            }

            if ((seqSet->config.enableBcw) && (!cu->pu.mergeFlag) && (cu->width * cu->height >= 256)
                    && (cu->pu.refIdx[0] >= 0) && (cu->pu.refIdx[1] >= 0))
            {
                Xin266WriteBcw (
                    cabacSet,
                    cu,
                    pictureWrite->checkLDC);
            }

            if (!cu->pu.mergeFlag)
            {
                Xin266WriteRootCbf (
                    cabacSet,
                    cu);
            }

            if (cu->rootCbf)
            {
                Xin266WriteTransTree (
                    secSet,
                    cu);
            }

        }

    }

    if ((cu->cuPelX == 0) && ((cu->offY + cu->height) == seqSet->ctuSize) && (cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        picSet->ctuRowRefQp[ctu->ctuY + 1] = secSet->refQp;
    }

}

void Xin266WriteCuRec (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    BOOL           realEntropy)
{
    xin_cabac_context *cabacSet;
    xin_cu_struct     *childCu;
    SINT32            partIdx;

    cabacSet = secSet->cabacSet;

    // Skip absent CB.
    if (!(cu->geomFlag & XIN_CB_PRESENT))
    {
        return;
    }

    Xin266WriteSplitType (
        cabacSet,
        cu,
        cu->splitType);

    if (cu->splitType != XIN_CU_NO_SPLIT)
    {
        if (cu->splitType == XIN_CU_QUAD_SPLIT)
        {
            for (partIdx = 0; partIdx < 4; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266WriteCuRec (
                    secSet,
                    childCu,
                    realEntropy);
            }

            if ((cu->width == 8) && (cu->height == 8))
            {
                childCu = cu->childCu[4];

                Xin266WriteCu (
                    secSet,
                    childCu,
                    realEntropy);
            }

        }
        else if ((cu->splitType == XIN_CU_HORZ_SPLIT) || (cu->splitType == XIN_CU_VERT_SPLIT))
        {
            for (partIdx = 0; partIdx < 2; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266WriteCuRec (
                    secSet,
                    childCu,
                    realEntropy);
            }
        }
        else if ((cu->splitType == XIN_CU_TRIH_SPLIT) || (cu->splitType == XIN_CU_TRIV_SPLIT))
        {
            for (partIdx = 0; partIdx < 3; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266WriteCuRec (
                    secSet,
                    childCu,
                    realEntropy);
            }
        }
        else
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid cu split type.\n");
        }

    }
    else
    {
        Xin266WriteCu (
            secSet,
            cu,
            realEntropy);
    }

}

void Xin266GetBitCount (
    xin_cabac_struct *cabac,
    UINT32           *bitNum)
{
    xin_bs_struct *bitstream;

    bitstream = &cabac->bitstream;
    *bitNum   = bitstream->bitCount + 23 - cabac->bitsLeft + (cabac->numBufferedBytes << 3);

}


