/***************************************************************************//**
*
* @file          h265p_trans_context.h
* @brief         This file contains av1 constant defintions.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_trans_context_h_
#define _h265p_trans_context_h_

#define XIN_SQRT2_BITS          12
#define XIN_FWD_SQRT2           5793
#define XIN_INV_SQRT2           2896
#define XIN_MAX_TX_STAGE_NUM    12
#define XIN_MAX_LG_TX_SIZE      6
#define XIN_MAX_TX_SIZE         (1 << XIN_MAX_LG_TX_SIZE)
#define XIN_MIN_LG_TX_SIZE      2
#define XIN_MIN_TX_SIZE         (1 << XIN_MIN_LG_TX_SIZE)
#define XIN_MAX_TX_SIZE_IN_UNIT (XIN_MAX_TX_SIZE >> 2)
#define XIN_COS_BIT_MIN         10
#define XIN_COS_BIT_MAX         16
#define XIN_INV_COS_BIT         12

typedef enum xin_2d_tx_type
{
    XIN_DCT_DCT = 0,        // DCT in both horizontal and vertical
    XIN_ADST_DCT,           // ADST in vertical, DCT in horizontal
    XIN_DCT_ADST,           // DCT in vertical, ADST in horizontal
    XIN_ADST_ADST,          // ADST in both directions
    XIN_FLIPADST_DCT,       // FLIPADST in vertical, DCT in horizontal
    XIN_DCT_FLIPADST,       // DCT in vertical, FLIPADST in horizontal
    XIN_FLIPADST_FLIPADST,  // FLIPADST in both directions
    XIN_ADST_FLIPADST,      // ADST in vertical, FLIPADST in horizontal
    XIN_FLIPADST_ADST,      // FLIPADST in vertical, ADST in horizontal
    XIN_IDTX,               // Identity in both directions
    XIN_V_DCT,              // DCT in vertical, identity in horizontal
    XIN_H_DCT,              // Identity in vertical, DCT in horizontal
    XIN_V_ADST,             // ADST in vertical, identity in horizontal
    XIN_H_ADST,             // Identity in vertical, ADST in horizontal
    XIN_V_FLIPADST,         // FLIPADST in vertical, identity in horizontal
    XIN_H_FLIPADST,         // Identity in vertical, FLIPADST in horizontal
    XIN_TX_2D_NUM,
} xin_2d_tx_type;

// 1D tx types
typedef enum xin_1d_tx_type
{
    XIN_DCT_1D = 0,
    XIN_ADST_1D,
    XIN_FLIPADST_1D,
    XIN_IDTX_1D,
    XIN_TX_1D_NUM,
} xin_1d_tx_type;

typedef enum xin_tx_set_type
{
    // DCT only
    XIN_EXT_TX_SET_DCTONLY,
    // DCT + Identity only
    XIN_EXT_TX_SET_DCT_IDTX,
    // Discrete Trig transforms w/o flip (4) + Identity (1)
    XIN_EXT_TX_SET_DTT4_IDTX,
    // Discrete Trig transforms w/o flip (4) + Identity (1) + 1D Hor/vert DCT (2)
    XIN_EXT_TX_SET_DTT4_IDTX_1DDCT,
    // Discrete Trig transforms w/ flip (9) + Identity (1) + 1D Hor/Ver DCT (2)
    XIN_EXT_TX_SET_DTT9_IDTX_1DDCT,
    // Discrete Trig transforms w/ flip (9) + Identity (1) + 1D Hor/Ver (6)
    XIN_EXT_TX_SET_ALL16,
    XIN_EXT_TX_SET_TYPE_NUM
} xin_tx_set_type;

// block transform size
typedef enum xin_tx_size
{
    XIN_TX_4X4 = 0,       // 4x4 transform
    XIN_TX_8X8,           // 8x8 transform
    XIN_TX_16X16,         // 16x16 transform
    XIN_TX_32X32,         // 32x32 transform
    XIN_TX_64X64,         // 64x64 transform
    XIN_TX_4X8,           // 4x8 transform
    XIN_TX_8X4,           // 8x4 transform
    XIN_TX_8X16,          // 8x16 transform
    XIN_TX_16X8,          // 16x8 transform
    XIN_TX_16X32,         // 16x32 transform
    XIN_TX_32X16,         // 32x16 transform
    XIN_TX_32X64,         // 32x64 transform
    XIN_TX_64X32,         // 64x32 transform
    XIN_TX_4X16,          // 4x16 transform
    XIN_TX_16X4,          // 16x4 transform
    XIN_TX_8X32,          // 8x32 transform
    XIN_TX_32X8,          // 32x8 transform
    XIN_TX_16X64,         // 16x64 transform
    XIN_TX_64X16,         // 64x16 transform
    XIN_TX_SIZE_NUM,      // Includes rectangular transforms
    XIN_TX_SIZE_TYPE = XIN_TX_4X8,
    XIN_TX_SIZE_INVALID   = 255, // Invalid transform size
} xin_tx_size;

typedef enum xin_tx_type
{
    XIN_TX_DCT4,
    XIN_TX_DCT8,
    XIN_TX_DCT16,
    XIN_TX_DCT32,
    XIN_TX_DCT64,
    XIN_TX_ADST4,
    XIN_TX_ADST8,
    XIN_TX_ADST16,
    XIN_TX_IDENTITY4,
    XIN_TX_IDENTITY8,
    XIN_TX_IDENTITY16,
    XIN_TX_IDENTITY32,
    XIN_TX_TYPE_NUM,
    XIN_TX_TYPE_INVALID = 255,
} xin_tx_type;

typedef struct xin_tx_cfg
{
    xin_tx_size txSize;
    SINT32      width;
    SINT32      height;
    SINT32      udFlip; // flip upside down
    SINT32      lrFlip; // flip left to right
    const SINT8 *shift;
    SINT8       cosBitHor;
    SINT8       cosBitVer;
    SINT8       stageRangeHor[XIN_MAX_TX_STAGE_NUM];
    SINT8       stageRangeVer[XIN_MAX_TX_STAGE_NUM];
    xin_tx_type txTypeHor;
    xin_tx_type txTypeVer;
    SINT32      stageNumHor;
    SINT32      stageNumVer;
} xin_tx_cfg;

extern const UINT8 txSize2TxDim[XIN_TX_SIZE_NUM][2];
extern const UINT8 tx1dHor[XIN_TX_2D_NUM];
extern const UINT8 tx1dVer[XIN_TX_2D_NUM];
extern const UINT8 tx1d2TxType[XIN_MAX_LG_TX_SIZE+1][XIN_TX_1D_NUM];
extern const SINT8 av1TxStageNum[XIN_TX_TYPE_NUM];
extern const SINT32 cosPiData[7][64];
extern const SINT32 sinPiData[7][5];

void Xin265pGetFlip (
    UINT32 tranType,
    BOOL   *udFlip,
    BOOL   *lrFlip);

void Xin265pRoundShift (
    SINT32 *buf,
    SINT32 bufSize,
    SINT32 shift);

#endif
