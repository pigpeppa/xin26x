/***************************************************************************//**
*
* @file          h265p_inverse_trans.c
* @brief         av1 inverse transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "string.h"
#include "stdint.h"
#include "h265p_inverse_1d_trans.h"
#include "h265p_trans_context.h"

static Xin265pIdct xinIdct[XIN_TX_TYPE_NUM] =
{
    Xin265pIdct4,
    Xin265pIdct8,
    Xin265pIdct16,
    Xin265pIdct32,
    Xin265pIdct64,
    Xin265pIadst4,
    Xin265pIadst8,
    Xin265pIadst16,
    Xin265pIidentity4,
    Xin265pIidentity8,
    Xin265pIidentity16,
    Xin265pIidentity32
};

const SINT8 invTxShift[XIN_TX_SIZE_NUM][2] =
{
    {  0, -4 },
    { -1, -4 },
    { -2, -4 },
    { -2, -4 },
    { -2, -4 },
    {  0, -4 },
    {  0, -4 },
    { -1, -4 },
    { -1, -4 },
    { -1, -4 },
    { -1, -4 },
    { -1, -4 },
    { -1, -4 },
    { -1, -4 },
    { -1, -4 },
    { -2, -4 },
    { -2, -4 },
    { -2, -4 },
    { -2, -4 },
};

const SINT8 invCosBitHor[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1] =
{
    { 0, 0,               0,               0,               0,               0,               0 },
    { 0, 0,               0,               0,               0,               0,               0 },
    { 0, 0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT,               0,               0 },
    { 0, 0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT,               0 },
    { 0, 0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT },
    { 0, 0,               0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT },
    { 0, 0,               0,               0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT }
};


const SINT8 invCosBitVer[XIN_MAX_LG_TX_SIZE+1][XIN_MAX_LG_TX_SIZE+1] =
{
    { 0, 0,               0,               0,               0,               0,               0 },
    { 0, 0,               0,               0,               0,               0,               0 },
    { 0, 0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT,               0,               0 },
    { 0, 0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT,               0 },
    { 0, 0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT },
    { 0, 0,               0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT },
    { 0, 0,               0,               0, XIN_INV_COS_BIT, XIN_INV_COS_BIT, XIN_INV_COS_BIT }
};

static const SINT8 iadst4Range[7] =
{
    0, 1, 0, 0, 0, 0, 0
};

static const SINT8 invShiftRange[XIN_TX_SIZE_NUM] =
{
    5, // 4x4 transform
    6, // 8x8 transform
    7, // 16x16 transform
    7, // 32x32 transform
    7, // 64x64 transform
    5, // 4x8 transform
    5, // 8x4 transform
    6, // 8x16 transform
    6, // 16x8 transform
    6, // 16x32 transform
    6, // 32x16 transform
    6, // 32x64 transform
    6, // 64x32 transform
    6, // 4x16 transform
    6, // 16x4 transform
    7, // 8x32 transform
    7, // 32x8 transform
    7, // 16x64 transform
    7, // 64x16 transform
};

static SINT32 XinRoundShift (
    SINT64 value,
    SINT32 bit)
{
    return (SINT32)((value + (1ll << (bit - 1))) >> bit);
}

static SINT32 XinClamp (
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

static void XinClampBuf (
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

static void XinSetInvStageRange (
    xin_tx_cfg *cfg)
{
    SINT32  stageNumHor;
    SINT32  stageNumVer;

    stageNumHor = cfg->stageNumHor;
    stageNumVer = cfg->stageNumVer;

    memset (cfg->stageRangeHor, 16, sizeof(SINT8)*stageNumHor);
    memset (cfg->stageRangeVer, 16, sizeof(SINT8)*stageNumVer);

}

static void Xin265pInvTxCfg (
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
    SINT8          cosBitHor;
    SINT8          cosBitVer;
    SINT32         stageNumHor;
    SINT32         stageNumVer;

    t1dHor      = tx1dHor[txType];
    t1dVer      = tx1dVer[txType];
    width       = txSize2TxDim[txSize][0];
    height      = txSize2TxDim[txSize][1];
    lgWidth     = calcLog2[width];
    lgHeight    = calcLog2[height];
    txTypeHor   = tx1d2TxType[lgHeight][t1dHor];
    txTypeVer   = tx1d2TxType[lgWidth][t1dVer];
    cosBitHor   = invCosBitHor[lgHeight][lgWidth];
    cosBitVer   = invCosBitVer[lgHeight][lgWidth];
    stageNumHor = av1TxStageNum[txTypeHor];
    stageNumVer = av1TxStageNum[txTypeVer];

    Xin265pGetFlip (
        txType,
        &cfg->udFlip,
        &cfg->lrFlip);

    cfg->txSize      = txSize;
    cfg->width       = width;
    cfg->height      = height;
    cfg->shift       = invTxShift[txSize];
    cfg->cosBitHor   = cosBitHor;
    cfg->cosBitVer   = cosBitVer;
    cfg->stageNumHor = stageNumHor;
    cfg->stageNumVer = stageNumVer;
    cfg->txTypeHor   = txTypeHor;
    cfg->txTypeVer   = txTypeVer;

    XinSetInvStageRange (
        cfg);

}

void Xin265pIDctWxH (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    UINT32      width, height;
    UINT32      rowIdx, colIdx;
    xin_tx_type txHor, txVer;
    xin_tx_cfg  txCfg;
    SINT32      *bufIn;
    SINT32      *bufIn1;
    SINT32      *bufOut;

    Xin265pInvTxCfg (
        tranType,
        tranSize,
        &txCfg);

    width   = txCfg.width;
    height  = txCfg.height;
    bufIn   = (SINT32 *)tempBuffer;
    bufOut  = bufIn + 2*XIN_MAX (width, height);
    txHor   = txCfg.txTypeHor;
    txVer   = txCfg.txTypeVer;

    // Rows
    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        if ((height == 2*width) || (width == 2*height))
        {
            for (colIdx = 0; colIdx < width; ++colIdx)
            {
                bufIn[colIdx] = XinRoundShift (input[colIdx] * XIN_INV_SQRT2, XIN_SQRT2_BITS);
            }

            XinClampBuf (
                bufIn,
                width,
                16);

            xinIdct[txVer] (
                bufIn,
                bufOut,
                txCfg.cosBitVer,
                txCfg.stageRangeVer);
        }
        else
        {
            for (colIdx = 0; colIdx < width; ++colIdx)
            {
                bufIn[colIdx] = input[colIdx];
            }

            XinClampBuf (
                bufIn,
                width,
                16);

            xinIdct[txVer] (
                bufIn,
                bufOut,
                txCfg.cosBitVer,
                txCfg.stageRangeVer);
        }

        Xin265pRoundShift (
            bufOut,
            width,
            -txCfg.shift[0]);

        input  += inputStride;
        bufOut += width;

    }

    bufOut = bufIn + XIN_MAX (width, height);
    bufIn1 = bufIn + 2*XIN_MAX (width, height);

    // Columns
    for (colIdx = 0; colIdx < width; ++colIdx)
    {
        if (txCfg.lrFlip == 0)
        {
            for (rowIdx = 0; rowIdx < height; ++rowIdx)
            {
                bufIn[rowIdx] = bufIn1[rowIdx * width + colIdx];
            }
        }
        else
        {
            // flip left right
            for (rowIdx = 0; rowIdx < height; ++rowIdx)
            {
                bufIn[rowIdx] = bufIn1[rowIdx * width + (width - colIdx - 1)];
            }
        }

        XinClampBuf (
            bufIn,
            height,
            16);

        xinIdct[txHor] (
            bufIn,
            bufOut,
            txCfg.cosBitHor,
            txCfg.stageRangeHor);

        Xin265pRoundShift (
            bufOut,
            height,
            -txCfg.shift[1]);

        if (txCfg.udFlip == 0)
        {
            for (rowIdx = 0; rowIdx < height; rowIdx++)
            {
                output[rowIdx * outputStride + colIdx] = (SINT16)bufOut[rowIdx];
            }
        }
        else
        {
            // flip upside down
            for (rowIdx = 0; rowIdx < height; rowIdx++)
            {
                output[rowIdx * outputStride + colIdx] = (SINT16)bufOut[height - rowIdx - 1];
            }
        }

    }

}

