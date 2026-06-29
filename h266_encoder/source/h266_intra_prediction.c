/***************************************************************************//**
 *
 * @file          h266_intra_prediction.c
 * @brief         h266 intra prediction subroutines.
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
#include "h266_constant.h"
#include "memory.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_common_data.h"
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
#include "video_macro.h"
#include "h266_intra_pred_filter.h"
#include "h26x_calc_log.h"
#include "h266_func_struct.h"
#include "h26x_block_transpose.h"
#include "assert.h"

static const SINT32 intraAngTable[32] =
{
    0, 1, 2, 3, 4, 6, 8, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 35, 39, 45, 51, 57, 64, 73, 86, 102, 128, 171, 256, 341, 512, 1024
};

static const SINT32 invIntraAngTable[32] =
{
    0,   16384, 8192, 5461, 4096, 2731, 2048, 1638, 1365, 1170, 1024, 910, 819, 712, 630, 565,
    512, 468,   420,  364,  321,  287,  256,  224,  191,  161,  128,  96,  64,  48,  32,  16
};

static const SINT32 preScale[32] =
{
    8,  7,  6,  5,  5,  4,  4,  4,
    3,  3,  3,  3,  3,  3,  2,  2,
    2,  2,  2,  2,  1,  1,  1,  1,
    1,  0,  0,  0, -1, -1, -2, -3
};

static const UINT8 intraFilter[] =
{
    24, //   1xn
    24, //   2xn
    24, //   4xn
    14, //   8xn
    2,  //  16xn
    0,  //  32xn
    0,  //  64xn
    0   // 128xn
};

static const SINT32 intraModeShift[] =
{
    0, 6, 10, 12, 14, 15
};

static void GetIntraNBAvail (
    xin_block_struct *block,
    UINT32           unitSize,
    UINT32           *availField,
    intptr_t         blockStride)
{
    UINT32      unitIndex;
    UINT32      internalAvail;

    internalAvail = 0;

    for (unitIndex = 0; unitIndex < unitSize; unitIndex++)
    {
        internalAvail |= (block->type != XIN_INVALID_MODE) << unitIndex;

        block += blockStride;
    }

    *availField = internalAvail;

}

void Xin266GetIntraAvail (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    UINT32         lgBlockSize,
    intptr_t       blockStride)
{
    UINT32         widthUnit;
    UINT32         heightUnit;
    SINT32         width;
    SINT32         height;
    UINT32         numIntraUnit;
    xin_cu_struct  *cu;
    xin_seq_struct *seqSet;
    UINT64         intraAvail;
    UINT32         tmpAvail;
    SINT32         strideMul;

    cu           = secSet->cu;
    width        = pu->width;
    height       = pu->height;
    strideMul    = (lgBlockSize == 2) && (width >= 8) && (height >= 8) ? 2 : 1;
    lgBlockSize  = (width >= 8) && (height >= 8) ? 3 : lgBlockSize;
    widthUnit    = width >> lgBlockSize;
    heightUnit   = height >> lgBlockSize;
    numIntraUnit = 0;
    seqSet       = secSet->seqSet;
    intraAvail   = 0;

    // bottom left
    if (secSet->lftBotBlock->type != XIN_INVALID_MODE)
    {
        GetIntraNBAvail (
            secSet->lftBotBlock + (heightUnit - 1)*blockStride*strideMul,
            heightUnit,
            &tmpAvail,
            -blockStride*strideMul);

        intraAvail |= (UINT64)tmpAvail;

        if (((cu->offY + pu->height*2) > seqSet->ctuSize) && (cu->offX == 0))
        {
            tmpAvail    = (1 << ((cu->offY + pu->height*2 - seqSet->ctuSize) >> lgBlockSize)) - 1;
            intraAvail &= (~(UINT64)tmpAvail);
        }

    }

    // left
    if (secSet->lftBBlock->type != XIN_INVALID_MODE)
    {
        intraAvail |= (((UINT64)1 << heightUnit) - 1) << heightUnit;
    }

    // top left
    if (secSet->topLftBlock->type != XIN_INVALID_MODE)
    {
        intraAvail |= ((UINT64)1 << (heightUnit*2));
    }

    // top
    if (secSet->topRBlock->type != XIN_INVALID_MODE)
    {
        intraAvail |= (((UINT64)1 << widthUnit) - 1) << (2 * heightUnit + 1);
    }

    // top right
    if (secSet->topRgtBlock->type != XIN_INVALID_MODE)
    {
        GetIntraNBAvail (
            secSet->topRgtBlock,
            widthUnit,
            &tmpAvail,
            strideMul);

        intraAvail |= ((UINT64)tmpAvail << (2 * heightUnit + widthUnit + 1));

        if ((secSet->topRgtCtu == NULL) && ((cu->offX + pu->width*2) > seqSet->ctuSize) && (cu->offY == 0))
        {
            tmpAvail    = (1 << ((cu->offX + pu->width*2 - seqSet->ctuSize) >> lgBlockSize)) - 1;
            intraAvail &= (~(((UINT64)tmpAvail) << (2 * heightUnit + 2 * widthUnit + 1 - ((cu->offX + pu->width*2 - seqSet->ctuSize) >> lgBlockSize))));
        }

    }

    secSet->intraAvailField = intraAvail;

}

void Xin266FilterIntraNB (
    PIXEL   *src,
    PIXEL   *dst,
    UINT32  width,
    UINT32  height,
    SINT32  multiRefIdx)
{
    UINT32  i;
    UINT32  length;

    length = width*2 + height*2 + multiRefIdx*2 + 1;

    // Regular filtering.
    *dst++ = *src++;

    for (i = 0; i < length - 2; i++)
    {
        *dst++ = (src[-1] + (src[0] << 1) + src[1] + 2) >> 2;
        ++src;
    }

    *dst = *src;

}

void Xin266ExtractIntraNB (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    SINT32         multiRefIdx)
{
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    PIXEL           *nIntraBuf[2];
    PIXEL           nRawBuf[264];
    PIXEL           *src;
    PIXEL           *nBuf;
    PIXEL           *nFltBuf;
    SINT32          idx;
    SINT32          i, j;
    intptr_t        srcStride;
    SINT32          height;
    SINT32          width;
    UINT32          heightUnit;
    UINT32          widthUnit;
    SINT32          totalUnits;
    SINT32          lgBlockSize;
    SINT32          blockSize;
    UINT64          intraAvail;

    nIntraBuf[0] = secSet->nIntraBufY;
    nIntraBuf[1] = secSet->nIntraFltBufY;
    height       = pu->height;
    width        = pu->width;
    seqSet       = secSet->seqSet;
    lgBlockSize  = (width >= 8) && (height >= 8) ? 3 : seqSet->lgBlockSize;
    blockSize    = 1 << lgBlockSize;
    widthUnit    = width >> lgBlockSize;
    heightUnit   = height >> lgBlockSize;
    totalUnits   = widthUnit*2 + heightUnit*2 + 1;
    srcStride    = secSet->reconYStride;
    src          = secSet->reconCu[PLANE_LUMA];
    nBuf         = secSet->nIntraBufY - height*2;
    nFltBuf      = secSet->nIntraFltBufY - height*2;
    intraAvail   = secSet->intraAvailField;
    funcSet      = secSet->funcSet;

    // BL BL BL BL L L L L TL TL TL TL T T T T TR TR TR TR
    if (intraAvail == 0)
    {
        // None of neighbour is available
#ifdef ENABLE_10BIT_ENCODER
        for (i = 0; i < (SINT32)(2 * width + 2 * height + blockSize); i++)
        {
            nRawBuf[i] = XIN_DEFAULT_PIXEL;
        }
#else
        memset(nRawBuf, 128, sizeof(PIXEL)*(2 * width + 2 * height + blockSize));
#endif
    }
    else
    {
        // Assume all edges are available
        memcpy (nRawBuf + height*2, src - (1 + multiRefIdx)*srcStride - blockSize, (width*2 + blockSize)*sizeof(PIXEL));

        for (idx = 0; idx < height*2; idx++)
        {
            nRawBuf[idx] = *(src + (height*2 - 1 - idx)*srcStride - (1 + multiRefIdx));
        }

        if (intraAvail != (((UINT64)1 << totalUnits) - 1))
        {
            idx = 1;

            if (!(intraAvail & 1))
            {
                while (!((intraAvail >> idx) & 1))
                {
                    idx++;
                }

#ifdef ENABLE_10BIT_ENCODER
                for (i = 0; i < (SINT32)(idx*blockSize); i++)
                {
                    nRawBuf[i] = nRawBuf[idx << lgBlockSize];
                }
#else
                memset (nRawBuf, nRawBuf[idx << lgBlockSize], idx*blockSize);
#endif

            }

            if (lgBlockSize == XIN_LOG_BLOCK_SIZE)
            {
                for (; idx < totalUnits; idx++)
                {
                    if (!((intraAvail >> idx) & 1))
                    {
#ifdef ENABLE_10BIT_ENCODER
                        UINT64 pelx4;

                        pelx4 = nRawBuf[(idx << lgBlockSize) - 1];
                        pelx4 = pelx4*0x0001000100010001;
                        *((UINT64 *)(nRawBuf + (idx << lgBlockSize))) = pelx4;

#else
                        UINT32 pelx4;

                        pelx4 = nRawBuf[(idx << lgBlockSize) - 1];
                        pelx4 = pelx4*0x01010101;
                        *((UINT32 *)(nRawBuf + (idx << lgBlockSize))) = pelx4;
#endif
                    }
                }
            }
            else
            {
                for (; idx < totalUnits; idx++)
                {
                    if (!((intraAvail >> idx) & 1))
                    {
#ifdef ENABLE_10BIT_ENCODER
                        UINT64 pelx4;

                        pelx4 = nRawBuf[(idx << lgBlockSize) - 1];
                        pelx4 = pelx4*0x0001000100010001;

                        *((UINT64 *)(nRawBuf + (idx << lgBlockSize) + 0)) = pelx4;
                        *((UINT64 *)(nRawBuf + (idx << lgBlockSize) + 4)) = pelx4;

#else
                        UINT64 pelx8;

                        pelx8 = nRawBuf[(idx << lgBlockSize) - 1];
                        pelx8 = pelx8*0x0101010101010101;
                        *((UINT64 *)(nRawBuf + (idx << lgBlockSize))) = pelx8;
#endif
                    }
                }
            }

        }

    }

    // re-arrange neighbour pixel layout
    memcpy (nBuf, nRawBuf, height*2*sizeof(PIXEL));

    if ((intraAvail >> (heightUnit * 2)) & 1)
    {
        for (idx = 0; idx < multiRefIdx; idx++)
        {
            nBuf[height*2 + idx] = src[-1 - multiRefIdx - srcStride*idx];
        }
    }
    else
    {
#ifdef ENABLE_10BIT_ENCODER
        for (i = 0; i < multiRefIdx; i++)
        {
            nBuf[height * 2 + i] = nBuf[height * 2 - 1];
        }
#else
        memset (nBuf + height*2, nBuf[height*2 - 1], multiRefIdx*sizeof(PIXEL));
#endif
    }

    memcpy (nBuf + height*2 + multiRefIdx, nRawBuf + height*2 + (blockSize - (1 + multiRefIdx)), (width*2 + 1 + multiRefIdx)*sizeof(PIXEL));

    // Filter the neighbour pixels of the block specified.
    funcSet->pfXinFilterIntraNB (
        nBuf,
        nFltBuf,
        width,
        height,
        multiRefIdx);

    // Rearrange intra neighour buffer
    // BL: Bottom Left
    // L:  Left
    // TL: Top Left
    // T:  Top
    // TR: Top Right
    // BL BL BL BL L L L L TL T T T T TR TR TR TR --> TL T T T T TR TR TR TR TL L L L L BL BL BL BL
    for (i = height*2 + multiRefIdx, j = width*4+1+multiRefIdx; i >= 0; i--, j++)
    {
        nIntraBuf[0][j] = nBuf[i];
        nIntraBuf[1][j] = nFltBuf[i];
    }

    *((UINT16 *)(&nIntraBuf[0][width*2 + 1])) = nIntraBuf[0][width*2]*0x0101;
    *((UINT16 *)(&nIntraBuf[1][width*2 + 1])) = nIntraBuf[1][width*2]*0x0101;

    *((UINT16 *)(&nIntraBuf[0][width*4 + 1 + height*2 + 1])) = nIntraBuf[0][width*4 + 1 + height*2]*0x0101;
    *((UINT16 *)(&nIntraBuf[1][width*4 + 1 + height*2 + 1])) = nIntraBuf[1][width*4 + 1 + height*2]*0x0101;

}

void Xin266ExtractIntraNBChroma (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu)
{
    PIXEL    *srcU;
    PIXEL    *srcV;
    PIXEL    nRawBufU[132];
    PIXEL    nRawBufV[132];
    intptr_t srcStride;
    UINT64   intraAvail;
    SINT32   width;
    SINT32   height;
    UINT32   widthUnit;
    UINT32   heightUnit;
    SINT32   totalUnits;
    PIXEL    *nIntraBufU;
    PIXEL    *nIntraBufV;
    SINT32   idx;
    PIXEL    *nBufU;
    PIXEL    *nBufV;
    PIXEL    *dstU;
    PIXEL    *dstV;
    SINT32   i, j;
    UINT32   minBlkSize;
    UINT32   lgBlockSize;
    xin_seq_struct *seqSet;

    seqSet      = secSet->seqSet;
    srcU        = secSet->reconCu[PLANE_CHROMA_U];
    srcV        = secSet->reconCu[PLANE_CHROMA_V];
    nIntraBufU  = secSet->nIntraBufU;
    nIntraBufV  = secSet->nIntraBufV;
    srcStride   = secSet->reconUvStride;
    intraAvail  = secSet->intraAvailField;
    width       = pu->width;
    height      = pu->height;
    lgBlockSize = (width >= 8) && (height >= 8) ? 3 : seqSet->lgBlockSize;
    widthUnit   = width >> lgBlockSize;
    heightUnit  = height >> lgBlockSize;
    width     >>= 1;
    height    >>= 1;
    minBlkSize  = 1 << (lgBlockSize - 1);
    totalUnits  = widthUnit*2 + heightUnit*2 + 1;
    nBufU       = nIntraBufU - height*2;
    nBufV       = nIntraBufV - height*2;

    if (intraAvail == 0)
    {
        // None of neighbour is available
#ifdef ENABLE_10BIT_ENCODER
        for (i = 0; i < (SINT32)(width * 2 + height * 2 + minBlkSize); i++)
        {
            nRawBufU[i] = XIN_DEFAULT_PIXEL;
            nRawBufV[i] = XIN_DEFAULT_PIXEL;
        }
#else
        memset (nRawBufU, 128, sizeof(PIXEL)*(width*2+height*2+minBlkSize));
        memset (nRawBufV, 128, sizeof(PIXEL)*(width*2+height*2+minBlkSize));
#endif
    }
    else
    {
        // All neighbours are available
        dstU = nRawBufU + height*2;
        dstV = nRawBufV + height*2;

        memcpy (dstU, srcU - srcStride - minBlkSize, (width*2 + minBlkSize)*sizeof(PIXEL));
        memcpy (dstV, srcV - srcStride - minBlkSize, (width*2 + minBlkSize)*sizeof(PIXEL));

        for (i = 0; i < height*2; i++)
        {
            nRawBufU[i] = *(srcU + (height*2 - 1 - i)*srcStride - 1);
            nRawBufV[i] = *(srcV + (height*2 - 1 - i)*srcStride - 1);
        }

        if (intraAvail != (((UINT64)1 << totalUnits) - 1))
        {
            idx = 1;

            if (!(intraAvail & 1))
            {
                while (!((intraAvail >> idx) & 1))
                {
                    idx++;
                }

#ifdef ENABLE_10BIT_ENCODER
                for (i = 0; i < (SINT32)(idx*minBlkSize); i++)
                {
                    nRawBufU[i] = nRawBufU[idx << (lgBlockSize - 1)];
                    nRawBufV[i] = nRawBufV[idx << (lgBlockSize - 1)];
                }
#else
                memset (nRawBufU, nRawBufU[idx << (lgBlockSize - 1)], idx*minBlkSize);
                memset (nRawBufV, nRawBufV[idx << (lgBlockSize - 1)], idx*minBlkSize);
#endif
            }

            if (lgBlockSize == XIN_LOG_BLOCK_SIZE)
            {
                for (; idx < totalUnits; idx++)
                {
                    if (!((intraAvail >> idx) & 1))
                    {
#ifdef ENABLE_10BIT_ENCODER
                        UINT32 pelx2;

                        dstU  = nRawBufU + (idx << (lgBlockSize - 1));
                        pelx2 = dstU[-1];
                        pelx2 = pelx2*0x00010001;
                        *((UINT32 *)dstU) = pelx2;

                        dstV  = nRawBufV + (idx << (lgBlockSize - 1));
                        pelx2 = dstV[-1];
                        pelx2 = pelx2*0x00010001;
                        *((UINT32 *)dstV) = pelx2;
#else
                        UINT16 pelx2;

                        dstU  = nRawBufU + (idx << (lgBlockSize - 1));
                        pelx2 = dstU[-1];
                        pelx2 = pelx2*0x0101;
                        *((UINT16 *)dstU) = pelx2;

                        dstV  = nRawBufV + (idx << (lgBlockSize - 1));
                        pelx2 = dstV[-1];
                        pelx2 = pelx2*0x0101;
                        *((UINT16 *)dstV) = pelx2;
#endif
                    }
                }
            }
            else
            {
                for (; idx < totalUnits; idx++)
                {
                    if (!((intraAvail >> idx) & 1))
                    {
#ifdef ENABLE_10BIT_ENCODER
                        UINT64 pelx4;

                        dstU  = nRawBufU + (idx << (lgBlockSize - 1));
                        pelx4 = dstU[-1];
                        pelx4 = pelx4*0x0001000100010001;
                        *((UINT64 *)dstU) = pelx4;

                        dstV  = nRawBufV + (idx << (lgBlockSize - 1));
                        pelx4 = dstV[-1];
                        pelx4 = pelx4*0x0001000100010001;
                        *((UINT64 *)dstV) = pelx4;
#else
                        UINT32 pelx4;

                        dstU  = nRawBufU + (idx << (lgBlockSize - 1));
                        pelx4 = dstU[-1];
                        pelx4 = pelx4*0x01010101;
                        *((UINT32 *)dstU) = pelx4;

                        dstV  = nRawBufV + (idx << (lgBlockSize - 1));
                        pelx4 = dstV[-1];
                        pelx4 = pelx4*0x01010101;
                        *((UINT32 *)dstV) = pelx4;
#endif
                    }
                }
            }
        }
    }

    // Rearrange intra neighour buffer
    memcpy (nBufU, nRawBufU, height*2*sizeof(PIXEL));
    memcpy (nBufU + height*2, nRawBufU + height*2 + (minBlkSize - 1), (width*2 + 1)*sizeof(PIXEL));
    memcpy (nBufV, nRawBufV, height*2*sizeof(PIXEL));
    memcpy (nBufV + height*2, nRawBufV + height*2 + (minBlkSize - 1), (width*2 + 1)*sizeof(PIXEL));

    // BL: Bottom Left
    // L:  Left
    // TL: Top Left
    // T:  Top
    // TR: Top Right
    // BL BL BL BL L L L L TL T T T T TR TR TR TR --> TL T T T T TR TR TR TR TL L L L L BL BL BL BL
    for (i = height*2, j = width*4+1; i >= 0; i--, j++)
    {
        nIntraBufU[j] = nBufU[i];
        nIntraBufV[j] = nBufV[i];
    }

    *((UINT16 *)(&nIntraBufU[width*2 + 1])) = nIntraBufU[width*2]*0x0101;
    *((UINT16 *)(&nIntraBufV[width*2 + 1])) = nIntraBufV[width*2]*0x0101;

    *((UINT16 *)(&nIntraBufU[width*4 + 1 + height*2 + 1])) = nIntraBufU[width*4 + 1 + height*2]*0x0101;
    *((UINT16 *)(&nIntraBufV[width*4 + 1 + height*2 + 1])) = nIntraBufV[width*4 + 1 + height*2]*0x0101;

}

static inline SINT32 Xin266GetWideAngle (
    SINT32 lgWidth,
    SINT32 lgHeight,
    SINT32 predMode)
{
    SINT32 deltaSize;

    if ((predMode > XIN_DC_IDX) && (predMode <= XIN_VDIA_IDX))
    {
        deltaSize = XIN_ABS (lgWidth - lgHeight);

        if ((lgWidth > lgHeight) && (predMode < 2 + intraModeShift[deltaSize]))
        {
            predMode += (XIN_VDIA_IDX - 1);
        }
        else if ((lgHeight > lgWidth) && (predMode > XIN_VDIA_IDX - intraModeShift[deltaSize]))
        {
            predMode -= (XIN_VDIA_IDX - 1);
        }
    }

    return predMode;

}

// LumaRecPixels
void Xin266LoadLMLumaRec (
    xin_sec_struct  *secSet,
    PIXEL           *dst,
    intptr_t        dstStride,
    SINT32          cclmMode)
{
    UINT32         blockSize;
    UINT32         lgBlockSize;
    UINT32         cWidth;
    UINT32         cHeight;
    xin_seq_struct *seqSet;
    xin_cu_struct  *cu;
    xin_pu_struct  *pu;
    UINT32         topStart;
    UINT32         topCount;
    UINT32         lftStart;
    UINT32         lftCount;
    UINT32         idx;
    UINT32         i, j;
    UINT32         topNum;
    UINT32         lftNum;
    PIXEL          *dstRow;
    PIXEL          *src;
    PIXEL          *srcRow;
    intptr_t       srcStride;
    intptr_t       srcStride2;
    BOOL           is1stRowOfCtu;
    UINT64         intraAvail;

    seqSet        = secSet->seqSet;
    cu            = secSet->cu;
    pu            = &cu->pu;
    cWidth        = pu->width >> 1;
    cHeight       = pu->height >> 1;
    lgBlockSize   = (pu->width >= 8) && (pu->height >= 8) ? 3 : seqSet->lgBlockSize;
    blockSize     = 1 << lgBlockSize;
    topStart      = ((pu->height*2) >> lgBlockSize) + 1;
    topCount      = ((cclmMode == XIN_LM_CHROMA_L_IDX) || (cclmMode == XIN_LM_CHROMA_T_IDX)) ? (pu->width*2)>>lgBlockSize : (pu->width>>lgBlockSize);
    lftStart      = ((cclmMode == XIN_LM_CHROMA_L_IDX) || (cclmMode == XIN_LM_CHROMA_T_IDX)) ? 0 : (pu->height >> lgBlockSize);
    lftCount      = ((cclmMode == XIN_LM_CHROMA_L_IDX) || (cclmMode == XIN_LM_CHROMA_T_IDX)) ? (pu->height*2)>>lgBlockSize : (pu->height>>lgBlockSize);
    topNum        = 0;
    lftNum        = 0;
    is1stRowOfCtu = !cu->offY;
    srcStride     = secSet->reconYStride;
    srcStride2    = srcStride*2;
    src           = secSet->reconCu[PLANE_LUMA];
    intraAvail    = secSet->intraAvailField;

    for (idx = topStart; idx < topStart + topCount; idx++)
    {
        if ((intraAvail >> idx) & 1)
        {
            topNum++;
        }
    }

    for (idx = lftStart; idx < lftStart + lftCount; idx++)
    {
        if ((intraAvail >> idx) & 1)
        {
            lftNum++;
        }
    }

    if (topNum)
    {
        dstRow = dst - dstStride;

        for (idx = 0; idx < topNum*blockSize>>1; idx++)
        {
            const BOOL leftPadding = idx == 0 && !lftNum;

            if (is1stRowOfCtu)
            {
                srcRow = src - srcStride;
                dstRow[idx] = (srcRow[2 * idx] * 2 + srcRow[2 * idx - (leftPadding ? 0 : (intptr_t)1)] + srcRow[2 * idx + 1] + 2) >> 2;
            }
            else if (seqSet->config.verCollocatedChroma)
            {
                srcRow = src - srcStride2;

                int s = 4;
                s += srcRow[2 * idx - srcStride];
                s += srcRow[2 * idx] * 4;
                s += srcRow[2 * idx - (leftPadding ? 0 : (intptr_t)1)];
                s += srcRow[2 * idx + 1];
                s += srcRow[2 * idx + srcStride];
                dstRow[idx] = (PIXEL)(s >> 3);
            }
            else
            {
                srcRow = src - srcStride2;

                int s = 4;
                s += srcRow[2 * idx] * 2;
                s += srcRow[2 * idx + 1];
                s += srcRow[2 * idx - (leftPadding ? 0 : (intptr_t)1)];
                s += srcRow[2 * idx + srcStride] * 2;
                s += srcRow[2 * idx + 1 + srcStride];
                s += srcRow[2 * idx + srcStride - (leftPadding ? 0 : (intptr_t)1)];
                dstRow[idx] = (PIXEL)(s >> 3);
            }

        }

    }

    if (lftNum)
    {
        dstRow = dst - 1;
        srcRow = src - 1 - 1;

        for (idx = 0; idx < (lftNum*blockSize>>1); idx++)
        {
            if (seqSet->config.verCollocatedChroma)
            {
                const BOOL abovePadding = idx == 0 && !topNum;

                int s = 4;
                s += srcRow[-(abovePadding ? 0 : srcStride)];
                s += srcRow[0] * 4;
                s += srcRow[-1];
                s += srcRow[1];
                s += srcRow[srcStride];
                dstRow[0] = (PIXEL)(s >> 3);
            }
            else
            {
                int s = 4;
                s += srcRow[0] * 2;
                s += srcRow[1];
                s += srcRow[-1];
                s += srcRow[srcStride] * 2;
                s += srcRow[srcStride + 1];
                s += srcRow[srcStride - 1];
                dstRow[0] = (PIXEL)(s >> 3);
            }

            srcRow += srcStride2;
            dstRow  += dstStride;

        }

    }

    // inner part from reconstructed picture buffer
    for (j = 0; j < cHeight; j++ )
    {
        for (i = 0; i < cWidth; i++ )
        {
            if (seqSet->config.verCollocatedChroma)
            {
                const BOOL leftPadding  = i == 0 && !lftNum;
                const BOOL abovePadding = j == 0 && !topNum;

                int s = 4;
                s += src[2 * i - (abovePadding ? 0 : srcStride)];
                s += src[2 * i] * 4;
                s += src[2 * i - (leftPadding ? 0 : 1)];
                s += src[2 * i + 1];
                s += src[2 * i + srcStride];
                dst[i] = (PIXEL)(s >> 3);
            }
            else
            {
                const BOOL leftPadding = i == 0 && !lftNum;

                int s = 4;
                s += src[2 * i] * 2;
                s += src[2 * i + 1];
                s += src[2 * i - (leftPadding ? 0 : (intptr_t)1)];
                s += src[2 * i + srcStride] * 2;
                s += src[2 * i + 1 + srcStride];
                s += src[2 * i + srcStride - (leftPadding ? 0 : (intptr_t)1)];
                dst[i] = (PIXEL)(s >> 3);
            }
        }

        dst += dstStride;
        src += srcStride2;

    }

}

void Xin266GetLMParameter (
    xin_sec_struct  *secSet,
    PIXEL           *rec,
    intptr_t        recStride,
    UINT32          compIdx,
    SINT32          cclmMode,
    SINT32          *a,
    SINT32          *b,
    SINT32          *shift)
{

    BOOL aboveAvailable, leftAvailable;

    int minLuma[2] = { XIN_MAX_S32, 0 };
    int maxLuma[2] = { XIN_MIN_S32, 0 };
    int actualTopTemplateSampNum = 0;
    int actualLeftTemplateSampNum = 0;
    SINT32         width;
    SINT32         height;
    SINT32         cWidth;
    SINT32         cHeight;
    SINT32         blockSize;
    SINT32         lgBlockSize;
    xin_seq_struct *seqSet;
    xin_cu_struct  *cu;
    SINT32         idx;
    SINT32         availStart;
    SINT32         availEnd;
    SINT32         avaiAboveUnits = 0;
    SINT32         avaiLeftUnits = 0;
    SINT32         avaiAboveRightUnits;
    SINT32         avaiLeftBelowUnits;
    PIXEL          *nIntraBuf;
    PIXEL          *src;
    UINT64         intraAvail;

    cu          = secSet->cu;
    width       = cu->pu.width;
    height      = cu->pu.height;
    cWidth      = width >> 1;
    cHeight     = height >> 1;
    seqSet      = secSet->seqSet;
    lgBlockSize = (width >= 8) && (height >= 8) ? 3 : seqSet->lgBlockSize;
    blockSize   = 1 << lgBlockSize;
    nIntraBuf   = (compIdx == PLANE_CHROMA_U) ? secSet->nIntraBufU : secSet->nIntraBufV;
    intraAvail  = secSet->intraAvailField;

    avaiAboveRightUnits = 0;
    avaiLeftBelowUnits  = 0;

    leftAvailable  = (intraAvail >> (((height * 2) >> lgBlockSize) - 1)) & 1;
    aboveAvailable = (intraAvail >> (((height * 2) >> lgBlockSize) + 1)) & 1;

    if (cclmMode == XIN_LM_CHROMA_T_IDX)
    {
        leftAvailable  = 0;
        avaiAboveUnits = aboveAvailable ? width >> lgBlockSize : 0;

        availStart = ((height*2 + width) >> lgBlockSize) + 1;
        availEnd   = ((height*2 + 2*width) >> lgBlockSize);

        for (idx = availStart; idx <= availEnd; idx++)
        {
            if ((intraAvail >> idx) & 1)
            {
                avaiAboveRightUnits++;
            }

        }

        avaiAboveRightUnits      = avaiAboveRightUnits > (height>>lgBlockSize) ?  height>>lgBlockSize : avaiAboveRightUnits;
        actualTopTemplateSampNum = (blockSize>>1)*(avaiAboveUnits + avaiAboveRightUnits);

    }
    else if (cclmMode == XIN_LM_CHROMA_L_IDX)
    {
        aboveAvailable = 0;
        avaiLeftUnits = leftAvailable ? height >> lgBlockSize : 0;

        availStart = 0;
        availEnd   = (height >> lgBlockSize) - 1;

        for (idx = availStart; idx <= availEnd; idx++)
        {
            if ((intraAvail >> idx) & 1)
            {
                avaiLeftBelowUnits++;
            }

        }

        avaiLeftBelowUnits        = avaiLeftBelowUnits > (width>>lgBlockSize) ? width>>lgBlockSize : avaiLeftBelowUnits;
        actualLeftTemplateSampNum = (blockSize>>1)*(avaiLeftUnits + avaiLeftBelowUnits);

    }
    else if (cclmMode == XIN_LM_CHROMA_IDX)
    {
        actualTopTemplateSampNum = cWidth;
        actualLeftTemplateSampNum = cHeight;
    }

    int startPos[2]; //0:Above, 1: Left
    int pickStep[2];

    int aboveIs4 = leftAvailable  ? 0 : 1;
    int leftIs4 =  aboveAvailable ? 0 : 1;

    startPos[0] = actualTopTemplateSampNum >> (2 + aboveIs4);
    pickStep[0] = XIN_MAX (1, actualTopTemplateSampNum >> (1 + aboveIs4));

    startPos[1] = actualLeftTemplateSampNum >> (2 + leftIs4);
    pickStep[1] = XIN_MAX (1, actualLeftTemplateSampNum >> (1 + leftIs4));

    PIXEL selectLumaPix[4] = { 0, 0, 0, 0 };
    PIXEL selectChromaPix[4] = { 0, 0, 0, 0 };

    int cntT, cntL;
    cntT = cntL = 0;
    int cnt;
    int pos;

    if (aboveAvailable)
    {
        cntT = XIN_MIN(actualTopTemplateSampNum, (1 + aboveIs4) << 1);
        src = rec - recStride;
        const PIXEL *cur = nIntraBuf + 1;
        for (pos = startPos[0], cnt = 0; cnt < cntT; pos += pickStep[0], cnt++)
        {
            selectLumaPix[cnt] = src[pos];
            selectChromaPix[cnt] = cur[pos];
        }
    }

    if (leftAvailable)
    {
        cntL = XIN_MIN(actualLeftTemplateSampNum, ( 1 + leftIs4 ) << 1 );
        src = rec - 1;
        const PIXEL *cur = nIntraBuf + 4*cWidth + 1 + 1;
        for (pos = startPos[1], cnt = 0; cnt < cntL; pos += pickStep[1], cnt++)
        {
            selectLumaPix[cnt + cntT] = src[pos * recStride];
            selectChromaPix[cnt + cntT] = cur[pos];
        }
    }

    cnt = cntL + cntT;

    if (cnt == 2)
    {
        selectLumaPix[3] = selectLumaPix[0];
        selectChromaPix[3] = selectChromaPix[0];
        selectLumaPix[2] = selectLumaPix[1];
        selectChromaPix[2] = selectChromaPix[1];
        selectLumaPix[0] = selectLumaPix[1];
        selectChromaPix[0] = selectChromaPix[1];
        selectLumaPix[1] = selectLumaPix[3];
        selectChromaPix[1] = selectChromaPix[3];
    }

    int minGrpIdx[2] = { 0, 2 };
    int maxGrpIdx[2] = { 1, 3 };
    int *tmpMinGrp = minGrpIdx;
    int *tmpMaxGrp = maxGrpIdx;
    if (selectLumaPix[tmpMinGrp[0]] > selectLumaPix[tmpMinGrp[1]]) XIN_SWAP (int, tmpMinGrp[0], tmpMinGrp[1]);
    if (selectLumaPix[tmpMaxGrp[0]] > selectLumaPix[tmpMaxGrp[1]]) XIN_SWAP (int, tmpMaxGrp[0], tmpMaxGrp[1]);
    if (selectLumaPix[tmpMinGrp[0]] > selectLumaPix[tmpMaxGrp[1]]) XIN_SWAP (int*, tmpMinGrp, tmpMaxGrp);
    if (selectLumaPix[tmpMinGrp[1]] > selectLumaPix[tmpMaxGrp[0]]) XIN_SWAP (int, tmpMinGrp[1], tmpMaxGrp[0]);

    minLuma[0] = (selectLumaPix[tmpMinGrp[0]] + selectLumaPix[tmpMinGrp[1]] + 1 )>>1;
    minLuma[1] = (selectChromaPix[tmpMinGrp[0]] + selectChromaPix[tmpMinGrp[1]] + 1) >> 1;
    maxLuma[0] = (selectLumaPix[tmpMaxGrp[0]] + selectLumaPix[tmpMaxGrp[1]] + 1 )>>1;
    maxLuma[1] = (selectChromaPix[tmpMaxGrp[0]] + selectChromaPix[tmpMaxGrp[1]] + 1) >> 1;

    if (leftAvailable || aboveAvailable)
    {
        int diff = maxLuma[0] - minLuma[0];
        if (diff > 0)
        {
            int diffC = maxLuma[1] - minLuma[1];
            int x = calcLog2[diff];
            static const uint8_t DivSigTable[1 << 4] =
            {
                // 4bit significands - 8 ( MSB is omitted )
                0,  7,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  1,  1,  0
            };
            int normDiff = (diff << 4 >> x) & 15;
            int v = DivSigTable[normDiff] | 8;
            x += normDiff != 0;

            int y = calcLog2[XIN_ABS(diffC)] + 1;
            int add = 1 << y >> 1;
            *a = (diffC * v + add) >> y;
            *shift = 3 + x - y;
            if ( *shift < 1 )
            {
                *shift = 1;
                *a = ( (*a == 0)? 0: (*a < 0)? -15 : 15 );   // a=Sign(a)*15
            }
            *b = minLuma[1] - ((*a * minLuma[0]) >> *shift);
        }
        else
        {
            *a = 0;
            *b = minLuma[1];
            *shift = 0;
        }
    }
    else
    {
        *a = 0;
        *b = 1 << (XIN_INTERNAL_BIT_DEPTH - 1);
        *shift = 0;
    }

}

void Xin266LinearTransform (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   scale,
    SINT32   offset,
    SINT32   shift,
    UINT32   width,
    UINT32   height)
{
    UINT32  colIdx;
    UINT32  rowIdx;
    SINT32  pelVal;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            pelVal = ((src[rowIdx*srcStride + colIdx]*scale) >> shift) + offset;
            pelVal = XIN_CLIP (pelVal, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);

            dst[rowIdx*dstStride + colIdx] = (PIXEL)pelVal;
        }
    }

}

void Xin266ExtendIntraRef (
    PIXEL  *refMain,
    PIXEL  *refSide,
    SINT32 intraPredAngleMode,
    SINT32 width,
    SINT32 height,
    BOOL   isModeVer)
{
    SINT32 sizeSide;
    SINT32 invAngle;

    assert (intraPredAngleMode < 0);

    sizeSide = isModeVer ? height : width;
    invAngle = invIntraAngTable[-intraPredAngleMode];

    // Extend the Main reference to the left.
    for (int k = -sizeSide; k <= -1; k++)
    {
        refMain[k] = refSide[XIN_MIN((-k * invAngle + 256) >> 9, sizeSide)];
    }

}

void Xin266IntraPredAng (
    xin_sec_struct *secSet,
    PIXEL          *dst,
    intptr_t       dstStride,
    SINT32         predMode,
    SINT32         multiRefIdx,
    UINT32         lgWidth,
    UINT32         lgHeight)
{
    xin_func_struct *funcSet;
    SINT32          width;
    SINT32          height;
    PIXEL           predBuf[64*64];
    PIXEL           *pred;
    intptr_t        predStride;
    PIXEL           *nBuf;
    PIXEL           *refAbove;
    PIXEL           *refLeft;
    PIXEL           *refMain;
    PIXEL           *refSide;
    SINT32          intraPredAngleMode;
    BOOL            isModeVer;
    SINT32          absAng;
    SINT32          absAngMode;
    SINT32          signAng;
    SINT32          intraPredAngle;
    BOOL            applyPDPC;
    SINT32          lgSideSize;
    SINT32          angularScale;
    SINT32          log2Size;
    BOOL            filterFlag;
    BOOL            isRefFilter;
    BOOL            interpFlag;

    width              = 1 << lgWidth;
    height             = 1 << lgHeight;
    predStride         = dstStride;
    pred               = dst;
    angularScale       = 0;
    isRefFilter        = FALSE;
    interpFlag         = FALSE;
    isModeVer          = predMode >= XIN_DIA_IDX;
    intraPredAngleMode = (isModeVer) ? (predMode - XIN_VER_IDX) : -(predMode - XIN_HOR_IDX);
    absAngMode         = XIN_ABS (intraPredAngleMode);
    signAng            = intraPredAngleMode < 0 ? -1 : 1;
    absAng             = intraAngTable[absAngMode];
    intraPredAngle     = signAng * absAng;
    applyPDPC          = FALSE;
    funcSet            = secSet->funcSet;

    if (applyPDPC)
    {
        if (intraPredAngleMode < 0)
        {
            applyPDPC = FALSE;
        }
        else if (intraPredAngleMode > 0)
        {
            lgSideSize   = isModeVer ? lgHeight : lgWidth;
            angularScale = XIN_MIN (2, lgSideSize - preScale[absAngMode]);
            applyPDPC   &= angularScale >= 0;
        }
    }

    // high level conditions and DC intra prediction
    if (multiRefIdx)
    {

    }
    else // HOR, VER and angular modes (MDIS)
    {
        log2Size   = ((lgWidth + lgHeight) >> 1);
        filterFlag = (absAngMode > intraFilter[log2Size]);

        // Selelection of either ([1 2 1] / 4 ) refrence filter OR Gaussian 4-tap interpolation filter
        if (filterFlag)
        {
            isRefFilter = !(absAng & 0x1F);
            interpFlag  = !isRefFilter;
        }
    }

    if (isRefFilter)
    {
        nBuf = secSet->nIntraFltBufY;
    }
    else
    {
        nBuf = secSet->nIntraBufY;
    }

    refAbove = nBuf;
    refLeft  = nBuf + 1 + multiRefIdx + width*4;
    refMain  = isModeVer ? refAbove : refLeft;
    refSide  = isModeVer ? refLeft : refAbove;

    if (intraPredAngleMode < 0)
    {
        funcSet->pfXinExtendIntraRef (
            refMain,
            refSide,
            intraPredAngleMode,
            width,
            height,
            isModeVer);
    }

    // swap width/height if we are doing a horizontal mode:
    if (!isModeVer)
    {
        XIN_SWAP (SINT32, width,   height);
        XIN_SWAP (UINT32, lgWidth, lgHeight);

        predStride = width;
        pred       = predBuf;
    }

    funcSet->pfXinLumaIntraFilter[lgWidth] (
        pred,
        predStride,
        interpFlag,
        intraPredAngle,
        multiRefIdx,
        refMain,
        width,
        height);

    // Flip the block if this is the horizontal mode
    if (!isModeVer)
    {
        XIN_SWAP (SINT32, width,   height);
        XIN_SWAP (UINT32, lgWidth, lgHeight);

        funcSet->pfXinBlockTranspose (
            predBuf,
            predStride,
            dst,
            dstStride,
            width,
            height);
    }

    if (applyPDPC)
    {
        funcSet->pfXinApplyAngPDPC[isModeVer][lgWidth] (
            dst,
            dstStride,
            absAngMode,
            angularScale,
            refSide,
            width,
            height);
    }

}

void Xin266IntraPred (
    xin_sec_struct *secSet,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         mode,
    SINT32         multiRefIdx,
    UINT32         lgWidth,
    UINT32         lgHeight)
{
    PIXEL           *nBuf;
    SINT32          predMode;
    BOOL            refFilterFlag;
    BOOL            applyPDPC;
    BOOL            isModeVer;
    xin_func_struct *funcSet;

    funcSet       = secSet->funcSet;
    predMode      = Xin266GetWideAngle (lgWidth, lgHeight, mode);
    isModeVer     = predMode >= XIN_DIA_IDX;
    applyPDPC     = multiRefIdx == 0;
    nBuf          = NULL;

    if (mode == XIN_INTRA_DC)
    {
        nBuf = secSet->nIntraBufY;

        funcSet->pfXinIntraPredDc[lgWidth] (
            pred,
            predStride,
            nBuf,
            multiRefIdx,
            lgWidth,
            lgHeight);

    }
    else if (mode == XIN_INTRA_PLANAR)
    {
        refFilterFlag = (lgWidth + lgHeight) > 5 ? TRUE : FALSE;
        nBuf          = refFilterFlag ? secSet->nIntraFltBufY : secSet->nIntraBufY;

        funcSet->pfXinIntraPredPlanar[lgWidth] (
            pred,
            predStride,
            nBuf,
            multiRefIdx,
            lgWidth,
            lgHeight);
    }
    else if (mode == XIN_INTRA_HOR)
    {
        nBuf = secSet->nIntraBufY;

        funcSet->pfXinIntraPredHor[lgWidth] (
            pred,
            predStride,
            nBuf,
            multiRefIdx,
            lgWidth,
            lgHeight);

    }
    else if (mode == XIN_INTRA_VER)
    {
        nBuf = secSet->nIntraBufY;

        funcSet->pfXinIntraPredVer[lgWidth] (
            pred,
            predStride,
            nBuf,
            multiRefIdx,
            lgWidth,
            lgHeight);
    }
    else
    {
        Xin266IntraPredAng (
            secSet,
            pred,
            predStride,
            predMode,
            multiRefIdx,
            lgWidth,
            lgHeight);
    }

    if (applyPDPC)
    {
        if ((mode == XIN_INTRA_PLANAR) || (mode == XIN_INTRA_DC))
        {
            funcSet->pfXinApplyPDPC[lgWidth] (
                pred,
                predStride,
                nBuf,
                lgWidth,
                lgHeight);
        }
    }

}

void Xin266IntraPredAngChroma (
    xin_sec_struct *secSet,
    PIXEL          *dst,
    intptr_t       dstStride,
    PIXEL          *nBuf,
    SINT32         predMode,
    UINT32         lgWidth,
    UINT32         lgHeight)
{
    xin_func_struct *funcSet;
    SINT32          width;
    SINT32          height;
    PIXEL           predBuf[32*32];
    PIXEL           *pred;
    intptr_t        predStride;
    PIXEL           *refAbove;
    PIXEL           *refLeft;
    PIXEL           *refMain;
    PIXEL           *refSide;
    SINT32          intraPredAngleMode;
    BOOL            isModeVer;
    SINT32          absAng;
    SINT32          absAngMode;
    SINT32          signAng;
    SINT32          intraPredAngle;
    BOOL            applyPDPC;
    SINT32          lgSideSize;
    SINT32          angularScale;
    SINT32          sizeSide;

    width              = 1 << lgWidth;
    height             = 1 << lgHeight;
    predStride         = dstStride;
    pred               = dst;
    isModeVer          = predMode >= XIN_DIA_IDX;
    angularScale       = 0;
    intraPredAngleMode = (isModeVer) ? (predMode - XIN_VER_IDX) : -(predMode - XIN_HOR_IDX);
    absAngMode         = XIN_ABS(intraPredAngleMode);
    signAng            = intraPredAngleMode < 0 ? -1 : 1;
    absAng             = intraAngTable[absAngMode];
    intraPredAngle     = signAng * absAng;
    sizeSide           = isModeVer ? height : width;
    applyPDPC          = TRUE;
    funcSet            = secSet->funcSet;

    if (applyPDPC)
    {
        if (intraPredAngleMode < 0)
        {
            applyPDPC = FALSE;
        }
        else if (intraPredAngleMode > 0)
        {
            lgSideSize   = isModeVer ? lgHeight : lgWidth;
            angularScale = XIN_MIN (2, lgSideSize - preScale[absAngMode]);
            applyPDPC   &= angularScale >= 0;
        }
    }

    refAbove = nBuf;
    refLeft  = nBuf + 1 + width*4;
    refMain  = isModeVer ? refAbove : refLeft;
    refSide  = isModeVer ? refLeft : refAbove;

    funcSet->pfXinExtendIntraRef (
        refMain,
        refSide,
        intraPredAngleMode,
        width,
        height,
        isModeVer);

    // swap width/height if we are doing a horizontal mode:
    if (!isModeVer)
    {
        XIN_SWAP (SINT32, width,   height);
        XIN_SWAP (UINT32, lgWidth, lgHeight);

        predStride = width;
        pred       = predBuf;
    }

    funcSet->pfXinChromaIntraFilter[lgWidth] (
        pred,
        predStride,
        intraPredAngle,
        refMain,
        width,
        height);

    // Flip the block if this is the horizontal mode
    if (!isModeVer)
    {
        XIN_SWAP (SINT32, width,   height);
        XIN_SWAP (UINT32, lgWidth, lgHeight);

        funcSet->pfXinBlockTranspose (
            predBuf,
            predStride,
            dst,
            dstStride,
            width,
            height);
    }

    if (applyPDPC)
    {
        funcSet->pfXinApplyAngPDPC[isModeVer][lgWidth] (
            dst,
            dstStride,
            absAngMode,
            angularScale,
            refSide,
            width,
            height);
    }

}

void Xin266IntraPredChroma (
    xin_sec_struct *secSet,
    PIXEL           *pred,
    intptr_t        predStride,
    UINT32          compIdx,
    SINT32          mode,
    UINT32          lgWidth,
    UINT32          lgHeight)
{
    PIXEL           *nIntraBuf;
    SINT32          predMode;
    BOOL            applyPDPC;
    xin_func_struct *funcSet;
    PIXEL           *rec;
    intptr_t        recStride;

    funcSet       = secSet->funcSet;
    nIntraBuf     = (compIdx == PLANE_CHROMA_U) ? secSet->nIntraBufU : secSet->nIntraBufV;
    predMode      = Xin266GetWideAngle (lgWidth, lgHeight, mode);
    applyPDPC     = (lgWidth >= XIN_MIN_LG_TU_SIZE) && (lgHeight >= XIN_MIN_LG_TU_SIZE);
    rec           = (PIXEL *)(secSet->tempBuffer + 1000);
    recStride     = XIN_MAX_TU_SIZE * 2 + 1;

    if (mode == XIN_INTRA_DC)
    {
        funcSet->pfXinIntraPredDc[lgWidth] (
            pred,
            predStride,
            nIntraBuf,
            0,
            lgWidth,
            lgHeight);
    }
    else if (mode == XIN_INTRA_PLANAR)
    {
        funcSet->pfXinIntraPredPlanar[lgWidth] (
            pred,
            predStride,
            nIntraBuf,
            0,
            lgWidth,
            lgHeight);
    }
    else if (mode == XIN_INTRA_HOR)
    {
        funcSet->pfXinIntraPredHor[lgWidth] (
            pred,
            predStride,
            nIntraBuf,
            0,
            lgWidth,
            lgHeight);
    }
    else if (mode == XIN_INTRA_VER)
    {
        funcSet->pfXinIntraPredVer[lgWidth] (
            pred,
            predStride,
            nIntraBuf,
            0,
            lgWidth,
            lgHeight);
    }
    else if ((mode >= XIN_LM_CHROMA_IDX) && (mode <= XIN_LM_CHROMA_T_IDX))
    {
        SINT32  paraA, paraB;
        SINT32  shift;

        Xin266GetLMParameter (
            secSet,
            rec,
            recStride,
            compIdx,
            mode,
            &paraA,
            &paraB,
            &shift);

        funcSet->pfXinLinearTransform[lgWidth] (
            rec,
            recStride,
            pred,
            predStride,
            paraA,
            paraB,
            shift,
            1 << lgWidth,
            1 << lgHeight);

    }
    else
    {
        Xin266IntraPredAngChroma (
            secSet,
            pred,
            predStride,
            nIntraBuf,
            predMode,
            lgWidth,
            lgHeight);
    }

    if (applyPDPC)
    {
        if ((mode == XIN_INTRA_PLANAR) || (mode == XIN_INTRA_DC))
        {
            funcSet->pfXinApplyPDPC[lgWidth] (
                pred,
                predStride,
                nIntraBuf,
                lgWidth,
                lgHeight);
        }
        else if (mode == XIN_INTRA_HOR)
        {
            funcSet->pfXinApplyHorPDPC[lgWidth] (
                pred,
                predStride,
                nIntraBuf,
                lgWidth,
                lgHeight);
        }
        else if (mode == XIN_INTRA_VER)
        {
            funcSet->pfXinApplyVerPDPC[lgWidth] (
                pred,
                predStride,
                nIntraBuf,
                lgWidth,
                lgHeight);
        }

    }

}

void Xin266ApplyAngPDPC (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_func_struct *funcSet;
    SINT32          predMode;
    BOOL            isModeVer;
    SINT32          intraPredAngleMode;
    SINT32          absAngMode;
    SINT32          lgWidth;
    SINT32          lgHeight;
    PIXEL           *pred;
    intptr_t        predStride;
    intptr_t        predPos;
    PIXEL           *refAbove;
    PIXEL           *refLeft;
    PIXEL           *refSide;
    PIXEL           *nIntraBuf;
    SINT32          lgSideSize;
    SINT32          angularScale;
    BOOL            applyPDPC;
    SINT32          log2Size;
    SINT32          filterFlag;
    SINT32          absAng;

    lgWidth    = cu->lgWidth;
    lgHeight   = cu->lgHeight;
    predMode   = fastBuf->intraLumaMode;
    predStride = fastBuf->lumaStride;
    predPos    = cu->offX + cu->offY*predStride;
    pred       = fastBuf->predBuf[PLANE_LUMA] + predPos;
    nIntraBuf  = secSet->nIntraBufY;
    funcSet    = secSet->funcSet;
    
    if (predMode == XIN_INTRA_VER)
    {
        funcSet->pfXinApplyVerPDPC[lgWidth] (
            pred,
            predStride,
            nIntraBuf,
            lgWidth,
            lgHeight);

    }
    else if (predMode == XIN_INTRA_HOR)
    {
        funcSet->pfXinApplyHorPDPC[lgWidth] (
            pred,
            predStride,
            nIntraBuf,
            lgWidth,
            lgHeight);
    }
    else
    {
        predMode           = Xin266GetWideAngle (lgWidth, lgHeight, predMode);
        isModeVer          = predMode >= XIN_DIA_IDX;
        intraPredAngleMode = (isModeVer) ? (predMode - XIN_VER_IDX) : -(predMode - XIN_HOR_IDX);
        absAngMode         = XIN_ABS (intraPredAngleMode);
        angularScale       = 0;

        if (intraPredAngleMode < 0)
        {
            applyPDPC = FALSE;
        }
        else
        {
            lgSideSize   = isModeVer ? lgHeight : lgWidth;
            angularScale = XIN_MIN (2, lgSideSize - preScale[absAngMode]);
            applyPDPC    = angularScale >= 0;
        }

        if (!applyPDPC)
        {
            return;
        }

        absAng     = intraAngTable[absAngMode];
        log2Size   = ((lgWidth + lgHeight) >> 1);
        filterFlag = (absAngMode > intraFilter[log2Size]);
        filterFlag = filterFlag ? !(absAng & 0x1F) : FALSE;
        refAbove   = filterFlag ? secSet->nIntraFltBufY : nIntraBuf;
        refLeft    = refAbove + 1 + (1 << lgWidth)*4;
        refSide    = isModeVer ? refLeft : refAbove;

        funcSet->pfXinApplyAngPDPC[isModeVer][lgWidth] (
            pred,
            predStride,
            absAngMode,
            angularScale,
            refSide,
            1 << lgWidth,
            1 << lgHeight);
        
    }

}

void Xin266ApplyNonAngPDPC (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_func_struct *funcSet;
    SINT32          predMode;
    SINT32          lgWidth;
    SINT32          lgHeight;
    PIXEL           *pred;
    intptr_t        predStride;
    intptr_t        predPos;
    PIXEL           *nBuf;
    BOOL            refFilterFlag;

    lgWidth   = cu->lgWidth;
    lgHeight  = cu->lgHeight;
    predMode  = fastBuf->intraLumaMode;

    if (predMode == XIN_INTRA_DC)
    {
        nBuf = secSet->nIntraBufY;
    }
    else
    {
        refFilterFlag = (lgWidth + lgHeight) > 5 ? TRUE : FALSE;
        nBuf          = refFilterFlag ? secSet->nIntraFltBufY : secSet->nIntraBufY;
    }

    funcSet    = secSet->funcSet;
    predStride = fastBuf->lumaStride;
    predPos    = cu->offX + cu->offY*predStride;
    pred       = fastBuf->predBuf[0] + predPos;

    funcSet->pfXinApplyPDPC[lgWidth] (
        pred,
        predStride,
        nBuf,
        lgWidth,
        lgHeight);

}

