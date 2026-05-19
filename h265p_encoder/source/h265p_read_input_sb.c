/***************************************************************************//**
*
* @file          h265p_read_input_sb.c
* @brief         Read an input super block from input frame buffer
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
#include "xin26x_logger.h"
#include "h26x_block_utility.h"
#include "h265p_analyze_mb.h"
#include "h265p_enc_init.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_trans_recon.h"
#include "h26x_block_utility.h"

void Xin265pReadInputSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb)
{
    UINT32   width;
    UINT32   height;
    UINT32   restWidth;
    UINT32   restHeight;
    intptr_t srcYStride;
    intptr_t srcUvStride;
    intptr_t dstYStride;
    intptr_t dstUvStride;
    PIXEL    *srcY;
    PIXEL    *srcU;
    PIXEL    *srcV;
    PIXEL    *dstY;
    PIXEL    *dstU;
    PIXEL    *dstV;
    UINT32   rowIdx;

    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;

    picSet  = secSet->picSet;
    seqSet  = secSet->seqSet;
    funcSet = secSet->funcSet;
    width   = sb->width;
    height  = sb->height;

    pictureWrite = picSet->pictureWrite;
    srcYStride   = pictureWrite->inputStride[PLANE_LUMA];
    srcUvStride  = pictureWrite->inputStride[PLANE_CHROMA];
    srcY         = pictureWrite->inputBuf[PLANE_LUMA] + sb->sbPelY*srcYStride + sb->sbPelX;
    srcU         = pictureWrite->inputBuf[PLANE_CHROMA_U] + ((sb->sbPelY*srcUvStride + sb->sbPelX) >> 1);
    srcV         = pictureWrite->inputBuf[PLANE_CHROMA_V] + ((sb->sbPelY*srcUvStride + sb->sbPelX) >> 1);
    dstYStride   = secSet->inputYStride;
    dstUvStride  = secSet->inputUvStride;
    dstY         = secSet->inputSb[PLANE_LUMA];
    dstU         = secSet->inputSb[PLANE_CHROMA_U];
    dstV         = secSet->inputSb[PLANE_CHROMA_V];
    restWidth    = seqSet->config.inputWidth - sb->sbPelX;
    restHeight   = seqSet->config.inputHeight - sb->sbPelY;

    if ((restWidth >= width) && (restHeight >= height))
    {
        funcSet->pfXinBlockCopy[sb->lgWidth] (
            srcY,
            srcYStride,
            dstY,
            dstYStride,
            width,
            height);

        funcSet->pfXinBlockCopy[sb->lgWidth-1] (
            srcU,
            srcUvStride,
            dstU,
            dstUvStride,
            width/2,
            height/2);

        funcSet->pfXinBlockCopy[sb->lgWidth-1] (
            srcV,
            srcUvStride,
            dstV,
            dstUvStride,
            width/2,
            height/2);

    }
    else
    {
        width   = (restWidth < width) ? restWidth : width;
        height  = (restHeight < height) ? restHeight: height;

        Xin26xBlockCopy (
            srcY,
            srcYStride,
            dstY,
            dstYStride,
            width,
            height);

        Xin26xBlockCopy (
            srcU,
            srcUvStride,
            dstU,
            dstUvStride,
            width/2,
            height/2);

        Xin26xBlockCopy (
            srcV,
            srcUvStride,
            dstV,
            dstUvStride,
            width/2,
            height/2);

        if (width < sb->width)
        {
            for (rowIdx = 0; rowIdx < height; rowIdx++)
            {
                memset (dstY + width, *(dstY + width - 1), sb->width - width);
                dstY += dstYStride;
            }

            for (rowIdx = 0; rowIdx < height/2; rowIdx++)
            {
                memset (dstU + width/2, *(dstU + width/2 - 1), (sb->width - width)/2);
                memset (dstV + width/2, *(dstV + width/2 - 1), (sb->width - width)/2);
                dstU += dstUvStride;
                dstV += dstUvStride;
            }
        }

        if (height < sb->height)
        {
            dstY = secSet->inputSb[PLANE_LUMA] + height*dstYStride;
            dstU = secSet->inputSb[PLANE_CHROMA_U] + height*dstUvStride/2;
            dstV = secSet->inputSb[PLANE_CHROMA_V] + height*dstUvStride/2;

            for (rowIdx = height; rowIdx < sb->height; rowIdx++)
            {
                memcpy (dstY, dstY - dstYStride, sb->width);
                dstY += dstYStride;
            }

            for (rowIdx = height/2; rowIdx < sb->height/2; rowIdx++)
            {
                memcpy (dstU, dstU - dstUvStride, sb->width/2);
                memcpy (dstV, dstV - dstUvStride, sb->width/2);
                dstU += dstUvStride;
                dstV += dstUvStride;
            }
        }

    }

}

