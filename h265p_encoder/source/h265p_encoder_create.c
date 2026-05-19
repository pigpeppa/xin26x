/***************************************************************************//**
*
* @file          h265p_encoder_create.c
* @brief         av1 encoder Application Programming Interface.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "stdlib.h"
#include "string.h"
#include "xin26x_params.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_picture_struct.h"
#include "h265p_seq_struct.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_pic_struct.h"
#include "h265p_common_data.h"
#include "h265p_cabac_struct.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h26x_cpu_detection.h"
#include "h265p_encoder_create.h"
#include "h26x_thread_pool.h"
#include "h26x_definition.h"

static UINT32 splitMask[XIN_MAX_MB_DEPTH] =
{
    XIN_CAN_PART_HORZ | XIN_CAN_PART_VERT | XIN_CAN_PART_SPLIT | XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B,
    XIN_CAN_PART_HORZ | XIN_CAN_PART_VERT | XIN_CAN_PART_SPLIT | XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4,
    XIN_CAN_PART_HORZ | XIN_CAN_PART_VERT | XIN_CAN_PART_SPLIT | XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4,
    XIN_CAN_PART_HORZ | XIN_CAN_PART_VERT | XIN_CAN_PART_SPLIT | XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4,
    XIN_CAN_PART_HORZ | XIN_CAN_PART_VERT | XIN_CAN_PART_SPLIT,
};

static const SINT16 dcQuantQtx[XIN_QP_NUM] =
{
    4,    8,    8,    9,    10,  11,  12,  12,  13,  14,  15,   16,   17,   18,
    19,   19,   20,   21,   22,  23,  24,  25,  26,  26,  27,   28,   29,   30,
    31,   32,   32,   33,   34,  35,  36,  37,  38,  38,  39,   40,   41,   42,
    43,   43,   44,   45,   46,  47,  48,  48,  49,  50,  51,   52,   53,   53,
    54,   55,   56,   57,   57,  58,  59,  60,  61,  62,  62,   63,   64,   65,
    66,   66,   67,   68,   69,  70,  70,  71,  72,  73,  74,   74,   75,   76,
    77,   78,   78,   79,   80,  81,  81,  82,  83,  84,  85,   85,   87,   88,
    90,   92,   93,   95,   96,  98,  99,  101, 102, 104, 105,  107,  108,  110,
    111,  113,  114,  116,  117, 118, 120, 121, 123, 125, 127,  129,  131,  134,
    136,  138,  140,  142,  144, 146, 148, 150, 152, 154, 156,  158,  161,  164,
    166,  169,  172,  174,  177, 180, 182, 185, 187, 190, 192,  195,  199,  202,
    205,  208,  211,  214,  217, 220, 223, 226, 230, 233, 237,  240,  243,  247,
    250,  253,  257,  261,  265, 269, 272, 276, 280, 284, 288,  292,  296,  300,
    304,  309,  313,  317,  322, 326, 330, 335, 340, 344, 349,  354,  359,  364,
    369,  374,  379,  384,  389, 395, 400, 406, 411, 417, 423,  429,  435,  441,
    447,  454,  461,  467,  475, 482, 489, 497, 505, 513, 522,  530,  539,  549,
    559,  569,  579,  590,  602, 614, 626, 640, 654, 668, 684,  700,  717,  736,
    755,  775,  796,  819,  843, 869, 896, 925, 955, 988, 1022, 1058, 1098, 1139,
    1184, 1232, 1282, 1336,
};

static const SINT16 acQuantQtx[XIN_QP_NUM] =
{
    4,    8,    9,    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
    33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
    46,   47,   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,
    59,   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,
    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
    98,   99,   100,  101,  102,  104,  106,  108,  110,  112,  114,  116,  118,
    120,  122,  124,  126,  128,  130,  132,  134,  136,  138,  140,  142,  144,
    146,  148,  150,  152,  155,  158,  161,  164,  167,  170,  173,  176,  179,
    182,  185,  188,  191,  194,  197,  200,  203,  207,  211,  215,  219,  223,
    227,  231,  235,  239,  243,  247,  251,  255,  260,  265,  270,  275,  280,
    285,  290,  295,  300,  305,  311,  317,  323,  329,  335,  341,  347,  353,
    359,  366,  373,  380,  387,  394,  401,  408,  416,  424,  432,  440,  448,
    456,  465,  474,  483,  492,  501,  510,  520,  530,  540,  550,  560,  571,
    582,  593,  604,  615,  627,  639,  651,  663,  676,  689,  702,  715,  729,
    743,  757,  771,  786,  801,  816,  832,  848,  864,  881,  898,  915,  933,
    951,  969,  988,  1007, 1026, 1046, 1066, 1087, 1108, 1129, 1151, 1173, 1196,
    1219, 1243, 1267, 1292, 1317, 1343, 1369, 1396, 1423, 1451, 1479, 1508, 1537,
    1567, 1597, 1628, 1660, 1692, 1725, 1759, 1793, 1828,
};

static xin_rps_struct rpsLdRef1[1] =
{
    {
        1,                    // numberOfNegPics
        0,                    // numberOfPosPics
        0,                    // temporalId
        1,                    // isRefFrame
        {1, 0, 0, 0, 0, 0},   // deltaNegPos
        {0, 0, 0, 0, 0, 0},   // deltaPosPos
        {1, 0, 0, 0, 0, 0},   // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0},   // usedByNegPicFlag
    }
};

static xin_rps_struct rpsLdRef2[1] =
{
    {
        2,                    // numberOfNegPics
        0,                    // numberOfPosPics
        0,                    // temporalId
        1,                    // isRefFrame
        {1, 1, 0, 0, 0, 0},   // deltaNegPos
        {0, 0, 0, 0, 0, 0},   // deltaPosPos
        {1, 1, 0, 0, 0, 0},   // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0},   // usedByNegPicFlag
    }
};

static xin_rps_struct rpsLdRef3[1] =
{
    {
        3,                    // numberOfNegPics
        0,                    // numberOfPosPics
        0,                    // temporalId
        1,                    // isRefFrame
        {1, 1, 1, 0, 0, 0},   // deltaNegPos
        {0, 0, 0, 0, 0, 0},   // deltaPosPos
        {1, 1, 1, 0, 0, 0},   // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0},   // usedByNegPicFlag
    }
};

static xin_rps_struct rpsLdRef4[1] =
{
    {
        4,              // numberOfNegPics
        0,              // numberOfPosPics
        0,              // temporalId
        1,              // isRefFrame
        {1, 1, 1, 1},   // deltaNegPos
        {0, 0, 0, 0},   // deltaPosPos
        {1, 1, 1, 1},   // usedByNegPicFlag
        {0, 0, 0, 0},   // usedByNegPicFlag
    }
};

static xin_rps_struct* rpsLdRef[XIN_MAX_REF_FRAMES+1] =
{
    NULL,
    rpsLdRef1,
    rpsLdRef2,
    rpsLdRef3,
    rpsLdRef4
};

static void Xin265pInitSbTsRsAddrMaps (
    xin_seq_struct   *seqSet,
    xin265p_tile_dim *tileDim,
    UINT32           *sbTsToRsAddrMap)
{
    UINT32  colIdx;
    UINT32  rowIdx;
    UINT32  firstRsSb;
    UINT32  frameWidthInSb;
    UINT32  tileWidthInSb;
    UINT32  tileHeightInSb;

    firstRsSb      = tileDim->firstRsSb;
    frameWidthInSb = seqSet->frameWidthInSb;
    tileWidthInSb  = tileDim->tileWidthInSb;
    tileHeightInSb = tileDim->tileHeightInSb;

    for (rowIdx = 0; rowIdx < tileHeightInSb; rowIdx++)
    {
        for (colIdx = 0; colIdx < tileWidthInSb; colIdx++)
        {
            sbTsToRsAddrMap[colIdx + rowIdx * tileWidthInSb] = firstRsSb + rowIdx * frameWidthInSb + colIdx;
        }
    }

}

static void ConstructTileDim (
    xin_seq_struct *seqSet)
{
    UINT32           tileIdx;
    UINT32           colIdx;
    UINT32           rowIdx;
    UINT32           tileWidthInSb;
    UINT32           tileHeightInSb;
    UINT32           frameWidthInSb;
    UINT32           frameHeightInSb;
    UINT32           tileWidth;
    UINT32           tileHeight;
    UINT32           frameWidth;
    UINT32           frameHeight;
    xin265p_tile_dim *tileDim;
    UINT32           *sbTsToRsAddrMap;
    UINT32           firstTsSb;

    frameWidthInSb  = seqSet->frameWidthInSb;
    frameHeightInSb = seqSet->frameHeightInSb;
    tileWidthInSb   = frameWidthInSb / seqSet->config.numTileCols;
    tileHeightInSb  = frameHeightInSb / seqSet->config.numTileRows;
    sbTsToRsAddrMap = seqSet->sbTsToRsAddrMap;

    frameWidth  = seqSet->frameWidth;
    frameHeight = seqSet->frameHeight;
    tileWidth   = tileWidthInSb * seqSet->sbSize;
    tileHeight  = tileHeightInSb * seqSet->sbSize;
    tileIdx     = 0;
    firstTsSb   = 0;

    for (rowIdx = 0; rowIdx < seqSet->config.numTileRows; rowIdx++)
    {
        for (colIdx = 0; colIdx < seqSet->config.numTileCols; colIdx++)
        {
            tileDim = seqSet->tileDim + tileIdx;

            tileDim->firstRsSb = rowIdx * tileHeightInSb * frameWidthInSb + colIdx * tileWidthInSb;
            tileDim->firstTsSb = firstTsSb;
            tileDim->tileIndex = tileIdx;
            tileDim->tilePelX  = colIdx * tileWidthInSb * seqSet->sbSize;
            tileDim->tilePelY  = rowIdx * tileHeightInSb * seqSet->sbSize;
            tileDim->tileSbX   = colIdx * tileWidthInSb;
            tileDim->tileSbY   = rowIdx * tileHeightInSb;

            if ((colIdx + 1) != seqSet->config.numTileCols)
            {
                tileDim->tileWidthInSb = tileWidthInSb;
                tileDim->tileWidth     = tileWidth;
            }
            else
            {
                tileDim->tileWidthInSb = frameWidthInSb - colIdx * tileWidthInSb;
                tileDim->tileWidth     = frameWidth - colIdx * tileWidth;
            }

            if ((rowIdx + 1) != seqSet->config.numTileRows)
            {
                tileDim->tileHeightInSb = tileHeightInSb;
                tileDim->tileHeight     = tileHeight;
            }
            else
            {
                tileDim->tileHeightInSb = frameHeightInSb - rowIdx * tileHeightInSb;
                tileDim->tileHeight     = frameHeight - rowIdx * tileHeight;
            }

            tileDim->sbNumInTile = tileDim->tileHeightInSb * tileDim->tileWidthInSb;

            Xin265pInitSbTsRsAddrMaps (
                seqSet,
                tileDim,
                sbTsToRsAddrMap);

            firstTsSb       += tileDim->sbNumInTile;
            sbTsToRsAddrMap += tileDim->sbNumInTile;
            tileIdx++;

        }
    }

}

static void ContructTileSb (
    xin_pic_struct   *picSet,
    xin265p_tile_dim *tile)
{
    UINT32         colIdx;
    UINT32         rowIdx;
    UINT32         sbIndex;
    UINT32         tileCol;
    UINT32         tileRow;
    UINT32         sbAddr;
    UINT32         firstTsAddr;
    xin_seq_struct *seqSet;
    xin_sb_struct  *sb;
    UINT32         widthInSb;
    UINT32         heightInSb;
    UINT32         availField;

    seqSet      = picSet->seqSet;
    firstTsAddr = tile->firstTsSb;
    widthInSb   = tile->tileWidthInSb;
    heightInSb  = tile->tileHeightInSb;
    tileCol     = tile->tileSbX;
    tileRow     = tile->tileSbY;

    if (seqSet->config.enableWpp)
    {
        for (rowIdx = 0; rowIdx < heightInSb; rowIdx++)
        {
            for (colIdx = 0; colIdx < widthInSb; colIdx++)
            {
                sbIndex = rowIdx*widthInSb + colIdx;
                sbAddr  = seqSet->sbTsToRsAddrMap[firstTsAddr + sbIndex];
                sb      = picSet->sb + sbAddr;

                sb->sbX = colIdx + tileCol;
                sb->sbY = rowIdx + tileRow;

                availField  = 0;
                availField |= (sb->sbX != 0) ? XIN_LFT_SB_AVAIL : 0;
                availField |= (sb->sbY != 0) ? XIN_TOP_SB_AVAIL : 0;
                availField |= ((sb->sbX + 1) != seqSet->frameWidthInSb) ? XIN_RGT_SB_AVAIL : 0;
                availField |= ((sb->sbY + 1) != seqSet->frameHeightInSb) ? XIN_BOT_SB_AVAIL : 0;

                sb->availField = availField;
                sb->sbIndex    = sbIndex;
                sb->sbAddr     = sbAddr;

                sb->sbPelX = sb->sbX*seqSet->sbSize;
                sb->sbPelY = sb->sbY*seqSet->sbSize;

                sb->width  = ((sb->sbX + 1) == seqSet->frameWidthInSb) ? (seqSet->frameWidth - sb->sbPelX) : (seqSet->sbSize);
                sb->height = ((sb->sbY + 1) == seqSet->frameHeightInSb) ? (seqSet->frameHeight - sb->sbPelY) : (seqSet->sbSize);

                sb->pixelNum   = sb->width * sb->height;
                sb->lgWidth    = calcLog2[sb->width];

            }
        }

    }
    else
    {
        for (rowIdx = 0; rowIdx < heightInSb; rowIdx++)
        {
            for (colIdx = 0; colIdx < widthInSb; colIdx++)
            {
                sbIndex = rowIdx*widthInSb + colIdx;
                sbAddr  = seqSet->sbTsToRsAddrMap[firstTsAddr + sbIndex];
                sb      = picSet->sb + sbAddr;

                availField  = 0;
                availField |= (colIdx) ? XIN_LFT_SB_AVAIL : 0;
                availField |= (rowIdx) ? XIN_TOP_SB_AVAIL : 0;
                availField |= ((colIdx + 1) != widthInSb) ? XIN_RGT_SB_AVAIL : 0;
                availField |= ((rowIdx + 1) != heightInSb) ? XIN_BOT_SB_AVAIL : 0;

                sb->availField = availField;
                sb->sbIndex    = sbIndex;
                sb->sbAddr     = sbAddr;

                sb->sbX = colIdx + tileCol;
                sb->sbY = rowIdx + tileRow;

                sb->sbPelX = sb->sbX*seqSet->sbSize;
                sb->sbPelY = sb->sbY*seqSet->sbSize;

                sb->width  = ((sb->sbX + 1) == seqSet->frameWidthInSb) ? (seqSet->frameWidth - sb->sbPelX) : (seqSet->sbSize);
                sb->height = ((sb->sbY + 1) == seqSet->frameHeightInSb) ? (seqSet->frameHeight - sb->sbPelY) : (seqSet->sbSize);

                sb->lgWidth    = calcLog2[sb->width];

            }
        }
    }

}

static SINT32 Xin265pContructPicSb (
    xin_pic_struct *picSet)
{
    xin_seq_struct   *seqSet;
    UINT32           colIdx;
    UINT32           rowIdx;
    UINT32           tileIdx;
    xin265p_tile_dim *tileDim;

    seqSet  = picSet->seqSet;
    tileIdx = 0;

    XIN_MALLOC_CHECK (picSet->sb, seqSet->frameSizeInSb * sizeof(xin_sb_struct));

    for (rowIdx = 0; rowIdx < seqSet->config.numTileRows; rowIdx++)
    {
        for (colIdx = 0; colIdx < seqSet->config.numTileCols; colIdx++)
        {
            tileDim = seqSet->tileDim + tileIdx;

            ContructTileSb (
                picSet,
                tileDim);

            tileIdx++;
        }
    }

    return XIN_SUCCESS;

}

static SINT32 GetDcQuantQtx (
    SINT32  qp,
    SINT32  deltaQp,
    SINT32  bitDepth)
{
    SINT32 dcQuantTx;

    qp = XIN_CLIP (qp + deltaQp, XIN_MIN_QP, XIN_MAX_QP);

    switch (bitDepth)
    {
    case XIN_8_BIT_DEPTH:

        dcQuantTx = dcQuantQtx[qp];
        break;

    default:

        dcQuantTx = -1;
        break;
    }

    return dcQuantTx;

}

static SINT32 GetZbinFactor (
    SINT32 qp,
    SINT32 bitDepth)
{
    SINT32 quantTx;
    SINT32 zbin;

    quantTx = GetDcQuantQtx (qp, 0, bitDepth);

    switch (bitDepth)
    {
    case XIN_8_BIT_DEPTH:

        zbin = (qp == 0) ? 64 : ((quantTx < 148) ? 84 : 80);
        break;

    default:

        zbin = -1;
    }

    return zbin;

}

static void Xin265pInvertQuant (
    SINT32 *quant,
    SINT32 *shift,
    SINT32 d)
{
    UINT32 t;
    SINT32 l, m;

    t = d;

    for (l = 0; t > 1; l++)
    {
        t >>= 1;
    }

    m      = 1 + (1 << (16 + l)) / d;
    *quant = (SINT16)(m - (1 << 16));
    *shift = 1 << (16 - l);
}

SINT32 Xin265pDcQuantQtx (
    SINT32  qp,
    SINT32  deltaQp,
    SINT32  bitDepth)
{
    SINT32 dcQuantTx;

    qp = XIN_CLIP (qp + deltaQp, XIN_MIN_QP, XIN_MAX_QP);

    switch (bitDepth)
    {
    case XIN_8_BIT_DEPTH:

        dcQuantTx = dcQuantQtx[qp];
        break;

    default:

        dcQuantTx = -1;
        break;
    }

    return dcQuantTx;

}

SINT32 Xin265pAcQuantQtx (
    SINT32  qp,
    SINT32  deltaQp,
    SINT32  bitDepth)
{
    SINT32 dcQuantTx;

    qp = XIN_CLIP (qp + deltaQp, XIN_MIN_QP, XIN_MAX_QP);

    switch (bitDepth)
    {
    case XIN_8_BIT_DEPTH:

        dcQuantTx = acQuantQtx[qp];
        break;

    default:

        dcQuantTx = -1;
        break;
    }

    return dcQuantTx;

}


static SINT32 ConstructQuantParam (
    xin_pic_struct *picSet)
{
    UINT32          qp;
    SINT32          quantQtx;
    xin_quant_param *quantParam;
    SINT32          zbinFactor;
    SINT32          roundingFactor;

    XIN_MALLOC_CHECK (picSet->quantParam, XIN_QP_NUM*sizeof(xin_quant_param));

    for (qp = XIN_MIN_QP; qp <= XIN_MAX_QP; qp++)
    {
        roundingFactor = (qp == 0) ? 64 : 48;

        quantParam = picSet->quantParam + qp;
        zbinFactor = GetZbinFactor (qp, XIN_8_BIT_DEPTH);
        quantQtx   = Xin265pDcQuantQtx (qp, 0, XIN_8_BIT_DEPTH);

        Xin265pInvertQuant (
            &quantParam->quant[0],
            &quantParam->quantShift[0],
            quantQtx);

        quantParam->quantFp[0] = (1 << 16) / quantQtx;
        quantParam->roundFp[0] = (64 * quantQtx) >> 7;
        quantParam->zBin[0]    = XIN_ROUND_POWER2 (zbinFactor * quantQtx, 7);
        quantParam->round[0]   = (roundingFactor * quantQtx) >> 7;
        quantParam->dequant[0] = quantQtx;

        quantQtx = Xin265pAcQuantQtx (qp, 0, XIN_8_BIT_DEPTH);

        Xin265pInvertQuant (
            &quantParam->quant[1],
            &quantParam->quantShift[1],
            quantQtx);

        quantParam->quantFp[1] = (1 << 16) / quantQtx;
        quantParam->roundFp[1] = (64 * quantQtx) >> 7;
        quantParam->zBin[1]    = XIN_ROUND_POWER2 (zbinFactor * quantQtx, 7);
        quantParam->round[1]   = (roundingFactor * quantQtx) >> 7;
        quantParam->dequant[1] = quantQtx;

    }

    return XIN_SUCCESS;

}

static SINT32 Xin265pConstructEntropy (
    xin_pic_struct *picSet)
{
    xin265p_tile_dim  *tileDim;
    xin_cabac_context *cabacSet;
    xin_cabac_struct  *cabac;
    xin_seq_struct    *seqSet;
    UINT8             *bitBuf;
    UINT16            *preBuf;
    UINT32            bufSize;
    UINT32            sectionIdx;

    seqSet  = picSet->seqSet;
    tileDim = seqSet->tileDim;
    bufSize = tileDim->tileWidth*tileDim->tileHeight*2;

    for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
    {
        XIN_MALLOC_CHECK (cabacSet, sizeof(xin_cabac_context));
        XIN_MALLOC_CHECK (bitBuf, bufSize*sizeof(UINT8));
        XIN_MALLOC_CHECK (preBuf, bufSize*sizeof(UINT16));

        cabac = &cabacSet->cabac;

        cabac->preBufBase     = preBuf;
        cabac->preBufSize     = bufSize;
        cabac->bitstream.base = bitBuf;
        cabac->bitstream.size = bufSize;

        picSet->cabacSet[sectionIdx] = cabacSet;
    }

    return XIN_SUCCESS;

}

SINT32 Xin265pPicCreate (
    xin_pic_struct **dblPicSet,
    xin_seq_struct *seqSet)
{
    xin_pic_struct  *picSet;
    xin_bs_struct   *bitstream;
    UINT32          sbSizeInMi;

    XIN_MALLOC_CHECK (picSet, sizeof(xin_pic_struct));

    *dblPicSet = picSet;

    memset (picSet, 0, sizeof(xin_pic_struct));

    picSet->seqSet  = seqSet;
    picSet->funcSet = seqSet->funcSet;

    if (Xin265pContructPicSb (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (ConstructQuantParam (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (Xin265pConstructEntropy (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    // Malloc bitstream memory for vps sps and pps
    XIN_MALLOC_CHECK (bitstream,       sizeof(xin_bs_struct));
    XIN_MALLOC_CHECK (bitstream->base, XIN_ENTROPY_HEADER_SIZE);

    bitstream->size   = XIN_ENTROPY_HEADER_SIZE;
    picSet->bitstream = bitstream;

    sbSizeInMi = seqSet->sbSize >> XIN_LOG_MI_SIZE;

    XIN_MALLOC_CHECK (picSet->topCtx[PLANE_LUMA],     seqSet->frameWidthInSb*sizeof(UINT8)*sbSizeInMi);
    XIN_MALLOC_CHECK (picSet->topCtx[PLANE_CHROMA_U], seqSet->frameWidthInSb*sizeof(UINT8)*sbSizeInMi);
    XIN_MALLOC_CHECK (picSet->topCtx[PLANE_CHROMA_V], seqSet->frameWidthInSb*sizeof(UINT8)*sbSizeInMi);

    picSet->isFree  = TRUE;

    return XIN_SUCCESS;

}

void Xin265pDestructRefPicBuf (
    xin_seq_struct *seqSet)
{
    xin_ref_picture *refPicture;
    UINT32          frameIdx;

    for (frameIdx = 0; frameIdx < seqSet->allocPicNum; frameIdx++)
    {
        refPicture = seqSet->allocPictures + frameIdx;

        free (refPicture->refBuffer);
        free (refPicture->miBuffer);

        if (seqSet->config.motionSearchMode == XIN_ME_HIER_SEARCH)
        {
            free (refPicture->ref1Buffer);
            free (refPicture->ref2Buffer);
        }

        free (refPicture->rps);
        free (refPicture->qpMap);
        free (refPicture->qpNum);

        free (refPicture->lumaFlt[0]);
        free (refPicture->lumaFlt[1]);
        free (refPicture->chromaFlt[0]);
        free (refPicture->chromaFlt[1]);

    }

    free (seqSet->allocPictures);

}

SINT32 Xin265pConstructRefPicBuf (
    xin_seq_struct *seqSet)
{
    xin_ref_picture *refPicture;
    UINT32          refFrameNum;
    UINT32          frameIdx;
    intptr_t        miStride;
    UINT32          miHeight;
    UINT32          padWidth;
    UINT32          padHeight;
    UINT32          lumaStride;
    UINT32          luma1Stride;
    UINT32          luma2Stride;
    UINT32          chromaStride;
    UINT32          lumaSize;
    UINT32          luma1Size;
    UINT32          luma2Size;
    UINT32          chromaSize;

    refFrameNum = (seqSet->config.temporalLayerNum > 1) ? (seqSet->config.temporalLayerNum + 1) : (seqSet->config.refFrameNum + 1);
    refFrameNum = (seqSet->config.bFrameNum) ? (seqSet->config.bFrameNum + seqSet->config.refFrameNum + 1) : refFrameNum;
    refFrameNum = (seqSet->config.frameThreadNum == 1) ? refFrameNum : (refFrameNum + seqSet->config.frameThreadNum);

    XIN_MALLOC_CHECK (seqSet->allocPictures, sizeof(xin_ref_picture)*refFrameNum);

    seqSet->allocPicNum = refFrameNum;

    padWidth  = seqSet->sbSize + XIN_PADDING_OFFSET_X;
    padHeight = seqSet->sbSize + XIN_PADDING_OFFSET_Y;

    lumaStride   = seqSet->frameWidth + 2*padWidth;
    luma1Stride  = (lumaStride + 1) >> 1;
    luma2Stride  = (lumaStride + 2) >> 2;
    chromaStride = (seqSet->frameWidth + 2*padWidth) >> 1;

    lumaSize   = lumaStride*(seqSet->frameHeight + 2*padHeight);
    luma1Size  = luma1Stride*((seqSet->frameHeight + 2*padHeight + 1) >> 1);
    luma2Size  = luma2Stride*((seqSet->frameHeight + 2*padHeight + 2) >> 2);
    chromaSize = chromaStride*((seqSet->frameHeight + 2*padHeight) >> 1);

    miStride = (seqSet->frameWidthInSb + 1)*seqSet->sbSize / XIN_MI_SIZE + 1;
    miHeight = (seqSet->frameHeightInSb + 1)*seqSet->sbSize / XIN_MI_SIZE + 1;

    for (frameIdx = 0; frameIdx < refFrameNum; frameIdx++)
    {
        refPicture = seqSet->allocPictures + frameIdx;

        XIN_MALLOC_CHECK (refPicture->refBuffer,   (lumaSize + 2*chromaSize)*sizeof(PIXEL));
        XIN_MALLOC_CHECK (refPicture->miBuffer,    miStride * miHeight * sizeof(xin_mi_struct));

        refPicture->refBuf[PLANE_LUMA]     = refPicture->refBuffer + padHeight*lumaStride + padWidth;
        refPicture->refBuf[PLANE_CHROMA_U] = refPicture->refBuffer + lumaSize + ((padHeight*chromaStride + padWidth) >> 1);
        refPicture->refBuf[PLANE_CHROMA_V] = refPicture->refBuffer + lumaSize + chromaSize + ((padHeight*chromaStride + padWidth) >> 1);

        if (seqSet->config.motionSearchMode == XIN_ME_HIER_SEARCH)
        {
            XIN_MALLOC_CHECK (refPicture->ref1Buffer, luma1Size*sizeof(PIXEL));
            XIN_MALLOC_CHECK (refPicture->ref2Buffer, luma2Size*sizeof(PIXEL));

            refPicture->ref1Stride = luma1Stride;
            refPicture->ref2Stride = luma2Stride;
            refPicture->ref1Buf    = refPicture->ref1Buffer + padHeight*luma1Stride/2 + padWidth/2;
            refPicture->ref2Buf    = refPicture->ref2Buffer + padHeight*luma2Stride/4 + padWidth/4;
        }

        refPicture->refStride[0] = lumaStride;
        refPicture->refStride[1] = chromaStride;

        refPicture->miSize   = miHeight*miStride;
        refPicture->miStride = miStride;
        refPicture->miBuf    = refPicture->miBuffer + miStride + 1;

        refPicture->lumaWidth  = seqSet->frameWidth;
        refPicture->lumaHeight = seqSet->frameHeight;

        refPicture->inputWidth  = seqSet->config.inputWidth;
        refPicture->inputHeight = seqSet->config.inputHeight;

        refPicture->paddingWidth  = padWidth;
        refPicture->paddingHeight = padHeight;

        refPicture->widthInPel4 = seqSet->frameWidth >> 2;
        refPicture->widthInPel8 = seqSet->frameWidth >> 3;

        refPicture->heightInPel4 = seqSet->frameHeight >> 2;
        refPicture->heightInPel8 = seqSet->frameHeight >> 3;

        refPicture->widthInMi  = refPicture->widthInPel4;
        refPicture->heightInMi = refPicture->heightInPel4;

        XIN_MALLOC_CHECK (refPicture->rps,      sizeof(xin_rps_struct));
        XIN_MALLOC_CHECK (refPicture->qpMap,    refPicture->widthInPel8 * refPicture->heightInPel8);
        XIN_MALLOC_CHECK (refPicture->qpNum,    seqSet->frameWidthInSb*seqSet->frameHeightInSb*sizeof(UINT8));

        XIN_MALLOC_CHECK (refPicture->lumaFlt[0],   (refPicture->widthInPel4*refPicture->heightInPel4)*sizeof(UINT8));
        XIN_MALLOC_CHECK (refPicture->lumaFlt[1],   (refPicture->widthInPel4*refPicture->heightInPel4)*sizeof(UINT8));
        XIN_MALLOC_CHECK (refPicture->chromaFlt[0], (refPicture->widthInPel8*refPicture->heightInPel8)*sizeof(UINT8));
        XIN_MALLOC_CHECK (refPicture->chromaFlt[1], (refPicture->widthInPel8*refPicture->heightInPel8)*sizeof(UINT8));

        refPicture->lumaFltStride[0]   = refPicture->heightInPel4;
        refPicture->lumaFltStride[1]   = refPicture->widthInPel4;
        refPicture->chromaFltStride[0] = refPicture->heightInPel8;
        refPicture->chromaFltStride[1] = refPicture->widthInPel8;

        refPicture->refBuf[PLANE_LUMA]     = refPicture->refBuffer + padHeight*lumaStride + padWidth;
        refPicture->refBuf[PLANE_CHROMA_U] = refPicture->refBuffer + lumaSize + ((padHeight*chromaStride + padWidth) >> 1);
        refPicture->refBuf[PLANE_CHROMA_V] = refPicture->refBuffer + lumaSize + chromaSize + ((padHeight*chromaStride + padWidth) >> 1);

        refPicture->isFree = TRUE;

    }

    seqSet->dpbSize = 0;

    return XIN_SUCCESS;

}

static void Xin265pConstructRps (
    xin_seq_struct *seqSet)
{
    xin_rps_struct *rpsSet;
    xin_rps_struct *rpsInit;
    SINT32         refFrameNum;
    SINT32         gopSize;
    UINT32         rpsSize;

    refFrameNum = seqSet->config.refFrameNum;
    rpsSet      = seqSet->rpsSet;

    gopSize = 1;
    rpsInit = rpsLdRef[refFrameNum];
    rpsSize = gopSize;

    memcpy (rpsSet, rpsInit, sizeof(xin_rps_struct)*rpsSize);

    seqSet->rpsSize     = rpsSize;
    seqSet->predGopSize = gopSize;

    if ((seqSet->config.temporalLayerNum > 1) || (seqSet->config.bFrameNum))
    {
        seqSet->rcGopSize = seqSet->predGopSize;
    }

    if (seqSet->config.intraPeriod)
    {
        seqSet->config.intraPeriod = (seqSet->config.intraPeriod + seqSet->predGopSize - 1) & (~(seqSet->predGopSize - 1));
    }

}

static SINT32 Xin265pConstructInputPicBuf (
    xin_seq_struct *seqSet)
{
    xin_input_picture *inputPicture;
    UINT32            inputFrameNum;
    UINT32            lumaStride;
    UINT32            chromaStride;
    UINT32            lumaSize;
    UINT32            chromaSize;
    UINT32            frameIdx;
    UINT32            wdtInBlock;
    UINT32            hgtInBlock;
    UINT32            lowSize;
    UINT32            lowWidth;
    UINT32            lowHeight;

    if (seqSet->config.lookAhead)
    {
        inputFrameNum = seqSet->config.lookAhead + seqSet->config.bFrameNum + 3;
    }
    else
    {
        inputFrameNum = (seqSet->config.bFrameNum) ?  (seqSet->config.bFrameNum + 1) : 1;
        inputFrameNum = (seqSet->config.frameThreadNum == 1) ? inputFrameNum : (inputFrameNum + seqSet->config.frameThreadNum - 1);
    }

    lumaStride    = seqSet->config.inputWidth;
    chromaStride  = seqSet->config.inputWidth >> 1;
    lumaSize      = lumaStride*seqSet->config.inputHeight;
    chromaSize    = chromaStride*(seqSet->config.inputHeight >> 1);

    XIN_MALLOC_CHECK (seqSet->allocInputPic, sizeof(xin_input_picture)*inputFrameNum);

    seqSet->allocInputPicNum = inputFrameNum;

    if (inputFrameNum <= 1)
    {
        return XIN_SUCCESS;
    }

    lowWidth   = seqSet->config.inputWidth >> 1;
    lowHeight  = seqSet->config.inputHeight >> 1;
    wdtInBlock = lowWidth / XIN_LOW_UNIT_SIZE;
    hgtInBlock = lowHeight / XIN_LOW_UNIT_SIZE;
    lowSize    = (lowWidth + 2 * XIN_LOW_PADDING_SIZE)*(lowHeight + 2 * XIN_LOW_PADDING_SIZE);

    for (frameIdx = 0; frameIdx < inputFrameNum; frameIdx++)
    {
        inputPicture = seqSet->allocInputPic + frameIdx;

        XIN_MALLOC_CHECK (inputPicture->inputBuffer, (lumaSize + 2*chromaSize)*sizeof(PIXEL));

        inputPicture->inputBuf[PLANE_LUMA]     = inputPicture->inputBuffer;
        inputPicture->inputBuf[PLANE_CHROMA_U] = inputPicture->inputBuffer + lumaSize;
        inputPicture->inputBuf[PLANE_CHROMA_V] = inputPicture->inputBuffer + lumaSize + chromaSize;

        inputPicture->inputStride[0] = lumaStride;
        inputPicture->inputStride[1] = chromaStride;

        if (seqSet->config.lookAhead)
        {
            XIN_MALLOC_CHECK (inputPicture->lowerBuffer, lowSize*sizeof(PIXEL));
            XIN_MALLOC_CHECK (inputPicture->intraCost,   wdtInBlock*hgtInBlock*sizeof(UINT32));
            XIN_MALLOC_CHECK (inputPicture->interCost,   wdtInBlock*hgtInBlock*sizeof(UINT32));
            XIN_MALLOC_CHECK (inputPicture->laMv[0],     wdtInBlock*hgtInBlock*sizeof(xin_mv_u));
            XIN_MALLOC_CHECK (inputPicture->laMv[1],     wdtInBlock*hgtInBlock*sizeof(xin_mv_u));
            XIN_MALLOC_CHECK (inputPicture->interDir,    wdtInBlock*hgtInBlock*sizeof(UINT8));
            XIN_MALLOC_CHECK (inputPicture->propCost,    wdtInBlock*hgtInBlock*sizeof(UINT16));
            XIN_MALLOC_CHECK (inputPicture->qpOffset,    wdtInBlock*hgtInBlock*sizeof(SINT16));

            inputPicture->lowerStride = lowWidth + 2*XIN_LOW_PADDING_SIZE;
            inputPicture->lowerBuf    = inputPicture->lowerBuffer + XIN_LOW_PADDING_SIZE*inputPicture->lowerStride + XIN_LOW_PADDING_SIZE;
            inputPicture->laTotalUnit = wdtInBlock*hgtInBlock;
            inputPicture->laWdtInUnit = wdtInBlock;
            inputPicture->laHgtInUnit = hgtInBlock;

        }

        inputPicture->bufStage = XIN_BUF_INVALID;

    }

    return XIN_SUCCESS;

}

static void Xin265pDestructInputPicBuf (
    xin_seq_struct *seqSet)
{
    xin_input_picture *inputPicture;
    UINT32            frameIdx;

    if (seqSet->allocInputPicNum > 1)
    {
        for (frameIdx = 0; frameIdx < seqSet->allocInputPicNum; frameIdx++)
        {
            inputPicture = seqSet->allocInputPic + frameIdx;

            free (inputPicture->inputBuffer);

            if (seqSet->config.lookAhead)
            {
                free (inputPicture->lowerBuffer);
                free (inputPicture->intraCost);
                free (inputPicture->interCost);
                free (inputPicture->laMv[0]);
                free (inputPicture->laMv[1]);
                free (inputPicture->interDir);
                free (inputPicture->propCost);
                free (inputPicture->qpOffset);
            }
        }
    }

    free (seqSet->allocInputPic);

}

SINT32 Xin265pSeqCreate (
    xin_seq_struct **dblSeqSet,
    xin26x_params  *config)
{
    xin_seq_struct *seqSet;
    xin_lb_struct  *outputBuf;

    XIN_MALLOC_CHECK (seqSet,  sizeof(xin_seq_struct));

    *dblSeqSet = seqSet;

    memset (seqSet, 0, sizeof(xin_seq_struct));

    memcpy (seqSet->splitMask, splitMask, sizeof(UINT32)*XIN_MAX_MB_DEPTH);

    seqSet->splitMask[0] &= ~(XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4);
    seqSet->splitMask[1] &= ~(XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4);
    seqSet->splitMask[2] &= ~(XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4);
    seqSet->splitMask[3] &= ~(XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4);
    seqSet->splitMask[4] &= ~(XIN_CAN_PART_HORZ_B | XIN_CAN_PART_HORZ_A | XIN_CAN_PART_VERT_A | XIN_CAN_PART_VERT_B | XIN_CAN_PART_HORZ_4 | XIN_CAN_PART_VERT_4);

    seqSet->config.inputWidth  = config->inputWidth;
    seqSet->config.inputHeight = config->inputHeight;
    seqSet->config.sbSize      = config->sbSize;

    seqSet->lgSbSize = calcLog2[config->sbSize];
    seqSet->sbSize   = config->sbSize;

    seqSet->frameWidth  = (config->inputWidth + 7) & (~7);
    seqSet->frameHeight = (config->inputHeight + 7) & (~7);
    seqSet->rcGopSize   = 1;

    seqSet->frameWidthInSb  = (seqSet->frameWidth + (seqSet->sbSize - 1)) / seqSet->sbSize;
    seqSet->frameHeightInSb = (seqSet->frameHeight + (seqSet->sbSize - 1)) / seqSet->sbSize;
    seqSet->frameSizeInSb   = seqSet->frameWidthInSb * seqSet->frameHeightInSb;
    seqSet->frameWidthInMi  = (seqSet->frameWidth + (XIN_MI_SIZE - 1)) / XIN_MI_SIZE;
    seqSet->frameHeightInMi = (seqSet->frameHeight + (XIN_MI_SIZE - 1)) / XIN_MI_SIZE;

    seqSet->config.calcPsnr    = config->calcPsnr;
    seqSet->config.refFrameNum = 1;
    seqSet->config.intraPeriod = config->intraPeriod;
    seqSet->config.numTileRows = config->numTileRows;
    seqSet->config.numTileCols = config->numTileCols;
    seqSet->config.enableTiles = ((config->numTileRows > 1) || (config->numTileCols > 1)) && (!config->enableWpp);
    seqSet->config.numTileCols = (config->enableWpp) ? 1 : seqSet->config.numTileCols;
    seqSet->config.numTileRows = (config->enableWpp) ? seqSet->frameHeightInSb : seqSet->config.numTileRows;
    seqSet->tileNum            = seqSet->config.numTileRows*seqSet->config.numTileCols;
    seqSet->config.frameRate   = config->frameRate;

    seqSet->config.frameToBeEncoded = config->frameToBeEncoded;
    seqSet->config.outputFormat     = config->outputFormat;
    seqSet->config.frameThreadNum   = 1;
    seqSet->config.enablePartMask   = XIN_CAN_PART_SPLIT;
    seqSet->config.enablePartMask   = config->enableRectPartType ? (seqSet->config.enablePartMask | XIN_CAN_PART_RECT) : seqSet->config.enablePartMask;
    seqSet->config.threadNum        = 1;

    if (seqSet->config.encoderMode == 0)
    {
        seqSet->config.motionSearchMode = XIN_ME_BBDGS_SEARCH;
        seqSet->config.intraRdoNum      = 1;
        seqSet->config.maxMdCandNum     = 1;
        seqSet->config.earlyStopMode    = 0;
    }
    else
    {
        seqSet->config.motionSearchMode = XIN_ME_BBDGS_SEARCH;
        seqSet->config.intraRdoNum      = 1;
        seqSet->config.maxMdCandNum     = 1;
        seqSet->config.earlyStopMode    = 0;
    }

    XIN_MALLOC_CHECK (seqSet->sbTsToRsAddrMap, seqSet->frameSizeInSb * sizeof(UINT32));
    XIN_MALLOC_CHECK (seqSet->rpsSet,          sizeof(xin_rps_struct)*XIN_MAX_RPS_NUM);

    ConstructTileDim (
        seqSet);

    Xin265pConstructRps (
        seqSet);

    if (Xin265pConstructInputPicBuf (seqSet))
    {
        return XIN_FAIL;
    }

    if (Xin265pConstructRefPicBuf (seqSet))
    {
        return XIN_FAIL;
    }

    // Malloc linear buffer memory
    XIN_MALLOC_CHECK (outputBuf, sizeof(xin_lb_struct));
    XIN_MALLOC_CHECK (outputBuf->base, seqSet->frameWidth*seqSet->frameHeight*3/2);

    outputBuf->size   = seqSet->frameWidth*seqSet->frameHeight*3/2;
    outputBuf->index  = 0;
    seqSet->outputBuf = outputBuf;

    return XIN_SUCCESS;

}

static SINT32 Xin265pVerifyConfig (
    xin26x_params *config)
{
    UINT32 predGopSize;

    config->rcMode               = 0;
    config->bFrameNum            = 0;
    config->lookAhead            = 0;
    config->enableWpp            = FALSE;
    config->enableFpp            = FALSE;
    config->threadNum            = 1;
    config->intraPeriod          = 1;
    config->enableRdoq           = FALSE;
    config->enableSignDataHiding = FALSE;
    config->refreshType          = 1;
    config->unitTree             = FALSE;
    config->enableDeblock        = FALSE;
    config->enableSao            = FALSE;
    config->transformSkipFlag    = FALSE;

    if (((config->inputWidth & 0x01) != 0)
            || ((config->inputHeight & 0x01) != 0)
            || (config->inputWidth <= XIN_MAX_SB_SIZE)
            || (config->inputHeight <= XIN_MAX_SB_SIZE)
            || (config->inputWidth > XIN_MAX_FRAME_WIDTH)
            || (config->inputHeight > XIN_MAX_FRAME_HEIGHT))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Input picture height or width is not correct.\n");

        return XIN_FAIL;
    }

    if ((config->temporalLayerNum > XIN_MAX_TEMPORAL_LAYER) || (config->temporalLayerNum < XIN_MIN_TEMPORAL_LAYER))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Temporal layer num is not correct.\n");

        return XIN_FAIL;
    }

    if ((config->temporalLayerNum > 1) && (config->refFrameNum > 1))
    {
        _XIN_LOGGER (XIN_LOGGER_WARNING, "Only support one frame reference, when temporal scalability is on.\n");
    }

    if (config->qp > XIN_MAX_QP)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Qp parameter is out of range.\n");

        return XIN_FAIL;
    }

    if ((config->numTileCols < XIN_MIN_WIDTH_IN_TILE) || (config->numTileRows < XIN_MIN_HEIGHT_IN_TILE))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Picture at least contains one tile.\n");

        return XIN_FAIL;
    }

    if (((config->numTileCols > XIN_MAX_WIDTH_IN_TILE) || (config->numTileRows > XIN_MAX_HEIGHT_IN_TILE))
            && (config->enableWpp != TRUE))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "The max tile horizontal or vertical num is 2.\n");

        return XIN_FAIL;
    }

    if ((config->refFrameNum < XIN_MIN_REF_FRAMES) || (config->refFrameNum > XIN_MAX_REF_FRAMES))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Reference frame number is out of range.\n");

        return XIN_FAIL;
    }

    if ((config->sbSize != 128) && (config->sbSize != 64))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Super block is not correct.\n");

        return XIN_FAIL;
    }

    if ((config->bFrameNum != 1) && (config->bFrameNum != 3) && (config->bFrameNum != 7) && (config->bFrameNum != 0))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "B frame num is not correct.\n");

        return XIN_FAIL;
    }

    if (config->bFrameNum > 0)
    {
        predGopSize = config->bFrameNum + 1;
    }
    else
    {
        predGopSize = 1 << (config->temporalLayerNum - 1);
    }

    if ((config->intraPeriod != 0) && (config->intraPeriod < predGopSize))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Intra period should be at least a prediction gop size.\n");

        return XIN_FAIL;
    }

    if (config->threadNum > XIN_MAX_THREAD_POOL_NUM)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Thread num is exceed the max thread pool num.\n");

        return XIN_FAIL;
    }

    if ((config->frameSkip) && (config->bFrameNum))
    {
        _XIN_LOGGER (XIN_LOGGER_WARNING, "Frame skip is not supported, when B frame is on.\n");
    }

    return XIN_SUCCESS;

}

SINT32 Xin265pEncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config)
{
    xin265p_encoder_struct *h265pEncoder;
    xin_seq_struct         *seqSet;
    xin_func_struct        *funcSet;
    UINT32                 sectionIdx;
    UINT32                 frameIdx;
    UINT32                 cpuFeature;
    UINT32                 cpuCoreNum;

    if (Xin265pVerifyConfig(config) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    // malloc memory for encoder instance
    XIN_MALLOC_CHECK (h265pEncoder, sizeof(xin265p_encoder_struct));

    *encoderHandle = (XIN_HANDLE)h265pEncoder;
    memset (h265pEncoder, 0, sizeof(xin265p_encoder_struct));

    XIN_MALLOC_CHECK (funcSet, sizeof(xin_func_struct));

    memset (funcSet, 0, sizeof(xin_func_struct));
    h265pEncoder->funcSet = funcSet;

    Xin26xCpuDetection (
        &cpuFeature,
        &cpuCoreNum);

    _XIN_LOGGER (XIN_LOGGER_STATUS, "Xin26x lib build date: %s %s", __DATE__, __TIME__);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "yuv size %dx%d fps:%3.3f frame num:%d", config->inputWidth, config->inputHeight, config->frameRate, config->frameToBeEncoded);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "algorithm:%d encoder mode:%d rdoq:%d screen mode:%d", config->algorithmMode, config->encoderMode, config->enableRdoq, config->screenContentMode);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "smp:%d mctf:%d sao:%d deblock:%d alf:%d", config->enableSmp, config->enableMctf, config->enableSao, config->enableDeblock, config->enableAlf);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "rc mode:%d target bit:%d", config->rcMode, config->bitRate);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "thread num:%d wpp:%d fpp:%d", config->threadNum > 0 ? XIN_MIN (config->threadNum, cpuCoreNum) : cpuCoreNum, config->enableWpp, config->enableFpp);

    Xin265pFuncInit (
        funcSet,
        cpuFeature);

    Xin265pSeqCreate (
        &h265pEncoder->seqSet,
        config);

    seqSet          = h265pEncoder->seqSet;
    seqSet->funcSet = funcSet;

    for (frameIdx = 0; frameIdx < seqSet->config.frameThreadNum; frameIdx++)
    {
        if (Xin265pPicCreate (h265pEncoder->picSet + frameIdx, seqSet) == XIN_FAIL)
        {
            return XIN_FAIL;
        }
    }

    for (sectionIdx = 0; sectionIdx < seqSet->config.threadNum; sectionIdx++)
    {
        if (Xin265pSecCreate (h265pEncoder->secSet + sectionIdx, seqSet) == XIN_FAIL)
        {
            return XIN_FAIL;
        }
    }

    return XIN_SUCCESS;

}

void Xin265pPicDelete (
    xin_pic_struct *picSet)
{
    xin_bs_struct  *bitstream;
    xin_seq_struct *seqSet;
    UINT32         tileIdx;

    bitstream = picSet->bitstream;
    seqSet    = picSet->seqSet;

    free (picSet->sb);
    free (picSet->quantParam);

    free (bitstream->base);
    free (bitstream);
    free (picSet->topCtx[PLANE_LUMA]);
    free (picSet->topCtx[PLANE_CHROMA_U]);
    free (picSet->topCtx[PLANE_CHROMA_V]);

    for (tileIdx = 0; tileIdx < seqSet->tileNum; tileIdx++)
    {
        free (picSet->cabacSet[tileIdx]->cabac.bitstream.base);
        free (picSet->cabacSet[tileIdx]->cabac.preBufBase);
        free (picSet->cabacSet[tileIdx]);
    }

    free (picSet);

}

void Xin265pSeqDelete (
    xin_seq_struct *seqSet)
{
    free (seqSet->sbTsToRsAddrMap);
    free (seqSet->rpsSet);

    Xin265pDestructInputPicBuf (
        seqSet);

    Xin265pDestructRefPicBuf (
        seqSet);

    free (seqSet->outputBuf->base);
    free (seqSet->outputBuf);

    free (seqSet);
}

void Xin265pEncoderDelete (
    XIN_HANDLE encoderHandle)
{
    xin265p_encoder_struct *h265pEncoder;
    xin_pic_struct         *picSet;
    xin_seq_struct         *seqSet;
    UINT32                 frameIdx;
    UINT32                 sectionIdx;

    h265pEncoder = (xin265p_encoder_struct *)encoderHandle;
    seqSet       = h265pEncoder->seqSet;

    for (frameIdx = 0; frameIdx < seqSet->config.frameThreadNum; frameIdx++)
    {
        picSet = h265pEncoder->picSet[frameIdx];

        Xin265pPicDelete (
            picSet);
    }

    for (sectionIdx = 0; sectionIdx < seqSet->config.threadNum; sectionIdx++)
    {
        Xin265pSecDelete(
            h265pEncoder->secSet[sectionIdx],
            seqSet);
    }

    Xin265pSeqDelete (
        seqSet);

    free (h265pEncoder->funcSet);

    free (h265pEncoder);

}


