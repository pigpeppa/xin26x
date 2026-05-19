/***************************************************************************//**
*
* @file          h265p_forward_trans.c
* @brief         av1 forward transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "basic_macro.h"
#include "string.h"
#include "stdint.h"
#include "h265p_forward_1d_trans.h"
#include "h265p_trans_context.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "h265p_common_data.h"

const SINT8 fwdTxShift[XIN_TX_SIZE_NUM][3] =
{
    { 2,  0,  0 },
    { 2, -1,  0 },
    { 2, -2,  0 },
    { 2, -4,  0 },
    { 0, -2, -2 },
    { 2, -1,  0 },
    { 2, -1,  0 },
    { 2, -2,  0 },
    { 2, -2,  0 },
    { 2, -4,  0 },
    { 2, -4,  0 },
    { 0, -2, -2 },
    { 2, -4, -2 },
    { 2, -1,  0 },
    { 2, -1,  0 },
    { 2, -2,  0 },
    { 2, -2,  0 },
    { 0, -2,  0 },
    { 2, -4,  0 },
};

static const UINT8 txDim2TxSize[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1] =
{
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID},
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID},
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_4X4,          XIN_TX_4X8,          XIN_TX_4X16,         XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID},
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_8X4,          XIN_TX_8X8,          XIN_TX_8X16,         XIN_TX_8X32,         XIN_TX_SIZE_INVALID},
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_16X4,         XIN_TX_16X8,         XIN_TX_16X16,        XIN_TX_16X32,        XIN_TX_16X64       },
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_32X8,         XIN_TX_32X16,        XIN_TX_32X32,        XIN_TX_32X64       },
    {XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_SIZE_INVALID, XIN_TX_64X16,        XIN_TX_64X32,        XIN_TX_64X64       },
};

const SINT8 fwdCosBitHor[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1] =
{
    { 0,  0,  0,  0,  0,  0,  0 },
    { 0,  0,  0,  0,  0,  0,  0 },
    { 0,  0, 13, 13, 13,  0,  0 },
    { 0,  0, 13, 13, 13, 12,  0 },
    { 0,  0, 13, 13, 13, 12, 13 },
    { 0,  0,  0, 13, 13, 12, 13 },
    { 0,  0,  0,  0, 13, 12, 13 },
};

const SINT8 fwdCosBitVer[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1] =
{
    { 0,  0,  0,  0,  0,  0,  0 },
    { 0,  0,  0,  0,  0,  0,  0 },
    { 0,  0, 13, 13, 12,  0,  0 },
    { 0,  0, 13, 13, 13, 12,  0 },
    { 0,  0, 13, 13, 12, 13, 12 },
    { 0,  0,  0, 12, 13, 12, 11 },
    { 0,  0,  0,  0, 12, 11, 10 }
};

static const Xin265pFdct xinFdct[XIN_TX_TYPE_NUM] =
{
    Xin265pFdct4,
    Xin265pFdct8,
    Xin265pFdct16,
    Xin265pFdct32,
    Xin265pFdct64,
    Xin265pFadst4,
    Xin265pFadst8,
    Xin265pFadst16,
    Xin265pFidentity4,
    Xin265pFidentity8,
    Xin265pFidentity16,
    Xin265pFidentity32
};

static const SINT8 fdct4RangeMult2[4] =
{
    0, 2, 3, 3
};

static const SINT8 fdct8RangeMult2[6] =
{
    0, 2, 4, 5, 5, 5
};

static const SINT8 fdct16RangeMult2[8] =
{
    0, 2, 4, 6, 7, 7, 7, 7
};

static const SINT8 fdct32RangeMult2[10] =
{
    0, 2, 4, 6, 8, 9, 9, 9, 9, 9
};

static const SINT8 fdct64RangeMult2[12] =
{
    0, 2, 4, 6, 8, 10, 11, 11, 11, 11, 11, 11
};

static const SINT8 fadst4RangeMult2[7] =
{
    0, 2, 4, 3, 3, 3, 3
};

static const SINT8 fadst8RangeMult2[8] =
{
    0, 0, 1, 3, 3, 5, 5, 5
};

static const SINT8 fadst16RangeMult2[10] =
{
    0, 0, 1, 3, 3, 5, 5, 7, 7, 7
};

static const SINT8 fadst32RangeMult2[12] =
{
    0, 0, 1, 3, 3, 5, 5, 7, 7, 9, 9, 9
};

static const SINT8 fidtx4RangeMult2[1] =
{
    1
};

static const SINT8 fidtx8RangeMult2[1] =
{
    2
};

static const SINT8 fidtx16RangeMult2[1] =
{
    3
};

static const SINT8 fidtx32RangeMult2[1] =
{
    4
};

static const SINT8 fidtx64RangeMult2[1] =
{
    5
};

static const SINT8 maxFwdRangeMult2Hor[7] =
{
    0, 0, 3, 5, 7, 9, 11
};

static const SINT8 *fwdTxRangeMult2List[XIN_TX_TYPE_NUM] =
{
    fdct4RangeMult2,
    fdct8RangeMult2,
    fdct16RangeMult2,
    fdct32RangeMult2,
    fdct64RangeMult2,
    fadst4RangeMult2,
    fadst8RangeMult2,
    fadst16RangeMult2,
    fidtx4RangeMult2,
    fidtx8RangeMult2,
    fidtx16RangeMult2,
    fidtx32RangeMult2
};

static SINT32 XinRoundShift (
    SINT64 value,
    SINT32 bit)
{
    return (SINT32)((value + (1ll << (bit - 1))) >> bit);
}

static void XinSetFwdStageRange (
    xin_tx_cfg *cfg)
{
    SINT32      lgHeight;
    SINT32      stageNumHor;
    SINT32      stageNumVer;
    SINT32      idx;
    const SINT8 *rangeMult2;

    lgHeight    = calcLog2[cfg->height];
    stageNumHor = cfg->stageNumHor;
    stageNumVer = cfg->stageNumVer;

    memset (cfg->stageRangeHor, 0, sizeof(SINT8)*XIN_MAX_TX_STAGE_NUM);
    memset (cfg->stageRangeVer, 0, sizeof(SINT8)*XIN_MAX_TX_STAGE_NUM);

    if (cfg->txTypeHor != XIN_TX_TYPE_INVALID)
    {
        rangeMult2 = fwdTxRangeMult2List[cfg->txTypeHor];

        for (idx = 0; idx < stageNumHor; ++idx)
        {
            cfg->stageRangeHor[idx] = (rangeMult2[idx] + 1) >> 1;
        }
    }

    if (cfg->txTypeVer != XIN_TX_TYPE_INVALID)
    {
        rangeMult2 = fwdTxRangeMult2List[cfg->txTypeVer];

        for (idx = 0; idx < stageNumVer; ++idx)
        {
            cfg->stageRangeVer[idx] = (maxFwdRangeMult2Hor[lgHeight] + rangeMult2[idx] + 1) >> 1;
        }
    }

}

static void Xin265pFwdTxCfg (
    xin_tx_type txType,
    xin_tx_size txSize,
    xin_tx_cfg  *cfg)
{
    xin_tx_type    txTypeHor;
    xin_tx_type    txTypeVer;
    xin_1d_tx_type t1dHor;
    xin_1d_tx_type t1dVer;
    UINT32         width;
    UINT32         height;
    UINT32         lgWidth;
    UINT32         lgHeight;
    SINT32         cosBitHor;
    SINT32         cosBitVer;
    SINT32         stageNumHor;
    SINT32         stageNumVer;

    t1dHor      = tx1dHor[txType];
    t1dVer      = tx1dVer[txType];
    width       = txSize2TxDim[txSize][0];
    height      = txSize2TxDim[txSize][1];
    lgWidth     = calcLog2[width];
    lgHeight    = calcLog2[height];
    txTypeVer   = tx1d2TxType[lgWidth][t1dVer];
    txTypeHor   = tx1d2TxType[lgHeight][t1dHor];
	cosBitHor   = fwdCosBitHor[lgWidth][lgHeight];
	cosBitVer   = fwdCosBitVer[lgWidth][lgHeight];
    stageNumHor = av1TxStageNum[txTypeHor];
    stageNumVer = av1TxStageNum[txTypeVer];

    Xin265pGetFlip (
        txType,
        &cfg->udFlip,
        &cfg->lrFlip);

    cfg->txSize      = txSize;
    cfg->width       = width;
    cfg->height      = height;
    cfg->shift       = fwdTxShift[txSize];
    cfg->cosBitHor   = (SINT8)cosBitHor;
    cfg->cosBitVer   = (SINT8)cosBitVer;
    cfg->stageNumHor = stageNumHor;
    cfg->stageNumVer = stageNumVer;
    cfg->txTypeHor   = txTypeHor;
    cfg->txTypeVer   = txTypeVer;

    XinSetFwdStageRange (
        cfg);

}

void Xin265pFDctWxH (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    UINT32      width, height;
    xin_tx_type txHor, txVer;
    xin_tx_cfg  txCfg;
    UINT32      rowIdx, colIdx;
    SINT8       cosBitHor, cosBitVer;
    SINT32      *buf0;
    SINT32      *buf1;

    Xin265pFwdTxCfg (
        tranType,
        tranSize,
        &txCfg);

    txHor     = txCfg.txTypeHor;
    txVer     = txCfg.txTypeVer;
    cosBitHor = txCfg.cosBitHor;
    cosBitVer = txCfg.cosBitVer;
    height    = txCfg.height;
    width     = txCfg.width;
    buf0      = output;
    buf1      = buf0 + height;

    // Columns
    for (colIdx = 0; colIdx < width; ++colIdx)
    {
        if (txCfg.udFlip == FALSE)
        {
            for (rowIdx = 0; rowIdx < height; ++rowIdx)
            {
                buf0[rowIdx] = input[rowIdx*inputStride + colIdx];
            }
        }
        else
        {
            for (rowIdx = 0; rowIdx < height; ++rowIdx)
            {
                // flip upside down
                buf0[rowIdx] = input[(height - rowIdx - 1) * inputStride + colIdx];
            }
        }

        Xin265pRoundShift (
            buf0,
            height,
            -txCfg.shift[0]);

        xinFdct[txHor] (
            buf0,
            buf1,
            cosBitHor);

        Xin265pRoundShift (
            buf1,
            height,
            -txCfg.shift[1]);

        if (txCfg.lrFlip == FALSE)
        {
            for (rowIdx = 0; rowIdx < height; ++rowIdx)
            {
                tempBuffer[rowIdx*width + colIdx] = buf1[rowIdx];
            }
        }
        else
        {
            for (rowIdx = 0; rowIdx < height; ++rowIdx)
            {
                // flip from left to right
                tempBuffer[rowIdx*width + (width - colIdx - 1)] = buf1[rowIdx];
            }
        }
    }

    // Rows
    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        xinFdct[txVer] (
            tempBuffer + rowIdx * width,
            output + rowIdx * outputStride,
            cosBitVer);

        Xin265pRoundShift (
            output + rowIdx * outputStride,
            width,
            -txCfg.shift[2]);

        if ((height == 2*width) || (width == 2*height))
        {
            // Multiply everything by Sqrt2 if the transform is rectangular and the
            // size difference is a factor of 2.
            for (colIdx = 0; colIdx < width; ++colIdx)
            {
                output[rowIdx * outputStride + colIdx] = XinRoundShift ((SINT64)output[rowIdx*outputStride + colIdx] * XIN_FWD_SQRT2, XIN_SQRT2_BITS);
            }
        }

    }

}


