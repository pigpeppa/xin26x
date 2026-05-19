/***************************************************************************//**
*
* @file          h265p_deblock_sb.c
* @brief         Av1 loop filter on super block level.
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
#include "h265p_pic_struct.h"
#include "h265p_trans_context.h"
#include "h265p_common_data.h"
#include "h265p_cabac_struct.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h265p_loop_filter.h"

void Xin265pDeblockSbVer (
    xin_pic_struct *picSet,
    xin_sb_struct  *sb)
{
    xin_ref_picture *pictureWrite;
    xin_lf_thresh   *lfThreshY;
    xin_lf_thresh   *lfThreshU;
    xin_lf_thresh   *lfThreshV;
    SINT32          sbWidthInMi;
    SINT32          sbHeightInMi;
    intptr_t        reconYStride;
    intptr_t        reconUvStride;
    intptr_t        fltYStride;
    intptr_t        fltUvStride;
    UINT8           *lumaFlt;
    UINT8           *chromaFlt;
    UINT8           fltLength;
    PIXEL           *reconY;
    PIXEL           *reconU;
    PIXEL           *reconV;
    SINT32          rowIdx;
    SINT32          colIdx;
    SINT32          levelY;
    SINT32          levelU;
    SINT32          levelV;

    pictureWrite  = picSet->pictureWrite;
    sbWidthInMi   = sb->width >> XIN_LOG_MI_SIZE;
    sbHeightInMi  = sb->height >> XIN_LOG_MI_SIZE;
    reconYStride  = pictureWrite->refStride[PLANE_LUMA];
    reconUvStride = pictureWrite->refStride[PLANE_CHROMA];
    fltYStride    = pictureWrite->lumaFltStride[0];
    fltUvStride   = pictureWrite->chromaFltStride[0];

    reconY = pictureWrite->refBuf[PLANE_LUMA] + sb->sbPelY*reconYStride + sb->sbPelX;
    reconU = pictureWrite->refBuf[PLANE_CHROMA_U] + ((sb->sbPelY*reconUvStride + sb->sbPelX) >> 1);
    reconV = pictureWrite->refBuf[PLANE_CHROMA_V] + ((sb->sbPelY*reconUvStride + sb->sbPelX) >> 1);

    levelY = picSet->lfInfo.lvl[PLANE_LUMA][0];
    levelU = picSet->lfInfo.lvl[PLANE_CHROMA_U][0];
    levelV = picSet->lfInfo.lvl[PLANE_CHROMA_V][0];

    lfThreshY = picSet->lfInfo.lfThr + levelY;
    lfThreshU = picSet->lfInfo.lfThr + levelU;
    lfThreshV = picSet->lfInfo.lfThr + levelV;

    lumaFlt   = pictureWrite->lumaFlt[0] + (sb->sbPelX >> XIN_LOG_MI_SIZE)*fltYStride + (sb->sbPelY >> XIN_LOG_MI_SIZE);
    chromaFlt = pictureWrite->chromaFlt[0] + (((sb->sbPelX >> XIN_LOG_MI_SIZE)*fltUvStride + (sb->sbPelY >> XIN_LOG_MI_SIZE)) >> 1);

    for (colIdx = 0; colIdx < sbWidthInMi; colIdx++)
    {
        for (rowIdx = 0; rowIdx < sbHeightInMi; rowIdx++)
        {
            fltLength = lumaFlt[rowIdx];

            switch (fltLength)
            {
            case 0:

                break;

            case 4:

                Xin265pLpfVert4 (
                    reconY + rowIdx*reconYStride*XIN_MI_SIZE,
                    reconYStride,
                    lfThreshY->mblim,
                    lfThreshY->lim,
                    lfThreshY->hevThr);

                break;

            case 8:

                Xin265pLpfVert8 (
                    reconY + rowIdx*reconYStride*XIN_MI_SIZE,
                    reconYStride,
                    lfThreshY->mblim,
                    lfThreshY->lim,
                    lfThreshY->hevThr);

                break;

            case 14:

                Xin265pLpfVert14 (
                    reconY + rowIdx*reconYStride*XIN_MI_SIZE,
                    reconYStride,
                    lfThreshY->mblim,
                    lfThreshY->lim,
                    lfThreshY->hevThr);

                break;

            default:

                _XIN_LOGGER (XIN_LOGGER_ERROR, "Luma loop filter length is invalid.\n");

                break;

            }

        }

        reconY  += XIN_MI_SIZE;
        lumaFlt += fltYStride;
    }

    sbWidthInMi  = sbWidthInMi >> 1;
    sbHeightInMi = sbHeightInMi >> 1;

    for (colIdx = 0; colIdx < sbWidthInMi; colIdx++)
    {
        for (rowIdx = 0; rowIdx < sbHeightInMi; rowIdx++)
        {
            fltLength = chromaFlt[rowIdx];

            switch (fltLength)
            {
            case 0:

                break;

            case 4:

                Xin265pLpfVert4 (
                    reconU + rowIdx*reconUvStride*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshU->mblim,
                    lfThreshU->lim,
                    lfThreshU->hevThr);

                Xin265pLpfVert4 (
                    reconV + rowIdx*reconUvStride*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshV->mblim,
                    lfThreshV->lim,
                    lfThreshV->hevThr);

                break;

            case 6:

                Xin265pLpfVert6 (
                    reconU + rowIdx*reconUvStride*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshU->mblim,
                    lfThreshU->lim,
                    lfThreshU->hevThr);

                Xin265pLpfVert6 (
                    reconV + rowIdx*reconUvStride*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshV->mblim,
                    lfThreshV->lim,
                    lfThreshV->hevThr);

                break;

            default:

                _XIN_LOGGER (XIN_LOGGER_ERROR, "Chroma loop filter length is invalid.\n");

                break;

            }

        }

        reconU    += XIN_MI_SIZE;
        reconV    += XIN_MI_SIZE;
        chromaFlt += fltUvStride;

    }

}

void Xin265pDeblockSbHor (
    xin_pic_struct *picSet,
    xin_sb_struct  *sb)
{
    xin_ref_picture *pictureWrite;
    xin_lf_thresh   *lfThreshY;
    xin_lf_thresh   *lfThreshU;
    xin_lf_thresh   *lfThreshV;
    SINT32          sbWidthInMi;
    SINT32          sbHeightInMi;
    intptr_t        reconYStride;
    intptr_t        reconUvStride;
    intptr_t        fltYStride;
    intptr_t        fltUvStride;
    UINT8           *lumaFlt;
    UINT8           *chromaFlt;
    UINT8           fltLength;
    PIXEL           *reconY;
    PIXEL           *reconU;
    PIXEL           *reconV;
    SINT32          rowIdx;
    SINT32          colIdx;
    SINT32          levelY;
    SINT32          levelU;
    SINT32          levelV;

    pictureWrite  = picSet->pictureWrite;
    sbWidthInMi   = sb->width >> XIN_LOG_MI_SIZE;
    sbHeightInMi  = sb->height >> XIN_LOG_MI_SIZE;
    reconYStride  = pictureWrite->refStride[PLANE_LUMA];
    reconUvStride = pictureWrite->refStride[PLANE_CHROMA];
    fltYStride    = pictureWrite->lumaFltStride[1];
    fltUvStride   = pictureWrite->chromaFltStride[1];

    reconY = pictureWrite->refBuf[PLANE_LUMA] + sb->sbPelY*reconYStride + sb->sbPelX;
    reconU = pictureWrite->refBuf[PLANE_CHROMA_U] + ((sb->sbPelY*reconUvStride + sb->sbPelX) >> 1);
    reconV = pictureWrite->refBuf[PLANE_CHROMA_V] + ((sb->sbPelY*reconUvStride + sb->sbPelX) >> 1);

    levelY = picSet->lfInfo.lvl[PLANE_LUMA][1];
    levelU = picSet->lfInfo.lvl[PLANE_CHROMA_U][1];
    levelV = picSet->lfInfo.lvl[PLANE_CHROMA_V][1];

    lfThreshY = picSet->lfInfo.lfThr + levelY;
    lfThreshU = picSet->lfInfo.lfThr + levelU;
    lfThreshV = picSet->lfInfo.lfThr + levelV;

    lumaFlt   = pictureWrite->lumaFlt[1] + (sb->sbPelY >> XIN_LOG_MI_SIZE)*fltYStride + (sb->sbPelX >> XIN_LOG_MI_SIZE);
    chromaFlt = pictureWrite->chromaFlt[1] + (((sb->sbPelY >> XIN_LOG_MI_SIZE)*fltUvStride + (sb->sbPelX >> XIN_LOG_MI_SIZE)) >> 1);

    for (rowIdx = 0; rowIdx < sbHeightInMi; rowIdx++)
    {
        for (colIdx = 0; colIdx < sbWidthInMi; colIdx++)
        {
            fltLength = lumaFlt[colIdx];

            switch (fltLength)
            {
            case 0:

                break;

            case 4:

                Xin265pLpfHorz4 (
                    reconY + colIdx*XIN_MI_SIZE,
                    reconYStride,
                    lfThreshY->mblim,
                    lfThreshY->lim,
                    lfThreshY->hevThr);

                break;

            case 8:

                Xin265pLpfHorz8 (
                    reconY + colIdx*XIN_MI_SIZE,
                    reconYStride,
                    lfThreshY->mblim,
                    lfThreshY->lim,
                    lfThreshY->hevThr);

                break;

            case 14:

                Xin265pLpfHorz14 (
                    reconY + colIdx*XIN_MI_SIZE,
                    reconYStride,
                    lfThreshY->mblim,
                    lfThreshY->lim,
                    lfThreshY->hevThr);

                break;

            default:

                _XIN_LOGGER (XIN_LOGGER_ERROR, "Luma loop filter length is invalid.\n");

                break;

            }

        }

        reconY  += XIN_MI_SIZE*reconYStride;
        lumaFlt += fltYStride;

    }

    sbWidthInMi  = sbWidthInMi >> 1;
    sbHeightInMi = sbHeightInMi >> 1;

    for (rowIdx = 0; rowIdx < sbHeightInMi; rowIdx++)
    {
        for (colIdx = 0; colIdx < sbWidthInMi; colIdx++)
        {
            fltLength = chromaFlt[colIdx];

            switch (fltLength)
            {
            case 0:

                break;

            case 4:

                Xin265pLpfHorz4 (
                    reconU + colIdx*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshU->mblim,
                    lfThreshU->lim,
                    lfThreshU->hevThr);

                Xin265pLpfHorz4 (
                    reconV + colIdx*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshV->mblim,
                    lfThreshV->lim,
                    lfThreshV->hevThr);

                break;

            case 6:

                Xin265pLpfHorz6 (
                    reconU + colIdx*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshU->mblim,
                    lfThreshU->lim,
                    lfThreshU->hevThr);

                Xin265pLpfHorz6 (
                    reconV + colIdx*XIN_MI_SIZE,
                    reconUvStride,
                    lfThreshV->mblim,
                    lfThreshV->lim,
                    lfThreshV->hevThr);

                break;

            default:

                _XIN_LOGGER (XIN_LOGGER_ERROR, "Chroma loop filter length is invalid.\n");
                break;

            }

        }

        reconU    += XIN_MI_SIZE*reconUvStride;
        reconV    += XIN_MI_SIZE*reconUvStride;
        chromaFlt += fltUvStride;

    }

}

