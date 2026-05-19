/***************************************************************************//**
*
* @file          h265p_trans_context.c
* @brief         This file contains av1 transform related data.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "h265p_trans_context.h"
#include "basic_macro.h"
#include "h26x_definition.h"

const SINT8 av1TxStageNum[XIN_TX_TYPE_NUM] =
{
    4,      // TXFM_TYPE_DCT4
    6,      // TXFM_TYPE_DCT8
    8,      // TXFM_TYPE_DCT16
    10,     // TXFM_TYPE_DCT32
    12,     // TXFM_TYPE_DCT64
    7,      // TXFM_TYPE_ADST4
    8,      // TXFM_TYPE_ADST8
    10,     // TXFM_TYPE_ADST16
    1,      // TXFM_TYPE_IDENTITY4
    1,      // TXFM_TYPE_IDENTITY8
    1,      // TXFM_TYPE_IDENTITY16
    1,      // TXFM_TYPE_IDENTITY32
};

const UINT8 txSize2TxDim[XIN_TX_SIZE_NUM][2] =
{
    {  4,  4 },     // 4x4 transform
    {  8,  8 },     // 8x8 transform
    { 16, 16 },     // 16x16 transform
    { 32, 32 },     // 32x32 transform
    { 64, 64 },     // 64x64 transform
    {  4,  8 },     // 4x8 transform
    {  8,  4 },     // 8x4 transform
    {  8, 16 },     // 8x16 transform
    { 16,  8 },     // 16x8 transform
    { 16, 32 },     // 16x32 transform
    { 32, 16 },     // 32x16 transform
    { 32, 64 },     // 32x64 transform
    { 64, 32 },     // 64x32 transform
    {  4, 16 },     // 4x16 transform
    { 16,  4 },     // 16x4 transform
    {  8, 32 },     // 8x32 transform
    { 32,  8 },     // 32x8 transform
    { 16, 64 },     // 16x64 transform
    { 64, 16 },     // 64x16 transform
};

const UINT8 tx1dHor[XIN_TX_2D_NUM] =
{
    XIN_DCT_1D,
    XIN_ADST_1D,
    XIN_DCT_1D,
    XIN_ADST_1D,
    XIN_FLIPADST_1D,
    XIN_DCT_1D,
    XIN_FLIPADST_1D,
    XIN_ADST_1D,
    XIN_FLIPADST_1D,
    XIN_IDTX_1D,
    XIN_DCT_1D,
    XIN_IDTX_1D,
    XIN_ADST_1D,
    XIN_IDTX_1D,
    XIN_FLIPADST_1D,
    XIN_IDTX_1D,
};

const UINT8 tx1dVer[XIN_TX_2D_NUM] =
{
    XIN_DCT_1D,
    XIN_DCT_1D,
    XIN_ADST_1D,
    XIN_ADST_1D,
    XIN_DCT_1D,
    XIN_FLIPADST_1D,
    XIN_FLIPADST_1D,
    XIN_FLIPADST_1D,
    XIN_ADST_1D,
    XIN_IDTX_1D,
    XIN_IDTX_1D,
    XIN_DCT_1D,
    XIN_IDTX_1D,
    XIN_ADST_1D,
    XIN_IDTX_1D,
    XIN_FLIPADST_1D,
};

const UINT8 tx1d2TxType[XIN_MAX_LG_TX_SIZE+1][XIN_TX_1D_NUM] =
{
    { XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID },
    { XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID },
    { XIN_TX_DCT4,         XIN_TX_ADST4,        XIN_TX_ADST4,        XIN_TX_IDENTITY4    },
    { XIN_TX_DCT8,         XIN_TX_ADST8,        XIN_TX_ADST8,        XIN_TX_IDENTITY8    },
    { XIN_TX_DCT16,        XIN_TX_ADST16,       XIN_TX_ADST16,       XIN_TX_IDENTITY16   },
    { XIN_TX_DCT32,        XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID, XIN_TX_IDENTITY32   },
    { XIN_TX_DCT64,        XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID, XIN_TX_TYPE_INVALID }
};

const SINT32 cosPiData[7][64] =
{
    {
        1024, 1024, 1023, 1021, 1019, 1016, 1013, 1009, 1004, 999, 993, 987, 980,
        972,  964,  955,  946,  936,  926,  915,  903,  891,  878, 865, 851, 837,
        822,  807,  792,  775,  759,  742,  724,  706,  688,  669, 650, 630, 610,
        590,  569,  548,  526,  505,  483,  460,  438,  415,  392, 369, 345, 321,
        297,  273,  249,  224,  200,  175,  150,  125,  100,  75,  50,  25
    },
    {
        2048, 2047, 2046, 2042, 2038, 2033, 2026, 2018, 2009, 1998, 1987,
        1974, 1960, 1945, 1928, 1911, 1892, 1872, 1851, 1829, 1806, 1782,
        1757, 1730, 1703, 1674, 1645, 1615, 1583, 1551, 1517, 1483, 1448,
        1412, 1375, 1338, 1299, 1260, 1220, 1179, 1138, 1096, 1053, 1009,
        965,  921,  876,  830,  784,  737,  690,  642,  595,  546,  498,
        449,  400,  350,  301,  251,  201,  151,  100,  50
    },
    {
        4096, 4095, 4091, 4085, 4076, 4065, 4052, 4036, 4017, 3996, 3973,
        3948, 3920, 3889, 3857, 3822, 3784, 3745, 3703, 3659, 3612, 3564,
        3513, 3461, 3406, 3349, 3290, 3229, 3166, 3102, 3035, 2967, 2896,
        2824, 2751, 2675, 2598, 2520, 2440, 2359, 2276, 2191, 2106, 2019,
        1931, 1842, 1751, 1660, 1567, 1474, 1380, 1285, 1189, 1092, 995,
        897,  799,  700,  601,  501,  401,  301,  201,  101
    },
    {
        8192, 8190, 8182, 8170, 8153, 8130, 8103, 8071, 8035, 7993, 7946,
        7895, 7839, 7779, 7713, 7643, 7568, 7489, 7405, 7317, 7225, 7128,
        7027, 6921, 6811, 6698, 6580, 6458, 6333, 6203, 6070, 5933, 5793,
        5649, 5501, 5351, 5197, 5040, 4880, 4717, 4551, 4383, 4212, 4038,
        3862, 3683, 3503, 3320, 3135, 2948, 2760, 2570, 2378, 2185, 1990,
        1795, 1598, 1401, 1202, 1003, 803,  603,  402,  201
    },
    {
        16384, 16379, 16364, 16340, 16305, 16261, 16207, 16143, 16069, 15986, 15893,
        15791, 15679, 15557, 15426, 15286, 15137, 14978, 14811, 14635, 14449, 14256,
        14053, 13842, 13623, 13395, 13160, 12916, 12665, 12406, 12140, 11866, 11585,
        11297, 11003, 10702, 10394, 10080, 9760,  9434,  9102,  8765,  8423,  8076,
        7723,  7366,  7005,  6639,  6270,  5897,  5520,  5139,  4756,  4370,  3981,
        3590,  3196,  2801,  2404,  2006,  1606,  1205,  804,   402
    },
    {
        32768, 32758, 32729, 32679, 32610, 32522, 32413, 32286, 32138, 31972, 31786,
        31581, 31357, 31114, 30853, 30572, 30274, 29957, 29622, 29269, 28899, 28511,
        28106, 27684, 27246, 26791, 26320, 25833, 25330, 24812, 24279, 23732, 23170,
        22595, 22006, 21403, 20788, 20160, 19520, 18868, 18205, 17531, 16846, 16151,
        15447, 14733, 14010, 13279, 12540, 11793, 11039, 10279, 9512,  8740,  7962,
        7180,  6393,  5602,  4808,  4011,  3212,  2411,  1608,  804
    },
    {
        65536, 65516, 65457, 65358, 65220, 65043, 64827, 64571, 64277, 63944, 63572,
        63162, 62714, 62228, 61705, 61145, 60547, 59914, 59244, 58538, 57798, 57022,
        56212, 55368, 54491, 53581, 52639, 51665, 50660, 49624, 48559, 47464, 46341,
        45190, 44011, 42806, 41576, 40320, 39040, 37736, 36410, 35062, 33692, 32303,
        30893, 29466, 28020, 26558, 25080, 23586, 22078, 20557, 19024, 17479, 15924,
        14359, 12785, 11204, 9616,  8022,  6424,  4821,  3216,  1608
    }
};

const SINT32 sinPiData[7][5] =
{
    { 0,   330,   621,   836,   951 },
    { 0,   660,  1241,  1672,  1901 },
    { 0,  1321,  2482,  3344,  3803 },
    { 0,  2642,  4964,  6689,  7606 },
    { 0,  5283,  9929, 13377, 15212 },
    { 0, 10566, 19858, 26755, 30424 },
    { 0, 21133, 39716, 53510, 60849 }
};

void Xin265pRoundShift (
    SINT32 *buf,
    SINT32 bufSize,
    SINT32 shift)
{
    SINT32 idx;

    if (shift == 0)
    {
        return;
    }
    else
    {
        if (shift > 0)
        {
            for (idx = 0; idx < bufSize; idx++)
            {
                buf[idx] = (buf[idx] + (1ll << (shift - 1))) >> shift;
            }
        }
        else
        {
            for (idx = 0; idx < bufSize; idx++)
            {
                buf[idx] = XIN_CLIP(buf[idx] * ((SINT64)1 << (-shift)), XIN_MIN_S32, XIN_MAX_S32);
            }
        }
    }

}

void Xin265pGetFlip (
    UINT32 tranType,
    BOOL   *udFlip,
    BOOL   *lrFlip)
{
    switch (tranType)
    {

    case XIN_DCT_DCT:
    case XIN_ADST_DCT:
    case XIN_DCT_ADST:
    case XIN_ADST_ADST:
        *udFlip = FALSE;
        *lrFlip = FALSE;
        break;

    case XIN_IDTX:
    case XIN_V_DCT:
    case XIN_H_DCT:
    case XIN_V_ADST:
    case XIN_H_ADST:
        *udFlip = FALSE;
        *lrFlip = FALSE;
        break;

    case XIN_FLIPADST_DCT:
    case XIN_FLIPADST_ADST:
    case XIN_V_FLIPADST:
        *udFlip = TRUE;
        *lrFlip = FALSE;
        break;

    case XIN_DCT_FLIPADST:
    case XIN_ADST_FLIPADST:
    case XIN_H_FLIPADST:
        *udFlip = FALSE;
        *lrFlip = TRUE;
        break;

    case XIN_FLIPADST_FLIPADST:
        *udFlip = TRUE;
        *lrFlip = TRUE;
        break;

    default:
        *udFlip = FALSE;
        *lrFlip = FALSE;

    }

}

