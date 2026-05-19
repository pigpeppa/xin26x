/***************************************************************************//**
*
* @file          h265p_inverse_1d_trans.c
* @brief         av1 invserse transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "h265p_trans_context.h"
#include "basic_macro.h"
#include "assert.h"

static const SINT32 *XinGetCosPi (
    SINT32 n)
{
    return cosPiData[n - XIN_COS_BIT_MIN];
}

static const SINT32 *XinGetSinPi (
    SINT32 n)
{
    return sinPiData[n - XIN_COS_BIT_MIN];
}

static SINT32 XinRoundShift (
    SINT64 value,
    SINT32 bit)
{
    return (SINT32)((value + (1ll << (bit - 1))) >> bit);
}

static SINT32 XinHalfBtf (
    SINT32 w0,
    SINT32 in0,
    SINT32 w1,
    SINT32 in1,
    SINT32 bit)
{
    SINT64 result;

    result = (SINT64)(w0 * in0) + (SINT64)(w1 * in1);

    return XinRoundShift (result, bit);
}

static inline SINT32 XinClamp (
    SINT32 value,
    SINT32 bit)
{
    SINT32 maxValue;
    SINT32 minValue;

    if (bit <= 0)
    {
        return value;
    }

    maxValue = (1LL << (bit - 1)) - 1;
    minValue = -(1LL << (bit - 1));

    return XIN_CLIP (value, minValue, maxValue);

}

static inline void XinClampBuf (
    SINT32  *buf,
    SINT32  size,
    SINT8   bit)
{
    SINT32 idx;

    for (idx = 0; idx < size; ++idx)
    {
        buf[idx] = XinClamp ( buf[idx], bit);
    }
}

void Xin265pIdct4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0;
    SINT32       *bf1;
    SINT32       step[4];

    assert (output != input);

    cosPi = XinGetCosPi(cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1    = output;
    bf1[0] = input[0];
    bf1[1] = input[2];
    bf1[2] = input[1];
    bf1[3] = input[3];

    // stage 2
    stage++;
    bf0    = output;
    bf1    = step;
    bf1[0] = XinHalfBtf (cosPi[32], bf0[0],  cosPi[32], bf0[1], cosBit);
    bf1[1] = XinHalfBtf (cosPi[32], bf0[0], -cosPi[32], bf0[1], cosBit);
    bf1[2] = XinHalfBtf (cosPi[48], bf0[2], -cosPi[16], bf0[3], cosBit);
    bf1[3] = XinHalfBtf (cosPi[16], bf0[2],  cosPi[48], bf0[3], cosBit);

    // stage 3
    stage++;
    bf0    = step;
    bf1    = output;
    bf1[0] = XinClamp (bf0[0] + bf0[3], rangeBit[stage]);
    bf1[1] = XinClamp (bf0[1] + bf0[2], rangeBit[stage]);
    bf1[2] = XinClamp (bf0[1] - bf0[2], rangeBit[stage]);
    bf1[3] = XinClamp (bf0[0] - bf0[3], rangeBit[stage]);

}

void Xin265pIdct8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *rangeBit)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0;
    SINT32       *bf1;
    SINT32       step[8];

    assert(output != input);

    cosPi = XinGetCosPi(cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1    = output;
    bf1[0] = input[0];
    bf1[1] = input[4];
    bf1[2] = input[2];
    bf1[3] = input[6];
    bf1[4] = input[1];
    bf1[5] = input[5];
    bf1[6] = input[3];
    bf1[7] = input[7];

    // stage 2
    stage++;
    bf0    = output;
    bf1    = step;
    bf1[0] = bf0[0];
    bf1[1] = bf0[1];
    bf1[2] = bf0[2];
    bf1[3] = bf0[3];
    bf1[4] = XinHalfBtf (cosPi[56], bf0[4], -cosPi[8],  bf0[7], cosBit);
    bf1[5] = XinHalfBtf (cosPi[24], bf0[5], -cosPi[40], bf0[6], cosBit);
    bf1[6] = XinHalfBtf (cosPi[40], bf0[5],  cosPi[24], bf0[6], cosBit);
    bf1[7] = XinHalfBtf (cosPi[8],  bf0[4],  cosPi[56], bf0[7], cosBit);

    // stage 3
    stage++;
    bf0    = step;
    bf1    = output;
    bf1[0] = XinHalfBtf (cosPi[32], bf0[0],  cosPi[32], bf0[1], cosBit);
    bf1[1] = XinHalfBtf (cosPi[32], bf0[0], -cosPi[32], bf0[1], cosBit);
    bf1[2] = XinHalfBtf (cosPi[48], bf0[2], -cosPi[16], bf0[3], cosBit);
    bf1[3] = XinHalfBtf (cosPi[16], bf0[2],  cosPi[48], bf0[3], cosBit);
    bf1[4] = XinClamp ( bf0[4] + bf0[5], rangeBit[stage]);
    bf1[5] = XinClamp ( bf0[4] - bf0[5], rangeBit[stage]);
    bf1[6] = XinClamp (-bf0[6] + bf0[7], rangeBit[stage]);
    bf1[7] = XinClamp ( bf0[6] + bf0[7], rangeBit[stage]);

    // stage 4
    stage++;
    bf0    = output;
    bf1    = step;
    bf1[0] = XinClamp (bf0[0] + bf0[3], rangeBit[stage]);
    bf1[1] = XinClamp (bf0[1] + bf0[2], rangeBit[stage]);
    bf1[2] = XinClamp (bf0[1] - bf0[2], rangeBit[stage]);
    bf1[3] = XinClamp (bf0[0] - bf0[3], rangeBit[stage]);
    bf1[4] = bf0[4];
    bf1[5] = XinHalfBtf (-cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[6] = XinHalfBtf ( cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[7] = bf0[7];

    // stage 5
    stage++;
    bf0    = step;
    bf1    = output;
    bf1[0] = XinClamp (bf0[0] + bf0[7], rangeBit[stage]);
    bf1[1] = XinClamp (bf0[1] + bf0[6], rangeBit[stage]);
    bf1[2] = XinClamp (bf0[2] + bf0[5], rangeBit[stage]);
    bf1[3] = XinClamp (bf0[3] + bf0[4], rangeBit[stage]);
    bf1[4] = XinClamp (bf0[3] - bf0[4], rangeBit[stage]);
    bf1[5] = XinClamp (bf0[2] - bf0[5], rangeBit[stage]);
    bf1[6] = XinClamp (bf0[1] - bf0[6], rangeBit[stage]);
    bf1[7] = XinClamp (bf0[0] - bf0[7], rangeBit[stage]);

}

void Xin265pIdct16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0;
    SINT32       *bf1;
    SINT32       step[16];

    assert(output != input);

    cosPi = XinGetCosPi(cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1     = output;
    bf1[0]  = input[0];
    bf1[1]  = input[8];
    bf1[2]  = input[4];
    bf1[3]  = input[12];
    bf1[4]  = input[2];
    bf1[5]  = input[10];
    bf1[6]  = input[6];
    bf1[7]  = input[14];
    bf1[8]  = input[1];
    bf1[9]  = input[9];
    bf1[10] = input[5];
    bf1[11] = input[13];
    bf1[12] = input[3];
    bf1[13] = input[11];
    bf1[14] = input[7];
    bf1[15] = input[15];

    // stage 2
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = XinHalfBtf (cosPi[60], bf0[8],  -cosPi[4],  bf0[15], cosBit);
    bf1[9]  = XinHalfBtf (cosPi[28], bf0[9],  -cosPi[36], bf0[14], cosBit);
    bf1[10] = XinHalfBtf (cosPi[44], bf0[10], -cosPi[20], bf0[13], cosBit);
    bf1[11] = XinHalfBtf (cosPi[12], bf0[11], -cosPi[52], bf0[12], cosBit);
    bf1[12] = XinHalfBtf (cosPi[52], bf0[11],  cosPi[12], bf0[12], cosBit);
    bf1[13] = XinHalfBtf (cosPi[20], bf0[10],  cosPi[44], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (cosPi[36], bf0[9],   cosPi[28], bf0[14], cosBit);
    bf1[15] = XinHalfBtf (cosPi[4],  bf0[8],   cosPi[60], bf0[15], cosBit);

    // stage 3
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = XinHalfBtf (cosPi[56], bf0[4], -cosPi[8],  bf0[7], cosBit);
    bf1[5]  = XinHalfBtf (cosPi[24], bf0[5], -cosPi[40], bf0[6], cosBit);
    bf1[6]  = XinHalfBtf (cosPi[40], bf0[5],  cosPi[24], bf0[6], cosBit);
    bf1[7]  = XinHalfBtf (cosPi[8],  bf0[4],  cosPi[56], bf0[7], cosBit);
    bf1[8]  = XinClamp ( bf0[8]  + bf0[9],  bitRange[stage]);
    bf1[9]  = XinClamp ( bf0[8]  - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (-bf0[10] + bf0[11], bitRange[stage]);
    bf1[11] = XinClamp ( bf0[10] + bf0[11], bitRange[stage]);
    bf1[12] = XinClamp ( bf0[12] + bf0[13], bitRange[stage]);
    bf1[13] = XinClamp ( bf0[12] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (-bf0[14] + bf0[15], bitRange[stage]);
    bf1[15] = XinClamp ( bf0[14] + bf0[15], bitRange[stage]);

    // stage 4
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinHalfBtf (cosPi[32], bf0[0],  cosPi[32], bf0[1], cosBit);
    bf1[1]  = XinHalfBtf (cosPi[32], bf0[0], -cosPi[32], bf0[1], cosBit);
    bf1[2]  = XinHalfBtf (cosPi[48], bf0[2], -cosPi[16], bf0[3], cosBit);
    bf1[3]  = XinHalfBtf (cosPi[16], bf0[2],  cosPi[48], bf0[3], cosBit);
    bf1[4]  = XinClamp ( bf0[4] + bf0[5], bitRange[stage]);
    bf1[5]  = XinClamp ( bf0[4] - bf0[5], bitRange[stage]);
    bf1[6]  = XinClamp (-bf0[6] + bf0[7], bitRange[stage]);
    bf1[7]  = XinClamp ( bf0[6] + bf0[7], bitRange[stage]);
    bf1[8]  = bf0[8];
    bf1[9]  = XinHalfBtf (-cosPi[16], bf0[9],   cosPi[48], bf0[14], cosBit);
    bf1[10] = XinHalfBtf (-cosPi[48], bf0[10], -cosPi[16], bf0[13], cosBit);
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = XinHalfBtf (-cosPi[16], bf0[10], cosPi[48], bf0[13], cosBit);
    bf1[14] = XinHalfBtf ( cosPi[48], bf0[9],  cosPi[16], bf0[14], cosBit);
    bf1[15] = bf0[15];

    // stage 5
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0] + bf0[3], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[2], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[1] - bf0[2], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[0] - bf0[3], bitRange[stage]);
    bf1[4]  = bf0[4];
    bf1[5]  = XinHalfBtf (-cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[6]  = XinHalfBtf ( cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[7]  = bf0[7];
    bf1[8]  = XinClamp ( bf0[8]  + bf0[11], bitRange[stage]);
    bf1[9]  = XinClamp ( bf0[9]  + bf0[10], bitRange[stage]);
    bf1[10] = XinClamp ( bf0[9]  - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp ( bf0[8]  - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (-bf0[12] + bf0[15], bitRange[stage]);
    bf1[13] = XinClamp (-bf0[13] + bf0[14], bitRange[stage]);
    bf1[14] = XinClamp ( bf0[13] + bf0[14], bitRange[stage]);
    bf1[15] = XinClamp ( bf0[12] + bf0[15], bitRange[stage]);

    // stage 6
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinClamp (bf0[0] + bf0[7], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[6], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[5], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[4], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[3] - bf0[4], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[2] - bf0[5], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[1] - bf0[6], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[0] - bf0[7], bitRange[stage]);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = XinHalfBtf (-cosPi[32], bf0[10], cosPi[32], bf0[13], cosBit);
    bf1[11] = XinHalfBtf (-cosPi[32], bf0[11], cosPi[32], bf0[12], cosBit);
    bf1[12] = XinHalfBtf ( cosPi[32], bf0[11], cosPi[32], bf0[12], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[32], bf0[10], cosPi[32], bf0[13], cosBit);
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];

    // stage 7
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0] + bf0[15], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[14], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[13], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[12], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4] + bf0[11], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5] + bf0[10], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6] + bf0[9],  bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7] + bf0[8],  bitRange[stage]);
    bf1[8]  = XinClamp (bf0[7] - bf0[8],  bitRange[stage]);
    bf1[9]  = XinClamp (bf0[6] - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (bf0[5] - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp (bf0[4] - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (bf0[3] - bf0[12], bitRange[stage]);
    bf1[13] = XinClamp (bf0[2] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (bf0[1] - bf0[14], bitRange[stage]);
    bf1[15] = XinClamp (bf0[0] - bf0[15], bitRange[stage]);

}

void Xin265pIdct32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0;
    SINT32       *bf1;
    SINT32       step[32];

    assert(output != input);

    cosPi = XinGetCosPi(cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1     = output;
    bf1[0]  = input[0];
    bf1[1]  = input[16];
    bf1[2]  = input[8];
    bf1[3]  = input[24];
    bf1[4]  = input[4];
    bf1[5]  = input[20];
    bf1[6]  = input[12];
    bf1[7]  = input[28];
    bf1[8]  = input[2];
    bf1[9]  = input[18];
    bf1[10] = input[10];
    bf1[11] = input[26];
    bf1[12] = input[6];
    bf1[13] = input[22];
    bf1[14] = input[14];
    bf1[15] = input[30];
    bf1[16] = input[1];
    bf1[17] = input[17];
    bf1[18] = input[9];
    bf1[19] = input[25];
    bf1[20] = input[5];
    bf1[21] = input[21];
    bf1[22] = input[13];
    bf1[23] = input[29];
    bf1[24] = input[3];
    bf1[25] = input[19];
    bf1[26] = input[11];
    bf1[27] = input[27];
    bf1[28] = input[7];
    bf1[29] = input[23];
    bf1[30] = input[15];
    bf1[31] = input[31];

    // stage 2
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = bf0[10];
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = bf0[13];
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];
    bf1[16] = XinHalfBtf (cosPi[62], bf0[16], -cosPi[2],  bf0[31], cosBit);
    bf1[17] = XinHalfBtf (cosPi[30], bf0[17], -cosPi[34], bf0[30], cosBit);
    bf1[18] = XinHalfBtf (cosPi[46], bf0[18], -cosPi[18], bf0[29], cosBit);
    bf1[19] = XinHalfBtf (cosPi[14], bf0[19], -cosPi[50], bf0[28], cosBit);
    bf1[20] = XinHalfBtf (cosPi[54], bf0[20], -cosPi[10], bf0[27], cosBit);
    bf1[21] = XinHalfBtf (cosPi[22], bf0[21], -cosPi[42], bf0[26], cosBit);
    bf1[22] = XinHalfBtf (cosPi[38], bf0[22], -cosPi[26], bf0[25], cosBit);
    bf1[23] = XinHalfBtf (cosPi[6],  bf0[23], -cosPi[58], bf0[24], cosBit);
    bf1[24] = XinHalfBtf (cosPi[58], bf0[23],  cosPi[6],  bf0[24], cosBit);
    bf1[25] = XinHalfBtf (cosPi[26], bf0[22],  cosPi[38], bf0[25], cosBit);
    bf1[26] = XinHalfBtf (cosPi[42], bf0[21],  cosPi[22], bf0[26], cosBit);
    bf1[27] = XinHalfBtf (cosPi[10], bf0[20],  cosPi[54], bf0[27], cosBit);
    bf1[28] = XinHalfBtf (cosPi[50], bf0[19],  cosPi[14], bf0[28], cosBit);
    bf1[29] = XinHalfBtf (cosPi[18], bf0[18],  cosPi[46], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (cosPi[34], bf0[17],  cosPi[30], bf0[30], cosBit);
    bf1[31] = XinHalfBtf (cosPi[2], bf0[16],   cosPi[62], bf0[31], cosBit);

    // stage 3
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = XinHalfBtf (cosPi[60], bf0[8], -cosPi[4],   bf0[15], cosBit);
    bf1[9]  = XinHalfBtf (cosPi[28], bf0[9], -cosPi[36],  bf0[14], cosBit);
    bf1[10] = XinHalfBtf (cosPi[44], bf0[10], -cosPi[20], bf0[13], cosBit);
    bf1[11] = XinHalfBtf (cosPi[12], bf0[11], -cosPi[52], bf0[12], cosBit);
    bf1[12] = XinHalfBtf (cosPi[52], bf0[11],  cosPi[12], bf0[12], cosBit);
    bf1[13] = XinHalfBtf (cosPi[20], bf0[10],  cosPi[44], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (cosPi[36], bf0[9],   cosPi[28], bf0[14], cosBit);
    bf1[15] = XinHalfBtf (cosPi[4],  bf0[8],   cosPi[60], bf0[15], cosBit);
    bf1[16] = XinClamp ( bf0[16] + bf0[17], bitRange[stage]);
    bf1[17] = XinClamp ( bf0[16] - bf0[17], bitRange[stage]);
    bf1[18] = XinClamp (-bf0[18] + bf0[19], bitRange[stage]);
    bf1[19] = XinClamp ( bf0[18] + bf0[19], bitRange[stage]);
    bf1[20] = XinClamp ( bf0[20] + bf0[21], bitRange[stage]);
    bf1[21] = XinClamp ( bf0[20] - bf0[21], bitRange[stage]);
    bf1[22] = XinClamp (-bf0[22] + bf0[23], bitRange[stage]);
    bf1[23] = XinClamp ( bf0[22] + bf0[23], bitRange[stage]);
    bf1[24] = XinClamp ( bf0[24] + bf0[25], bitRange[stage]);
    bf1[25] = XinClamp ( bf0[24] - bf0[25], bitRange[stage]);
    bf1[26] = XinClamp (-bf0[26] + bf0[27], bitRange[stage]);
    bf1[27] = XinClamp ( bf0[26] + bf0[27], bitRange[stage]);
    bf1[28] = XinClamp ( bf0[28] + bf0[29], bitRange[stage]);
    bf1[29] = XinClamp ( bf0[28] - bf0[29], bitRange[stage]);
    bf1[30] = XinClamp (-bf0[30] + bf0[31], bitRange[stage]);
    bf1[31] = XinClamp ( bf0[30] + bf0[31], bitRange[stage]);

    // stage 4
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = XinHalfBtf (cosPi[56], bf0[4], -cosPi[8],  bf0[7], cosBit);
    bf1[5]  = XinHalfBtf (cosPi[24], bf0[5], -cosPi[40], bf0[6], cosBit);
    bf1[6]  = XinHalfBtf (cosPi[40], bf0[5],  cosPi[24], bf0[6], cosBit);
    bf1[7]  = XinHalfBtf (cosPi[8],  bf0[4],  cosPi[56], bf0[7], cosBit);
    bf1[8]  = XinClamp ( bf0[8]  + bf0[9],  bitRange[stage]);
    bf1[9]  = XinClamp ( bf0[8]  - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (-bf0[10] + bf0[11], bitRange[stage]);
    bf1[11] = XinClamp ( bf0[10] + bf0[11], bitRange[stage]);
    bf1[12] = XinClamp ( bf0[12] + bf0[13], bitRange[stage]);
    bf1[13] = XinClamp ( bf0[12] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (-bf0[14] + bf0[15], bitRange[stage]);
    bf1[15] = XinClamp ( bf0[14] + bf0[15], bitRange[stage]);
    bf1[16] = bf0[16];
    bf1[17] = XinHalfBtf (-cosPi[8], bf0[17], cosPi[56], bf0[30], cosBit);
    bf1[18] = XinHalfBtf (-cosPi[56], bf0[18], -cosPi[8], bf0[29], cosBit);
    bf1[19] = bf0[19];
    bf1[20] = bf0[20];
    bf1[21] = XinHalfBtf (-cosPi[40], bf0[21], cosPi[24], bf0[26], cosBit);
    bf1[22] = XinHalfBtf (-cosPi[24], bf0[22], -cosPi[40], bf0[25], cosBit);
    bf1[23] = bf0[23];
    bf1[24] = bf0[24];
    bf1[25] = XinHalfBtf (-cosPi[40], bf0[22], cosPi[24], bf0[25], cosBit);
    bf1[26] = XinHalfBtf (cosPi[24], bf0[21], cosPi[40], bf0[26], cosBit);
    bf1[27] = bf0[27];
    bf1[28] = bf0[28];
    bf1[29] = XinHalfBtf (-cosPi[8], bf0[18], cosPi[56], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (cosPi[56], bf0[17], cosPi[8], bf0[30], cosBit);
    bf1[31] = bf0[31];
    //range_check_buf(stage, input, bf1, size, bitRange[stage]);

    // stage 5
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinHalfBtf (cosPi[32], bf0[0],  cosPi[32], bf0[1], cosBit);
    bf1[1]  = XinHalfBtf (cosPi[32], bf0[0], -cosPi[32], bf0[1], cosBit);
    bf1[2]  = XinHalfBtf (cosPi[48], bf0[2], -cosPi[16], bf0[3], cosBit);
    bf1[3]  = XinHalfBtf (cosPi[16], bf0[2],  cosPi[48], bf0[3], cosBit);
    bf1[4]  = XinClamp ( bf0[4] + bf0[5], bitRange[stage]);
    bf1[5]  = XinClamp ( bf0[4] - bf0[5], bitRange[stage]);
    bf1[6]  = XinClamp (-bf0[6] + bf0[7], bitRange[stage]);
    bf1[7]  = XinClamp ( bf0[6] + bf0[7], bitRange[stage]);
    bf1[8]  = bf0[8];
    bf1[9]  = XinHalfBtf (-cosPi[16], bf0[9],   cosPi[48], bf0[14], cosBit);
    bf1[10] = XinHalfBtf (-cosPi[48], bf0[10], -cosPi[16], bf0[13], cosBit);
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = XinHalfBtf (-cosPi[16], bf0[10], cosPi[48], bf0[13], cosBit);
    bf1[14] = XinHalfBtf ( cosPi[48], bf0[9],  cosPi[16], bf0[14], cosBit);
    bf1[15] = bf0[15];
    bf1[16] = XinClamp ( bf0[16] + bf0[19], bitRange[stage]);
    bf1[17] = XinClamp ( bf0[17] + bf0[18], bitRange[stage]);
    bf1[18] = XinClamp ( bf0[17] - bf0[18], bitRange[stage]);
    bf1[19] = XinClamp ( bf0[16] - bf0[19], bitRange[stage]);
    bf1[20] = XinClamp (-bf0[20] + bf0[23], bitRange[stage]);
    bf1[21] = XinClamp (-bf0[21] + bf0[22], bitRange[stage]);
    bf1[22] = XinClamp ( bf0[21] + bf0[22], bitRange[stage]);
    bf1[23] = XinClamp ( bf0[20] + bf0[23], bitRange[stage]);
    bf1[24] = XinClamp ( bf0[24] + bf0[27], bitRange[stage]);
    bf1[25] = XinClamp ( bf0[25] + bf0[26], bitRange[stage]);
    bf1[26] = XinClamp ( bf0[25] - bf0[26], bitRange[stage]);
    bf1[27] = XinClamp ( bf0[24] - bf0[27], bitRange[stage]);
    bf1[28] = XinClamp (-bf0[28] + bf0[31], bitRange[stage]);
    bf1[29] = XinClamp (-bf0[29] + bf0[30], bitRange[stage]);
    bf1[30] = XinClamp ( bf0[29] + bf0[30], bitRange[stage]);
    bf1[31] = XinClamp ( bf0[28] + bf0[31], bitRange[stage]);

    // stage 6
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinClamp (bf0[0] + bf0[3], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[2], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[1] - bf0[2], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[0] - bf0[3], bitRange[stage]);
    bf1[4]  = bf0[4];
    bf1[5]  = XinHalfBtf (-cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[6]  = XinHalfBtf ( cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[7]  = bf0[7];
    bf1[8]  = XinClamp ( bf0[8]  + bf0[11], bitRange[stage]);
    bf1[9]  = XinClamp ( bf0[9]  + bf0[10], bitRange[stage]);
    bf1[10] = XinClamp ( bf0[9]  - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp ( bf0[8]  - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (-bf0[12] + bf0[15], bitRange[stage]);
    bf1[13] = XinClamp (-bf0[13] + bf0[14], bitRange[stage]);
    bf1[14] = XinClamp ( bf0[13] + bf0[14], bitRange[stage]);
    bf1[15] = XinClamp ( bf0[12] + bf0[15], bitRange[stage]);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = XinHalfBtf (-cosPi[16], bf0[18],  cosPi[48], bf0[29], cosBit);
    bf1[19] = XinHalfBtf (-cosPi[16], bf0[19],  cosPi[48], bf0[28], cosBit);
    bf1[20] = XinHalfBtf (-cosPi[48], bf0[20], -cosPi[16], bf0[27], cosBit);
    bf1[21] = XinHalfBtf (-cosPi[48], bf0[21], -cosPi[16], bf0[26], cosBit);
    bf1[22] = bf0[22];
    bf1[23] = bf0[23];
    bf1[24] = bf0[24];
    bf1[25] = bf0[25];
    bf1[26] = XinHalfBtf (-cosPi[16], bf0[21], cosPi[48], bf0[26], cosBit);
    bf1[27] = XinHalfBtf (-cosPi[16], bf0[20], cosPi[48], bf0[27], cosBit);
    bf1[28] = XinHalfBtf ( cosPi[48], bf0[19], cosPi[16], bf0[28], cosBit);
    bf1[29] = XinHalfBtf ( cosPi[48], bf0[18], cosPi[16], bf0[29], cosBit);
    bf1[30] = bf0[30];
    bf1[31] = bf0[31];

    // stage 7
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0] + bf0[7], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[6], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[5], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[4], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[3] - bf0[4], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[2] - bf0[5], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[1] - bf0[6], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[0] - bf0[7], bitRange[stage]);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = XinHalfBtf (-cosPi[32], bf0[10], cosPi[32], bf0[13], cosBit);
    bf1[11] = XinHalfBtf (-cosPi[32], bf0[11], cosPi[32], bf0[12], cosBit);
    bf1[12] = XinHalfBtf ( cosPi[32], bf0[11], cosPi[32], bf0[12], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[32], bf0[10], cosPi[32], bf0[13], cosBit);
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];
    bf1[16] = XinClamp ( bf0[16] + bf0[23], bitRange[stage]);
    bf1[17] = XinClamp ( bf0[17] + bf0[22], bitRange[stage]);
    bf1[18] = XinClamp ( bf0[18] + bf0[21], bitRange[stage]);
    bf1[19] = XinClamp ( bf0[19] + bf0[20], bitRange[stage]);
    bf1[20] = XinClamp ( bf0[19] - bf0[20], bitRange[stage]);
    bf1[21] = XinClamp ( bf0[18] - bf0[21], bitRange[stage]);
    bf1[22] = XinClamp ( bf0[17] - bf0[22], bitRange[stage]);
    bf1[23] = XinClamp ( bf0[16] - bf0[23], bitRange[stage]);
    bf1[24] = XinClamp (-bf0[24] + bf0[31], bitRange[stage]);
    bf1[25] = XinClamp (-bf0[25] + bf0[30], bitRange[stage]);
    bf1[26] = XinClamp (-bf0[26] + bf0[29], bitRange[stage]);
    bf1[27] = XinClamp (-bf0[27] + bf0[28], bitRange[stage]);
    bf1[28] = XinClamp ( bf0[27] + bf0[28], bitRange[stage]);
    bf1[29] = XinClamp ( bf0[26] + bf0[29], bitRange[stage]);
    bf1[30] = XinClamp ( bf0[25] + bf0[30], bitRange[stage]);
    bf1[31] = XinClamp ( bf0[24] + bf0[31], bitRange[stage]);

    // stage 8
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinClamp (bf0[0] + bf0[15], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[14], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[13], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[12], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4] + bf0[11], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5] + bf0[10], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6] + bf0[9],  bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7] + bf0[8],  bitRange[stage]);
    bf1[8]  = XinClamp (bf0[7] - bf0[8],  bitRange[stage]);
    bf1[9]  = XinClamp (bf0[6] - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (bf0[5] - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp (bf0[4] - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (bf0[3] - bf0[12], bitRange[stage]);
    bf1[13] = XinClamp (bf0[2] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (bf0[1] - bf0[14], bitRange[stage]);
    bf1[15] = XinClamp (bf0[0] - bf0[15], bitRange[stage]);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = bf0[18];
    bf1[19] = bf0[19];
    bf1[20] = XinHalfBtf (-cosPi[32], bf0[20], cosPi[32], bf0[27], cosBit);
    bf1[21] = XinHalfBtf (-cosPi[32], bf0[21], cosPi[32], bf0[26], cosBit);
    bf1[22] = XinHalfBtf (-cosPi[32], bf0[22], cosPi[32], bf0[25], cosBit);
    bf1[23] = XinHalfBtf (-cosPi[32], bf0[23], cosPi[32], bf0[24], cosBit);
    bf1[24] = XinHalfBtf ( cosPi[32], bf0[23], cosPi[32], bf0[24], cosBit);
    bf1[25] = XinHalfBtf ( cosPi[32], bf0[22], cosPi[32], bf0[25], cosBit);
    bf1[26] = XinHalfBtf ( cosPi[32], bf0[21], cosPi[32], bf0[26], cosBit);
    bf1[27] = XinHalfBtf ( cosPi[32], bf0[20], cosPi[32], bf0[27], cosBit);
    bf1[28] = bf0[28];
    bf1[29] = bf0[29];
    bf1[30] = bf0[30];
    bf1[31] = bf0[31];

    // stage 9
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0]  + bf0[31], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1]  + bf0[30], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2]  + bf0[29], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3]  + bf0[28], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4]  + bf0[27], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5]  + bf0[26], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6]  + bf0[25], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7]  + bf0[24], bitRange[stage]);
    bf1[8]  = XinClamp (bf0[8]  + bf0[23], bitRange[stage]);
    bf1[9]  = XinClamp (bf0[9]  + bf0[22], bitRange[stage]);
    bf1[10] = XinClamp (bf0[10] + bf0[21], bitRange[stage]);
    bf1[11] = XinClamp (bf0[11] + bf0[20], bitRange[stage]);
    bf1[12] = XinClamp (bf0[12] + bf0[19], bitRange[stage]);
    bf1[13] = XinClamp (bf0[13] + bf0[18], bitRange[stage]);
    bf1[14] = XinClamp (bf0[14] + bf0[17], bitRange[stage]);
    bf1[15] = XinClamp (bf0[15] + bf0[16], bitRange[stage]);
    bf1[16] = XinClamp (bf0[15] - bf0[16], bitRange[stage]);
    bf1[17] = XinClamp (bf0[14] - bf0[17], bitRange[stage]);
    bf1[18] = XinClamp (bf0[13] - bf0[18], bitRange[stage]);
    bf1[19] = XinClamp (bf0[12] - bf0[19], bitRange[stage]);
    bf1[20] = XinClamp (bf0[11] - bf0[20], bitRange[stage]);
    bf1[21] = XinClamp (bf0[10] - bf0[21], bitRange[stage]);
    bf1[22] = XinClamp (bf0[9]  - bf0[22], bitRange[stage]);
    bf1[23] = XinClamp (bf0[8]  - bf0[23], bitRange[stage]);
    bf1[24] = XinClamp (bf0[7]  - bf0[24], bitRange[stage]);
    bf1[25] = XinClamp (bf0[6]  - bf0[25], bitRange[stage]);
    bf1[26] = XinClamp (bf0[5]  - bf0[26], bitRange[stage]);
    bf1[27] = XinClamp (bf0[4]  - bf0[27], bitRange[stage]);
    bf1[28] = XinClamp (bf0[3]  - bf0[28], bitRange[stage]);
    bf1[29] = XinClamp (bf0[2]  - bf0[29], bitRange[stage]);
    bf1[30] = XinClamp (bf0[1]  - bf0[30], bitRange[stage]);
    bf1[31] = XinClamp (bf0[0]  - bf0[31], bitRange[stage]);

}

void Xin265pIdct64 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0;
    SINT32       *bf1;
    SINT32       step[64];

    assert(output != input);

    cosPi = XinGetCosPi(cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1     = output;
    bf1[0]  = input[0];
    bf1[1]  = input[32];
    bf1[2]  = input[16];
    bf1[3]  = input[48];
    bf1[4]  = input[8];
    bf1[5]  = input[40];
    bf1[6]  = input[24];
    bf1[7]  = input[56];
    bf1[8]  = input[4];
    bf1[9]  = input[36];
    bf1[10] = input[20];
    bf1[11] = input[52];
    bf1[12] = input[12];
    bf1[13] = input[44];
    bf1[14] = input[28];
    bf1[15] = input[60];
    bf1[16] = input[2];
    bf1[17] = input[34];
    bf1[18] = input[18];
    bf1[19] = input[50];
    bf1[20] = input[10];
    bf1[21] = input[42];
    bf1[22] = input[26];
    bf1[23] = input[58];
    bf1[24] = input[6];
    bf1[25] = input[38];
    bf1[26] = input[22];
    bf1[27] = input[54];
    bf1[28] = input[14];
    bf1[29] = input[46];
    bf1[30] = input[30];
    bf1[31] = input[62];
    bf1[32] = input[1];
    bf1[33] = input[33];
    bf1[34] = input[17];
    bf1[35] = input[49];
    bf1[36] = input[9];
    bf1[37] = input[41];
    bf1[38] = input[25];
    bf1[39] = input[57];
    bf1[40] = input[5];
    bf1[41] = input[37];
    bf1[42] = input[21];
    bf1[43] = input[53];
    bf1[44] = input[13];
    bf1[45] = input[45];
    bf1[46] = input[29];
    bf1[47] = input[61];
    bf1[48] = input[3];
    bf1[49] = input[35];
    bf1[50] = input[19];
    bf1[51] = input[51];
    bf1[52] = input[11];
    bf1[53] = input[43];
    bf1[54] = input[27];
    bf1[55] = input[59];
    bf1[56] = input[7];
    bf1[57] = input[39];
    bf1[58] = input[23];
    bf1[59] = input[55];
    bf1[60] = input[15];
    bf1[61] = input[47];
    bf1[62] = input[31];
    bf1[63] = input[63];

    // stage 2
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = bf0[10];
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = bf0[13];
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = bf0[18];
    bf1[19] = bf0[19];
    bf1[20] = bf0[20];
    bf1[21] = bf0[21];
    bf1[22] = bf0[22];
    bf1[23] = bf0[23];
    bf1[24] = bf0[24];
    bf1[25] = bf0[25];
    bf1[26] = bf0[26];
    bf1[27] = bf0[27];
    bf1[28] = bf0[28];
    bf1[29] = bf0[29];
    bf1[30] = bf0[30];
    bf1[31] = bf0[31];
    bf1[32] = XinHalfBtf (cosPi[63], bf0[32], -cosPi[1],  bf0[63], cosBit);
    bf1[33] = XinHalfBtf (cosPi[31], bf0[33], -cosPi[33], bf0[62], cosBit);
    bf1[34] = XinHalfBtf (cosPi[47], bf0[34], -cosPi[17], bf0[61], cosBit);
    bf1[35] = XinHalfBtf (cosPi[15], bf0[35], -cosPi[49], bf0[60], cosBit);
    bf1[36] = XinHalfBtf (cosPi[55], bf0[36], -cosPi[9],  bf0[59], cosBit);
    bf1[37] = XinHalfBtf (cosPi[23], bf0[37], -cosPi[41], bf0[58], cosBit);
    bf1[38] = XinHalfBtf (cosPi[39], bf0[38], -cosPi[25], bf0[57], cosBit);
    bf1[39] = XinHalfBtf (cosPi[7],  bf0[39], -cosPi[57], bf0[56], cosBit);
    bf1[40] = XinHalfBtf (cosPi[59], bf0[40], -cosPi[5],  bf0[55], cosBit);
    bf1[41] = XinHalfBtf (cosPi[27], bf0[41], -cosPi[37], bf0[54], cosBit);
    bf1[42] = XinHalfBtf (cosPi[43], bf0[42], -cosPi[21], bf0[53], cosBit);
    bf1[43] = XinHalfBtf (cosPi[11], bf0[43], -cosPi[53], bf0[52], cosBit);
    bf1[44] = XinHalfBtf (cosPi[51], bf0[44], -cosPi[13], bf0[51], cosBit);
    bf1[45] = XinHalfBtf (cosPi[19], bf0[45], -cosPi[45], bf0[50], cosBit);
    bf1[46] = XinHalfBtf (cosPi[35], bf0[46], -cosPi[29], bf0[49], cosBit);
    bf1[47] = XinHalfBtf (cosPi[3],  bf0[47], -cosPi[61], bf0[48], cosBit);
    bf1[48] = XinHalfBtf (cosPi[61], bf0[47],  cosPi[3],  bf0[48], cosBit);
    bf1[49] = XinHalfBtf (cosPi[29], bf0[46],  cosPi[35], bf0[49], cosBit);
    bf1[50] = XinHalfBtf (cosPi[45], bf0[45],  cosPi[19], bf0[50], cosBit);
    bf1[51] = XinHalfBtf (cosPi[13], bf0[44],  cosPi[51], bf0[51], cosBit);
    bf1[52] = XinHalfBtf (cosPi[53], bf0[43],  cosPi[11], bf0[52], cosBit);
    bf1[53] = XinHalfBtf (cosPi[21], bf0[42],  cosPi[43], bf0[53], cosBit);
    bf1[54] = XinHalfBtf (cosPi[37], bf0[41],  cosPi[27], bf0[54], cosBit);
    bf1[55] = XinHalfBtf (cosPi[5],  bf0[40],  cosPi[59], bf0[55], cosBit);
    bf1[56] = XinHalfBtf (cosPi[57], bf0[39],  cosPi[7],  bf0[56], cosBit);
    bf1[57] = XinHalfBtf (cosPi[25], bf0[38],  cosPi[39], bf0[57], cosBit);
    bf1[58] = XinHalfBtf (cosPi[41], bf0[37],  cosPi[23], bf0[58], cosBit);
    bf1[59] = XinHalfBtf (cosPi[9],  bf0[36],  cosPi[55], bf0[59], cosBit);
    bf1[60] = XinHalfBtf (cosPi[49], bf0[35],  cosPi[15], bf0[60], cosBit);
    bf1[61] = XinHalfBtf (cosPi[17], bf0[34],  cosPi[47], bf0[61], cosBit);
    bf1[62] = XinHalfBtf (cosPi[33], bf0[33],  cosPi[31], bf0[62], cosBit);
    bf1[63] = XinHalfBtf (cosPi[1],  bf0[32],  cosPi[63], bf0[63], cosBit);

    // stage 3
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = bf0[10];
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = bf0[13];
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];
    bf1[16] = XinHalfBtf (cosPi[62], bf0[16], -cosPi[2],  bf0[31], cosBit);
    bf1[17] = XinHalfBtf (cosPi[30], bf0[17], -cosPi[34], bf0[30], cosBit);
    bf1[18] = XinHalfBtf (cosPi[46], bf0[18], -cosPi[18], bf0[29], cosBit);
    bf1[19] = XinHalfBtf (cosPi[14], bf0[19], -cosPi[50], bf0[28], cosBit);
    bf1[20] = XinHalfBtf (cosPi[54], bf0[20], -cosPi[10], bf0[27], cosBit);
    bf1[21] = XinHalfBtf (cosPi[22], bf0[21], -cosPi[42], bf0[26], cosBit);
    bf1[22] = XinHalfBtf (cosPi[38], bf0[22], -cosPi[26], bf0[25], cosBit);
    bf1[23] = XinHalfBtf (cosPi[6],  bf0[23], -cosPi[58], bf0[24], cosBit);
    bf1[24] = XinHalfBtf (cosPi[58], bf0[23],  cosPi[6],  bf0[24], cosBit);
    bf1[25] = XinHalfBtf (cosPi[26], bf0[22],  cosPi[38], bf0[25], cosBit);
    bf1[26] = XinHalfBtf (cosPi[42], bf0[21],  cosPi[22], bf0[26], cosBit);
    bf1[27] = XinHalfBtf (cosPi[10], bf0[20],  cosPi[54], bf0[27], cosBit);
    bf1[28] = XinHalfBtf (cosPi[50], bf0[19],  cosPi[14], bf0[28], cosBit);
    bf1[29] = XinHalfBtf (cosPi[18], bf0[18],  cosPi[46], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (cosPi[34], bf0[17],  cosPi[30], bf0[30], cosBit);
    bf1[31] = XinHalfBtf (cosPi[2], bf0[16],   cosPi[62], bf0[31], cosBit);
    bf1[32] = XinClamp ( bf0[32] + bf0[33], bitRange[stage]);
    bf1[33] = XinClamp ( bf0[32] - bf0[33], bitRange[stage]);
    bf1[34] = XinClamp (-bf0[34] + bf0[35], bitRange[stage]);
    bf1[35] = XinClamp ( bf0[34] + bf0[35], bitRange[stage]);
    bf1[36] = XinClamp ( bf0[36] + bf0[37], bitRange[stage]);
    bf1[37] = XinClamp ( bf0[36] - bf0[37], bitRange[stage]);
    bf1[38] = XinClamp (-bf0[38] + bf0[39], bitRange[stage]);
    bf1[39] = XinClamp ( bf0[38] + bf0[39], bitRange[stage]);
    bf1[40] = XinClamp ( bf0[40] + bf0[41], bitRange[stage]);
    bf1[41] = XinClamp ( bf0[40] - bf0[41], bitRange[stage]);
    bf1[42] = XinClamp (-bf0[42] + bf0[43], bitRange[stage]);
    bf1[43] = XinClamp ( bf0[42] + bf0[43], bitRange[stage]);
    bf1[44] = XinClamp ( bf0[44] + bf0[45], bitRange[stage]);
    bf1[45] = XinClamp ( bf0[44] - bf0[45], bitRange[stage]);
    bf1[46] = XinClamp (-bf0[46] + bf0[47], bitRange[stage]);
    bf1[47] = XinClamp ( bf0[46] + bf0[47], bitRange[stage]);
    bf1[48] = XinClamp ( bf0[48] + bf0[49], bitRange[stage]);
    bf1[49] = XinClamp ( bf0[48] - bf0[49], bitRange[stage]);
    bf1[50] = XinClamp (-bf0[50] + bf0[51], bitRange[stage]);
    bf1[51] = XinClamp ( bf0[50] + bf0[51], bitRange[stage]);
    bf1[52] = XinClamp ( bf0[52] + bf0[53], bitRange[stage]);
    bf1[53] = XinClamp ( bf0[52] - bf0[53], bitRange[stage]);
    bf1[54] = XinClamp (-bf0[54] + bf0[55], bitRange[stage]);
    bf1[55] = XinClamp ( bf0[54] + bf0[55], bitRange[stage]);
    bf1[56] = XinClamp ( bf0[56] + bf0[57], bitRange[stage]);
    bf1[57] = XinClamp ( bf0[56] - bf0[57], bitRange[stage]);
    bf1[58] = XinClamp (-bf0[58] + bf0[59], bitRange[stage]);
    bf1[59] = XinClamp ( bf0[58] + bf0[59], bitRange[stage]);
    bf1[60] = XinClamp ( bf0[60] + bf0[61], bitRange[stage]);
    bf1[61] = XinClamp ( bf0[60] - bf0[61], bitRange[stage]);
    bf1[62] = XinClamp (-bf0[62] + bf0[63], bitRange[stage]);
    bf1[63] = XinClamp ( bf0[62] + bf0[63], bitRange[stage]);

    // stage 4
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = XinHalfBtf (cosPi[60], bf0[8],  -cosPi[4],  bf0[15], cosBit);
    bf1[9]  = XinHalfBtf (cosPi[28], bf0[9],  -cosPi[36], bf0[14], cosBit);
    bf1[10] = XinHalfBtf (cosPi[44], bf0[10], -cosPi[20], bf0[13], cosBit);
    bf1[11] = XinHalfBtf (cosPi[12], bf0[11], -cosPi[52], bf0[12], cosBit);
    bf1[12] = XinHalfBtf (cosPi[52], bf0[11],  cosPi[12], bf0[12], cosBit);
    bf1[13] = XinHalfBtf (cosPi[20], bf0[10],  cosPi[44], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (cosPi[36], bf0[9],   cosPi[28], bf0[14], cosBit);
    bf1[15] = XinHalfBtf (cosPi[4],  bf0[8],   cosPi[60], bf0[15], cosBit);
    bf1[16] = XinClamp ( bf0[16] + bf0[17], bitRange[stage]);
    bf1[17] = XinClamp ( bf0[16] - bf0[17], bitRange[stage]);
    bf1[18] = XinClamp (-bf0[18] + bf0[19], bitRange[stage]);
    bf1[19] = XinClamp ( bf0[18] + bf0[19], bitRange[stage]);
    bf1[20] = XinClamp ( bf0[20] + bf0[21], bitRange[stage]);
    bf1[21] = XinClamp ( bf0[20] - bf0[21], bitRange[stage]);
    bf1[22] = XinClamp (-bf0[22] + bf0[23], bitRange[stage]);
    bf1[23] = XinClamp ( bf0[22] + bf0[23], bitRange[stage]);
    bf1[24] = XinClamp ( bf0[24] + bf0[25], bitRange[stage]);
    bf1[25] = XinClamp ( bf0[24] - bf0[25], bitRange[stage]);
    bf1[26] = XinClamp (-bf0[26] + bf0[27], bitRange[stage]);
    bf1[27] = XinClamp ( bf0[26] + bf0[27], bitRange[stage]);
    bf1[28] = XinClamp ( bf0[28] + bf0[29], bitRange[stage]);
    bf1[29] = XinClamp ( bf0[28] - bf0[29], bitRange[stage]);
    bf1[30] = XinClamp (-bf0[30] + bf0[31], bitRange[stage]);
    bf1[31] = XinClamp ( bf0[30] + bf0[31], bitRange[stage]);
    bf1[32] = bf0[32];
    bf1[33] = XinHalfBtf (-cosPi[4],  bf0[33],  cosPi[60], bf0[62], cosBit);
    bf1[34] = XinHalfBtf (-cosPi[60], bf0[34], -cosPi[4],  bf0[61], cosBit);
    bf1[35] = bf0[35];
    bf1[36] = bf0[36];
    bf1[37] = XinHalfBtf (-cosPi[36], bf0[37],  cosPi[28], bf0[58], cosBit);
    bf1[38] = XinHalfBtf (-cosPi[28], bf0[38], -cosPi[36], bf0[57], cosBit);
    bf1[39] = bf0[39];
    bf1[40] = bf0[40];
    bf1[41] = XinHalfBtf (-cosPi[20], bf0[41],  cosPi[44], bf0[54], cosBit);
    bf1[42] = XinHalfBtf (-cosPi[44], bf0[42], -cosPi[20], bf0[53], cosBit);
    bf1[43] = bf0[43];
    bf1[44] = bf0[44];
    bf1[45] = XinHalfBtf (-cosPi[52], bf0[45],  cosPi[12], bf0[50], cosBit);
    bf1[46] = XinHalfBtf (-cosPi[12], bf0[46], -cosPi[52], bf0[49], cosBit);
    bf1[47] = bf0[47];
    bf1[48] = bf0[48];
    bf1[49] = XinHalfBtf (-cosPi[52], bf0[46], cosPi[12], bf0[49], cosBit);
    bf1[50] = XinHalfBtf ( cosPi[12], bf0[45], cosPi[52], bf0[50], cosBit);
    bf1[51] = bf0[51];
    bf1[52] = bf0[52];
    bf1[53] = XinHalfBtf (-cosPi[20], bf0[42], cosPi[44], bf0[53], cosBit);
    bf1[54] = XinHalfBtf ( cosPi[44], bf0[41], cosPi[20], bf0[54], cosBit);
    bf1[55] = bf0[55];
    bf1[56] = bf0[56];
    bf1[57] = XinHalfBtf (-cosPi[36], bf0[38], cosPi[28], bf0[57], cosBit);
    bf1[58] = XinHalfBtf ( cosPi[28], bf0[37], cosPi[36], bf0[58], cosBit);
    bf1[59] = bf0[59];
    bf1[60] = bf0[60];
    bf1[61] = XinHalfBtf (-cosPi[4],  bf0[34], cosPi[60], bf0[61], cosBit);
    bf1[62] = XinHalfBtf ( cosPi[60], bf0[33], cosPi[4],  bf0[62], cosBit);
    bf1[63] = bf0[63];

    // stage 5
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = XinHalfBtf (cosPi[56], bf0[4], -cosPi[8],  bf0[7], cosBit);
    bf1[5]  = XinHalfBtf (cosPi[24], bf0[5], -cosPi[40], bf0[6], cosBit);
    bf1[6]  = XinHalfBtf (cosPi[40], bf0[5],  cosPi[24], bf0[6], cosBit);
    bf1[7]  = XinHalfBtf (cosPi[8],  bf0[4],  cosPi[56], bf0[7], cosBit);
    bf1[8]  = XinClamp ( bf0[8]  + bf0[9],  bitRange[stage]);
    bf1[9]  = XinClamp ( bf0[8]  - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (-bf0[10] + bf0[11], bitRange[stage]);
    bf1[11] = XinClamp ( bf0[10] + bf0[11], bitRange[stage]);
    bf1[12] = XinClamp ( bf0[12] + bf0[13], bitRange[stage]);
    bf1[13] = XinClamp ( bf0[12] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (-bf0[14] + bf0[15], bitRange[stage]);
    bf1[15] = XinClamp ( bf0[14] + bf0[15], bitRange[stage]);
    bf1[16] = bf0[16];
    bf1[17] = XinHalfBtf (-cosPi[8],  bf0[17],  cosPi[56], bf0[30], cosBit);
    bf1[18] = XinHalfBtf (-cosPi[56], bf0[18], -cosPi[8],  bf0[29], cosBit);
    bf1[19] = bf0[19];
    bf1[20] = bf0[20];
    bf1[21] = XinHalfBtf (-cosPi[40], bf0[21],  cosPi[24], bf0[26], cosBit);
    bf1[22] = XinHalfBtf (-cosPi[24], bf0[22], -cosPi[40], bf0[25], cosBit);
    bf1[23] = bf0[23];
    bf1[24] = bf0[24];
    bf1[25] = XinHalfBtf (-cosPi[40], bf0[22], cosPi[24], bf0[25], cosBit);
    bf1[26] = XinHalfBtf ( cosPi[24], bf0[21], cosPi[40], bf0[26], cosBit);
    bf1[27] = bf0[27];
    bf1[28] = bf0[28];
    bf1[29] = XinHalfBtf (-cosPi[8],  bf0[18], cosPi[56], bf0[29], cosBit);
    bf1[30] = XinHalfBtf ( cosPi[56], bf0[17], cosPi[8],  bf0[30], cosBit);
    bf1[31] = bf0[31];
    bf1[32] = XinClamp ( bf0[32] + bf0[35], bitRange[stage]);
    bf1[33] = XinClamp ( bf0[33] + bf0[34], bitRange[stage]);
    bf1[34] = XinClamp ( bf0[33] - bf0[34], bitRange[stage]);
    bf1[35] = XinClamp ( bf0[32] - bf0[35], bitRange[stage]);
    bf1[36] = XinClamp (-bf0[36] + bf0[39], bitRange[stage]);
    bf1[37] = XinClamp (-bf0[37] + bf0[38], bitRange[stage]);
    bf1[38] = XinClamp ( bf0[37] + bf0[38], bitRange[stage]);
    bf1[39] = XinClamp ( bf0[36] + bf0[39], bitRange[stage]);
    bf1[40] = XinClamp ( bf0[40] + bf0[43], bitRange[stage]);
    bf1[41] = XinClamp ( bf0[41] + bf0[42], bitRange[stage]);
    bf1[42] = XinClamp ( bf0[41] - bf0[42], bitRange[stage]);
    bf1[43] = XinClamp ( bf0[40] - bf0[43], bitRange[stage]);
    bf1[44] = XinClamp (-bf0[44] + bf0[47], bitRange[stage]);
    bf1[45] = XinClamp (-bf0[45] + bf0[46], bitRange[stage]);
    bf1[46] = XinClamp ( bf0[45] + bf0[46], bitRange[stage]);
    bf1[47] = XinClamp ( bf0[44] + bf0[47], bitRange[stage]);
    bf1[48] = XinClamp ( bf0[48] + bf0[51], bitRange[stage]);
    bf1[49] = XinClamp ( bf0[49] + bf0[50], bitRange[stage]);
    bf1[50] = XinClamp ( bf0[49] - bf0[50], bitRange[stage]);
    bf1[51] = XinClamp ( bf0[48] - bf0[51], bitRange[stage]);
    bf1[52] = XinClamp (-bf0[52] + bf0[55], bitRange[stage]);
    bf1[53] = XinClamp (-bf0[53] + bf0[54], bitRange[stage]);
    bf1[54] = XinClamp ( bf0[53] + bf0[54], bitRange[stage]);
    bf1[55] = XinClamp ( bf0[52] + bf0[55], bitRange[stage]);
    bf1[56] = XinClamp ( bf0[56] + bf0[59], bitRange[stage]);
    bf1[57] = XinClamp ( bf0[57] + bf0[58], bitRange[stage]);
    bf1[58] = XinClamp ( bf0[57] - bf0[58], bitRange[stage]);
    bf1[59] = XinClamp ( bf0[56] - bf0[59], bitRange[stage]);
    bf1[60] = XinClamp (-bf0[60] + bf0[63], bitRange[stage]);
    bf1[61] = XinClamp (-bf0[61] + bf0[62], bitRange[stage]);
    bf1[62] = XinClamp ( bf0[61] + bf0[62], bitRange[stage]);
    bf1[63] = XinClamp ( bf0[60] + bf0[63], bitRange[stage]);

    // stage 6
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinHalfBtf (cosPi[32], bf0[0],  cosPi[32], bf0[1], cosBit);
    bf1[1]  = XinHalfBtf (cosPi[32], bf0[0], -cosPi[32], bf0[1], cosBit);
    bf1[2]  = XinHalfBtf (cosPi[48], bf0[2], -cosPi[16], bf0[3], cosBit);
    bf1[3]  = XinHalfBtf (cosPi[16], bf0[2],  cosPi[48], bf0[3], cosBit);
    bf1[4]  = XinClamp ( bf0[4] + bf0[5], bitRange[stage]);
    bf1[5]  = XinClamp ( bf0[4] - bf0[5], bitRange[stage]);
    bf1[6]  = XinClamp (-bf0[6] + bf0[7], bitRange[stage]);
    bf1[7]  = XinClamp ( bf0[6] + bf0[7], bitRange[stage]);
    bf1[8]  = bf0[8];
    bf1[9]  = XinHalfBtf (-cosPi[16], bf0[9],   cosPi[48], bf0[14], cosBit);
    bf1[10] = XinHalfBtf (-cosPi[48], bf0[10], -cosPi[16], bf0[13], cosBit);
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = XinHalfBtf (-cosPi[16], bf0[10], cosPi[48], bf0[13], cosBit);
    bf1[14] = XinHalfBtf ( cosPi[48], bf0[9],  cosPi[16], bf0[14], cosBit);
    bf1[15] = bf0[15];
    bf1[16] = XinClamp ( bf0[16] + bf0[19], bitRange[stage]);
    bf1[17] = XinClamp ( bf0[17] + bf0[18], bitRange[stage]);
    bf1[18] = XinClamp ( bf0[17] - bf0[18], bitRange[stage]);
    bf1[19] = XinClamp ( bf0[16] - bf0[19], bitRange[stage]);
    bf1[20] = XinClamp (-bf0[20] + bf0[23], bitRange[stage]);
    bf1[21] = XinClamp (-bf0[21] + bf0[22], bitRange[stage]);
    bf1[22] = XinClamp ( bf0[21] + bf0[22], bitRange[stage]);
    bf1[23] = XinClamp ( bf0[20] + bf0[23], bitRange[stage]);
    bf1[24] = XinClamp ( bf0[24] + bf0[27], bitRange[stage]);
    bf1[25] = XinClamp ( bf0[25] + bf0[26], bitRange[stage]);
    bf1[26] = XinClamp ( bf0[25] - bf0[26], bitRange[stage]);
    bf1[27] = XinClamp ( bf0[24] - bf0[27], bitRange[stage]);
    bf1[28] = XinClamp (-bf0[28] + bf0[31], bitRange[stage]);
    bf1[29] = XinClamp (-bf0[29] + bf0[30], bitRange[stage]);
    bf1[30] = XinClamp ( bf0[29] + bf0[30], bitRange[stage]);
    bf1[31] = XinClamp ( bf0[28] + bf0[31], bitRange[stage]);
    bf1[32] = bf0[32];
    bf1[33] = bf0[33];
    bf1[34] = XinHalfBtf (-cosPi[8],  bf0[34],  cosPi[56], bf0[61], cosBit);
    bf1[35] = XinHalfBtf (-cosPi[8],  bf0[35],  cosPi[56], bf0[60], cosBit);
    bf1[36] = XinHalfBtf (-cosPi[56], bf0[36], -cosPi[8],  bf0[59], cosBit);
    bf1[37] = XinHalfBtf (-cosPi[56], bf0[37], -cosPi[8],  bf0[58], cosBit);
    bf1[38] = bf0[38];
    bf1[39] = bf0[39];
    bf1[40] = bf0[40];
    bf1[41] = bf0[41];
    bf1[42] = XinHalfBtf (-cosPi[40], bf0[42],  cosPi[24], bf0[53], cosBit);
    bf1[43] = XinHalfBtf (-cosPi[40], bf0[43],  cosPi[24], bf0[52], cosBit);
    bf1[44] = XinHalfBtf (-cosPi[24], bf0[44], -cosPi[40], bf0[51], cosBit);
    bf1[45] = XinHalfBtf (-cosPi[24], bf0[45], -cosPi[40], bf0[50], cosBit);
    bf1[46] = bf0[46];
    bf1[47] = bf0[47];
    bf1[48] = bf0[48];
    bf1[49] = bf0[49];
    bf1[50] = XinHalfBtf (-cosPi[40], bf0[45], cosPi[24], bf0[50], cosBit);
    bf1[51] = XinHalfBtf (-cosPi[40], bf0[44], cosPi[24], bf0[51], cosBit);
    bf1[52] = XinHalfBtf ( cosPi[24], bf0[43], cosPi[40], bf0[52], cosBit);
    bf1[53] = XinHalfBtf ( cosPi[24], bf0[42], cosPi[40], bf0[53], cosBit);
    bf1[54] = bf0[54];
    bf1[55] = bf0[55];
    bf1[56] = bf0[56];
    bf1[57] = bf0[57];
    bf1[58] = XinHalfBtf (-cosPi[8],  bf0[37], cosPi[56], bf0[58], cosBit);
    bf1[59] = XinHalfBtf (-cosPi[8],  bf0[36], cosPi[56], bf0[59], cosBit);
    bf1[60] = XinHalfBtf ( cosPi[56], bf0[35], cosPi[8],  bf0[60], cosBit);
    bf1[61] = XinHalfBtf ( cosPi[56], bf0[34], cosPi[8],  bf0[61], cosBit);
    bf1[62] = bf0[62];
    bf1[63] = bf0[63];

    // stage 7
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0] + bf0[3], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[2], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[1] - bf0[2], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[0] - bf0[3], bitRange[stage]);
    bf1[4]  = bf0[4];
    bf1[5]  = XinHalfBtf (-cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[6]  = XinHalfBtf ( cosPi[32], bf0[5], cosPi[32], bf0[6], cosBit);
    bf1[7]  = bf0[7];
    bf1[8]  = XinClamp ( bf0[8]  + bf0[11], bitRange[stage]);
    bf1[9]  = XinClamp ( bf0[9]  + bf0[10], bitRange[stage]);
    bf1[10] = XinClamp ( bf0[9]  - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp ( bf0[8]  - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (-bf0[12] + bf0[15], bitRange[stage]);
    bf1[13] = XinClamp (-bf0[13] + bf0[14], bitRange[stage]);
    bf1[14] = XinClamp ( bf0[13] + bf0[14], bitRange[stage]);
    bf1[15] = XinClamp ( bf0[12] + bf0[15], bitRange[stage]);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = XinHalfBtf (-cosPi[16], bf0[18],  cosPi[48], bf0[29], cosBit);
    bf1[19] = XinHalfBtf (-cosPi[16], bf0[19],  cosPi[48], bf0[28], cosBit);
    bf1[20] = XinHalfBtf (-cosPi[48], bf0[20], -cosPi[16], bf0[27], cosBit);
    bf1[21] = XinHalfBtf (-cosPi[48], bf0[21], -cosPi[16], bf0[26], cosBit);
    bf1[22] = bf0[22];
    bf1[23] = bf0[23];
    bf1[24] = bf0[24];
    bf1[25] = bf0[25];
    bf1[26] = XinHalfBtf (-cosPi[16], bf0[21], cosPi[48], bf0[26], cosBit);
    bf1[27] = XinHalfBtf (-cosPi[16], bf0[20], cosPi[48], bf0[27], cosBit);
    bf1[28] = XinHalfBtf ( cosPi[48], bf0[19], cosPi[16], bf0[28], cosBit);
    bf1[29] = XinHalfBtf ( cosPi[48], bf0[18], cosPi[16], bf0[29], cosBit);
    bf1[30] = bf0[30];
    bf1[31] = bf0[31];
    bf1[32] = XinClamp ( bf0[32] + bf0[39], bitRange[stage]);
    bf1[33] = XinClamp ( bf0[33] + bf0[38], bitRange[stage]);
    bf1[34] = XinClamp ( bf0[34] + bf0[37], bitRange[stage]);
    bf1[35] = XinClamp ( bf0[35] + bf0[36], bitRange[stage]);
    bf1[36] = XinClamp ( bf0[35] - bf0[36], bitRange[stage]);
    bf1[37] = XinClamp ( bf0[34] - bf0[37], bitRange[stage]);
    bf1[38] = XinClamp ( bf0[33] - bf0[38], bitRange[stage]);
    bf1[39] = XinClamp ( bf0[32] - bf0[39], bitRange[stage]);
    bf1[40] = XinClamp (-bf0[40] + bf0[47], bitRange[stage]);
    bf1[41] = XinClamp (-bf0[41] + bf0[46], bitRange[stage]);
    bf1[42] = XinClamp (-bf0[42] + bf0[45], bitRange[stage]);
    bf1[43] = XinClamp (-bf0[43] + bf0[44], bitRange[stage]);
    bf1[44] = XinClamp ( bf0[43] + bf0[44], bitRange[stage]);
    bf1[45] = XinClamp ( bf0[42] + bf0[45], bitRange[stage]);
    bf1[46] = XinClamp ( bf0[41] + bf0[46], bitRange[stage]);
    bf1[47] = XinClamp ( bf0[40] + bf0[47], bitRange[stage]);
    bf1[48] = XinClamp ( bf0[48] + bf0[55], bitRange[stage]);
    bf1[49] = XinClamp ( bf0[49] + bf0[54], bitRange[stage]);
    bf1[50] = XinClamp ( bf0[50] + bf0[53], bitRange[stage]);
    bf1[51] = XinClamp ( bf0[51] + bf0[52], bitRange[stage]);
    bf1[52] = XinClamp ( bf0[51] - bf0[52], bitRange[stage]);
    bf1[53] = XinClamp ( bf0[50] - bf0[53], bitRange[stage]);
    bf1[54] = XinClamp ( bf0[49] - bf0[54], bitRange[stage]);
    bf1[55] = XinClamp ( bf0[48] - bf0[55], bitRange[stage]);
    bf1[56] = XinClamp (-bf0[56] + bf0[63], bitRange[stage]);
    bf1[57] = XinClamp (-bf0[57] + bf0[62], bitRange[stage]);
    bf1[58] = XinClamp (-bf0[58] + bf0[61], bitRange[stage]);
    bf1[59] = XinClamp (-bf0[59] + bf0[60], bitRange[stage]);
    bf1[60] = XinClamp ( bf0[59] + bf0[60], bitRange[stage]);
    bf1[61] = XinClamp ( bf0[58] + bf0[61], bitRange[stage]);
    bf1[62] = XinClamp ( bf0[57] + bf0[62], bitRange[stage]);
    bf1[63] = XinClamp ( bf0[56] + bf0[63], bitRange[stage]);

    // stage 8
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinClamp (bf0[0] + bf0[7], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[6], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[5], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[4], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[3] - bf0[4], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[2] - bf0[5], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[1] - bf0[6], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[0] - bf0[7], bitRange[stage]);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = XinHalfBtf (-cosPi[32], bf0[10], cosPi[32], bf0[13], cosBit);
    bf1[11] = XinHalfBtf (-cosPi[32], bf0[11], cosPi[32], bf0[12], cosBit);
    bf1[12] = XinHalfBtf ( cosPi[32], bf0[11], cosPi[32], bf0[12], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[32], bf0[10], cosPi[32], bf0[13], cosBit);
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];
    bf1[16] = XinClamp ( bf0[16] + bf0[23], bitRange[stage]);
    bf1[17] = XinClamp ( bf0[17] + bf0[22], bitRange[stage]);
    bf1[18] = XinClamp ( bf0[18] + bf0[21], bitRange[stage]);
    bf1[19] = XinClamp ( bf0[19] + bf0[20], bitRange[stage]);
    bf1[20] = XinClamp ( bf0[19] - bf0[20], bitRange[stage]);
    bf1[21] = XinClamp ( bf0[18] - bf0[21], bitRange[stage]);
    bf1[22] = XinClamp ( bf0[17] - bf0[22], bitRange[stage]);
    bf1[23] = XinClamp ( bf0[16] - bf0[23], bitRange[stage]);
    bf1[24] = XinClamp (-bf0[24] + bf0[31], bitRange[stage]);
    bf1[25] = XinClamp (-bf0[25] + bf0[30], bitRange[stage]);
    bf1[26] = XinClamp (-bf0[26] + bf0[29], bitRange[stage]);
    bf1[27] = XinClamp (-bf0[27] + bf0[28], bitRange[stage]);
    bf1[28] = XinClamp ( bf0[27] + bf0[28], bitRange[stage]);
    bf1[29] = XinClamp ( bf0[26] + bf0[29], bitRange[stage]);
    bf1[30] = XinClamp ( bf0[25] + bf0[30], bitRange[stage]);
    bf1[31] = XinClamp ( bf0[24] + bf0[31], bitRange[stage]);
    bf1[32] = bf0[32];
    bf1[33] = bf0[33];
    bf1[34] = bf0[34];
    bf1[35] = bf0[35];
    bf1[36] = XinHalfBtf (-cosPi[16], bf0[36],  cosPi[48], bf0[59], cosBit);
    bf1[37] = XinHalfBtf (-cosPi[16], bf0[37],  cosPi[48], bf0[58], cosBit);
    bf1[38] = XinHalfBtf (-cosPi[16], bf0[38],  cosPi[48], bf0[57], cosBit);
    bf1[39] = XinHalfBtf (-cosPi[16], bf0[39],  cosPi[48], bf0[56], cosBit);
    bf1[40] = XinHalfBtf (-cosPi[48], bf0[40], -cosPi[16], bf0[55], cosBit);
    bf1[41] = XinHalfBtf (-cosPi[48], bf0[41], -cosPi[16], bf0[54], cosBit);
    bf1[42] = XinHalfBtf (-cosPi[48], bf0[42], -cosPi[16], bf0[53], cosBit);
    bf1[43] = XinHalfBtf (-cosPi[48], bf0[43], -cosPi[16], bf0[52], cosBit);
    bf1[44] = bf0[44];
    bf1[45] = bf0[45];
    bf1[46] = bf0[46];
    bf1[47] = bf0[47];
    bf1[48] = bf0[48];
    bf1[49] = bf0[49];
    bf1[50] = bf0[50];
    bf1[51] = bf0[51];
    bf1[52] = XinHalfBtf (-cosPi[16], bf0[43], cosPi[48], bf0[52], cosBit);
    bf1[53] = XinHalfBtf (-cosPi[16], bf0[42], cosPi[48], bf0[53], cosBit);
    bf1[54] = XinHalfBtf (-cosPi[16], bf0[41], cosPi[48], bf0[54], cosBit);
    bf1[55] = XinHalfBtf (-cosPi[16], bf0[40], cosPi[48], bf0[55], cosBit);
    bf1[56] = XinHalfBtf ( cosPi[48], bf0[39], cosPi[16], bf0[56], cosBit);
    bf1[57] = XinHalfBtf ( cosPi[48], bf0[38], cosPi[16], bf0[57], cosBit);
    bf1[58] = XinHalfBtf ( cosPi[48], bf0[37], cosPi[16], bf0[58], cosBit);
    bf1[59] = XinHalfBtf ( cosPi[48], bf0[36], cosPi[16], bf0[59], cosBit);
    bf1[60] = bf0[60];
    bf1[61] = bf0[61];
    bf1[62] = bf0[62];
    bf1[63] = bf0[63];

    // stage 9
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0] + bf0[15], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[14], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[13], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[12], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4] + bf0[11], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5] + bf0[10], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6] + bf0[9],  bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7] + bf0[8],  bitRange[stage]);
    bf1[8]  = XinClamp (bf0[7] - bf0[8],  bitRange[stage]);
    bf1[9]  = XinClamp (bf0[6] - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (bf0[5] - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp (bf0[4] - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (bf0[3] - bf0[12], bitRange[stage]);
    bf1[13] = XinClamp (bf0[2] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (bf0[1] - bf0[14], bitRange[stage]);
    bf1[15] = XinClamp (bf0[0] - bf0[15], bitRange[stage]);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = bf0[18];
    bf1[19] = bf0[19];
    bf1[20] = XinHalfBtf (-cosPi[32], bf0[20], cosPi[32], bf0[27], cosBit);
    bf1[21] = XinHalfBtf (-cosPi[32], bf0[21], cosPi[32], bf0[26], cosBit);
    bf1[22] = XinHalfBtf (-cosPi[32], bf0[22], cosPi[32], bf0[25], cosBit);
    bf1[23] = XinHalfBtf (-cosPi[32], bf0[23], cosPi[32], bf0[24], cosBit);
    bf1[24] = XinHalfBtf ( cosPi[32], bf0[23], cosPi[32], bf0[24], cosBit);
    bf1[25] = XinHalfBtf ( cosPi[32], bf0[22], cosPi[32], bf0[25], cosBit);
    bf1[26] = XinHalfBtf ( cosPi[32], bf0[21], cosPi[32], bf0[26], cosBit);
    bf1[27] = XinHalfBtf ( cosPi[32], bf0[20], cosPi[32], bf0[27], cosBit);
    bf1[28] = bf0[28];
    bf1[29] = bf0[29];
    bf1[30] = bf0[30];
    bf1[31] = bf0[31];
    bf1[32] = XinClamp ( bf0[32] + bf0[47], bitRange[stage]);
    bf1[33] = XinClamp ( bf0[33] + bf0[46], bitRange[stage]);
    bf1[34] = XinClamp ( bf0[34] + bf0[45], bitRange[stage]);
    bf1[35] = XinClamp ( bf0[35] + bf0[44], bitRange[stage]);
    bf1[36] = XinClamp ( bf0[36] + bf0[43], bitRange[stage]);
    bf1[37] = XinClamp ( bf0[37] + bf0[42], bitRange[stage]);
    bf1[38] = XinClamp ( bf0[38] + bf0[41], bitRange[stage]);
    bf1[39] = XinClamp ( bf0[39] + bf0[40], bitRange[stage]);
    bf1[40] = XinClamp ( bf0[39] - bf0[40], bitRange[stage]);
    bf1[41] = XinClamp ( bf0[38] - bf0[41], bitRange[stage]);
    bf1[42] = XinClamp ( bf0[37] - bf0[42], bitRange[stage]);
    bf1[43] = XinClamp ( bf0[36] - bf0[43], bitRange[stage]);
    bf1[44] = XinClamp ( bf0[35] - bf0[44], bitRange[stage]);
    bf1[45] = XinClamp ( bf0[34] - bf0[45], bitRange[stage]);
    bf1[46] = XinClamp ( bf0[33] - bf0[46], bitRange[stage]);
    bf1[47] = XinClamp ( bf0[32] - bf0[47], bitRange[stage]);
    bf1[48] = XinClamp (-bf0[48] + bf0[63], bitRange[stage]);
    bf1[49] = XinClamp (-bf0[49] + bf0[62], bitRange[stage]);
    bf1[50] = XinClamp (-bf0[50] + bf0[61], bitRange[stage]);
    bf1[51] = XinClamp (-bf0[51] + bf0[60], bitRange[stage]);
    bf1[52] = XinClamp (-bf0[52] + bf0[59], bitRange[stage]);
    bf1[53] = XinClamp (-bf0[53] + bf0[58], bitRange[stage]);
    bf1[54] = XinClamp (-bf0[54] + bf0[57], bitRange[stage]);
    bf1[55] = XinClamp (-bf0[55] + bf0[56], bitRange[stage]);
    bf1[56] = XinClamp ( bf0[55] + bf0[56], bitRange[stage]);
    bf1[57] = XinClamp ( bf0[54] + bf0[57], bitRange[stage]);
    bf1[58] = XinClamp ( bf0[53] + bf0[58], bitRange[stage]);
    bf1[59] = XinClamp ( bf0[52] + bf0[59], bitRange[stage]);
    bf1[60] = XinClamp ( bf0[51] + bf0[60], bitRange[stage]);
    bf1[61] = XinClamp ( bf0[50] + bf0[61], bitRange[stage]);
    bf1[62] = XinClamp ( bf0[49] + bf0[62], bitRange[stage]);
    bf1[63] = XinClamp ( bf0[48] + bf0[63], bitRange[stage]);

    // stage 10
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinClamp (bf0[0]  + bf0[31], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1]  + bf0[30], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2]  + bf0[29], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3]  + bf0[28], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4]  + bf0[27], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5]  + bf0[26], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6]  + bf0[25], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7]  + bf0[24], bitRange[stage]);
    bf1[8]  = XinClamp (bf0[8]  + bf0[23], bitRange[stage]);
    bf1[9]  = XinClamp (bf0[9]  + bf0[22], bitRange[stage]);
    bf1[10] = XinClamp (bf0[10] + bf0[21], bitRange[stage]);
    bf1[11] = XinClamp (bf0[11] + bf0[20], bitRange[stage]);
    bf1[12] = XinClamp (bf0[12] + bf0[19], bitRange[stage]);
    bf1[13] = XinClamp (bf0[13] + bf0[18], bitRange[stage]);
    bf1[14] = XinClamp (bf0[14] + bf0[17], bitRange[stage]);
    bf1[15] = XinClamp (bf0[15] + bf0[16], bitRange[stage]);
    bf1[16] = XinClamp (bf0[15] - bf0[16], bitRange[stage]);
    bf1[17] = XinClamp (bf0[14] - bf0[17], bitRange[stage]);
    bf1[18] = XinClamp (bf0[13] - bf0[18], bitRange[stage]);
    bf1[19] = XinClamp (bf0[12] - bf0[19], bitRange[stage]);
    bf1[20] = XinClamp (bf0[11] - bf0[20], bitRange[stage]);
    bf1[21] = XinClamp (bf0[10] - bf0[21], bitRange[stage]);
    bf1[22] = XinClamp (bf0[9]  - bf0[22], bitRange[stage]);
    bf1[23] = XinClamp (bf0[8]  - bf0[23], bitRange[stage]);
    bf1[24] = XinClamp (bf0[7]  - bf0[24], bitRange[stage]);
    bf1[25] = XinClamp (bf0[6]  - bf0[25], bitRange[stage]);
    bf1[26] = XinClamp (bf0[5]  - bf0[26], bitRange[stage]);
    bf1[27] = XinClamp (bf0[4]  - bf0[27], bitRange[stage]);
    bf1[28] = XinClamp (bf0[3]  - bf0[28], bitRange[stage]);
    bf1[29] = XinClamp (bf0[2]  - bf0[29], bitRange[stage]);
    bf1[30] = XinClamp (bf0[1]  - bf0[30], bitRange[stage]);
    bf1[31] = XinClamp (bf0[0]  - bf0[31], bitRange[stage]);
    bf1[32] = bf0[32];
    bf1[33] = bf0[33];
    bf1[34] = bf0[34];
    bf1[35] = bf0[35];
    bf1[36] = bf0[36];
    bf1[37] = bf0[37];
    bf1[38] = bf0[38];
    bf1[39] = bf0[39];
    bf1[40] = XinHalfBtf (-cosPi[32], bf0[40], cosPi[32], bf0[55], cosBit);
    bf1[41] = XinHalfBtf (-cosPi[32], bf0[41], cosPi[32], bf0[54], cosBit);
    bf1[42] = XinHalfBtf (-cosPi[32], bf0[42], cosPi[32], bf0[53], cosBit);
    bf1[43] = XinHalfBtf (-cosPi[32], bf0[43], cosPi[32], bf0[52], cosBit);
    bf1[44] = XinHalfBtf (-cosPi[32], bf0[44], cosPi[32], bf0[51], cosBit);
    bf1[45] = XinHalfBtf (-cosPi[32], bf0[45], cosPi[32], bf0[50], cosBit);
    bf1[46] = XinHalfBtf (-cosPi[32], bf0[46], cosPi[32], bf0[49], cosBit);
    bf1[47] = XinHalfBtf (-cosPi[32], bf0[47], cosPi[32], bf0[48], cosBit);
    bf1[48] = XinHalfBtf ( cosPi[32], bf0[47], cosPi[32], bf0[48], cosBit);
    bf1[49] = XinHalfBtf ( cosPi[32], bf0[46], cosPi[32], bf0[49], cosBit);
    bf1[50] = XinHalfBtf ( cosPi[32], bf0[45], cosPi[32], bf0[50], cosBit);
    bf1[51] = XinHalfBtf ( cosPi[32], bf0[44], cosPi[32], bf0[51], cosBit);
    bf1[52] = XinHalfBtf ( cosPi[32], bf0[43], cosPi[32], bf0[52], cosBit);
    bf1[53] = XinHalfBtf ( cosPi[32], bf0[42], cosPi[32], bf0[53], cosBit);
    bf1[54] = XinHalfBtf ( cosPi[32], bf0[41], cosPi[32], bf0[54], cosBit);
    bf1[55] = XinHalfBtf ( cosPi[32], bf0[40], cosPi[32], bf0[55], cosBit);
    bf1[56] = bf0[56];
    bf1[57] = bf0[57];
    bf1[58] = bf0[58];
    bf1[59] = bf0[59];
    bf1[60] = bf0[60];
    bf1[61] = bf0[61];
    bf1[62] = bf0[62];
    bf1[63] = bf0[63];

    // stage 11
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0]  + bf0[63], bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1]  + bf0[62], bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2]  + bf0[61], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3]  + bf0[60], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4]  + bf0[59], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5]  + bf0[58], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6]  + bf0[57], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7]  + bf0[56], bitRange[stage]);
    bf1[8]  = XinClamp (bf0[8]  + bf0[55], bitRange[stage]);
    bf1[9]  = XinClamp (bf0[9]  + bf0[54], bitRange[stage]);
    bf1[10] = XinClamp (bf0[10] + bf0[53], bitRange[stage]);
    bf1[11] = XinClamp (bf0[11] + bf0[52], bitRange[stage]);
    bf1[12] = XinClamp (bf0[12] + bf0[51], bitRange[stage]);
    bf1[13] = XinClamp (bf0[13] + bf0[50], bitRange[stage]);
    bf1[14] = XinClamp (bf0[14] + bf0[49], bitRange[stage]);
    bf1[15] = XinClamp (bf0[15] + bf0[48], bitRange[stage]);
    bf1[16] = XinClamp (bf0[16] + bf0[47], bitRange[stage]);
    bf1[17] = XinClamp (bf0[17] + bf0[46], bitRange[stage]);
    bf1[18] = XinClamp (bf0[18] + bf0[45], bitRange[stage]);
    bf1[19] = XinClamp (bf0[19] + bf0[44], bitRange[stage]);
    bf1[20] = XinClamp (bf0[20] + bf0[43], bitRange[stage]);
    bf1[21] = XinClamp (bf0[21] + bf0[42], bitRange[stage]);
    bf1[22] = XinClamp (bf0[22] + bf0[41], bitRange[stage]);
    bf1[23] = XinClamp (bf0[23] + bf0[40], bitRange[stage]);
    bf1[24] = XinClamp (bf0[24] + bf0[39], bitRange[stage]);
    bf1[25] = XinClamp (bf0[25] + bf0[38], bitRange[stage]);
    bf1[26] = XinClamp (bf0[26] + bf0[37], bitRange[stage]);
    bf1[27] = XinClamp (bf0[27] + bf0[36], bitRange[stage]);
    bf1[28] = XinClamp (bf0[28] + bf0[35], bitRange[stage]);
    bf1[29] = XinClamp (bf0[29] + bf0[34], bitRange[stage]);
    bf1[30] = XinClamp (bf0[30] + bf0[33], bitRange[stage]);
    bf1[31] = XinClamp (bf0[31] + bf0[32], bitRange[stage]);
    bf1[32] = XinClamp (bf0[31] - bf0[32], bitRange[stage]);
    bf1[33] = XinClamp (bf0[30] - bf0[33], bitRange[stage]);
    bf1[34] = XinClamp (bf0[29] - bf0[34], bitRange[stage]);
    bf1[35] = XinClamp (bf0[28] - bf0[35], bitRange[stage]);
    bf1[36] = XinClamp (bf0[27] - bf0[36], bitRange[stage]);
    bf1[37] = XinClamp (bf0[26] - bf0[37], bitRange[stage]);
    bf1[38] = XinClamp (bf0[25] - bf0[38], bitRange[stage]);
    bf1[39] = XinClamp (bf0[24] - bf0[39], bitRange[stage]);
    bf1[40] = XinClamp (bf0[23] - bf0[40], bitRange[stage]);
    bf1[41] = XinClamp (bf0[22] - bf0[41], bitRange[stage]);
    bf1[42] = XinClamp (bf0[21] - bf0[42], bitRange[stage]);
    bf1[43] = XinClamp (bf0[20] - bf0[43], bitRange[stage]);
    bf1[44] = XinClamp (bf0[19] - bf0[44], bitRange[stage]);
    bf1[45] = XinClamp (bf0[18] - bf0[45], bitRange[stage]);
    bf1[46] = XinClamp (bf0[17] - bf0[46], bitRange[stage]);
    bf1[47] = XinClamp (bf0[16] - bf0[47], bitRange[stage]);
    bf1[48] = XinClamp (bf0[15] - bf0[48], bitRange[stage]);
    bf1[49] = XinClamp (bf0[14] - bf0[49], bitRange[stage]);
    bf1[50] = XinClamp (bf0[13] - bf0[50], bitRange[stage]);
    bf1[51] = XinClamp (bf0[12] - bf0[51], bitRange[stage]);
    bf1[52] = XinClamp (bf0[11] - bf0[52], bitRange[stage]);
    bf1[53] = XinClamp (bf0[10] - bf0[53], bitRange[stage]);
    bf1[54] = XinClamp (bf0[9]  - bf0[54], bitRange[stage]);
    bf1[55] = XinClamp (bf0[8]  - bf0[55], bitRange[stage]);
    bf1[56] = XinClamp (bf0[7]  - bf0[56], bitRange[stage]);
    bf1[57] = XinClamp (bf0[6]  - bf0[57], bitRange[stage]);
    bf1[58] = XinClamp (bf0[5]  - bf0[58], bitRange[stage]);
    bf1[59] = XinClamp (bf0[4]  - bf0[59], bitRange[stage]);
    bf1[60] = XinClamp (bf0[3]  - bf0[60], bitRange[stage]);
    bf1[61] = XinClamp (bf0[2]  - bf0[61], bitRange[stage]);
    bf1[62] = XinClamp (bf0[1]  - bf0[62], bitRange[stage]);
    bf1[63] = XinClamp (bf0[0]  - bf0[63], bitRange[stage]);

}

void Xin265pIadst4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    const SINT32 *sinPi;
    SINT32       x0, x1, x2, x3;
    SINT32       s0, s1, s2, s3, s4, s5, s6, s7;

    (void)bitRange;

    sinPi = XinGetSinPi (cosBit);

    // stage 0
    x0 = input[0];
    x1 = input[1];
    x2 = input[2];
    x3 = input[3];

    if (!(x0 | x1 | x2 | x3))
    {
        output[0] = output[1] = output[2] = output[3] = 0;

        return;
    }

    assert(sinPi[1] + sinPi[2] == sinPi[4]);

    // stage 1
    s0 = sinPi[1] * x0;
    s1 = sinPi[2] * x0;
    s2 = sinPi[3] * x1;
    s3 = sinPi[4] * x2;
    s4 = sinPi[1] * x2;
    s5 = sinPi[2] * x3;
    s6 = sinPi[4] * x3;

    // stage 2
    s7 = (x0 - x2) + x3;

    // stage 3
    s0 = s0 + s3;
    s1 = s1 - s4;
    s3 = s2;
    s2 = sinPi[3] * s7;

    // stage 4
    s0 = s0 + s5;
    s1 = s1 - s6;

    // stage 5
    x0 = s0 + s3;
    x1 = s1 + s3;
    x2 = s2;
    x3 = s0 + s1;

    // stage 6
    x3 = x3 - s3;

    output[0] = XinRoundShift (x0, cosBit);
    output[1] = XinRoundShift (x1, cosBit);
    output[2] = XinRoundShift (x2, cosBit);
    output[3] = XinRoundShift (x3, cosBit);

}

void Xin265pIadst8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0, *bf1;
    SINT32       step[8];

    assert(output != input);

    cosPi = XinGetCosPi (cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1    = output;
    bf1[0] = input[7];
    bf1[1] = input[0];
    bf1[2] = input[5];
    bf1[3] = input[2];
    bf1[4] = input[3];
    bf1[5] = input[4];
    bf1[6] = input[1];
    bf1[7] = input[6];

    // stage 2
    stage++;
    bf0    = output;
    bf1    = step;
    bf1[0] = XinHalfBtf (cosPi[4],  bf0[0],  cosPi[60], bf0[1], cosBit);
    bf1[1] = XinHalfBtf (cosPi[60], bf0[0], -cosPi[4],  bf0[1], cosBit);
    bf1[2] = XinHalfBtf (cosPi[20], bf0[2],  cosPi[44], bf0[3], cosBit);
    bf1[3] = XinHalfBtf (cosPi[44], bf0[2], -cosPi[20], bf0[3], cosBit);
    bf1[4] = XinHalfBtf (cosPi[36], bf0[4],  cosPi[28], bf0[5], cosBit);
    bf1[5] = XinHalfBtf (cosPi[28], bf0[4], -cosPi[36], bf0[5], cosBit);
    bf1[6] = XinHalfBtf (cosPi[52], bf0[6],  cosPi[12], bf0[7], cosBit);
    bf1[7] = XinHalfBtf (cosPi[12], bf0[6], -cosPi[52], bf0[7], cosBit);

    // stage 3
    stage++;
    bf0    = step;
    bf1    = output;
    bf1[0] = XinClamp (bf0[0] + bf0[4], bitRange[stage]);
    bf1[1] = XinClamp (bf0[1] + bf0[5], bitRange[stage]);
    bf1[2] = XinClamp (bf0[2] + bf0[6], bitRange[stage]);
    bf1[3] = XinClamp (bf0[3] + bf0[7], bitRange[stage]);
    bf1[4] = XinClamp (bf0[0] - bf0[4], bitRange[stage]);
    bf1[5] = XinClamp (bf0[1] - bf0[5], bitRange[stage]);
    bf1[6] = XinClamp (bf0[2] - bf0[6], bitRange[stage]);
    bf1[7] = XinClamp (bf0[3] - bf0[7], bitRange[stage]);

    // stage 4
    stage++;
    bf0    = output;
    bf1    = step;
    bf1[0] = bf0[0];
    bf1[1] = bf0[1];
    bf1[2] = bf0[2];
    bf1[3] = bf0[3];
    bf1[4] = XinHalfBtf ( cosPi[16], bf0[4],  cosPi[48], bf0[5], cosBit);
    bf1[5] = XinHalfBtf ( cosPi[48], bf0[4], -cosPi[16], bf0[5], cosBit);
    bf1[6] = XinHalfBtf (-cosPi[48], bf0[6],  cosPi[16], bf0[7], cosBit);
    bf1[7] = XinHalfBtf ( cosPi[16], bf0[6],  cosPi[48], bf0[7], cosBit);

    // stage 5
    stage++;
    bf0    = step;
    bf1    = output;
    bf1[0] = XinClamp (bf0[0] + bf0[2], bitRange[stage]);
    bf1[1] = XinClamp (bf0[1] + bf0[3], bitRange[stage]);
    bf1[2] = XinClamp (bf0[0] - bf0[2], bitRange[stage]);
    bf1[3] = XinClamp (bf0[1] - bf0[3], bitRange[stage]);
    bf1[4] = XinClamp (bf0[4] + bf0[6], bitRange[stage]);
    bf1[5] = XinClamp (bf0[5] + bf0[7], bitRange[stage]);
    bf1[6] = XinClamp (bf0[4] - bf0[6], bitRange[stage]);
    bf1[7] = XinClamp (bf0[5] - bf0[7], bitRange[stage]);

    // stage 6
    stage++;
    bf0    = output;
    bf1    = step;
    bf1[0] = bf0[0];
    bf1[1] = bf0[1];
    bf1[2] = XinHalfBtf (cosPi[32], bf0[2],  cosPi[32], bf0[3], cosBit);
    bf1[3] = XinHalfBtf (cosPi[32], bf0[2], -cosPi[32], bf0[3], cosBit);
    bf1[4] = bf0[4];
    bf1[5] = bf0[5];
    bf1[6] = XinHalfBtf (cosPi[32], bf0[6],  cosPi[32], bf0[7], cosBit);
    bf1[7] = XinHalfBtf (cosPi[32], bf0[6], -cosPi[32], bf0[7], cosBit);

    // stage 7
    stage++;
    bf0    =  step;
    bf1    =  output;
    bf1[0] =  bf0[0];
    bf1[1] = -bf0[4];
    bf1[2] =  bf0[6];
    bf1[3] = -bf0[2];
    bf1[4] =  bf0[3];
    bf1[5] = -bf0[7];
    bf1[6] =  bf0[5];
    bf1[7] = -bf0[1];

}

void Xin265pIadst16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0, *bf1;
    SINT32       step[16];

    assert(output != input);

    cosPi = XinGetCosPi (cosBit);
    stage = 0;

    // stage 1;
    stage++;
    bf1     = output;
    bf1[0]  = input[15];
    bf1[1]  = input[0];
    bf1[2]  = input[13];
    bf1[3]  = input[2];
    bf1[4]  = input[11];
    bf1[5]  = input[4];
    bf1[6]  = input[9];
    bf1[7]  = input[6];
    bf1[8]  = input[7];
    bf1[9]  = input[8];
    bf1[10] = input[5];
    bf1[11] = input[10];
    bf1[12] = input[3];
    bf1[13] = input[12];
    bf1[14] = input[1];
    bf1[15] = input[14];

    // stage 2
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinHalfBtf (cosPi[2],  bf0[0],   cosPi[62], bf0[1],  cosBit);
    bf1[1]  = XinHalfBtf (cosPi[62], bf0[0],  -cosPi[2],  bf0[1],  cosBit);
    bf1[2]  = XinHalfBtf (cosPi[10], bf0[2],   cosPi[54], bf0[3],  cosBit);
    bf1[3]  = XinHalfBtf (cosPi[54], bf0[2],  -cosPi[10], bf0[3],  cosBit);
    bf1[4]  = XinHalfBtf (cosPi[18], bf0[4],   cosPi[46], bf0[5],  cosBit);
    bf1[5]  = XinHalfBtf (cosPi[46], bf0[4],  -cosPi[18], bf0[5],  cosBit);
    bf1[6]  = XinHalfBtf (cosPi[26], bf0[6],   cosPi[38], bf0[7],  cosBit);
    bf1[7]  = XinHalfBtf (cosPi[38], bf0[6],  -cosPi[26], bf0[7],  cosBit);
    bf1[8]  = XinHalfBtf (cosPi[34], bf0[8],   cosPi[30], bf0[9],  cosBit);
    bf1[9]  = XinHalfBtf (cosPi[30], bf0[8],  -cosPi[34], bf0[9],  cosBit);
    bf1[10] = XinHalfBtf (cosPi[42], bf0[10],  cosPi[22], bf0[11], cosBit);
    bf1[11] = XinHalfBtf (cosPi[22], bf0[10], -cosPi[42], bf0[11], cosBit);
    bf1[12] = XinHalfBtf (cosPi[50], bf0[12],  cosPi[14], bf0[13], cosBit);
    bf1[13] = XinHalfBtf (cosPi[14], bf0[12], -cosPi[50], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (cosPi[58], bf0[14],  cosPi[6],  bf0[15], cosBit);
    bf1[15] = XinHalfBtf (cosPi[6],  bf0[14], -cosPi[58], bf0[15], cosBit);

    // stage 3
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0] + bf0[8],  bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1] + bf0[9],  bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2] + bf0[10], bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3] + bf0[11], bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4] + bf0[12], bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5] + bf0[13], bitRange[stage]);
    bf1[6]  = XinClamp (bf0[6] + bf0[14], bitRange[stage]);
    bf1[7]  = XinClamp (bf0[7] + bf0[15], bitRange[stage]);
    bf1[8]  = XinClamp (bf0[0] - bf0[8],  bitRange[stage]);
    bf1[9]  = XinClamp (bf0[1] - bf0[9],  bitRange[stage]);
    bf1[10] = XinClamp (bf0[2] - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp (bf0[3] - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (bf0[4] - bf0[12], bitRange[stage]);
    bf1[13] = XinClamp (bf0[5] - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (bf0[6] - bf0[14], bitRange[stage]);
    bf1[15] = XinClamp (bf0[7] - bf0[15], bitRange[stage]);

    // stage 4
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = XinHalfBtf ( cosPi[8],  bf0[8],   cosPi[56], bf0[9],  cosBit);
    bf1[9]  = XinHalfBtf ( cosPi[56], bf0[8],  -cosPi[8],  bf0[9],  cosBit);
    bf1[10] = XinHalfBtf ( cosPi[40], bf0[10],  cosPi[24], bf0[11], cosBit);
    bf1[11] = XinHalfBtf ( cosPi[24], bf0[10], -cosPi[40], bf0[11], cosBit);
    bf1[12] = XinHalfBtf (-cosPi[56], bf0[12],  cosPi[8],  bf0[13], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[8],  bf0[12],  cosPi[56], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (-cosPi[24], bf0[14],  cosPi[40], bf0[15], cosBit);
    bf1[15] = XinHalfBtf ( cosPi[40], bf0[14],  cosPi[24], bf0[15], cosBit);

    // stage 5
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0]  + bf0[4],  bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1]  + bf0[5],  bitRange[stage]);
    bf1[2]  = XinClamp (bf0[2]  + bf0[6],  bitRange[stage]);
    bf1[3]  = XinClamp (bf0[3]  + bf0[7],  bitRange[stage]);
    bf1[4]  = XinClamp (bf0[0]  - bf0[4],  bitRange[stage]);
    bf1[5]  = XinClamp (bf0[1]  - bf0[5],  bitRange[stage]);
    bf1[6]  = XinClamp (bf0[2]  - bf0[6],  bitRange[stage]);
    bf1[7]  = XinClamp (bf0[3]  - bf0[7],  bitRange[stage]);
    bf1[8]  = XinClamp (bf0[8]  + bf0[12], bitRange[stage]);
    bf1[9]  = XinClamp (bf0[9]  + bf0[13], bitRange[stage]);
    bf1[10] = XinClamp (bf0[10] + bf0[14], bitRange[stage]);
    bf1[11] = XinClamp (bf0[11] + bf0[15], bitRange[stage]);
    bf1[12] = XinClamp (bf0[8]  - bf0[12], bitRange[stage]);
    bf1[13] = XinClamp (bf0[9]  - bf0[13], bitRange[stage]);
    bf1[14] = XinClamp (bf0[10] - bf0[14], bitRange[stage]);
    bf1[15] = XinClamp (bf0[11] - bf0[15], bitRange[stage]);

    // stage 6
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = XinHalfBtf ( cosPi[16], bf0[4],  cosPi[48], bf0[5], cosBit);
    bf1[5]  = XinHalfBtf ( cosPi[48], bf0[4], -cosPi[16], bf0[5], cosBit);
    bf1[6]  = XinHalfBtf (-cosPi[48], bf0[6],  cosPi[16], bf0[7], cosBit);
    bf1[7]  = XinHalfBtf ( cosPi[16], bf0[6],  cosPi[48], bf0[7], cosBit);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = bf0[10];
    bf1[11] = bf0[11];
    bf1[12] = XinHalfBtf ( cosPi[16], bf0[12],  cosPi[48], bf0[13], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[48], bf0[12], -cosPi[16], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (-cosPi[48], bf0[14],  cosPi[16], bf0[15], cosBit);
    bf1[15] = XinHalfBtf ( cosPi[16], bf0[14],  cosPi[48], bf0[15], cosBit);

    // stage 7
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = XinClamp (bf0[0]  + bf0[2],  bitRange[stage]);
    bf1[1]  = XinClamp (bf0[1]  + bf0[3],  bitRange[stage]);
    bf1[2]  = XinClamp (bf0[0]  - bf0[2],  bitRange[stage]);
    bf1[3]  = XinClamp (bf0[1]  - bf0[3],  bitRange[stage]);
    bf1[4]  = XinClamp (bf0[4]  + bf0[6],  bitRange[stage]);
    bf1[5]  = XinClamp (bf0[5]  + bf0[7],  bitRange[stage]);
    bf1[6]  = XinClamp (bf0[4]  - bf0[6],  bitRange[stage]);
    bf1[7]  = XinClamp (bf0[5]  - bf0[7],  bitRange[stage]);
    bf1[8]  = XinClamp (bf0[8]  + bf0[10], bitRange[stage]);
    bf1[9]  = XinClamp (bf0[9]  + bf0[11], bitRange[stage]);
    bf1[10] = XinClamp (bf0[8]  - bf0[10], bitRange[stage]);
    bf1[11] = XinClamp (bf0[9]  - bf0[11], bitRange[stage]);
    bf1[12] = XinClamp (bf0[12] + bf0[14], bitRange[stage]);
    bf1[13] = XinClamp (bf0[13] + bf0[15], bitRange[stage]);
    bf1[14] = XinClamp (bf0[12] - bf0[14], bitRange[stage]);
    bf1[15] = XinClamp (bf0[13] - bf0[15], bitRange[stage]);

    // stage 8
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = XinHalfBtf (cosPi[32], bf0[2],  cosPi[32], bf0[3], cosBit);
    bf1[3]  = XinHalfBtf (cosPi[32], bf0[2], -cosPi[32], bf0[3], cosBit);
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = XinHalfBtf (cosPi[32], bf0[6],  cosPi[32], bf0[7], cosBit);
    bf1[7]  = XinHalfBtf (cosPi[32], bf0[6], -cosPi[32], bf0[7], cosBit);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = XinHalfBtf (cosPi[32], bf0[10],  cosPi[32], bf0[11], cosBit);
    bf1[11] = XinHalfBtf (cosPi[32], bf0[10], -cosPi[32], bf0[11], cosBit);
    bf1[12] = bf0[12];
    bf1[13] = bf0[13];
    bf1[14] = XinHalfBtf (cosPi[32], bf0[14],  cosPi[32], bf0[15], cosBit);
    bf1[15] = XinHalfBtf (cosPi[32], bf0[14], -cosPi[32], bf0[15], cosBit);

    // stage 9
    stage++;
    bf0     =  step;
    bf1     =  output;
    bf1[0]  =  bf0[0];
    bf1[1]  = -bf0[8];
    bf1[2]  =  bf0[12];
    bf1[3]  = -bf0[4];
    bf1[4]  =  bf0[6];
    bf1[5]  = -bf0[14];
    bf1[6]  =  bf0[10];
    bf1[7]  = -bf0[2];
    bf1[8]  =  bf0[3];
    bf1[9]  = -bf0[11];
    bf1[10] =  bf0[15];
    bf1[11] = -bf0[7];
    bf1[12] =  bf0[5];
    bf1[13] = -bf0[13];
    bf1[14] =  bf0[9];
    bf1[15] = -bf0[1];

}

void Xin265pIadst32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)

{
    const SINT32 *cosPi;
    SINT32       stage;
    SINT32       *bf0, *bf1;
    SINT32       step[32];

    assert(output != input);

    cosPi = XinGetCosPi (cosBit);
    stage = 0;

    // stage 0;
    XinClampBuf (
        (SINT32 *)input,
        32,
        bitRange[stage]);

    // stage 1;
    stage++;
    bf1     = output;
    bf1[0]  = input[0];
    bf1[1]  = -input[31];
    bf1[2]  = -input[15];
    bf1[3]  =  input[16];
    bf1[4]  = -input[7];
    bf1[5]  =  input[24];
    bf1[6]  =  input[8];
    bf1[7]  = -input[23];
    bf1[8]  = -input[3];
    bf1[9]  =  input[28];
    bf1[10] =  input[12];
    bf1[11] = -input[19];
    bf1[12] =  input[4];
    bf1[13] = -input[27];
    bf1[14] = -input[11];
    bf1[15] =  input[20];
    bf1[16] = -input[1];
    bf1[17] =  input[30];
    bf1[18] =  input[14];
    bf1[19] = -input[17];
    bf1[20] =  input[6];
    bf1[21] = -input[25];
    bf1[22] = -input[9];
    bf1[23] =  input[22];
    bf1[24] =  input[2];
    bf1[25] = -input[29];
    bf1[26] = -input[13];
    bf1[27] =  input[18];
    bf1[28] = -input[5];
    bf1[29] =  input[26];
    bf1[30] =  input[10];
    bf1[31] = -input[21];

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 2
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = XinHalfBtf (cosPi[32], bf0[2],  cosPi[32], bf0[3], cosBit);
    bf1[3]  = XinHalfBtf (cosPi[32], bf0[2], -cosPi[32], bf0[3], cosBit);
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = XinHalfBtf (cosPi[32], bf0[6],  cosPi[32], bf0[7], cosBit);
    bf1[7]  = XinHalfBtf (cosPi[32], bf0[6], -cosPi[32], bf0[7], cosBit);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = XinHalfBtf (cosPi[32], bf0[10],  cosPi[32], bf0[11], cosBit);
    bf1[11] = XinHalfBtf (cosPi[32], bf0[10], -cosPi[32], bf0[11], cosBit);
    bf1[12] = bf0[12];
    bf1[13] = bf0[13];
    bf1[14] = XinHalfBtf (cosPi[32], bf0[14],  cosPi[32], bf0[15], cosBit);
    bf1[15] = XinHalfBtf (cosPi[32], bf0[14], -cosPi[32], bf0[15], cosBit);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = XinHalfBtf (cosPi[32], bf0[18],  cosPi[32], bf0[19], cosBit);
    bf1[19] = XinHalfBtf (cosPi[32], bf0[18], -cosPi[32], bf0[19], cosBit);
    bf1[20] = bf0[20];
    bf1[21] = bf0[21];
    bf1[22] = XinHalfBtf (cosPi[32], bf0[22],  cosPi[32], bf0[23], cosBit);
    bf1[23] = XinHalfBtf (cosPi[32], bf0[22], -cosPi[32], bf0[23], cosBit);
    bf1[24] = bf0[24];
    bf1[25] = bf0[25];
    bf1[26] = XinHalfBtf (cosPi[32], bf0[26],  cosPi[32], bf0[27], cosBit);
    bf1[27] = XinHalfBtf (cosPi[32], bf0[26], -cosPi[32], bf0[27], cosBit);
    bf1[28] = bf0[28];
    bf1[29] = bf0[29];
    bf1[30] = XinHalfBtf (cosPi[32], bf0[30],  cosPi[32], bf0[31], cosBit);
    bf1[31] = XinHalfBtf (cosPi[32], bf0[30], -cosPi[32], bf0[31], cosBit);

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 3
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0] + bf0[2];
    bf1[1]  = bf0[1] + bf0[3];
    bf1[2]  = bf0[0] - bf0[2];
    bf1[3]  = bf0[1] - bf0[3];
    bf1[4]  = bf0[4] + bf0[6];
    bf1[5]  = bf0[5] + bf0[7];
    bf1[6]  = bf0[4] - bf0[6];
    bf1[7]  = bf0[5] - bf0[7];
    bf1[8]  = bf0[8] + bf0[10];
    bf1[9]  = bf0[9] + bf0[11];
    bf1[10] = bf0[8] - bf0[10];
    bf1[11] = bf0[9] - bf0[11];
    bf1[12] = bf0[12] + bf0[14];
    bf1[13] = bf0[13] + bf0[15];
    bf1[14] = bf0[12] - bf0[14];
    bf1[15] = bf0[13] - bf0[15];
    bf1[16] = bf0[16] + bf0[18];
    bf1[17] = bf0[17] + bf0[19];
    bf1[18] = bf0[16] - bf0[18];
    bf1[19] = bf0[17] - bf0[19];
    bf1[20] = bf0[20] + bf0[22];
    bf1[21] = bf0[21] + bf0[23];
    bf1[22] = bf0[20] - bf0[22];
    bf1[23] = bf0[21] - bf0[23];
    bf1[24] = bf0[24] + bf0[26];
    bf1[25] = bf0[25] + bf0[27];
    bf1[26] = bf0[24] - bf0[26];
    bf1[27] = bf0[25] - bf0[27];
    bf1[28] = bf0[28] + bf0[30];
    bf1[29] = bf0[29] + bf0[31];
    bf1[30] = bf0[28] - bf0[30];
    bf1[31] = bf0[29] - bf0[31];

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 4
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = XinHalfBtf ( cosPi[16], bf0[4],  cosPi[48], bf0[5], cosBit);
    bf1[5]  = XinHalfBtf ( cosPi[48], bf0[4], -cosPi[16], bf0[5], cosBit);
    bf1[6]  = XinHalfBtf (-cosPi[48], bf0[6],  cosPi[16], bf0[7], cosBit);
    bf1[7]  = XinHalfBtf ( cosPi[16], bf0[6],  cosPi[48], bf0[7], cosBit);
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = bf0[10];
    bf1[11] = bf0[11];
    bf1[12] = XinHalfBtf ( cosPi[16], bf0[12],  cosPi[48], bf0[13], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[48], bf0[12], -cosPi[16], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (-cosPi[48], bf0[14],  cosPi[16], bf0[15], cosBit);
    bf1[15] = XinHalfBtf ( cosPi[16], bf0[14],  cosPi[48], bf0[15], cosBit);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = bf0[18];
    bf1[19] = bf0[19];
    bf1[20] = XinHalfBtf ( cosPi[16], bf0[20],  cosPi[48], bf0[21], cosBit);
    bf1[21] = XinHalfBtf ( cosPi[48], bf0[20], -cosPi[16], bf0[21], cosBit);
    bf1[22] = XinHalfBtf (-cosPi[48], bf0[22],  cosPi[16], bf0[23], cosBit);
    bf1[23] = XinHalfBtf ( cosPi[16], bf0[22],  cosPi[48], bf0[23], cosBit);
    bf1[24] = bf0[24];
    bf1[25] = bf0[25];
    bf1[26] = bf0[26];
    bf1[27] = bf0[27];
    bf1[28] = XinHalfBtf ( cosPi[16], bf0[28],  cosPi[48], bf0[29], cosBit);
    bf1[29] = XinHalfBtf ( cosPi[48], bf0[28], -cosPi[16], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (-cosPi[48], bf0[30],  cosPi[16], bf0[31], cosBit);
    bf1[31] = XinHalfBtf ( cosPi[16], bf0[30],  cosPi[48], bf0[31], cosBit);

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 5
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0]  + bf0[4];
    bf1[1]  = bf0[1]  + bf0[5];
    bf1[2]  = bf0[2]  + bf0[6];
    bf1[3]  = bf0[3]  + bf0[7];
    bf1[4]  = bf0[0]  - bf0[4];
    bf1[5]  = bf0[1]  - bf0[5];
    bf1[6]  = bf0[2]  - bf0[6];
    bf1[7]  = bf0[3]  - bf0[7];
    bf1[8]  = bf0[8]  + bf0[12];
    bf1[9]  = bf0[9]  + bf0[13];
    bf1[10] = bf0[10] + bf0[14];
    bf1[11] = bf0[11] + bf0[15];
    bf1[12] = bf0[8]  - bf0[12];
    bf1[13] = bf0[9]  - bf0[13];
    bf1[14] = bf0[10] - bf0[14];
    bf1[15] = bf0[11] - bf0[15];
    bf1[16] = bf0[16] + bf0[20];
    bf1[17] = bf0[17] + bf0[21];
    bf1[18] = bf0[18] + bf0[22];
    bf1[19] = bf0[19] + bf0[23];
    bf1[20] = bf0[16] - bf0[20];
    bf1[21] = bf0[17] - bf0[21];
    bf1[22] = bf0[18] - bf0[22];
    bf1[23] = bf0[19] - bf0[23];
    bf1[24] = bf0[24] + bf0[28];
    bf1[25] = bf0[25] + bf0[29];
    bf1[26] = bf0[26] + bf0[30];
    bf1[27] = bf0[27] + bf0[31];
    bf1[28] = bf0[24] - bf0[28];
    bf1[29] = bf0[25] - bf0[29];
    bf1[30] = bf0[26] - bf0[30];
    bf1[31] = bf0[27] - bf0[31];

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 6
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = XinHalfBtf ( cosPi[8],  bf0[8],   cosPi[56], bf0[9],  cosBit);
    bf1[9]  = XinHalfBtf ( cosPi[56], bf0[8],  -cosPi[8],  bf0[9],  cosBit);
    bf1[10] = XinHalfBtf ( cosPi[40], bf0[10],  cosPi[24], bf0[11], cosBit);
    bf1[11] = XinHalfBtf ( cosPi[24], bf0[10], -cosPi[40], bf0[11], cosBit);
    bf1[12] = XinHalfBtf (-cosPi[56], bf0[12],  cosPi[8],  bf0[13], cosBit);
    bf1[13] = XinHalfBtf ( cosPi[8],  bf0[12],  cosPi[56], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (-cosPi[24], bf0[14],  cosPi[40], bf0[15], cosBit);
    bf1[15] = XinHalfBtf ( cosPi[40], bf0[14],  cosPi[24], bf0[15], cosBit);
    bf1[16] = bf0[16];
    bf1[17] = bf0[17];
    bf1[18] = bf0[18];
    bf1[19] = bf0[19];
    bf1[20] = bf0[20];
    bf1[21] = bf0[21];
    bf1[22] = bf0[22];
    bf1[23] = bf0[23];
    bf1[24] = XinHalfBtf ( cosPi[8],  bf0[24],  cosPi[56], bf0[25], cosBit);
    bf1[25] = XinHalfBtf ( cosPi[56], bf0[24], -cosPi[8],  bf0[25], cosBit);
    bf1[26] = XinHalfBtf ( cosPi[40], bf0[26],  cosPi[24], bf0[27], cosBit);
    bf1[27] = XinHalfBtf ( cosPi[24], bf0[26], -cosPi[40], bf0[27], cosBit);
    bf1[28] = XinHalfBtf (-cosPi[56], bf0[28],  cosPi[8],  bf0[29], cosBit);
    bf1[29] = XinHalfBtf ( cosPi[8],  bf0[28],  cosPi[56], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (-cosPi[24], bf0[30],  cosPi[40], bf0[31], cosBit);
    bf1[31] = XinHalfBtf ( cosPi[40], bf0[30],  cosPi[24], bf0[31], cosBit);

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 7
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0]  + bf0[8];
    bf1[1]  = bf0[1]  + bf0[9];
    bf1[2]  = bf0[2]  + bf0[10];
    bf1[3]  = bf0[3]  + bf0[11];
    bf1[4]  = bf0[4]  + bf0[12];
    bf1[5]  = bf0[5]  + bf0[13];
    bf1[6]  = bf0[6]  + bf0[14];
    bf1[7]  = bf0[7]  + bf0[15];
    bf1[8]  = bf0[0]  - bf0[8];
    bf1[9]  = bf0[1]  - bf0[9];
    bf1[10] = bf0[2]  - bf0[10];
    bf1[11] = bf0[3]  - bf0[11];
    bf1[12] = bf0[4]  - bf0[12];
    bf1[13] = bf0[5]  - bf0[13];
    bf1[14] = bf0[6]  - bf0[14];
    bf1[15] = bf0[7]  - bf0[15];
    bf1[16] = bf0[16] + bf0[24];
    bf1[17] = bf0[17] + bf0[25];
    bf1[18] = bf0[18] + bf0[26];
    bf1[19] = bf0[19] + bf0[27];
    bf1[20] = bf0[20] + bf0[28];
    bf1[21] = bf0[21] + bf0[29];
    bf1[22] = bf0[22] + bf0[30];
    bf1[23] = bf0[23] + bf0[31];
    bf1[24] = bf0[16] - bf0[24];
    bf1[25] = bf0[17] - bf0[25];
    bf1[26] = bf0[18] - bf0[26];
    bf1[27] = bf0[19] - bf0[27];
    bf1[28] = bf0[20] - bf0[28];
    bf1[29] = bf0[21] - bf0[29];
    bf1[30] = bf0[22] - bf0[30];
    bf1[31] = bf0[23] - bf0[31];

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 8
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = bf0[0];
    bf1[1]  = bf0[1];
    bf1[2]  = bf0[2];
    bf1[3]  = bf0[3];
    bf1[4]  = bf0[4];
    bf1[5]  = bf0[5];
    bf1[6]  = bf0[6];
    bf1[7]  = bf0[7];
    bf1[8]  = bf0[8];
    bf1[9]  = bf0[9];
    bf1[10] = bf0[10];
    bf1[11] = bf0[11];
    bf1[12] = bf0[12];
    bf1[13] = bf0[13];
    bf1[14] = bf0[14];
    bf1[15] = bf0[15];
    bf1[16] = XinHalfBtf ( cosPi[4],  bf0[16],  cosPi[60], bf0[17], cosBit);
    bf1[17] = XinHalfBtf ( cosPi[60], bf0[16], -cosPi[4],  bf0[17], cosBit);
    bf1[18] = XinHalfBtf ( cosPi[20], bf0[18],  cosPi[44], bf0[19], cosBit);
    bf1[19] = XinHalfBtf ( cosPi[44], bf0[18], -cosPi[20], bf0[19], cosBit);
    bf1[20] = XinHalfBtf ( cosPi[36], bf0[20],  cosPi[28], bf0[21], cosBit);
    bf1[21] = XinHalfBtf ( cosPi[28], bf0[20], -cosPi[36], bf0[21], cosBit);
    bf1[22] = XinHalfBtf ( cosPi[52], bf0[22],  cosPi[12], bf0[23], cosBit);
    bf1[23] = XinHalfBtf ( cosPi[12], bf0[22], -cosPi[52], bf0[23], cosBit);
    bf1[24] = XinHalfBtf (-cosPi[60], bf0[24],  cosPi[4],  bf0[25], cosBit);
    bf1[25] = XinHalfBtf ( cosPi[4],  bf0[24],  cosPi[60], bf0[25], cosBit);
    bf1[26] = XinHalfBtf (-cosPi[44], bf0[26],  cosPi[20], bf0[27], cosBit);
    bf1[27] = XinHalfBtf ( cosPi[20], bf0[26],  cosPi[44], bf0[27], cosBit);
    bf1[28] = XinHalfBtf (-cosPi[28], bf0[28],  cosPi[36], bf0[29], cosBit);
    bf1[29] = XinHalfBtf ( cosPi[36], bf0[28],  cosPi[28], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (-cosPi[12], bf0[30],  cosPi[52], bf0[31], cosBit);
    bf1[31] = XinHalfBtf ( cosPi[52], bf0[30],  cosPi[12], bf0[31], cosBit);

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 9
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[0]  + bf0[16];
    bf1[1]  = bf0[1]  + bf0[17];
    bf1[2]  = bf0[2]  + bf0[18];
    bf1[3]  = bf0[3]  + bf0[19];
    bf1[4]  = bf0[4]  + bf0[20];
    bf1[5]  = bf0[5]  + bf0[21];
    bf1[6]  = bf0[6]  + bf0[22];
    bf1[7]  = bf0[7]  + bf0[23];
    bf1[8]  = bf0[8]  + bf0[24];
    bf1[9]  = bf0[9]  + bf0[25];
    bf1[10] = bf0[10] + bf0[26];
    bf1[11] = bf0[11] + bf0[27];
    bf1[12] = bf0[12] + bf0[28];
    bf1[13] = bf0[13] + bf0[29];
    bf1[14] = bf0[14] + bf0[30];
    bf1[15] = bf0[15] + bf0[31];
    bf1[16] = bf0[0]  - bf0[16];
    bf1[17] = bf0[1]  - bf0[17];
    bf1[18] = bf0[2]  - bf0[18];
    bf1[19] = bf0[3]  - bf0[19];
    bf1[20] = bf0[4]  - bf0[20];
    bf1[21] = bf0[5]  - bf0[21];
    bf1[22] = bf0[6]  - bf0[22];
    bf1[23] = bf0[7]  - bf0[23];
    bf1[24] = bf0[8]  - bf0[24];
    bf1[25] = bf0[9]  - bf0[25];
    bf1[26] = bf0[10] - bf0[26];
    bf1[27] = bf0[11] - bf0[27];
    bf1[28] = bf0[12] - bf0[28];
    bf1[29] = bf0[13] - bf0[29];
    bf1[30] = bf0[14] - bf0[30];
    bf1[31] = bf0[15] - bf0[31];

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 10
    stage++;
    bf0     = output;
    bf1     = step;
    bf1[0]  = XinHalfBtf (cosPi[1],  bf0[0],   cosPi[63], bf0[1],  cosBit);
    bf1[1]  = XinHalfBtf (cosPi[63], bf0[0],  -cosPi[1],  bf0[1],  cosBit);
    bf1[2]  = XinHalfBtf (cosPi[5],  bf0[2],   cosPi[59], bf0[3],  cosBit);
    bf1[3]  = XinHalfBtf (cosPi[59], bf0[2],  -cosPi[5],  bf0[3],  cosBit);
    bf1[4]  = XinHalfBtf (cosPi[9],  bf0[4],   cosPi[55], bf0[5],  cosBit);
    bf1[5]  = XinHalfBtf (cosPi[55], bf0[4],  -cosPi[9],  bf0[5],  cosBit);
    bf1[6]  = XinHalfBtf (cosPi[13], bf0[6],   cosPi[51], bf0[7],  cosBit);
    bf1[7]  = XinHalfBtf (cosPi[51], bf0[6],  -cosPi[13], bf0[7],  cosBit);
    bf1[8]  = XinHalfBtf (cosPi[17], bf0[8],   cosPi[47], bf0[9],  cosBit);
    bf1[9]  = XinHalfBtf (cosPi[47], bf0[8],  -cosPi[17], bf0[9],  cosBit);
    bf1[10] = XinHalfBtf (cosPi[21], bf0[10],  cosPi[43], bf0[11], cosBit);
    bf1[11] = XinHalfBtf (cosPi[43], bf0[10], -cosPi[21], bf0[11], cosBit);
    bf1[12] = XinHalfBtf (cosPi[25], bf0[12],  cosPi[39], bf0[13], cosBit);
    bf1[13] = XinHalfBtf (cosPi[39], bf0[12], -cosPi[25], bf0[13], cosBit);
    bf1[14] = XinHalfBtf (cosPi[29], bf0[14],  cosPi[35], bf0[15], cosBit);
    bf1[15] = XinHalfBtf (cosPi[35], bf0[14], -cosPi[29], bf0[15], cosBit);
    bf1[16] = XinHalfBtf (cosPi[33], bf0[16],  cosPi[31], bf0[17], cosBit);
    bf1[17] = XinHalfBtf (cosPi[31], bf0[16], -cosPi[33], bf0[17], cosBit);
    bf1[18] = XinHalfBtf (cosPi[37], bf0[18],  cosPi[27], bf0[19], cosBit);
    bf1[19] = XinHalfBtf (cosPi[27], bf0[18], -cosPi[37], bf0[19], cosBit);
    bf1[20] = XinHalfBtf (cosPi[41], bf0[20],  cosPi[23], bf0[21], cosBit);
    bf1[21] = XinHalfBtf (cosPi[23], bf0[20], -cosPi[41], bf0[21], cosBit);
    bf1[22] = XinHalfBtf (cosPi[45], bf0[22],  cosPi[19], bf0[23], cosBit);
    bf1[23] = XinHalfBtf (cosPi[19], bf0[22], -cosPi[45], bf0[23], cosBit);
    bf1[24] = XinHalfBtf (cosPi[49], bf0[24],  cosPi[15], bf0[25], cosBit);
    bf1[25] = XinHalfBtf (cosPi[15], bf0[24], -cosPi[49], bf0[25], cosBit);
    bf1[26] = XinHalfBtf (cosPi[53], bf0[26],  cosPi[11], bf0[27], cosBit);
    bf1[27] = XinHalfBtf (cosPi[11], bf0[26], -cosPi[53], bf0[27], cosBit);
    bf1[28] = XinHalfBtf (cosPi[57], bf0[28],  cosPi[7],  bf0[29], cosBit);
    bf1[29] = XinHalfBtf (cosPi[7],  bf0[28], -cosPi[57], bf0[29], cosBit);
    bf1[30] = XinHalfBtf (cosPi[61], bf0[30],  cosPi[3],  bf0[31], cosBit);
    bf1[31] = XinHalfBtf (cosPi[3],  bf0[30], -cosPi[61], bf0[31], cosBit);

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

    // stage 11
    stage++;
    bf0     = step;
    bf1     = output;
    bf1[0]  = bf0[1];
    bf1[1]  = bf0[30];
    bf1[2]  = bf0[3];
    bf1[3]  = bf0[28];
    bf1[4]  = bf0[5];
    bf1[5]  = bf0[26];
    bf1[6]  = bf0[7];
    bf1[7]  = bf0[24];
    bf1[8]  = bf0[9];
    bf1[9]  = bf0[22];
    bf1[10] = bf0[11];
    bf1[11] = bf0[20];
    bf1[12] = bf0[13];
    bf1[13] = bf0[18];
    bf1[14] = bf0[15];
    bf1[15] = bf0[16];
    bf1[16] = bf0[17];
    bf1[17] = bf0[14];
    bf1[18] = bf0[19];
    bf1[19] = bf0[12];
    bf1[20] = bf0[21];
    bf1[21] = bf0[10];
    bf1[22] = bf0[23];
    bf1[23] = bf0[8];
    bf1[24] = bf0[25];
    bf1[25] = bf0[6];
    bf1[26] = bf0[27];
    bf1[27] = bf0[4];
    bf1[28] = bf0[29];
    bf1[29] = bf0[2];
    bf1[30] = bf0[31];
    bf1[31] = bf0[0];

    XinClampBuf (
        (SINT32 *)bf1,
        32,
        bitRange[stage]);

}

void Xin265pIidentity4 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    SINT32 idx;

    (void)cosBit;
    (void)bitRange;

    for (idx = 0; idx < 4; ++idx)
    {
        // Normal input should fit into 32-bit. Cast to 64-bit here to avoid
        // overflow with corrupted/fuzzed input. The same for av1_iidentity/16/64_c.
        output[idx] = XinRoundShift (XIN_INV_SQRT2 * input[idx], XIN_SQRT2_BITS);
    }
}

void Xin265pIidentity8 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    SINT32 idx;

    (void)cosBit;
    (void)bitRange;

    for (idx = 0; idx < 8; ++idx)
    {
        // Normal input should fit into 32-bit. Cast to 64-bit here to avoid
        // overflow with corrupted/fuzzed input. The same for av1_iidentity/16/64_c.
        output[idx] = XinRoundShift (XIN_INV_SQRT2 * input[idx], XIN_SQRT2_BITS);
    }
}

void Xin265pIidentity16 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    SINT32 idx;

    (void)cosBit;
    (void)bitRange;

    for (idx = 0; idx < 16; ++idx)
    {
        // Normal input should fit into 32-bit. Cast to 64-bit here to avoid
        // overflow with corrupted/fuzzed input. The same for av1_iidentity/16/64_c.
        output[idx] = XinRoundShift (XIN_INV_SQRT2 * input[idx], XIN_SQRT2_BITS);
    }
}

void Xin265pIidentity32 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    SINT32 idx;

    (void)cosBit;
    (void)bitRange;

    for (idx = 0; idx < 32; ++idx)
    {
        // Normal input should fit into 32-bit. Cast to 64-bit here to avoid
        // overflow with corrupted/fuzzed input. The same for av1_iidentity/16/64_c.
        output[idx] = XinRoundShift (XIN_INV_SQRT2 * input[idx], XIN_SQRT2_BITS);
    }
}

void Xin265pIidentity64 (
    const SINT32 *input,
    SINT32       *output,
    SINT8        cosBit,
    SINT8        *bitRange)
{
    SINT32 idx;

    (void)cosBit;
    (void)bitRange;

    for (idx = 0; idx < 64; ++idx)
    {
        // Normal input should fit into 32-bit. Cast to 64-bit here to avoid
        // overflow with corrupted/fuzzed input. The same for av1_iidentity/16/64_c.
        output[idx] = XinRoundShift (XIN_INV_SQRT2 * input[idx], XIN_SQRT2_BITS);
    }
}



