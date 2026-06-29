/***************************************************************************//**
 *
 * @file          h266_scan_order.c
 * @brief         This file defines tables for coefficient scan order.
 *
 * @authors       Chao Zhou
 *
 * Xin26x Video Codec Library
 *
 * Copyright (C) 2020-2026 Chao Zhou <czhou2@qq.com>
 *
 * This file is part of Xin26x.
 *
 * Licensed under the GNU General Public License, Version 3 or later
 * (GPL-3.0-or-later). See the LICENSE file for details.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 *******************************************************************************/
#include "xin_typedef.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "xin26x_params.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "basic_macro.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_section_struct.h"
#include "h266_common_data.h"

static void Xin266GenScanIndex (
    xin_scan_pos *coeffScan,
    UINT32       width,
    UINT32       height)
{
    UINT8   scanPos;
    UINT32  rowIdx;
    UINT32  colIdx;
    UINT32  scanIdx;

    rowIdx = 0;
    colIdx = 0;

    for (scanIdx = 0; scanIdx < width*height; scanIdx++)
    {
        scanPos = (UINT8)(rowIdx*width + colIdx);
        
        coeffScan[scanIdx].posIdx = scanPos;
        coeffScan[scanIdx].posX   = colIdx;
        coeffScan[scanIdx].posY   = rowIdx;

        if ((colIdx == width - 1) || (rowIdx == 0))
        {
            rowIdx += colIdx + 1;
            colIdx  = 0;

            if (rowIdx >= height)
            {
                colIdx += rowIdx - (height - 1);
                rowIdx  = height - 1;
            }
        }
        else
        {
            colIdx++;
            rowIdx--;
        }
        
    }
    
}

SINT32 Xin266ContructScanOrder (
    xin_seq_struct *seqSet)
{
    UINT32  widthIdx;
    UINT32  heightIdx;
    UINT32  widthNum;
    UINT32  heightNum;
    UINT32  cgWidth;
    UINT32  cgHeight;
    UINT32  width;
    UINT32  height;

    // malloc memory for coefficient scan order
    heightNum = XIN_MAX_LG_CG_SIZE;
    widthNum  = XIN_MAX_LG_CG_SIZE;
    
    for (heightIdx = 0; heightIdx <= heightNum; heightIdx++)
    {
        for (widthIdx = 0; widthIdx <= widthNum; widthIdx++)
        {
            width  = 1 << widthIdx;
            height = 1 << heightIdx;

            XIN_MALLOC_CHECK (seqSet->scanOrder[heightIdx][widthIdx], height*width*sizeof(xin_scan_pos));

            Xin266GenScanIndex (
                seqSet->scanOrder[heightIdx][widthIdx],
                width,
                height);
        }
    }

    // malloc memory for coefficient group scan order
    heightNum = XIN_MAX_LG_TU_SIZE;
    widthNum  = XIN_MAX_LG_TU_SIZE;
    
    for (heightIdx = 0; heightIdx <= heightNum; heightIdx++)
    {
        for (widthIdx = 0; widthIdx <= widthNum; widthIdx++)
        {
            width    = 1 << widthIdx;
            height   = 1 << heightIdx;
            width    = XIN_MIN (width, 32);
            height   = XIN_MIN (height, 32);
            cgWidth  = 1 << log2SbbSize[widthIdx][heightIdx][0];
            cgHeight = 1 << log2SbbSize[widthIdx][heightIdx][1];
            width    = width / cgWidth;
            height   = height / cgHeight;
            
            XIN_MALLOC_CHECK (seqSet->scanOrderCG[heightIdx][widthIdx], height*width*sizeof(xin_scan_pos));

            Xin266GenScanIndex (
                seqSet->scanOrderCG[heightIdx][widthIdx],
                width,
                height);
        }
        
    }

    return XIN_SUCCESS;
    
}

void Xin266DestructScanOrder (
    xin_seq_struct *seqSet)
{
    UINT32  widthIdx;
    UINT32  heightIdx;
    UINT32  widthNum;
    UINT32  heightNum;

    // malloc memory for coefficient scan order
    heightNum = XIN_MAX_LG_CG_SIZE;
    widthNum  = XIN_MAX_LG_CG_SIZE;
    
    for (heightIdx = 0; heightIdx <= heightNum; heightIdx++)
    {
        for (widthIdx = 0; widthIdx <= widthNum; widthIdx++)
        {
            free (seqSet->scanOrder[heightIdx][widthIdx]);
        }
    }

    // malloc memory for coefficient group scan order
    heightNum = XIN_MAX_LG_TU_SIZE;
    widthNum  = XIN_MAX_LG_TU_SIZE;
    
    for (heightIdx = 0; heightIdx <= heightNum; heightIdx++)
    {
        for (widthIdx = 0; widthIdx <= widthNum; widthIdx++)
        {   
            free (seqSet->scanOrderCG[heightIdx][widthIdx]);
        }
    }
    
}


