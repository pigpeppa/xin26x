/***************************************************************************//**
*
* @file          h265p_encode_init.c
* @brief         This file contains frame, section, super block and
*                macro block level initialization.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "xin26x_logger.h"
#include "assert.h"
#include "string.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_bit_stream.h"
#include "h265p_write_header.h"
#include "h265p_analyze_mb.h"
#include "h26x_calc_psnr_ssim.h"
#include "h26x_definition.h"

static const UINT32 sseLambda[XIN_QP_NUM] =
{
    58,      234,     234,     297,     366,     443,     528,     528,     619,     718,
    825,     938,     1059,    1188,    1323,    1323,    1466,    1617,    1774,    1939,
    2112,    2291,    2478,    2478,    2673,    2874,    3083,    3300,    3523,    3754,
    3754,    3993,    4238,    4491,    4752,    5019,    5294,    5294,    5577,    5866,
    6163,    6468,    6779,    6779,    7098,    7425,    7758,    8099,    8448,    8448,
    8803,    9166,    9537,    9914,    10299,   10299,   10692,   11091,   11498,   11913,
    11913,   12334,   12763,   13200,   13643,   14094,   14094,   14553,   15018,   15491,
    15972,   15972,   16459,   16954,   17457,   17966,   17966,   18483,   19008,   19539,
    20078,   20078,   20625,   21178,   21739,   22308,   22308,   22883,   23466,   24057,
    24057,   24654,   25259,   25872,   26491,   26491,   27753,   28394,   29700,   31034,
    31713,   33091,   33792,   35214,   35937,   37403,   38148,   39658,   40425,   41979,
    42768,   44366,   45177,   46819,   47652,   49338,   50193,   51054,   52800,   53683,
    55473,   57291,   59139,   61017,   62923,   65838,   67818,   69828,   71866,   73934,
    76032,   78158,   80314,   82500,   84714,   86958,   89232,   91534,   95043,   98618,
    101038,  104723,  108474,  111012,  114873,  118800,  121454,  125491,  128219,  132366,
    135168,  139425,  145203,  149614,  154091,  158634,  163243,  167918,  172659,  177466,
    182339,  187278,  193966,  199059,  205953,  211200,  216513,  223699,  229166,  234699,
    242179,  249777,  257491,  265323,  271274,  279312,  287466,  295738,  304128,  312634,
    321258,  330000,  338858,  350097,  359219,  368459,  380174,  389678,  399300,  411491,
    423866,  433898,  446603,  459492,  472563,  485818,  499257,  512878,  526683,  540672,
    554843,  572091,  586666,  604398,  619377,  637593,  656073,  674817,  693825,  713097,
    732633,  755758,  779243,  799659,  827291,  851854,  876777,  905699,  935091,  964953,
    999108,  1029966, 1065243, 1105137, 1145763, 1187123, 1229217, 1276366, 1328814, 1382318,
    1436878, 1501866, 1568292, 1636154, 1715472, 1796666, 1884993, 1986218, 2090091, 2202291,
    2323258, 2459457, 2605713, 2768923, 2943658, 3137291, 3344091, 3579194, 3829774, 4104334,
    4420548, 4756843, 5140138, 5565354, 6026254, 6544618
};

static const UINT32 sadLambda[XIN_QP_NUM] =
{
    86,    173,   173,   194,   216,   238,   259,   259,   281,   303,   324,   346,   368,
    389,   411,   411,   433,   454,   476,   498,   519,   541,   563,   563,   584,   606,
    628,   649,   671,   693,   693,   714,   736,   758,   779,   801,   823,   823,   844,
    866,   888,   909,   931,   931,   953,   974,   996,   1018,  1039,  1039,  1061,  1083,
    1104,  1126,  1148,  1148,  1169,  1191,  1213,  1234,  1234,  1256,  1278,  1299,  1321,
    1343,  1343,  1364,  1386,  1408,  1429,  1429,  1451,  1473,  1494,  1516,  1516,  1538,
    1559,  1581,  1603,  1603,  1624,  1646,  1668,  1689,  1689,  1711,  1733,  1754,  1754,
    1776,  1798,  1819,  1841,  1841,  1884,  1906,  1949,  1993,  2014,  2058,  2079,  2123,
    2144,  2188,  2209,  2253,  2274,  2318,  2339,  2383,  2404,  2448,  2469,  2513,  2534,
    2556,  2599,  2621,  2664,  2707,  2751,  2794,  2837,  2902,  2946,  2989,  3032,  3076,
    3119,  3162,  3206,  3249,  3292,  3336,  3379,  3422,  3487,  3552,  3596,  3661,  3726,
    3769,  3834,  3899,  3942,  4007,  4051,  4116,  4159,  4224,  4311,  4376,  4441,  4506,
    4571,  4636,  4701,  4766,  4831,  4896,  4982,  5047,  5134,  5199,  5264,  5351,  5416,
    5481,  5567,  5654,  5740,  5827,  5892,  5979,  6065,  6152,  6239,  6325,  6412,  6499,
    6585,  6694,  6780,  6867,  6975,  7062,  7149,  7257,  7365,  7452,  7560,  7669,  7777,
    7885,  7994,  8102,  8210,  8319,  8427,  8557,  8665,  8795,  8903,  9033,  9163,  9293,
    9423,  9553,  9683,  9835,  9987,  10117, 10290, 10442, 10593, 10767, 10940, 11113, 11308,
    11481, 11676, 11893, 12110, 12326, 12543, 12781, 13041, 13301, 13561, 13865, 14168, 14471,
    14818, 15164, 15533, 15944, 16356, 16789, 17244, 17742, 18262, 18826, 19411, 20039, 20689,
    21404, 22140, 22920, 23787, 24675, 25650, 26690, 27773, 28943
};

void Xin265pMbInit (
    xin_sec_struct *secSet,
    xin_mb_struct  *parentMb,
    UINT32         partType,
    UINT32         partIdx)
{
    xin_seq_struct  *seqSet;
    xin_sb_struct   *sb;
    xin_mb_struct   *mb;
    UINT32          sbSize;
    SINT32          mbWidth;
    SINT32          mbHeight;
    UINT32          mbDepth;
    UINT32          mbIndex;
    SINT32          mbPelX;
    SINT32          mbPelY;
    UINT32          mbOffX;
    UINT32          mbOffY;
    SINT32          frameHeight;
    SINT32          frameWidth;
    SINT32          botEdgeMi;
    SINT32          rgtEdgeMi;
    SINT32          gemFlag;

    if (partIdx > 3)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Child index is out of range.\n");
    }

    seqSet      = secSet->seqSet;
    sbSize      = seqSet->sbSize;
    sb          = secSet->sb;
    frameHeight = seqSet->frameHeight;
    frameWidth  = seqSet->frameWidth;

    if (parentMb == NULL)
    {
        mbWidth  = sbSize;
        mbHeight = sbSize;
        mbDepth  = 0;
        mbIndex  = 0;
        mbPelX   = sb->sbPelX;
        mbPelY   = sb->sbPelY;
        mb       = secSet->pqMbData[mbDepth] + mbIndex;
        mbOffX   = 0;
        mbOffY   = 0;
        gemFlag  = (mbPelY < frameHeight) && (mbPelX < frameWidth);
        gemFlag  = gemFlag ? XIN_MB_PRESENT : 0;
        gemFlag |= ((mbPelX + mbWidth / 2) < frameWidth) ? XIN_MB_HAS_HOR : 0;
        gemFlag |= ((mbPelY + mbHeight / 2) < frameHeight) ? XIN_MB_HAS_VER : 0;
    }
    else
    {
        switch (partType)
        {

        case XIN_PART_SPLIT:
            mbWidth  = parentMb->width[PLANE_LUMA] >> 1;
            mbHeight = parentMb->height[PLANE_LUMA] >> 1;
            mbDepth  = parentMb->depth + 1;
            mbOffX   = (partIdx & 1) ? mbWidth  : 0;
            mbOffY   = (partIdx > 1) ? mbHeight : 0;
            mbOffX  += parentMb->offX[PLANE_LUMA];
            mbOffY  += parentMb->offY[PLANE_LUMA];
            mbIndex  = (mbOffX / mbWidth) + (mbOffY / mbHeight)*(sbSize / mbWidth);
            mbPelX   = sb->sbPelX + mbOffX;
            mbPelY   = sb->sbPelY + mbOffY;
            mb       = secSet->pqMbData[mbDepth] + mbIndex;
            gemFlag  = (mbPelY < frameHeight) && (mbPelX < frameWidth);
            gemFlag  = gemFlag ? XIN_MB_PRESENT : 0;
            gemFlag |= ((mbPelX + mbWidth / 2) < frameWidth) ? XIN_MB_HAS_HOR : 0;
            gemFlag |= ((mbPelY + mbHeight / 2) < frameHeight) ? XIN_MB_HAS_VER : 0;

            break;

        case XIN_PART_HORZ:
            mbWidth  = parentMb->width[PLANE_LUMA];
            mbHeight = parentMb->height[PLANE_LUMA] >> 1;
            mbDepth  = parentMb->depth + 1;
            mbOffX   = 0;
            mbOffY   = (partIdx > 0) ? mbHeight : 0;
            mbOffX  += parentMb->offX[PLANE_LUMA];
            mbOffY  += parentMb->offY[PLANE_LUMA];
            mbIndex  = (mbOffX / mbWidth) + (mbOffY / mbHeight)*(sbSize / mbWidth);
            mbPelX   = sb->sbPelX + mbOffX;
            mbPelY   = sb->sbPelY + mbOffY;
            mb       = secSet->phMbData[mbDepth - 1] + mbIndex;
            gemFlag  = (mbPelY < frameHeight) && (mbPelX < frameWidth);
            gemFlag  = gemFlag ? XIN_MB_PRESENT : 0;

            break;

        case XIN_PART_VERT:
            mbWidth  = parentMb->width[PLANE_LUMA] >> 1;
            mbHeight = parentMb->height[PLANE_LUMA];
            mbDepth  = parentMb->depth + 1;
            mbOffX   = (partIdx > 0) ? mbWidth  : 0;
            mbOffY   = 0;
            mbOffX  += parentMb->offX[PLANE_LUMA];
            mbOffY  += parentMb->offY[PLANE_LUMA];
            mbIndex  = (mbOffX / mbWidth) + (mbOffY / mbHeight)*(sbSize / mbWidth);
            mbPelX   = sb->sbPelX + mbOffX;
            mbPelY   = sb->sbPelY + mbOffY;
            mb       = secSet->pvMbData[mbDepth - 1] + mbIndex;
            gemFlag  = (mbPelY < frameHeight) && (mbPelX < frameWidth);
            gemFlag  = gemFlag ? XIN_MB_PRESENT : 0;

            break;

        default:
            mb       = parentMb;
            mbWidth  = mb->width[PLANE_LUMA];
            mbHeight = mb->height[PLANE_LUMA];
            mbDepth  = mb->depth;
            mbPelX   = mb->mbPelX[PLANE_LUMA];
            mbPelY   = mb->mbPelY[PLANE_LUMA];
            mbOffX   = mb->offX[PLANE_LUMA];
            mbOffY   = mb->offY[PLANE_LUMA];
            gemFlag  = mb->geomFlag;
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Macroblock partition type is not supported!\n");
            break;

        }

    }

    mb->mbPelX[PLANE_LUMA]   = (UINT16)mbPelX;
    mb->mbPelY[PLANE_LUMA]   = (UINT16)mbPelY;
    mb->mbPelX[PLANE_CHROMA] = (UINT16)((mbPelX >> 1) & 0xFFFFFFFC);
    mb->mbPelY[PLANE_CHROMA] = (UINT16)((mbPelY >> 1) & 0xFFFFFFFC);

    mb->depth     = (UINT8)mbDepth;
    mb->geomFlag  = (UINT8)gemFlag;
    mb->parentMb  = parentMb;
    mb->partType  = (UINT8)partType;
    mb->bestBuf   = NULL;
    mb->sseCost   = XIN_MAX_U64_COST;
    mb->sadCost   = XIN_MAX_U64_COST;
    botEdgeMi     = (mbPelY + mbHeight - frameHeight) >> XIN_LOG_MI_SIZE;
    rgtEdgeMi     = (mbPelX + mbWidth - frameWidth) >> XIN_LOG_MI_SIZE;
    mb->botEdgeMi = (SINT8)XIN_MAX (0, botEdgeMi);
    mb->rgtEdgeMi = (SINT8)XIN_MAX (0, rgtEdgeMi);

    secSet->lftMb = mbOffX ? mb - 1 : NULL;
    secSet->topMb = mbOffY ? mb - (sbSize / mbWidth) : NULL;
    secSet->mb    = mb;

    secSet->inputMb[0] = secSet->inputSb[0] + mb->offX[PLANE_LUMA] + mb->offY[PLANE_LUMA]*secSet->inputYStride;
    secSet->inputMb[1] = secSet->inputSb[1] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA]*secSet->inputUvStride;
    secSet->inputMb[2] = secSet->inputSb[2] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA]*secSet->inputUvStride;

    secSet->reconMb[0] = secSet->reconSb[0] + mb->offX[PLANE_LUMA] + mb->offY[PLANE_LUMA]*secSet->reconStride[0];
    secSet->reconMb[1] = secSet->reconSb[1] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA]*secSet->reconStride[1];
    secSet->reconMb[2] = secSet->reconSb[2] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA]*secSet->reconStride[1];

    if (partType == XIN_PART_SPLIT)
    {
        mb->mbLftCtx[PLANE_LUMA]     = secSet->sbLftCtx[PLANE_LUMA]     + (mb->offY[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        mb->mbLftCtx[PLANE_CHROMA_U] = secSet->sbLftCtx[PLANE_CHROMA_U] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        mb->mbLftCtx[PLANE_CHROMA_V] = secSet->sbLftCtx[PLANE_CHROMA_V] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);

        mb->mbTopCtx[PLANE_LUMA]     = secSet->sbTopCtx[PLANE_LUMA] + (mb->offX[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        mb->mbTopCtx[PLANE_CHROMA_U] = secSet->sbTopCtx[PLANE_CHROMA_U] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        mb->mbTopCtx[PLANE_CHROMA_V] = secSet->sbTopCtx[PLANE_CHROMA_V] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    }
    else
    {
        mb->mbLftCtx[PLANE_LUMA]     = secSet->lftCtx[mbDepth][partType][PLANE_LUMA] + (mb->offY[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        mb->mbLftCtx[PLANE_CHROMA_U] = secSet->lftCtx[mbDepth][partType][PLANE_CHROMA_U] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        mb->mbLftCtx[PLANE_CHROMA_V] = secSet->lftCtx[mbDepth][partType][PLANE_CHROMA_V] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);

        mb->mbTopCtx[PLANE_LUMA]     = secSet->topCtx[mbDepth][partType][PLANE_LUMA] + (mb->offX[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        mb->mbTopCtx[PLANE_CHROMA_U] = secSet->topCtx[mbDepth][partType][PLANE_CHROMA_U] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        mb->mbTopCtx[PLANE_CHROMA_V] = secSet->topCtx[mbDepth][partType][PLANE_CHROMA_V] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    }

    if (mb->canSplit & XIN_CAN_PART_RECT)
    {
        memcpy (secSet->topCtx[mbDepth + 1][XIN_PART_HORZ][PLANE_LUMA] + (mb->offX[PLANE_LUMA] >> XIN_LOG_MI_SIZE),       mb->mbTopCtx[PLANE_LUMA],     sizeof(UINT8)*(mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->topCtx[mbDepth + 1][XIN_PART_VERT][PLANE_LUMA] + (mb->offX[PLANE_LUMA] >> XIN_LOG_MI_SIZE),       mb->mbTopCtx[PLANE_LUMA],     sizeof(UINT8)*(mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->topCtx[mbDepth + 1][XIN_PART_HORZ][PLANE_CHROMA_U] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbTopCtx[PLANE_CHROMA_U], sizeof(UINT8)*(mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->topCtx[mbDepth + 1][XIN_PART_VERT][PLANE_CHROMA_U] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbTopCtx[PLANE_CHROMA_U], sizeof(UINT8)*(mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->topCtx[mbDepth + 1][XIN_PART_HORZ][PLANE_CHROMA_V] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbTopCtx[PLANE_CHROMA_V], sizeof(UINT8)*(mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->topCtx[mbDepth + 1][XIN_PART_VERT][PLANE_CHROMA_V] + (mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbTopCtx[PLANE_CHROMA_V], sizeof(UINT8)*(mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));

        memcpy (secSet->lftCtx[mbDepth + 1][XIN_PART_HORZ][PLANE_LUMA] + (mb->offY[PLANE_LUMA] >> XIN_LOG_MI_SIZE),       mb->mbLftCtx[PLANE_LUMA],     sizeof(UINT8)*(mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->lftCtx[mbDepth + 1][XIN_PART_VERT][PLANE_LUMA] + (mb->offY[PLANE_LUMA] >> XIN_LOG_MI_SIZE),       mb->mbLftCtx[PLANE_LUMA],     sizeof(UINT8)*(mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->lftCtx[mbDepth + 1][XIN_PART_HORZ][PLANE_CHROMA_U] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbLftCtx[PLANE_CHROMA_U], sizeof(UINT8)*(mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->lftCtx[mbDepth + 1][XIN_PART_VERT][PLANE_CHROMA_U] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbLftCtx[PLANE_CHROMA_U], sizeof(UINT8)*(mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->lftCtx[mbDepth + 1][XIN_PART_HORZ][PLANE_CHROMA_V] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbLftCtx[PLANE_CHROMA_V], sizeof(UINT8)*(mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
        memcpy (secSet->lftCtx[mbDepth + 1][XIN_PART_VERT][PLANE_CHROMA_V] + (mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE), mb->mbLftCtx[PLANE_CHROMA_V], sizeof(UINT8)*(mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE));
    }

    Xin265pGetBlockAvail (
        secSet,
        mb);

}

void Xin265pMbPostInit (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    xin_full_md_buf *fullBuf;
    xin_fast_md_buf *fastBuf;
    SINT32          tuIdx;
    xin_tu_struct   *tuY;
    xin_tu_struct   *tuU;
    xin_tu_struct   *tuV;
    SINT32          offsetX;
    SINT32          offsetY;

    (void)secSet;
    fullBuf      = mb->bestBuf;
    fastBuf      = fullBuf->fastBuf;
    mb->sseCost  = fullBuf->sseCost;
    mb->sadCost  = fastBuf->sadCost;
    mb->tuNum    = (UINT8)fullBuf->tuNum;
    mb->predMode = (UINT8)fastBuf->predMode;
    mb->mbType   = (UINT8)fastBuf->type;
    mb->txSize   = (UINT8)fullBuf->tranSize[0];

    for (tuIdx = 0; tuIdx < mb->tuNum; tuIdx++)
    {
        tuY = mb->tu[PLANE_LUMA] + tuIdx;
        tuU = mb->tu[PLANE_CHROMA_U] + tuIdx;
        tuV = mb->tu[PLANE_CHROMA_V] + tuIdx;

        tuY->eob = fullBuf->eob[tuIdx][PLANE_LUMA];
        tuU->eob = fullBuf->eob[tuIdx][PLANE_CHROMA_U];
        tuV->eob = fullBuf->eob[tuIdx][PLANE_CHROMA_V];

        tuY->culLevel = fullBuf->culLevel[tuIdx][PLANE_LUMA];
        tuU->culLevel = fullBuf->culLevel[tuIdx][PLANE_CHROMA_U];
        tuV->culLevel = fullBuf->culLevel[tuIdx][PLANE_CHROMA_V];

        tuY->txSize = (UINT8)fullBuf->tranSize[PLANE_LUMA];
        tuU->txSize = (UINT8)fullBuf->tranSize[PLANE_CHROMA];
        tuV->txSize = (UINT8)fullBuf->tranSize[PLANE_CHROMA];

        tuY->txType = (UINT8)fullBuf->tranType[PLANE_LUMA];
        tuU->txType = (UINT8)fullBuf->tranType[PLANE_CHROMA];
        tuV->txType = (UINT8)fullBuf->tranType[PLANE_CHROMA];

        tuY->splitFlag = FALSE;
        tuU->splitFlag = FALSE;
        tuV->splitFlag = FALSE;

        tuY->txSkipCtx = fullBuf->txSkipCtx[tuIdx][PLANE_LUMA];
        tuU->txSkipCtx = fullBuf->txSkipCtx[tuIdx][PLANE_CHROMA_U];
        tuV->txSkipCtx = fullBuf->txSkipCtx[tuIdx][PLANE_CHROMA_V];

        tuY->dcSignCtx = fullBuf->dcSignCtx[tuIdx][PLANE_LUMA];
        tuU->dcSignCtx = fullBuf->dcSignCtx[tuIdx][PLANE_CHROMA_U];
        tuV->dcSignCtx = fullBuf->dcSignCtx[tuIdx][PLANE_CHROMA_V];

        tuY->coeffStride = fullBuf->coefStride[PLANE_LUMA];
        tuU->coeffStride = fullBuf->coefStride[PLANE_CHROMA];
        tuV->coeffStride = fullBuf->coefStride[PLANE_CHROMA];

        offsetX     = tuY->offsetX + mb->offX[PLANE_LUMA];
        offsetY     = tuY->offsetY + mb->offY[PLANE_LUMA];
        tuY->qCoeff = fullBuf->qCoefBuf[PLANE_LUMA] + offsetX + offsetY*tuY->coeffStride;

        offsetX     = tuU->offsetX + mb->offX[PLANE_CHROMA];
        offsetY     = tuU->offsetY + mb->offY[PLANE_CHROMA];
        tuU->qCoeff = fullBuf->qCoefBuf[PLANE_CHROMA_U] + offsetX + offsetY*tuU->coeffStride;

        offsetX     = tuV->offsetX + mb->offX[PLANE_CHROMA];
        offsetY     = tuV->offsetY + mb->offY[PLANE_CHROMA];
        tuV->qCoeff = fullBuf->qCoefBuf[PLANE_CHROMA_V] + offsetX + offsetY*tuV->coeffStride;

    }

    mb->skipCoeff   = (UINT8)fullBuf->skipCoeff;
    mb->intraUvMode = (UINT8)fastBuf->intraUvMode;

    mb->intraAngleDelta[PLANE_LUMA]   = fastBuf->angleDelta[PLANE_LUMA];
    mb->intraAngleDelta[PLANE_CHROMA] = fastBuf->angleDelta[PLANE_CHROMA];

    if (fastBuf->earlyStop)
    {
        mb->splitType = XIN_PARTITION_NONE;
    }

}

void Xin265pSbInit (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb)
{
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_sb_struct   *lftSb;
    xin_sb_struct   *topSb;
    xin_sb_struct   *topLftSb;
    xin_sb_struct   *topRgtSb;

    seqSet       = secSet->seqSet;
    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    secSet->sb   = sb;

    lftSb    = (sb->availField & XIN_LFT_SB_AVAIL) ? (sb - 1) : NULL;
    topSb    = (sb->availField & XIN_TOP_SB_AVAIL) ? (sb - seqSet->frameWidthInSb) : NULL;
    topLftSb = ((lftSb != NULL) & (topSb != NULL)) ? (sb - seqSet->frameWidthInSb - 1) : NULL;
    topRgtSb = ((sb->availField & XIN_RGT_SB_AVAIL) && (topSb != NULL)) ? (sb - seqSet->frameWidthInSb + 1) : NULL;

    secSet->reconSb[PLANE_LUMA]     = pictureWrite->refBuf[PLANE_LUMA] + sb->sbPelX + sb->sbPelY*secSet->reconStride[0];
    secSet->reconSb[PLANE_CHROMA_U] = pictureWrite->refBuf[PLANE_CHROMA_U] + ((sb->sbPelX + sb->sbPelY*secSet->reconStride[1]) >> 1);
    secSet->reconSb[PLANE_CHROMA_V] = pictureWrite->refBuf[PLANE_CHROMA_V] + ((sb->sbPelX + sb->sbPelY*secSet->reconStride[1]) >> 1);

    secSet->lftSb    = lftSb;
    secSet->topSb    = topSb;
    secSet->topLftSb = topLftSb;
    secSet->topRgtSb = topRgtSb;

    secSet->sbTopCtx[PLANE_LUMA]     = picSet->topCtx[PLANE_LUMA]     + (sb->sbPelX >> XIN_LOG_MI_SIZE);
    secSet->sbTopCtx[PLANE_CHROMA_U] = picSet->topCtx[PLANE_CHROMA_U] + ((sb->sbPelX / 2) >> XIN_LOG_MI_SIZE);
    secSet->sbTopCtx[PLANE_CHROMA_V] = picSet->topCtx[PLANE_CHROMA_V] + ((sb->sbPelX / 2) >> XIN_LOG_MI_SIZE);

    secSet->sadLambda[PLANE_LUMA]   = sadLambda[secSet->qp];
    secSet->sadLambda[PLANE_CHROMA] = sadLambda[secSet->uvQp];

    secSet->sseLambda[PLANE_LUMA]   = sseLambda[secSet->qp];
    secSet->sseLambda[PLANE_CHROMA] = sseLambda[secSet->uvQp];

    Xin265pInitModeRate (
        picSet,
        &secSet->cabacSet->cdfProb,
        &secSet->cabacEst);

    Xin265pInitCoeffRate (
        &secSet->cabacSet->cdfProb,
        &secSet->cabacEst);

}

static void ConstructPictureWrite (
    xin_pic_struct  *picSet,
    xin_ref_picture **dblFrame)
{
    xin_ref_picture *picture;
    xin_seq_struct  *seqSet;
    UINT32          picIdx;

    seqSet    = picSet->seqSet;
    picture   = seqSet->allocPictures;
    *dblFrame = NULL;

    for (picIdx = 0; picIdx < seqSet->allocPicNum; picIdx++)
    {
        if (picture[picIdx].isFree)
        {
            *dblFrame = picture + picIdx;

            break;
        }
    }

}

static void FindRefFrame (
    xin_ref_picture *refFrame,
    xin_ref_picture **refFrameMap,
    UINT8           *refIdx)
{
    UINT32 frameIdx;

    *refIdx = XIN_REF_FRAME_NUM;

    for (frameIdx = XIN_LAST_FRAME; frameIdx < XIN_REF_FRAME_NUM; frameIdx++)
    {
        if ((refFrame != NULL) && (refFrameMap[frameIdx] != NULL))
        {
            if (refFrame->framePoc == refFrameMap[frameIdx]->framePoc)
            {
                *refIdx = (UINT8)frameIdx;

                break;
            }
        }
    }

}

static void ConstructPictureRead (
    xin_pic_struct  *picSet,
    xin_ref_picture *pictureWrite)
{
    xin_rps_struct  *rps;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureRead;
    SINT32          refIdx;
    UINT32          dpbIdx;
    UINT32          readIdx;
    SINT32          targetPoc;
    SINT32          anchorPoc;
    UINT32          refPicNum;
    BOOL            foundIFrame;

    seqSet = picSet->seqSet;
    rps    = pictureWrite->rps;

    memset (picSet->pictureRead[XIN_LIST_0], 0, sizeof(xin_ref_picture *)*XIN_MAX_REF_FRAMES);
    memset (picSet->pictureRead[XIN_LIST_1], 0, sizeof(xin_ref_picture *)*XIN_MAX_REF_FRAMES);

    readIdx     = 0;
    pictureRead = NULL;
    anchorPoc   = pictureWrite->framePoc;
    refPicNum   = 0;
    foundIFrame = FALSE;

    for (refIdx = 0; refIdx < rps->numOfNegPics; refIdx++)
    {
        targetPoc   = anchorPoc - rps->deltaNegPos[refIdx];
        pictureRead = NULL;

        if ((targetPoc >= 0) && (foundIFrame != TRUE))
        {
            for (dpbIdx = 0; dpbIdx < seqSet->dpbSize; dpbIdx++)
            {
                if (targetPoc == seqSet->dpbQueue[dpbIdx]->framePoc)
                {
                    pictureRead = seqSet->dpbQueue[dpbIdx];

                    break;
                }
            }

            if (pictureRead == NULL)
            {
                _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find refernece picture.\n");
            }
            else
            {
                if (rps->usedByNegPicFlag[refIdx])
                {
                    picSet->pictureRead[XIN_LIST_0][readIdx] = pictureRead;
                    readIdx++;
                }

                anchorPoc = pictureRead->framePoc;
                refPicNum++;
            }

            pictureWrite->refFramePoc[XIN_LIST_0][refIdx] = pictureRead->framePoc;
            foundIFrame = pictureRead->isIntraFrame;

        }

    }

    pictureWrite->numOfRefs[XIN_LIST_0] = readIdx;
    pictureWrite->numOfNegPics          = rps->numOfNegPics;
    pictureWrite->refPicNum[XIN_LIST_0] = refPicNum;

    readIdx     = 0;
    pictureRead = NULL;
    anchorPoc   = pictureWrite->framePoc;
    refPicNum   = 0;

    for (refIdx = 0; refIdx < rps->numOfPosPics; refIdx++)
    {
        targetPoc   = anchorPoc + rps->deltaPosPos[refIdx];
        pictureRead = NULL;

        if ((targetPoc >= 0) && (targetPoc + (SINT32)seqSet->predGopSize >= seqSet->craFramePoc))
        {
            for (dpbIdx = 0; dpbIdx < seqSet->dpbSize; dpbIdx++)
            {
                if (targetPoc == seqSet->dpbQueue[dpbIdx]->framePoc)
                {
                    pictureRead = seqSet->dpbQueue[dpbIdx];

                    break;
                }
            }

            if (pictureRead == NULL)
            {
                _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find refernece picture.\n");
            }
            else
            {
                if (rps->usedByPosPicFlag[refIdx])
                {
                    picSet->pictureRead[XIN_LIST_1][readIdx] = pictureRead;
                    readIdx++;
                }

                anchorPoc = pictureRead->framePoc;
                refPicNum++;
            }

            pictureWrite->refFramePoc[XIN_LIST_1][refIdx] = pictureRead->framePoc;

        }

    }

    pictureWrite->numOfRefs[XIN_LIST_1] = readIdx;
    pictureWrite->numOfPosPics          = rps->numOfPosPics;
    pictureWrite->refPicNum[XIN_LIST_1] = refPicNum;

    memcpy (
        picSet->pictureRef,
        picSet->pictureRead[XIN_LIST_0],
        sizeof(xin_ref_picture *)*pictureWrite->numOfRefs[XIN_LIST_0]);

    memcpy (
        picSet->pictureRef + pictureWrite->numOfRefs[XIN_LIST_0],
        picSet->pictureRead[XIN_LIST_1],
        sizeof(xin_ref_picture *)*pictureWrite->numOfRefs[XIN_LIST_1]);

    picSet->validRefFrame = pictureWrite->numOfRefs[XIN_LIST_0] + pictureWrite->numOfRefs[XIN_LIST_1];

    if ((!pictureWrite->predIdxInGop) && (pictureWrite->frameType == XIN_INTER_FRAME))
    {
        memcpy (
            picSet->pictureRead[XIN_LIST_1],
            picSet->pictureRead[XIN_LIST_0],
            sizeof(xin_ref_picture *)*XIN_MAX_REF_FRAMES);

        memcpy (
            pictureWrite->refFramePoc[XIN_LIST_1],
            pictureWrite->refFramePoc[XIN_LIST_0],
            sizeof(UINT32)*XIN_MAX_REF_FRAMES);

        pictureWrite->numOfRefs[XIN_LIST_1] = pictureWrite->numOfRefs[XIN_LIST_0];
    }

    picSet->refFrameMap[XIN_INTRA_FRAME] = pictureWrite;

    for (refIdx = 0; refIdx < pictureWrite->numOfRefs[XIN_LIST_0]; refIdx++)
    {
        picSet->refFrameMap[XIN_LAST_FRAME + refIdx] = picSet->pictureRead[XIN_LIST_0][refIdx];
    }

    for (refIdx = 0; refIdx < pictureWrite->numOfRefs[XIN_LIST_1]; refIdx++)
    {
        picSet->refFrameMap[XIN_BWDREF_FRAME + refIdx] = picSet->pictureRead[XIN_LIST_1][refIdx];
    }

    for (refIdx = XIN_LAST_FRAME; refIdx < XIN_REF_FRAME_NUM; refIdx++)
    {
        FindRefFrame (
            picSet->refFrameMap[refIdx],
            seqSet->refFrameMap,
            picSet->refFrameIdx + refIdx);
    }

}

static void UpdateSharpness (
    xin_lf_info *lfInfo,
    SINT32      sharpLvl)
{
    SINT32  lvl;
    SINT32  insideLimit;

    // For each possible value for the loop filter fill out limits
    for (lvl = 0; lvl <= XIN_MAX_LOOP_FILTER; lvl++)
    {
        // Set loop filter parameters that control sharpness.
        insideLimit = lvl >> ((sharpLvl > 0) + (sharpLvl > 4));

        if (sharpLvl > 0)
        {
            if (insideLimit > (9 - sharpLvl))
            {
                insideLimit = (9 - sharpLvl);
            }
        }

        if (insideLimit < 1)
        {
            insideLimit = 1;
        }

        memset (lfInfo->lfThr[lvl].lim,   insideLimit,                    XIN_SIMD_WIDTH);
        memset (lfInfo->lfThr[lvl].mblim, (2 * (lvl + 2) + insideLimit), XIN_SIMD_WIDTH);

    }

}

void Xin265pDeblockFrameInit (
    xin_pic_struct *picSet)
{
    xin_ref_picture *pictureWrite;
    xin_lf_info     *lfInfo;
    SINT32          fltLvlVer[PLANE_NUM];
    SINT32          fltLvlHor[PLANE_NUM];
    SINT32          planeIdx;
    SINT32          dirIdx;
    SINT32          level;

    lfInfo       = &picSet->lfInfo;
    pictureWrite = picSet->pictureWrite;

    UpdateSharpness (
        lfInfo,
        pictureWrite->sharpLvl);

    fltLvlVer[0] = pictureWrite->fltLvl[0];
    fltLvlVer[1] = pictureWrite->fltLvlU;
    fltLvlVer[2] = pictureWrite->fltLvlV;

    fltLvlHor[0] = pictureWrite->fltLvl[1];
    fltLvlHor[1] = pictureWrite->fltLvlU;
    fltLvlHor[2] = pictureWrite->fltLvlV;

    for (planeIdx = 0; planeIdx < 3; planeIdx++)
    {
        if ((planeIdx == 0) && (!fltLvlVer[0]) && (!fltLvlHor[0]))
        {
            break;
        }
        else if ((planeIdx == 1) && (!fltLvlVer[1]))
        {
            continue;
        }
        else if ((planeIdx == 2) && (!fltLvlVer[2]))
        {
            continue;
        }

        for (dirIdx = 0; dirIdx < 2; ++dirIdx)
        {
            level = (dirIdx == 0) ? fltLvlVer[planeIdx] : fltLvlHor[planeIdx];

            lfInfo->lvl[planeIdx][dirIdx] = (UINT8)level;
        }

    }

}

static void CalcFramePsnr (
    xin_ref_picture *pictureWrite,
    double          *psnrYuv)
{

    Xin26xCalcPsnr (
        pictureWrite->inputBuf[PLANE_LUMA],
        pictureWrite->inputStride[PLANE_LUMA],
        pictureWrite->refBuf[PLANE_LUMA],
        pictureWrite->refStride[PLANE_LUMA],
        pictureWrite->inputWidth,
        pictureWrite->inputHeight,
        psnrYuv);

    Xin26xCalcPsnr (
        pictureWrite->inputBuf[PLANE_CHROMA_U],
        pictureWrite->inputStride[PLANE_CHROMA],
        pictureWrite->refBuf[PLANE_CHROMA_U],
        pictureWrite->refStride[PLANE_CHROMA],
        pictureWrite->inputWidth/2,
        pictureWrite->inputHeight/2,
        psnrYuv+1);

    Xin26xCalcPsnr (
        pictureWrite->inputBuf[PLANE_CHROMA_V],
        pictureWrite->inputStride[PLANE_CHROMA],
        pictureWrite->refBuf[PLANE_CHROMA_V],
        pictureWrite->refStride[PLANE_CHROMA],
        pictureWrite->inputWidth/2,
        pictureWrite->inputHeight/2,
        psnrYuv+2);

}

void Xin265pFrameInit (
    xin_pic_struct *picSet,
    xin_input_picture *inputPicture)
{
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_rps_struct  *rps;
    UINT32          predGopIdx;
    xin_lb_struct   *outputBuf;

    seqSet     = picSet->seqSet;
    outputBuf  = seqSet->outputBuf;
    predGopIdx = inputPicture->predGopIdx;
    rps        = &inputPicture->rps;

    ConstructPictureWrite (
        picSet,
        &pictureWrite);

    if (pictureWrite == NULL)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "There is no encoding buffer available.\n");

        return;
    }

    memcpy (pictureWrite->rps, rps, sizeof(xin_rps_struct));

    pictureWrite->isAvail           = FALSE;
    pictureWrite->framePoc          = inputPicture->inputNumber;
    pictureWrite->gopIdx            = pictureWrite->framePoc / seqSet->predGopSize;
    pictureWrite->isFree            = FALSE;
    pictureWrite->isReferenced      = rps->isRefFrame;
    pictureWrite->predIdxInGop      = predGopIdx;
    pictureWrite->predGopSize       = inputPicture->predGopSize;
    pictureWrite->temporalId        = rps->temporalId;
    pictureWrite->frameType         = inputPicture->frameType;
    pictureWrite->isIntraFrame      = (pictureWrite->frameType == XIN_IDR_FRAME) || (pictureWrite->frameType == XIN_I_FRAME);
    pictureWrite->refreshFrameFlags = 0xFF;
    pictureWrite->showFrame         = TRUE;

    pictureWrite->sharpLvl          = 0;
    pictureWrite->fltLvl[0]         = 0;
    pictureWrite->fltLvl[1]         = 0;
    pictureWrite->fltLvlU           = 0;
    pictureWrite->fltLvlV           = 0;

    pictureWrite->inputBuf[PLANE_LUMA]     = inputPicture->inputBuf[PLANE_LUMA];
    pictureWrite->inputBuf[PLANE_CHROMA_U] = inputPicture->inputBuf[PLANE_CHROMA_U];
    pictureWrite->inputBuf[PLANE_CHROMA_V] = inputPicture->inputBuf[PLANE_CHROMA_V];

    pictureWrite->inputStride[PLANE_LUMA]   = inputPicture->inputStride[PLANE_LUMA];
    pictureWrite->inputStride[PLANE_CHROMA] = inputPicture->inputStride[PLANE_CHROMA];

    if (pictureWrite->frameType <= XIN_I_FRAME)
    {
        ConstructPictureRead (
            picSet,
            pictureWrite);
    }

    picSet->codingFrame  = TRUE;
    picSet->inputPicture = inputPicture;
    picSet->pictureWrite = pictureWrite;
    picSet->baseQIdx     = 99;
    outputBuf->index     = 0;

    // Reset block type to an invalid value
    memset (
        pictureWrite->miBuffer,
        XIN_INVALID_MODE,
        pictureWrite->miSize*sizeof(xin_mi_struct));

    memset (
        pictureWrite->lumaFlt[0],
        0,
        pictureWrite->widthInPel4*pictureWrite->heightInPel4*sizeof(UINT8));

    memset (
        pictureWrite->lumaFlt[1],
        0,
        pictureWrite->widthInPel4*pictureWrite->heightInPel4*sizeof(UINT8));

    memset (
        pictureWrite->chromaFlt[0],
        0,
        pictureWrite->widthInPel8*pictureWrite->heightInPel8*sizeof(UINT8));

    memset (
        pictureWrite->chromaFlt[1],
        0,
        pictureWrite->widthInPel8*pictureWrite->heightInPel8*sizeof(UINT8));

    Xin265pDeblockFrameInit (
        picSet);

}

void Xin265pFramePostInit (
    xin_pic_struct *picSet)
{
    xin_ref_picture  *pictureWrite;
    xin_seq_struct   *seqSet;
    UINT32           frameIdx;
    xin_ref_picture **dpbQueue;
    xin_lb_struct    *outputBuf;
    xin_bs_struct    *globeBs;
    xin_bs_struct    *localBs;
    xin_cabac_struct *cabac;
    UINT32           tileIdx;
    UINT8            obuHeader;
    UINT32           headerSize;
    UINT32           dataSize;
    UINT8            ulebValue[4];
    UINT8            ulebLength;
    UINT32           rdIdx;
    UINT32           wrIdx;

    seqSet       = picSet->seqSet;
    pictureWrite = picSet->pictureWrite;
    outputBuf    = seqSet->outputBuf;
    globeBs      = picSet->bitstream;
    dpbQueue     = seqSet->dpbQueue;

    if (pictureWrite->refreshFrameFlags)
    {
        for (frameIdx = 0; frameIdx < XIN_REF_FRAME_NUM; frameIdx++)
        {
            if (pictureWrite->refreshFrameFlags & (1 << frameIdx))
            {
                seqSet->refFrameMap[frameIdx] = pictureWrite;
            }
        }
    }

    // Reserve bytes for ivf header
    if (seqSet->config.outputFormat)
    {
        outputBuf->index += pictureWrite->framePoc == 0 ? XIN_IVF_FILE_HEADER_SIZE + XIN_IVF_FRAME_HEADER_SIZE: XIN_IVF_FRAME_HEADER_SIZE;
    }

    Xin265pGenObuHeaderAndUleb (
        &obuHeader,
        ulebValue,
        &ulebLength,
        XIN_OBU_TEMPORAL_DELIMITER,
        0);

    Xin265pOutputBitToLinearBuffer (
        NULL,
        obuHeader,
        ulebValue,
        ulebLength,
        outputBuf);

    // Encode parameter sets, if this frame is IDR.
    if (pictureWrite->frameType == XIN_IDR_FRAME)
    {
        Xin265pInitBitstream (
            globeBs);

        Xin265pWriteSeqHeader (
            globeBs,
            seqSet);

        Xin265pBitstreamSize (
            globeBs,
            &headerSize);

        Xin265pGenObuHeaderAndUleb (
            &obuHeader,
            ulebValue,
            &ulebLength,
            XIN_OBU_SEQUENCE_HEADER,
            headerSize);

        Xin265pOutputBitToLinearBuffer (
            globeBs,
            obuHeader,
            ulebValue,
            ulebLength,
            outputBuf);

    }

    // Packet bitstream and send it out
    for (tileIdx = 0; tileIdx < seqSet->tileNum; tileIdx++)
    {
        cabac   = &picSet->cabacSet[tileIdx]->cabac;
        localBs = &cabac->bitstream;

        Xin265pInitBitstream (
            globeBs);

        // Write slice header
        Xin265pWriteSliceHeader (
            globeBs,
            picSet);

        Xin265pBitstreamSize (
            globeBs,
            &headerSize);

        Xin265pInitBitstream (
            localBs);

        Xin265pWriteDone (
            cabac,
            localBs);

        Xin265pBitstreamSize (
            localBs,
            &dataSize);

        Xin265pGenObuHeaderAndUleb (
            &obuHeader,
            ulebValue,
            &ulebLength,
            XIN_OBU_FRAME,
            headerSize+dataSize);

        Xin265pOutputBitToLinearBuffer (
            globeBs,
            obuHeader,
            ulebValue,
            ulebLength,
            outputBuf);

        Xin265pOutputBitToLinearBuffer (
            localBs,
            obuHeader,
            NULL,
            ulebLength,
            outputBuf);

    }

    // Write ivf header
    if (seqSet->config.outputFormat)
    {
        if (pictureWrite->framePoc == 0)
        {
            Xin265pGenIvfFileHeader (
                seqSet,
                (UINT8 *)outputBuf->base);

            Xin265pGenIvfFrameHeader (
                seqSet,
                ((UINT8 *)outputBuf->base) + XIN_IVF_FILE_HEADER_SIZE,
                outputBuf->index - (XIN_IVF_FILE_HEADER_SIZE + XIN_IVF_FRAME_HEADER_SIZE),
                pictureWrite->framePoc);
        }
        else
        {
            Xin265pGenIvfFrameHeader (
                seqSet,
                (UINT8 *)outputBuf->base,
                outputBuf->index - XIN_IVF_FRAME_HEADER_SIZE,
                pictureWrite->framePoc);
        }

    }

    // If frame is a referenced frame for future frame,
    // then we put it into DPB buffer, Otherwise, we
    // recyle this frame buffer.
    if (pictureWrite->isReferenced)
    {
        // Remove all previous gop pictures, whose gop index is not zero
        if ((seqSet->config.bFrameNum > 0) && (pictureWrite->predIdxInGop == 0))
        {
            wrIdx = 0;

            for (rdIdx = 0; rdIdx < seqSet->dpbSize; rdIdx++)
            {
                if (dpbQueue[rdIdx]->predIdxInGop == 0)
                {
                    dpbQueue[wrIdx] = dpbQueue[rdIdx];

                    wrIdx++;
                }
                else
                {
                    dpbQueue[rdIdx]->isFree = TRUE;
                }
            }

            seqSet->dpbSize = wrIdx;

        }

        memmove(dpbQueue + 1, dpbQueue, seqSet->dpbSize*sizeof(xin_ref_picture *));

        dpbQueue[0] = pictureWrite;
        seqSet->dpbSize++;

        // If DPB size exceed allocated picture size,
        // then we shrink DPB buffer size.
        if ((seqSet->dpbSize + seqSet->config.frameThreadNum - 1) >= seqSet->allocPicNum)
        {
            dpbQueue[seqSet->dpbSize - 1]->isFree = TRUE;
            seqSet->dpbSize--;
        }

    }
    else
    {
        pictureWrite->isFree = TRUE;
    }

    if (seqSet->config.calcPsnr)
    {
        CalcFramePsnr (
            pictureWrite,
            pictureWrite->psnrYuv);

        seqSet->psnrYuv[PLANE_LUMA]     += pictureWrite->psnrYuv[PLANE_LUMA];
        seqSet->psnrYuv[PLANE_CHROMA_U] += pictureWrite->psnrYuv[PLANE_CHROMA_U];
        seqSet->psnrYuv[PLANE_CHROMA_V] += pictureWrite->psnrYuv[PLANE_CHROMA_V];
    }

}

void Xin265pMbUpdateMi (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    SINT32          mbPelX;
    SINT32          mbPelY;
    intptr_t        miIdx;
    intptr_t        miStride;
    SINT32          widthInMi;
    SINT32          heightInMi;
    SINT32          rowIdx;
    SINT32          colIdx;
    xin_pic_struct  *picSet;
    xin_mi_struct   *miBuf;
    xin_mi_struct   *curMi;
    xin_tu_struct   *tuY;
    xin_tu_struct   *tuU;
    xin_tu_struct   *tuV;
    SINT32          tuIdx;
    SINT32          rgtEdgeMi;
    SINT32          botEdgeMi;
    xin_ref_picture *pictureWrite;

    picSet = secSet->picSet;
    mbPelX = mb->mbPelX[PLANE_LUMA];
    mbPelY = mb->mbPelY[PLANE_LUMA];

    pictureWrite = picSet->pictureWrite;
    miBuf        = pictureWrite->miBuf;
    miStride     = pictureWrite->miStride;

    PEL_XY_TO_BLOCK_INDEX (mbPelX, mbPelY, miIdx, miStride, XIN_LOG_MI_SIZE);

    botEdgeMi  = mb->botEdgeMi;
    rgtEdgeMi  = mb->rgtEdgeMi;
    widthInMi  = mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    heightInMi = mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    widthInMi  = widthInMi - rgtEdgeMi;
    heightInMi = heightInMi - botEdgeMi;
    curMi      = miBuf + miIdx;

    for (rowIdx = 0; rowIdx < heightInMi; rowIdx++)
    {
        for (colIdx = 0; colIdx < widthInMi; colIdx++)
        {
            curMi[colIdx].predMode    = (UINT8)mb->predMode;
            curMi[colIdx].partType    = (UINT8)mb->splitType;
            curMi[colIdx].blockSize   = (UINT8)mb->blockSize;

            curMi[colIdx].mv[0].s32x1 = mb->mv[0].s32x1;
            curMi[colIdx].refFrame[0] = (UINT8)mb->refFrame[0];
            curMi[colIdx].mv[1].s32x1 = mb->mv[1].s32x1;
            curMi[colIdx].refFrame[1] = (UINT8)mb->refFrame[1];
            curMi[colIdx].coeffSkip   = (UINT8)mb->skipCoeff;
            curMi[colIdx].txSize      = mb->txSize;
        }

        curMi += miStride;
    }

    for (tuIdx = 0; tuIdx < mb->tuNum; tuIdx++)
    {
        tuY = mb->tu[PLANE_LUMA] + tuIdx;
        tuU = mb->tu[PLANE_CHROMA_U] + tuIdx;
        tuV = mb->tu[PLANE_CHROMA_V] + tuIdx;

        memset (mb->mbLftCtx[PLANE_LUMA] + (tuY->offsetY >> XIN_LOG_MI_SIZE),     tuY->culLevel, sizeof(UINT8)*tuY->heightInMi);
        memset (mb->mbLftCtx[PLANE_CHROMA_U] + (tuV->offsetY >> XIN_LOG_MI_SIZE), tuU->culLevel, sizeof(UINT8)*tuU->heightInMi);
        memset (mb->mbLftCtx[PLANE_CHROMA_V] + (tuV->offsetY >> XIN_LOG_MI_SIZE), tuV->culLevel, sizeof(UINT8)*tuV->heightInMi);

        memset (mb->mbTopCtx[PLANE_LUMA] + (tuY->offsetX >> XIN_LOG_MI_SIZE),     tuY->culLevel, sizeof(UINT8)*tuY->widthInMi);
        memset (mb->mbTopCtx[PLANE_CHROMA_U] + (tuU->offsetX >> XIN_LOG_MI_SIZE), tuU->culLevel, sizeof(UINT8)*tuU->widthInMi);
        memset (mb->mbTopCtx[PLANE_CHROMA_V] + (tuV->offsetX >> XIN_LOG_MI_SIZE), tuV->culLevel, sizeof(UINT8)*tuV->widthInMi);
    }

    if (botEdgeMi > 0)
    {
        memset (mb->mbLftCtx[PLANE_LUMA] + heightInMi,       0, sizeof(UINT8)*botEdgeMi);
        memset (mb->mbLftCtx[PLANE_CHROMA_U] + heightInMi/2, 0, sizeof(UINT8)*botEdgeMi/2);
        memset (mb->mbLftCtx[PLANE_CHROMA_V] + heightInMi/2, 0, sizeof(UINT8)*botEdgeMi/2);

    }

    if (rgtEdgeMi > 0)
    {
        memset (mb->mbTopCtx[PLANE_LUMA] + widthInMi,       0, sizeof(UINT8)*rgtEdgeMi);
        memset (mb->mbTopCtx[PLANE_CHROMA_U] + widthInMi/2, 0, sizeof(UINT8)*rgtEdgeMi/2);
        memset (mb->mbTopCtx[PLANE_CHROMA_V] + widthInMi/2, 0, sizeof(UINT8)*rgtEdgeMi/2);
    }

}

void Xin265pSectionInit (
    xin_sec_struct *secSet)
{
    xin_cabac_context *cabacSet;
    xin_seq_struct    *seqSet;
    xin_pic_struct    *picSet;
    UINT32            sectionIdx;

    secSet->qp   = 99;
    secSet->uvQp = 99;
    seqSet       = secSet->seqSet;
    sectionIdx   = secSet->sectionIdx;
    picSet       = secSet->picSet;
    cabacSet     = picSet->cabacSet[sectionIdx];

    Xin265pInitModeProb (
        cabacSet);

    Xin265pInitCoeffProb (
        cabacSet,
        secSet->qp);

    Xin265pCabacContextInit (
        cabacSet,
        seqSet->cabacContext,
        secSet->qp,
        FALSE);

    secSet->tileDim       = seqSet->tileDim + sectionIdx;
    secSet->cabacSet      = cabacSet;
    secSet->codingDeltaQp = FALSE;

}

