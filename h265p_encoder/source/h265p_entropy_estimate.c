/***************************************************************************//**
*
* @file          h265p_entropy_estimate.c
* @brief         av1 syntax and coefficient rate estimation
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
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
#include "h265p_intra_prediction.h"
#include "h265p_common_data.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_scan_order.h"

static const UINT16 probCost[128] =
{
    512, 506, 501, 495, 489, 484, 478, 473, 467, 462, 456, 451, 446, 441, 435,
    430, 425, 420, 415, 410, 405, 400, 395, 390, 385, 380, 375, 371, 366, 361,
    356, 352, 347, 343, 338, 333, 329, 324, 320, 316, 311, 307, 302, 298, 294,
    289, 285, 281, 277, 273, 268, 264, 260, 256, 252, 248, 244, 240, 236, 232,
    228, 224, 220, 216, 212, 209, 205, 201, 197, 194, 190, 186, 182, 179, 175,
    171, 168, 164, 161, 157, 153, 150, 146, 143, 139, 136, 132, 129, 125, 122,
    119, 115, 112, 109, 105, 102, 99, 95, 92, 89, 86, 82, 79, 76, 73,
    70, 66, 63, 60, 57, 54, 51, 48, 45, 42, 38, 35, 32, 29, 26,
    23, 20, 18, 15, 12, 9, 6, 3,
};

static const SINT32 av1UseInterExtTxForTxSize[XIN_EXT_TX_SETS_INTER][XIN_EXT_TX_SIZE_NUM] =
{
    { 1, 1, 1, 1 },  // unused
    { 1, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 1, 1, 1 },
};

static const SINT32 av1UseIntraExtTxForTxSize[XIN_EXT_TX_SETS_INTRA][XIN_EXT_TX_SIZE_NUM] =
{
    { 1, 1, 1, 1 },  // unused
    { 1, 1, 0, 0 },
    { 0, 0, 1, 0 },
};

static const SINT32 av1ExtTxSetIdx2Type[2][XIN_MAX(XIN_EXT_TX_SETS_INTRA, XIN_EXT_TX_SETS_INTER)] =
{
    {
        // Intra
        XIN_EXT_TX_SET_DCTONLY,
        XIN_EXT_TX_SET_DTT4_IDTX_1DDCT,
        XIN_EXT_TX_SET_DTT4_IDTX,
    },
    {
        // Inter
        XIN_EXT_TX_SET_DCTONLY,
        XIN_EXT_TX_SET_ALL16,
        XIN_EXT_TX_SET_DTT9_IDTX_1DDCT,
        XIN_EXT_TX_SET_DCT_IDTX,
    },
};

static const SINT32 av1ExtTxInv[XIN_EXT_TX_SET_TYPE_NUM][XIN_TX_2D_NUM] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 9, 0, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 9, 0, 10, 11, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 9, 10, 11, 0, 1, 2, 4, 5, 3, 6, 7, 8, 0, 0, 0, 0 },
    { 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 4, 5, 3, 6, 7, 8 },
};

static const SINT8 dcSignMap[3] =
{
    0, -1, 1
};

static const SINT8 dcSignContext[4 * XIN_MAX_TX_SIZE_IN_UNIT + 1] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2
};

static const UINT8 txSkipContext[5][5] =
{
    { 1, 2, 2, 2, 3 },
    { 2, 4, 4, 4, 5 },
    { 2, 4, 4, 4, 5 },
    { 2, 4, 4, 4, 5 },
    { 3, 5, 5, 5, 6 }
};

extern const SINT32 av1EobOffsetBits[12];

static UINT8 XinGetProb (
    UINT32 num,
    UINT32 den)
{
    SINT32 prob;
    SINT32 clippedProb;

    prob = (SINT32)(((UINT64) num * 256 + (den >> 1)) / den);

    // (p > 255) ? 255 : (p < 1) ? 1 : p;
    clippedProb = prob | ((255 - prob) >> 23) | (prob == 0);

    return (UINT8)clippedProb;

}

static inline SINT32 XinGetMsb (
    UINT32 n)
{
    UINT32 firstSetBit;

    BIT_SCAN_REVERSE_32 (n, firstSetBit);

    return firstSetBit;
}

static SINT32 XinRateSymbol (
    UINT16 prob15)
{
    SINT32 shift;
    SINT32 prob;

    shift = XIN_CDF_PROB_BITS - 1 - XinGetMsb(prob15);
    prob  = XinGetProb (prob15 << shift, XIN_CDF_PROB_TOP);

    return probCost[prob - 128] + XIN_RATE_LITERAL(shift);
}

void Xin265pRateFromCdf (
    SINT32       *rate,
    const UINT16 *cdf,
    const int    *invMap)
{
    SINT16  idx;
    UINT16  prevCdf;
    UINT16  prob15;

    prevCdf = 0;

    for (idx = 0; ; ++idx)
    {
        prob15  = XIN_ICDF (cdf[idx]) - prevCdf;
        prob15  = (prob15 < XIN_MIN_PROB) ? XIN_MIN_PROB : prob15;
        prevCdf = XIN_ICDF (cdf[idx]);

        if (invMap)
        {
            rate[invMap[idx]] = XinRateSymbol(prob15);
        }
        else
        {
            rate[idx] = XinRateSymbol(prob15);
        }

        // Stop once we reach the end of the CDF
        if (cdf[idx] == XIN_ICDF(XIN_CDF_PROB_TOP))
        {
            break;
        }

    }

}

void Xin265pInitCoeffRate (
    xin_cdf_prob   *context,
    xin_cabac_est  *cabacEst)
{
    SINT32  eobIdx;
    SINT32  ctxIdx;
    SINT32  planeIdx;
    SINT32  coefIdx0;
    SINT32  coefIdx1;
    SINT32  txIdx;
    UINT16  *pcdf;
    SINT32  prevRate;
    SINT32  brRate[XIN_BR_CDF_SIZE];

    lv_map_coef_rate *pCoefRate;
    lv_map_eob_rate  *pEobRate;

    for (eobIdx = 0; eobIdx < 7; ++eobIdx)
    {
        for (planeIdx = 0; planeIdx < 2; ++planeIdx)
        {
            pEobRate = &cabacEst->eobFracRate[eobIdx][planeIdx];

            for (ctxIdx = 0; ctxIdx < 2; ++ctxIdx)
            {
                switch (eobIdx)
                {
                case 0:
                    pcdf = context->eobFlagCdf16[planeIdx][ctxIdx];
                    break;
                case 1:
                    pcdf = context->eobFlagCdf32[planeIdx][ctxIdx];
                    break;
                case 2:
                    pcdf = context->eobFlagCdf64[planeIdx][ctxIdx];
                    break;
                case 3:
                    pcdf = context->eobFlagCdf128[planeIdx][ctxIdx];
                    break;
                case 4:
                    pcdf = context->eobFlagCdf256[planeIdx][ctxIdx];
                    break;
                case 5:
                    pcdf = context->eobFlagCdf512[planeIdx][ctxIdx];
                    break;
                case 6:
                default:
                    pcdf = context->eobFlagCdf1024[planeIdx][ctxIdx];
                    break;
                }

                Xin265pRateFromCdf (
                    pEobRate->eobRate[ctxIdx],
                    pcdf,
                    NULL);

            }

        }

    }

    for (txIdx = 0; txIdx < XIN_TX_SIZE_NUM; ++txIdx)
    {
        for (planeIdx = 0; planeIdx < 2; ++planeIdx)
        {
            pCoefRate = &cabacEst->coefFacRate[txIdx][planeIdx];

            for (ctxIdx = 0; ctxIdx < XIN_NUM_TXB_SKIP_CTX; ++ctxIdx)
            {
                Xin265pRateFromCdf (
                    pCoefRate->txbSkipRate[ctxIdx],
                    context->txbSkipCdf[txIdx][ctxIdx],
                    NULL);
            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_SIG_COEF_EOB_CTX; ++ctxIdx)
            {
                Xin265pRateFromCdf (
                    pCoefRate->baseEobRate[ctxIdx],
                    context->coefBaseEobCdf[txIdx][planeIdx][ctxIdx],
                    NULL);
            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_SIG_COEF_CTX; ++ctxIdx)
            {
                Xin265pRateFromCdf (
                    pCoefRate->baseRate[ctxIdx],
                    context->coefBaseCdf[txIdx][planeIdx][ctxIdx],
                    NULL);
            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_SIG_COEF_CTX; ++ctxIdx)
            {
                pCoefRate->baseRate[ctxIdx][4] = 0;
                pCoefRate->baseRate[ctxIdx][5] = pCoefRate->baseRate[ctxIdx][1] + (1<<XIN_RATE_FRACTION) - pCoefRate->baseRate[ctxIdx][0];
                pCoefRate->baseRate[ctxIdx][6] = pCoefRate->baseRate[ctxIdx][2] - pCoefRate->baseRate[ctxIdx][1];
                pCoefRate->baseRate[ctxIdx][7] = pCoefRate->baseRate[ctxIdx][3] - pCoefRate->baseRate[ctxIdx][2];
            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_EOB_COEF_CTX; ++ctxIdx)
            {
                Xin265pRateFromCdf (
                    pCoefRate->eobExtraRate[ctxIdx],
                    context->eobExtraCdf[txIdx][planeIdx][ctxIdx],
                    NULL);
            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_DC_SIGN_CTX; ++ctxIdx)
            {
                Xin265pRateFromCdf (
                    pCoefRate->dcSignRate[ctxIdx],
                    context->dcSignCdf[planeIdx][ctxIdx],
                    NULL);
            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_LEVEL_CTX; ++ctxIdx)
            {
                prevRate = 0;

                Xin265pRateFromCdf (
                    brRate,
                    context->coefBrCdf[txIdx][planeIdx][ctxIdx],
                    NULL);

                for (coefIdx0 = 0; coefIdx0 < XIN_COEF_BASE_RANGE; coefIdx0 += XIN_BR_CDF_SIZE - 1)
                {
                    for (coefIdx1 = 0; coefIdx1 < XIN_BR_CDF_SIZE - 1; coefIdx1++)
                    {
                        pCoefRate->lpsRate[ctxIdx][coefIdx0 + coefIdx1] = prevRate + brRate[coefIdx1];
                    }

                    prevRate += brRate[coefIdx1];
                }

                pCoefRate->lpsRate[ctxIdx][coefIdx0] = prevRate;

            }

            for (ctxIdx = 0; ctxIdx < XIN_NUM_LEVEL_CTX; ++ctxIdx)
            {
                pCoefRate->lpsRate[ctxIdx][0 + XIN_COEF_BASE_RANGE + 1] = pCoefRate->lpsRate[ctxIdx][0];

                for (coefIdx0 = 1; coefIdx0 <= XIN_COEF_BASE_RANGE; ++coefIdx0)
                {
                    pCoefRate->lpsRate[ctxIdx][coefIdx0 + XIN_COEF_BASE_RANGE + 1] = pCoefRate->lpsRate[ctxIdx][coefIdx0] - pCoefRate->lpsRate[ctxIdx][coefIdx0 - 1];
                }
            }

        }

    }

}

void Xin265pInitModeRate (
    xin_pic_struct *picSet,
    xin_cdf_prob   *context,
    xin_cabac_est  *cabacEst)
{
    xin_seq_struct  *seqSet;
    SINT32          ctxIdx0, ctxIdx1, ctxIdx2;
    xin_ref_picture *pictureWrite;
    SINT32          signRate[XIN_CFL_JOINT_SIGN_NUM];

    seqSet       = picSet->seqSet;
    pictureWrite = picSet->pictureWrite;

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_PARTITION_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->partitionRate[ctxIdx0],
            context->partitionCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_SKIP_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->skipModeRate[ctxIdx0],
            context->skipModeCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_SKIP_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->skipRate[ctxIdx0],
            context->skipCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_KF_MODE_CTX; ++ctxIdx0)
    {
        for (ctxIdx1 = 0; ctxIdx1 < XIN_NUM_KF_MODE_CTX; ++ctxIdx1)
        {
            Xin265pRateFromCdf (
                cabacEst->yModeRate[ctxIdx0][ctxIdx1],
                context->kfYCdf[ctxIdx0][ctxIdx1],
                NULL);
        }
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_BLOCK_SIZE_GROUP_NUM; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->mbModeRate[ctxIdx0],
            context->yModeCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_CFL_ALLOWED_TYPE_NUM; ++ctxIdx0)
    {
        for (ctxIdx1 = 0; ctxIdx1 < XIN_INTRA_MODE_NUM; ++ctxIdx1)
        {
            Xin265pRateFromCdf (
                cabacEst->intraUvModeRate[ctxIdx0][ctxIdx1],
                context->uvModeCdf[ctxIdx0][ctxIdx1],
                NULL);
        }
    }

    Xin265pRateFromCdf (
        cabacEst->filterIntraModeRate,
        context->filterIntraModeCdf,
        NULL);

    for (ctxIdx0 = 0; ctxIdx0 < XIN_BLOCK_SIZE_NUM; ++ctxIdx0)
    {
        if (Xin265pFilterIntraAllowed(seqSet->config.enableIntraEdgeFilter, ctxIdx0))
        {
            Xin265pRateFromCdf (
                cabacEst->filterIntraRate[ctxIdx0],
                context->filterIntraCdf[ctxIdx0],
                NULL);
        }
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_SWITCHABLE_FILTER_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->switchableInterpRate[ctxIdx0],
            context->switchableInterpCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_PALATTE_BSIZE_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->paletteYSizeRate[ctxIdx0],
            context->paletteYSizeCdf[ctxIdx0],
            NULL);

        Xin265pRateFromCdf (
            cabacEst->paletteUvSizeRate[ctxIdx0],
            context->paletteUvSizeCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_PALETTE_UV_MODE_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->paletteUvModeRate[ctxIdx0],
            context->paletteUvModeCdf[ctxIdx0], NULL);
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_PALETTE_SIZE_NUM; ++ctxIdx0)
    {
        for (ctxIdx1 = 0; ctxIdx1 < XIN_NUM_PALETTE_COLOR_INDEX_CTX; ++ctxIdx1)
        {
            Xin265pRateFromCdf (
                cabacEst->paletteYColorRate[ctxIdx0][ctxIdx1],
                context->paletteYColorIndexCdf[ctxIdx0][ctxIdx1],
                NULL);

            Xin265pRateFromCdf (
                cabacEst->paletteUvColorRate[ctxIdx0][ctxIdx1],
                context->paletteUvColorIndexCdf[ctxIdx0][ctxIdx1],
                NULL);
        }
    }

    Xin265pRateFromCdf (
        signRate,
        context->cflSignCdf,
        NULL);

    for (ctxIdx0 = 0; ctxIdx0 < XIN_CFL_JOINT_SIGN_NUM; ctxIdx0++)
    {
        if (XIN_CFL_SIGN_U(ctxIdx0) == XIN_CFL_SIGN_ZERO)
        {
            memset (cabacEst->cflRate[ctxIdx0][XIN_CFL_PRED_U], 0, XIN_CFL_ALPHABET_SIZE*sizeof(SINT32));
        }
        else
        {
            Xin265pRateFromCdf (
                cabacEst->cflRate[ctxIdx0][XIN_CFL_PRED_U],
                context->cflAlphaCdf[XIN_CFL_CONTEXT_U(ctxIdx0)],
                NULL);
        }

        if (XIN_CFL_SIGN_V(ctxIdx0) == XIN_CFL_SIGN_ZERO)
        {
            memset (cabacEst->cflRate[ctxIdx0][XIN_CFL_PRED_V], 0, XIN_CFL_ALPHABET_SIZE*sizeof(SINT32));
        }
        else
        {
            Xin265pRateFromCdf (
                cabacEst->cflRate[ctxIdx0][XIN_CFL_PRED_V],
                context->cflAlphaCdf[XIN_CFL_CONTEXT_V(ctxIdx0)],
                NULL);
        }

        for (ctxIdx1 = 0; ctxIdx1 < XIN_CFL_ALPHABET_SIZE; ctxIdx1++)
        {
            cabacEst->cflRate[ctxIdx0][XIN_CFL_PRED_U][ctxIdx1] += signRate[ctxIdx0];
        }

    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_MAX_TX_CAT_NUM; ++ctxIdx0)
    {
        for (ctxIdx1 = 0; ctxIdx1 < XIN_NUM_TX_SIZE_CTX; ++ctxIdx1)
        {
            Xin265pRateFromCdf (
                cabacEst->txSizeRate[ctxIdx0][ctxIdx1],
                context->txSizeCdf[ctxIdx0][ctxIdx1],
                NULL);
        }
    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_TXFM_PARTITION_CTX; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->txPartitionRate[ctxIdx0],
            context->txfmPartitionCdf[ctxIdx0],
            NULL);
    }

    for (ctxIdx0 = XIN_TX_4X4; ctxIdx0 < XIN_EXT_TX_SIZE_NUM; ++ctxIdx0)
    {
        for (ctxIdx1 = 1; ctxIdx1 < XIN_EXT_TX_SETS_INTER; ++ctxIdx1)
        {
            if (av1UseInterExtTxForTxSize[ctxIdx1][ctxIdx0])
            {
                Xin265pRateFromCdf (
                    cabacEst->interTxTypeRate[ctxIdx1][ctxIdx0],
                    context->interExtTxCdf[ctxIdx1][ctxIdx0],
                    av1ExtTxInv[av1ExtTxSetIdx2Type[1][ctxIdx1]]);
            }
        }

        for (ctxIdx1 = 1; ctxIdx1 < XIN_EXT_TX_SETS_INTRA; ++ctxIdx1)
        {
            if (av1UseIntraExtTxForTxSize[ctxIdx1][ctxIdx0])
            {
                for (ctxIdx2 = 0; ctxIdx2 < XIN_INTRA_MODE_NUM; ++ctxIdx2)
                {
                    Xin265pRateFromCdf (
                        cabacEst->intraTxTypeRate[ctxIdx1][ctxIdx0][ctxIdx2],
                        context->intraExtTxCdf[ctxIdx1][ctxIdx0][ctxIdx2],
                        av1ExtTxInv[av1ExtTxSetIdx2Type[0][ctxIdx1]]);
                }
            }
        }

    }

    for (ctxIdx0 = 0; ctxIdx0 < XIN_DIRECTIONAL_MODE_NUM; ++ctxIdx0)
    {
        Xin265pRateFromCdf (
            cabacEst->angleDeltaRate[ctxIdx0],
            context->angleDeltaCdf[ctxIdx0],
            NULL);
    }

    Xin265pRateFromCdf (
        cabacEst->switchableRestoreRate,
        context->switchableRestoreCdf,
        NULL);

    Xin265pRateFromCdf (
        cabacEst->wienerRestoreRate,
        context->wienerRestoreCdf,
        NULL);

    Xin265pRateFromCdf (
        cabacEst->sgrprojRestoreRate,
        context->sgrprojRestoreCdf,
        NULL);

    Xin265pRateFromCdf (
        cabacEst->intraBCRate,
        context->intrabcCdf,
        NULL);

    if (pictureWrite->isIntraFrame == FALSE)
    {
        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_COMP_INTER_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->compInterRate[ctxIdx0],
                context->compInterCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_REF_CTX; ++ctxIdx0)
        {
            for (ctxIdx1 = 0; ctxIdx1 < XIN_SINGLE_REF_NUM - 1; ++ctxIdx1)
            {
                Xin265pRateFromCdf (
                    cabacEst->singleRefRate[ctxIdx0][ctxIdx1],
                    context->singleRefCdf[ctxIdx0][ctxIdx1],
                    NULL);
            }
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_COMP_REF_TYPE_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->compRefTypeRate[ctxIdx0],
                context->compRefTypeCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_UNI_COMP_REF_CTX; ++ctxIdx0)
        {
            for (ctxIdx1 = 0; ctxIdx1 < XIN_UNIDIR_COMP_REF_NUM - 1; ++ctxIdx1)
            {
                Xin265pRateFromCdf (
                    cabacEst->uniCompRefRate[ctxIdx0][ctxIdx1],
                    context->uniCompRefCdf[ctxIdx0][ctxIdx1],
                    NULL);
            }
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_REF_CTX; ++ctxIdx0)
        {
            for (ctxIdx1 = 0; ctxIdx1 < XIN_FWD_REF_NUM - 1; ++ctxIdx1)
            {
                Xin265pRateFromCdf (
                    cabacEst->compFwdRefRate[ctxIdx0][ctxIdx1],
                    context->compFwdRefCdf[ctxIdx0][ctxIdx1],
                    NULL);
            }
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_REF_CTX; ++ctxIdx0)
        {
            for (ctxIdx1 = 0; ctxIdx1 < XIN_BWD_REF_NUM - 1; ++ctxIdx1)
            {
                Xin265pRateFromCdf (
                    cabacEst->compBwdRefRate[ctxIdx0][ctxIdx1],
                    context->compBwdrefCdf[ctxIdx0][ctxIdx1],
                    NULL);
            }
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_INTRA_INTER_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->intraInterRate[ctxIdx0],
                context->intraInterCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_NEWMV_MODE_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->newMvModeRate[ctxIdx0],
                context->newMvCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_GLOBALMV_MODE_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->zeroMvModeRate[ctxIdx0],
                context->zeroMvCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_REFMV_MODE_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->refMvModeRate[ctxIdx0],
                context->refMvCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_DRL_MODE_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->drlModeRate0[ctxIdx0],
                context->drlCdf[ctxIdx0],
                NULL);
        }
        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_INTER_MODE_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->interCompoundModeRate[ctxIdx0],
                context->interCompoundModeCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_BLOCK_SIZE_NUM; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->compoundTypeRate[ctxIdx0],
                context->compoundTypeCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_BLOCK_SIZE_GROUP_NUM; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->interIntraRate[ctxIdx0],
                context->interIntraCdf[ctxIdx0],
                NULL);

            Xin265pRateFromCdf (
                cabacEst->interIntraModeRate[ctxIdx0],
                context->interIntraModeCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = XIN_BLOCK_8X8; ctxIdx0 < XIN_BLOCK_SIZE_NUM; ctxIdx0++)
        {
            Xin265pRateFromCdf (
                cabacEst->motionModeRate[ctxIdx0],
                context->motionModeCdf[ctxIdx0],
                NULL);

        }

        for (ctxIdx0 = XIN_BLOCK_8X8; ctxIdx0 < XIN_BLOCK_SIZE_NUM; ctxIdx0++)
        {
            Xin265pRateFromCdf (
                cabacEst->motionModeRate1[ctxIdx0],
                context->obmcCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_COMP_INDEX_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->compIdxRate[ctxIdx0],
                context->compoundIndexCdf[ctxIdx0],
                NULL);
        }

        for (ctxIdx0 = 0; ctxIdx0 < XIN_NUM_COMP_GROUP_IDX_CTX; ++ctxIdx0)
        {
            Xin265pRateFromCdf (
                cabacEst->compGroupIdxRate[ctxIdx0],
                context->compGroupIdxCdf[ctxIdx0],
                NULL);
        }

    }

}

static void Xin265pEstimateIntraFrameYMode (
    xin_cabac_est *cabacEst,
    xin_mb_struct *mb,
    UINT32        intraMode,
    UINT32        *bitNum)
{
    UINT32  topCtx;
    UINT32  lftCtx;

    topCtx  = mb->topMi->predMode;
    topCtx  = topCtx == XIN_INVALID_MODE ? 0 : av1IntraModeContext[topCtx];
    lftCtx  = mb->lftMi->predMode;
    lftCtx  = lftCtx == XIN_INVALID_MODE ? 0 : av1IntraModeContext[lftCtx];

    *bitNum = cabacEst->yModeRate[topCtx][lftCtx][intraMode];
}

static void Xin265pEstimateAngleDeltaY (
    xin_cabac_est *cabacEst,
    xin_mb_struct *mb,
    SINT32        predMode,
    SINT32        angleDeltaY,
    UINT32        *bitNum)
{
    *bitNum = 0;

    if ((predMode >= XIN_V_PRED) && (predMode <= XIN_D67_PRED))
    {
        if (mb->blockSize >= XIN_BLOCK_8X8)
        {
            *bitNum = cabacEst->angleDeltaRate[predMode - XIN_V_PRED][XIN_MAX_ANGLE_DELTA + angleDeltaY];
        }
    }
}

static UINT32 XinGetTxSizeCtx (
    UINT32 txSize)
{
    UINT32 txCtx;

    txCtx = ((txSizeSqrMap[txSize] + txSizeSqrUpMap[txSize] + 1) >> 1);

    return txCtx;
}

static void Xin265pGetEobRate (
    SINT32           eob,
    lv_map_eob_rate  *eobFacRate,
    lv_map_coef_rate *coefFacRate,
    UINT32           txClass,
    SINT32           *eobRate)
{
    SINT32 extra;
    SINT32 eobPt;
    SINT32 eobCtx;
    SINT32 eobShift;
    SINT32 eobMulCtx;
    SINT32 symbol;
    SINT32 offsetBits;

    Xin265pGetEobPosToken (
        eob,
        &extra,
        &eobPt);

    eobMulCtx  = (txClass == XIN_TX_CLASS_2D) ? 0 : 1;
    *eobRate   = eobFacRate->eobRate[eobMulCtx][eobPt - 1];
    offsetBits = av1EobOffsetBits[eobPt];

    if (av1EobOffsetBits[eobPt] > 0)
    {
        eobCtx   = eobPt - 3;
        eobShift = offsetBits - 1;
        symbol   = (extra & (1 << eobShift)) ? 1 : 0;

        *eobRate += coefFacRate->eobExtraRate[eobCtx][symbol];

        if (offsetBits > 1)
        {
            *eobRate += (offsetBits - 1) << XIN_RATE_FRACTION;
        }
    }

}

static inline SINT32 XinGetGolombRate (
    SINT32 absQc)
{
    SINT32  idx;
    SINT32  length;

    if (absQc >= 1 + XIN_NUM_BASE_LEVEL + XIN_COEF_BASE_RANGE)
    {
        idx    = absQc - XIN_COEF_BASE_RANGE - XIN_NUM_BASE_LEVEL;
        length = XinGetMsb(idx) + 1;

        return (2 * length - 1) << XIN_RATE_FRACTION;
    }

    return 0;
}

static inline SINT32 XinGetBrRate (
    SINT32 coefVal,
    SINT32 *coefLps)
{
    SINT32 baseRange;

    baseRange = XIN_MIN (coefVal - 1 - XIN_NUM_BASE_LEVEL, XIN_COEF_BASE_RANGE);

    return coefLps[baseRange] + XinGetGolombRate (coefVal);
}

void Xin265pComputeEob (
    xin_full_md_buf *fullBuf,
    intptr_t        coefAddr,
    xin_tu_struct   *tu,
    UINT32          planeIdx)
{
    UINT32   txSize;
    UINT32   txType;
    UINT32   planeType;
    UINT32   coefPos;
    SINT32   posX;
    SINT32   posY;
    UINT32   coefIdx;
    UINT32   nzCount;
    UINT32   nzIdx;
    UINT32   tuIdx;
    SINT32   lgWidth;
    UINT32   culLevel;
    SINT32   coefVal;
    SINT32   eob;
    COEFF    *coeff;
    intptr_t coeffStride;

    const xin_scan_order *scanOrder;

    tuIdx       = tu->tuIdx;
    planeType   = (planeIdx != PLANE_LUMA);
    txSize      = fullBuf->tranSize[planeType];
    txType      = fullBuf->tranType[planeType];
    lgWidth     = XIN_MIN (tu->lgWidth, 5);
    scanOrder   = &av1ScanOrder[txSize][txType];
    coeff       = fullBuf->qCoefBuf[planeIdx] + coefAddr;
    coeffStride = fullBuf->coefStride[planeType];
    nzCount     = fullBuf->nzCount[tuIdx][planeIdx];
    nzIdx       = 0;
    eob         = 0;
    culLevel    = 0;

    for (coefIdx = 0; ; coefIdx++)
    {
        coefPos = scanOrder->scan[coefIdx];
        posY    = coefPos >> lgWidth;
        posX    = coefPos - (posY << lgWidth);
        coefVal = coeff[posX + posY*coeffStride];
        nzIdx  += (coefVal != 0);
        eob++;

        culLevel += XIN_ABS (coefVal);

        if (nzIdx == nzCount)
        {
            break;
        }
    }

    culLevel = XIN_MIN (XIN_COEF_CONTEXT_MASK, culLevel);

    if (coeff[0] < 0)
    {
        culLevel |= 1 << XIN_COEF_CONTEXT_BIT;
    }
    else if (coeff[0] > 0)
    {
        culLevel += 2 << XIN_COEF_CONTEXT_BIT;
    }

    fullBuf->eob[tuIdx][planeIdx]      = eob;
    fullBuf->culLevel[tuIdx][planeIdx] = (UINT8)culLevel;

}

void Xin265pFullBufLoadCtx (
    xin_full_md_buf *fullBuf,
    xin_mb_struct   *mb)
{

    if (mb->lftMi->predMode != XIN_INVALID_MODE)
    {
        memcpy (fullBuf->lftCtx[PLANE_LUMA],     mb->mbLftCtx[PLANE_LUMA],     sizeof(UINT8)*mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        memcpy (fullBuf->lftCtx[PLANE_CHROMA_U], mb->mbLftCtx[PLANE_CHROMA_U], sizeof(UINT8)*mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        memcpy (fullBuf->lftCtx[PLANE_CHROMA_V], mb->mbLftCtx[PLANE_CHROMA_V], sizeof(UINT8)*mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    }
    else
    {
        memset (fullBuf->lftCtx[PLANE_LUMA],     0, sizeof(UINT8)*mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        memset (fullBuf->lftCtx[PLANE_CHROMA_U], 0, sizeof(UINT8)*mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        memset (fullBuf->lftCtx[PLANE_CHROMA_V], 0, sizeof(UINT8)*mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    }

    if (mb->topMi->predMode != XIN_INVALID_MODE)
    {
        memcpy (fullBuf->topCtx[PLANE_LUMA],     mb->mbTopCtx[PLANE_LUMA],     sizeof(UINT8)*mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        memcpy (fullBuf->topCtx[PLANE_CHROMA_U], mb->mbTopCtx[PLANE_CHROMA_U], sizeof(UINT8)*mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        memcpy (fullBuf->topCtx[PLANE_CHROMA_V], mb->mbTopCtx[PLANE_CHROMA_V], sizeof(UINT8)*mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    }
    else
    {
        memset (fullBuf->topCtx[PLANE_LUMA],     0, sizeof(UINT8)*mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
        memset (fullBuf->topCtx[PLANE_CHROMA_U], 0, sizeof(UINT8)*mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
        memset (fullBuf->topCtx[PLANE_CHROMA_V], 0, sizeof(UINT8)*mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    }

}

void Xin265pComputeTxCtx (
    xin_full_md_buf *fullBuf,
    xin_mb_struct   *mb,
    xin_tu_struct   *tu,
    UINT32          planeIdx)
{
    UINT32  rowIdx;
    UINT32  colIdx;
    SINT32  dcSign;
    SINT32  skipTop;
    SINT32  skipLft;
    UINT8   *topCtx;
    UINT8   *lftCtx;
    UINT32  tuIdx;
    UINT8   signIdx;

    topCtx  = fullBuf->topCtx[planeIdx];
    lftCtx  = fullBuf->lftCtx[planeIdx];
    skipTop = 0;
    skipLft = 0;
    dcSign  = 0;
    tuIdx   = tu->tuIdx;

    for (rowIdx = 0; rowIdx < tu->heightInMi; rowIdx++)
    {
        signIdx = lftCtx[rowIdx] >> XIN_COEF_CONTEXT_BIT;

        assert (signIdx <= 2);
        dcSign += dcSignMap[signIdx];
    }

    for (colIdx = 0; colIdx < tu->widthInMi; colIdx++)
    {
        signIdx = topCtx[colIdx] >> XIN_COEF_CONTEXT_BIT;

        assert (signIdx <= 2);
        dcSign += dcSignMap[signIdx];
    }

    fullBuf->dcSignCtx[tuIdx][planeIdx] = dcSignContext[dcSign + 2 * XIN_MAX_TX_SIZE_IN_UNIT];

    // Luma
    if (planeIdx == PLANE_LUMA)
    {
        if ((tu->width == mb->width[PLANE_LUMA]) && (tu->height == mb->height[PLANE_LUMA]))
        {
            fullBuf->txSkipCtx[tuIdx][planeIdx] = 0;
        }
        else
        {
            for (rowIdx = 0; rowIdx < tu->heightInMi; rowIdx++)
            {
                skipLft |= lftCtx[rowIdx];
            }

            skipLft &= XIN_COEF_CONTEXT_MASK;
            skipLft  = XIN_MIN (skipLft, 4);

            for (colIdx = 0; colIdx < tu->widthInMi; colIdx++)
            {
                skipTop |= topCtx[colIdx];
            }

            skipTop &= XIN_COEF_CONTEXT_MASK;
            skipTop  = XIN_MIN (skipTop, 4);

            fullBuf->txSkipCtx[tuIdx][planeIdx] = txSkipContext[skipTop][skipLft];
        }

    }
    else
    {
        // Chroma
        for (rowIdx = 0; rowIdx < tu->heightInMi; rowIdx++)
        {
            skipLft |= lftCtx[rowIdx];
        }

        skipLft &= XIN_COEF_CONTEXT_MASK;

        for (colIdx = 0; colIdx < tu->widthInMi; colIdx++)
        {
            skipTop |= topCtx[colIdx];
        }

        skipTop &= XIN_COEF_CONTEXT_MASK;

        if (mb->width[PLANE_CHROMA]*mb->height[PLANE_CHROMA] > tu->width*tu->height)
        {
            fullBuf->txSkipCtx[tuIdx][planeIdx] = 10 + (!!skipTop) + (!!skipLft);
        }
        else
        {
            fullBuf->txSkipCtx[tuIdx][planeIdx] = 7 + (!!skipTop) + (!!skipLft);
        }

    }

}

void Xin265pEstimateTxType (
    xin_full_md_buf *fullBuf,
    xin_cabac_est   *cabacEst,
    SINT32          *bitNum)
{
    xin_fast_md_buf *fastBuf;
    UINT32          txSize;
    UINT32          txSetType;
    UINT32          squTxSize;
    UINT32          intraDir;
    BOOL            isInter;
    UINT32          txType;
    SINT32          extSetIdx;

    fastBuf = fullBuf->fastBuf;
    txSize  = fullBuf->tranSize[PLANE_LUMA];
    isInter = fastBuf->type < XIN_INTRA_MODE;
    txType  = fullBuf->tranType[PLANE_LUMA];
    *bitNum = 0;

    Xin265pGetExtTxSetType (
        txSize,
        isInter,
        TRUE,
        &txSetType);

    if (av1NumExtTxSet[txSetType] > 1)
    {
        extSetIdx = av1ExtTxSetIndex[isInter][txSetType];

        if (extSetIdx > 0)
        {
            squTxSize = txSizeSqrMap[txSize];
            intraDir  = fastBuf->predMode;
            *bitNum   = cabacEst->intraTxTypeRate[txSetType][squTxSize][intraDir][txType];
        }
    }

}

void Xin265pEstimateTuCoeff (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    intptr_t        coefAddr,
    xin_cabac_est   *cabacEst,
    xin_tu_struct   *tu,
    UINT32          planeIdx,
    UINT32          *bitNum)
{
    SINT32   eob;
    UINT32   txSize;
    SINT32   tuIdx;
    UINT32   txCtx;
    UINT32   planeType;
    intptr_t coefStride;
    SINT32   eobMulSize;
    UINT32   width;
    UINT32   height;
    UINT32   lgWidth;
    UINT32   txType;
    UINT32   txClass;
    UINT8    levelBuf[XIN_TX_PAD_2D];
    SINT8    coeffContext[XIN_MAX_TX_SIZE*XIN_MAX_TX_SIZE];
    SINT32   numOfBit;
    SINT32   (*lpsRate)[XIN_COEF_BASE_RANGE + 1 + XIN_COEF_BASE_RANGE + 1];
    SINT32   (*baseRate)[8];
    SINT32   coefPos;
    SINT32   posX;
    SINT32   posY;
    SINT32   coefAbs;
    SINT32   coefIdx;
    SINT32   coefVal;
    SINT32   coefCtx;
    SINT32   brCtx;
    UINT8    dcSignCtx;
    UINT8    txSkipCtx;
    COEFF    *coeff;

    const xin_scan_order *scanOrder;
    lv_map_coef_rate     *coefFacRate;
    lv_map_eob_rate      *eobFacRate;
    xin_func_struct      *funcSet;

    funcSet     = secSet->funcSet;
    tuIdx       = tu->tuIdx;
    planeType   = (planeIdx != PLANE_LUMA);
    txSize      = fullBuf->tranSize[planeType];
    txType      = fullBuf->tranType[planeType];
    txCtx       = XinGetTxSizeCtx (txSize);
    eob         = fullBuf->eob[tuIdx][planeIdx];
    eobMulSize  = txSizeLogMinus4[txSize];
    coefFacRate = &cabacEst->coefFacRate[txCtx][planeType];
    eobFacRate  = &cabacEst->eobFracRate[eobMulSize][planeType];
    coefStride  = fullBuf->coefStride[planeType];
    width       = XIN_MIN (tu->width,  32);
    height      = XIN_MIN (tu->height, 32);
    lgWidth     = XIN_MIN (tu->lgWidth, 5);
    txClass     = txType2Class[txType];
    scanOrder   = &av1ScanOrder[txSize][txType];
    lpsRate     = coefFacRate->lpsRate;
    baseRate    = coefFacRate->baseRate;
    coeff       = fullBuf->qCoefBuf[planeIdx] + coefAddr;
    dcSignCtx   = fullBuf->dcSignCtx[tuIdx][planeIdx];
    txSkipCtx   = fullBuf->txSkipCtx[tuIdx][planeIdx];

    if (eob == 0)
    {
        *bitNum = coefFacRate->txbSkipRate[txSkipCtx][1];

        return;
    }

    *bitNum = coefFacRate->txbSkipRate[txSkipCtx][0];

    if (planeIdx == 0)
    {
        Xin265pEstimateTxType (
            fullBuf,
            cabacEst,
            &numOfBit);

        *bitNum += numOfBit;
    }

    funcSet->pfXinTxInitLevel[lgWidth] (
        coeff,
        coefStride,
        width,
        height,
        levelBuf,
        XIN_TX_BUF_STRIDE);

    Xin265pGetEobRate (
        eob,
        eobFacRate,
        coefFacRate,
        txClass,
        &numOfBit);

    *bitNum += numOfBit;

    Xin265pGetNzMapContext (
        levelBuf,
        XIN_TX_BUF_STRIDE,
        scanOrder->scan,
        eob,
        txSize,
        txClass,
        coeffContext);

    coefIdx = eob - 1;
    coefPos = scanOrder->scan[coefIdx];
    posY    = coefPos >> lgWidth;
    posX    = coefPos - width*posY;
    coefVal = coeff[posY*coefStride + posX];
    coefCtx = coeffContext[coefPos];
    coefAbs = XIN_ABS (coefVal);

    *bitNum += coefFacRate->baseEobRate[coefCtx][XIN_MIN (coefAbs, 3) - 1];

    if (coefVal)
    {
        // sign bit cost
        if (coefAbs > XIN_NUM_BASE_LEVEL)
        {
            brCtx    = Xin265pGetBrCtx (levelBuf, XIN_TX_BUF_STRIDE, posX, posY, txClass);
            *bitNum += XinGetBrRate (coefVal, lpsRate[coefCtx]);
        }

        if (coefIdx)
        {
            *bitNum += 1 << XIN_RATE_FRACTION;
        }
        else
        {
            *bitNum += coefFacRate->dcSignRate[dcSignCtx][(coefVal < 0) ? 1 : 0];

            return;
        }
    }

    for (coefIdx = eob - 2; coefIdx >= 1; --coefIdx)
    {
        coefPos = scanOrder->scan[coefIdx];
        posY    = coefPos >> lgWidth;
        posX    = coefPos - width*posY;
        coefCtx = coeffContext[coefPos];
        coefVal = coeff[posY*coefStride + posX];
        coefAbs = XIN_ABS (coefVal);

        if (coefVal)
        {
            // sign bit cost
            *bitNum += 1 << XIN_RATE_FRACTION;

            if (coefAbs > XIN_NUM_BASE_LEVEL)
            {
                brCtx    = Xin265pGetBrCtx (levelBuf, XIN_TX_BUF_STRIDE, posX, posY, txClass);
                *bitNum += XinGetBrRate (coefAbs, lpsRate[brCtx]);
            }
        }

        *bitNum += baseRate[coefCtx][XIN_MIN(coefAbs, 3)];

    }

    if (coefIdx == 0)
    {
        coefPos = scanOrder->scan[coefIdx];
        posY    = coefPos >> lgWidth;
        posX    = coefPos - width*posY;
        coefCtx = coeffContext[coefPos];
        coefVal = coeff[posY*coefStride + posX];
        coefAbs = XIN_ABS (coefVal);

        if (coefVal)
        {
            // sign bit cost
            *bitNum += coefFacRate->dcSignRate[dcSignCtx][(coefVal < 0) ? 1 : 0];

            if (coefAbs > XIN_NUM_BASE_LEVEL)
            {
                brCtx    = Xin265pGetBrCtx (levelBuf, XIN_TX_BUF_STRIDE, posX, posY, txClass);
                *bitNum += XinGetBrRate (coefAbs, lpsRate[brCtx]);
            }
        }

        *bitNum += baseRate[coefCtx][XIN_MIN(coefAbs, 3)];

    }

}

static void Xin265pEstimateFilterIntraMode (
    xin_cabac_est *cabacEst,
    UINT32        blockSize,
    UINT32        intraPredMode,
    UINT32        intraFilerMode,
    BOOL          enableFilterIntra,
    UINT32        *bitNum)
{
    if (Xin265pFilterIntraAllowed (enableFilterIntra, blockSize) && (intraPredMode == XIN_DC_PRED))
    {
        *bitNum = cabacEst->filterIntraRate[blockSize][intraFilerMode != XIN_FILTER_INTRA_MODE_NUM];

        if (intraFilerMode != XIN_FILTER_INTRA_MODE_NUM)
        {
            *bitNum += cabacEst->filterIntraModeRate[intraFilerMode];
        }
    }
    else
    {
        *bitNum = 0;
    }

}

void Xin265pEstimateMbCoeff (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf,
    UINT32          *lumaBits,
    UINT32          *chromaBits)
{
    UINT32          tuIdx;
    xin_tu_struct   *tuY;
    xin_tu_struct   *tuU;
    xin_tu_struct   *tuV;
    UINT32          yCoefBits;
    UINT32          uCoefBits;
    UINT32          vCoefBits;
    SINT32          offsetX;
    SINT32          offsetY;
    intptr_t        coefAddr;

    *lumaBits   = 0;
    *chromaBits = 0;

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        tuY = mb->tu[PLANE_LUMA] + tuIdx;

        Xin265pComputeTxCtx (
            fullBuf,
            mb,
            tuY,
            PLANE_LUMA);

        if (fullBuf->nzCount[tuIdx][PLANE_LUMA])
        {
            yCoefBits = 0;
            offsetX   = mb->offX[PLANE_LUMA] + tuY->offsetX;
            offsetY   = mb->offY[PLANE_LUMA] + tuY->offsetY;
            coefAddr  = offsetX + offsetY*fullBuf->coefStride[PLANE_LUMA];

            Xin265pComputeEob (
                fullBuf,
                coefAddr,
                tuY,
                PLANE_LUMA);

            Xin265pEstimateTuCoeff (
                secSet,
                fullBuf,
                coefAddr,
                &secSet->cabacEst,
                tuY,
                PLANE_LUMA,
                &yCoefBits);

            *lumaBits += yCoefBits;

        }
        else
        {
            fullBuf->eob[tuIdx][PLANE_LUMA]      = 0;
            fullBuf->culLevel[tuIdx][PLANE_LUMA] = 0;
        }

    }

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {

        tuU = mb->tu[PLANE_CHROMA_U] + tuIdx;
        tuV = mb->tu[PLANE_CHROMA_V] + tuIdx;

        Xin265pComputeTxCtx (
            fullBuf,
            mb,
            tuU,
            PLANE_CHROMA_U);

        if (fullBuf->nzCount[tuIdx][PLANE_CHROMA_U])
        {
            uCoefBits = 0;
            offsetX   = mb->offX[PLANE_CHROMA] + tuU->offsetX;
            offsetY   = mb->offY[PLANE_CHROMA] + tuU->offsetY;
            coefAddr  = offsetX + offsetY*fullBuf->coefStride[PLANE_CHROMA];

            Xin265pComputeEob (
                fullBuf,
                coefAddr,
                tuU,
                PLANE_CHROMA_U);

            Xin265pEstimateTuCoeff (
                secSet,
                fullBuf,
                coefAddr,
                &secSet->cabacEst,
                tuU,
                PLANE_CHROMA_U,
                &uCoefBits);

            *chromaBits += uCoefBits;

        }
        else
        {
            fullBuf->eob[tuIdx][PLANE_CHROMA_U]      = 0;
            fullBuf->culLevel[tuIdx][PLANE_CHROMA_U] = 0;
        }

        Xin265pComputeTxCtx (
            fullBuf,
            mb,
            tuV,
            PLANE_CHROMA_V);

        if (fullBuf->nzCount[tuIdx][PLANE_CHROMA_V])
        {
            vCoefBits = 0;
            offsetX   = mb->offX[PLANE_CHROMA] + tuV->offsetX;
            offsetY   = mb->offY[PLANE_CHROMA] + tuV->offsetY;
            coefAddr  = offsetX + offsetY*fullBuf->coefStride[PLANE_CHROMA];

            Xin265pComputeEob (
                fullBuf,
                coefAddr,
                tuV,
                PLANE_CHROMA_V);

            Xin265pEstimateTuCoeff (
                secSet,
                fullBuf,
                coefAddr,
                &secSet->cabacEst,
                tuV,
                PLANE_CHROMA_V,
                &vCoefBits);

            *chromaBits += vCoefBits;

        }
        else
        {
            fullBuf->eob[tuIdx][PLANE_CHROMA_V]      = 0;
            fullBuf->culLevel[tuIdx][PLANE_CHROMA_V] = 0;
        }

    }

}

void Xin265pEstimateCoeff (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf,
    UINT32          *coefBits,
    UINT32          planeIdx)
{
    UINT32          tuIdx;
    UINT32          planeType;
    xin_tu_struct   *tu;
    UINT32          localBits;
    SINT32          offsetX;
    SINT32          offsetY;
    intptr_t        coefAddr;

    planeType = planeIdx != PLANE_LUMA;
    *coefBits = 0;

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        tu = mb->tu[planeIdx] + tuIdx;

        Xin265pComputeTxCtx (
            fullBuf,
            mb,
            tu,
            planeIdx);

        if (fullBuf->nzCount[tuIdx][planeIdx])
        {
            localBits = 0;
            offsetX   = mb->offX[planeType] + tu->offsetX;
            offsetY   = mb->offY[planeType] + tu->offsetY;
            coefAddr  = offsetX + offsetY*fullBuf->coefStride[planeType];

            Xin265pComputeEob (
                fullBuf,
                coefAddr,
                tu,
                planeIdx);

            Xin265pEstimateTuCoeff (
                secSet,
                fullBuf,
                coefAddr,
                &secSet->cabacEst,
                tu,
                planeIdx,
                &localBits);

            *coefBits += localBits;

        }
        else
        {
            fullBuf->eob[tuIdx][planeIdx]      = 0;
            fullBuf->culLevel[tuIdx][planeIdx] = 0;
        }

    }

}


void Xin265pEstimateCoeffSkipFlag (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    xin_mb_struct   *mb,
    UINT32          *bitNum)
{
    UINT32        tuIdx;
    BOOL          hasCoeff;
    UINT8         skipCoeffCtx;
    xin_cabac_est *cabacEst;

    hasCoeff     = FALSE;
    cabacEst     = &secSet->cabacEst;
    skipCoeffCtx = mb->skipCoeffCtx;

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        hasCoeff |= (fullBuf->eob[tuIdx][0] != 0);
    }

    hasCoeff |= (fullBuf->eob[0][1] != 0) | (fullBuf->eob[0][2] != 0);

    fullBuf->skipCoeff = !hasCoeff;

    *bitNum = cabacEst->skipRate[skipCoeffCtx][fullBuf->skipCoeff];

}

void Xin265pEstimateMbSynatax (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf *fastBuf,
    UINT32          *bitNum)
{
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    xin_cabac_est   *cabacEst;
    UINT32          blockSize;
    UINT32          isIntraFrame;
    UINT32          numOfBit;
    UINT32          predMode;
    UINT32          totalBits;

    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    cabacEst     = &secSet->cabacEst;
    totalBits    = 0;
    predMode     = fastBuf->predMode;
    pictureWrite = picSet->pictureWrite;
    isIntraFrame = pictureWrite->frameType >= XIN_I_FRAME;
    blockSize    = mb->blockSize;

    // Write skip flag, if frame type is not I frame
    if (isIntraFrame)
    {
        // intra_frame_y_mode
        Xin265pEstimateIntraFrameYMode (
            cabacEst,
            mb,
            fastBuf->predMode,
            &numOfBit);

        totalBits += numOfBit;

        // angle_delta_y
        Xin265pEstimateAngleDeltaY (
            cabacEst,
            mb,
            predMode,
            fastBuf->angleDelta[PLANE_LUMA],
            &numOfBit);

        totalBits += numOfBit;

        // filter intra mode
        Xin265pEstimateFilterIntraMode (
            cabacEst,
            blockSize,
            predMode,
            fastBuf->intraFilterMode,
            seqSet->config.enableIntraEdgeFilter,
            &numOfBit);

        totalBits += numOfBit;

    }

    *bitNum = totalBits;

}

