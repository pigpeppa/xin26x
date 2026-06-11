/***************************************************************************//**
 *
 * @file          h266_alf.c
 * @brief         h266 ALF application.
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
#include "basic_macro.h"
#include "h266_alf_context.h"
#include "h26x_definition.h"

static inline int alfClip (
    int clip,
    int ref,
    int val0,
    int val1)
{
    int clip1 = XIN_CLIP (val0 - ref, -clip, clip);
    int clip2 = XIN_CLIP (val1 - ref, -clip, clip);

    return clip1 + clip2;
}

void Xin266AlfBlockLuma (
    PIXEL         *src,
    intptr_t      srcStride,
    PIXEL         *dst,
    intptr_t      dstStride,
    SINT16        *filterSet,
    SINT16        *fClipSet,
    xin_alf_class *alfClass,
    intptr_t      alfClassStride,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight)
{
    SINT32        i, j;
    SINT32        clsSizeY;
    SINT32        clsSizeX;
    UINT8         transposeIdx;
    UINT8         classIdx;
    xin_alf_class *curClass;
    SINT16        *fltCoeff;
    SINT16        *fltClip;
    SINT32        ii, jj;
    PIXEL         *pImg0, *pImg1, *pImg2, *pImg3, *pImg4, *pImg5, *pImg6;
    PIXEL         *pRec0, *pRec1;
    intptr_t      dstStride2;
    intptr_t      srcStride2;
    SINT32        shift;
    SINT32        offset;
    SINT32        filterCoeff[XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32        filterClipp[XIN_ALF_MAX_LUMA_COEF_NUM];

    clsSizeX   = 4;
    clsSizeY   = 4;
    pRec0      = dst;
    pRec1      = pRec0 + dstStride;
    dstStride2 = dstStride*clsSizeY;
    srcStride2 = srcStride*clsSizeY;
    shift      = 8 - 1;
    offset     = 1 << (shift - 1);

    for (i = 0; i < height; i += clsSizeY)
    {
        for (j = 0; j < width; j += clsSizeX)
        {
            curClass     = alfClass + (i>>2) * alfClassStride + (j>>2);
            transposeIdx = curClass->transposeIdx;
            classIdx     = curClass->classIdx;

            fltCoeff = filterSet + classIdx * XIN_ALF_MAX_LUMA_COEF_NUM;
            fltClip  = fClipSet + classIdx * XIN_ALF_MAX_LUMA_COEF_NUM;

            if( transposeIdx == 1 )
            {
                filterCoeff[0]  = fltCoeff[9];
                filterCoeff[1]  = fltCoeff[4];
                filterCoeff[2]  = fltCoeff[10];
                filterCoeff[3]  = fltCoeff[8];
                filterCoeff[4]  = fltCoeff[1];
                filterCoeff[5]  = fltCoeff[5];
                filterCoeff[6]  = fltCoeff[11];
                filterCoeff[7]  = fltCoeff[7];
                filterCoeff[8]  = fltCoeff[3];
                filterCoeff[9]  = fltCoeff[0];
                filterCoeff[10] = fltCoeff[2];
                filterCoeff[11] = fltCoeff[6];
                filterCoeff[12] = fltCoeff[12];

                filterClipp[0]  = fltClip[9];
                filterClipp[1]  = fltClip[4];
                filterClipp[2]  = fltClip[10];
                filterClipp[3]  = fltClip[8];
                filterClipp[4]  = fltClip[1];
                filterClipp[5]  = fltClip[5];
                filterClipp[6]  = fltClip[11];
                filterClipp[7]  = fltClip[7];
                filterClipp[8]  = fltClip[3];
                filterClipp[9]  = fltClip[0];
                filterClipp[10] = fltClip[2];
                filterClipp[11] = fltClip[6];
                filterClipp[12] = fltClip[12];

            }
            else if( transposeIdx == 2 )
            {
                filterCoeff[0]  = fltCoeff[0];
                filterCoeff[1]  = fltCoeff[3];
                filterCoeff[2]  = fltCoeff[2];
                filterCoeff[3]  = fltCoeff[1];
                filterCoeff[4]  = fltCoeff[8];
                filterCoeff[5]  = fltCoeff[7];
                filterCoeff[6]  = fltCoeff[6];
                filterCoeff[7]  = fltCoeff[5];
                filterCoeff[8]  = fltCoeff[4];
                filterCoeff[9]  = fltCoeff[9];
                filterCoeff[10] = fltCoeff[10];
                filterCoeff[11] = fltCoeff[11];
                filterCoeff[12] = fltCoeff[12];

                filterClipp[0]  = fltClip[0];
                filterClipp[1]  = fltClip[3];
                filterClipp[2]  = fltClip[2];
                filterClipp[3]  = fltClip[1];
                filterClipp[4]  = fltClip[8];
                filterClipp[5]  = fltClip[7];
                filterClipp[6]  = fltClip[6];
                filterClipp[7]  = fltClip[5];
                filterClipp[8]  = fltClip[4];
                filterClipp[9]  = fltClip[9];
                filterClipp[10] = fltClip[10];
                filterClipp[11] = fltClip[11];
                filterClipp[12] = fltClip[12];

            }
            else if( transposeIdx == 3 )
            {
                filterCoeff[0]  = fltCoeff[9];
                filterCoeff[1]  = fltCoeff[8];
                filterCoeff[2]  = fltCoeff[10];
                filterCoeff[3]  = fltCoeff[4];
                filterCoeff[4]  = fltCoeff[3];
                filterCoeff[5]  = fltCoeff[7];
                filterCoeff[6]  = fltCoeff[11];
                filterCoeff[7]  = fltCoeff[5];
                filterCoeff[8]  = fltCoeff[1];
                filterCoeff[9]  = fltCoeff[0];
                filterCoeff[10] = fltCoeff[2];
                filterCoeff[11] = fltCoeff[6];
                filterCoeff[12] = fltCoeff[12];

                filterClipp[0]  = fltClip[9];
                filterClipp[1]  = fltClip[8];
                filterClipp[2]  = fltClip[10];
                filterClipp[3]  = fltClip[4];
                filterClipp[4]  = fltClip[3];
                filterClipp[5]  = fltClip[7];
                filterClipp[6]  = fltClip[11];
                filterClipp[7]  = fltClip[5];
                filterClipp[8]  = fltClip[1];
                filterClipp[9]  = fltClip[0];
                filterClipp[10] = fltClip[2];
                filterClipp[11] = fltClip[6];
                filterClipp[12] = fltClip[12];

            }
            else
            {
                filterCoeff[0]  = fltCoeff[0];
                filterCoeff[1]  = fltCoeff[1];
                filterCoeff[2]  = fltCoeff[2];
                filterCoeff[3]  = fltCoeff[3];
                filterCoeff[4]  = fltCoeff[4];
                filterCoeff[5]  = fltCoeff[5];
                filterCoeff[6]  = fltCoeff[6];
                filterCoeff[7]  = fltCoeff[7];
                filterCoeff[8]  = fltCoeff[8];
                filterCoeff[9]  = fltCoeff[9];
                filterCoeff[10] = fltCoeff[10];
                filterCoeff[11] = fltCoeff[11];
                filterCoeff[12] = fltCoeff[12];

                filterClipp[0]  = fltClip[0];
                filterClipp[1]  = fltClip[1];
                filterClipp[2]  = fltClip[2];
                filterClipp[3]  = fltClip[3];
                filterClipp[4]  = fltClip[4];
                filterClipp[5]  = fltClip[5];
                filterClipp[6]  = fltClip[6];
                filterClipp[7]  = fltClip[7];
                filterClipp[8]  = fltClip[8];
                filterClipp[9]  = fltClip[9];
                filterClipp[10] = fltClip[10];
                filterClipp[11] = fltClip[11];
                filterClipp[12] = fltClip[12];

            }

            for (ii = 0; ii < clsSizeY; ii++)
            {
                pImg0 = src + j + ii * srcStride;
                pImg1 = pImg0 + srcStride;
                pImg2 = pImg0 - srcStride;
                pImg3 = pImg1 + srcStride;
                pImg4 = pImg2 - srcStride;
                pImg5 = pImg3 + srcStride;
                pImg6 = pImg4 - srcStride;

                pRec1 = pRec0 + j + ii * dstStride;

                const int y_vb = (i + ii) & (vbCtuHeight - 1);

                if (y_vb < vbPos && (y_vb >= vbPos - 4))   // above
                {
                    pImg1 = (y_vb == vbPos - 1) ? pImg0 : pImg1;
                    pImg3 = (y_vb >= vbPos - 2) ? pImg1 : pImg3;
                    pImg5 = (y_vb >= vbPos - 3) ? pImg3 : pImg5;

                    pImg2 = (y_vb == vbPos - 1) ? pImg0 : pImg2;
                    pImg4 = (y_vb >= vbPos - 2) ? pImg2 : pImg4;
                    pImg6 = (y_vb >= vbPos - 3) ? pImg4 : pImg6;
                }

                else if (y_vb >= vbPos && (y_vb <= vbPos + 3))   // bottom
                {
                    pImg2 = (y_vb == vbPos) ? pImg0 : pImg2;
                    pImg4 = (y_vb <= vbPos + 1) ? pImg2 : pImg4;
                    pImg6 = (y_vb <= vbPos + 2) ? pImg4 : pImg6;

                    pImg1 = (y_vb == vbPos) ? pImg0 : pImg1;
                    pImg3 = (y_vb <= vbPos + 1) ? pImg1 : pImg3;
                    pImg5 = (y_vb <= vbPos + 2) ? pImg3 : pImg5;
                }

                BOOL is_near_vb_above = y_vb < vbPos && (y_vb >= vbPos - 1);
                BOOL is_near_vb_below = y_vb >= vbPos && (y_vb <= vbPos);

                for (jj = 0; jj < clsSizeX; jj++)
                {
                    int sum = 0;
                    const int16_t curr = pImg0[+0];

                    sum += filterCoeff[0] * alfClip (filterClipp[0], curr, pImg5[+0], pImg6[+0]);
                    sum += filterCoeff[1] * alfClip (filterClipp[1], curr, pImg3[+1], pImg4[-1]);

                    sum += filterCoeff[2] * alfClip (filterClipp[2], curr, pImg3[+0], pImg4[+0]);
                    sum += filterCoeff[3] * alfClip (filterClipp[3], curr, pImg3[-1], pImg4[+1]);

                    sum += filterCoeff[4] * alfClip (filterClipp[4], curr, pImg1[+2], pImg2[-2]);
                    sum += filterCoeff[5] * alfClip (filterClipp[5], curr, pImg1[+1], pImg2[-1]);

                    sum += filterCoeff[6] * alfClip (filterClipp[6], curr, pImg1[+0], pImg2[+0]);
                    sum += filterCoeff[7] * alfClip (filterClipp[7], curr, pImg1[-1], pImg2[+1]);

                    sum += filterCoeff[8] * alfClip (filterClipp[8], curr, pImg1[-2], pImg2[+2]);
                    sum += filterCoeff[9] * alfClip (filterClipp[9], curr, pImg0[+3], pImg0[-3]);

                    sum += filterCoeff[10] * alfClip (filterClipp[10], curr, pImg0[+2], pImg0[-2]);
                    sum += filterCoeff[11] * alfClip (filterClipp[11], curr, pImg0[+1], pImg0[-1]);

                    if (!(is_near_vb_above || is_near_vb_below))
                    {
                        sum = (sum + offset) >> shift;
                    }
                    else
                    {
                        sum = (sum + (1 << ((shift + 3) - 1))) >> (shift + 3);
                    }

                    sum += curr;
                    pRec1[jj] = (PIXEL)XIN_CLIP(sum, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);

                    pImg0++;
                    pImg1++;
                    pImg2++;
                    pImg3++;
                    pImg4++;
                    pImg5++;
                    pImg6++;

                }
            }
        }

        pRec0 += dstStride2;
        pRec1 += dstStride2;

        src += srcStride2;
        dst += dstStride2;

    }

}

void Xin266AlfBlockChroma (
    PIXEL         *src,
    intptr_t      srcStride,
    PIXEL         *dst,
    intptr_t      dstStride,
    SINT16        *filterSet,
    SINT16        *fClipSet,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight)
{
    SINT32        i, j;
    SINT32        clsSizeY;
    SINT32        clsSizeX;
    SINT32        ii, jj;
    PIXEL         *pImg0, *pImg1, *pImg2, *pImg3, *pImg4;
    PIXEL         *pRec0, *pRec1;
    intptr_t      dstStride2;
    intptr_t      srcStride2;
    SINT32        shift;
    SINT32        offset;
    SINT32        filterCoeff[XIN_ALF_MAX_CHROMA_COEF_NUM];
    SINT32        filterClipp[XIN_ALF_MAX_CHROMA_COEF_NUM];

    clsSizeX   = 4;
    clsSizeY   = 4;
    pRec0      = dst;
    pRec1      = pRec0 + dstStride;
    dstStride2 = dstStride*clsSizeY;
    srcStride2 = srcStride*clsSizeY;
    shift      = 8 - 1;
    offset     = 1 << (shift - 1);

    for (i = 0; i < height; i += clsSizeY)
    {
        for (j = 0; j < width; j += clsSizeX)
        {
            filterCoeff[0]  = filterSet[0];
            filterCoeff[1]  = filterSet[1];
            filterCoeff[2]  = filterSet[2];
            filterCoeff[3]  = filterSet[3];
            filterCoeff[4]  = filterSet[4];
            filterCoeff[5]  = filterSet[5];
            filterCoeff[6]  = filterSet[6];

            filterClipp[0]  = fClipSet[0];
            filterClipp[1]  = fClipSet[1];
            filterClipp[2]  = fClipSet[2];
            filterClipp[3]  = fClipSet[3];
            filterClipp[4]  = fClipSet[4];
            filterClipp[5]  = fClipSet[5];
            filterClipp[6]  = fClipSet[6];

            for (ii = 0; ii < clsSizeY; ii++)
            {
                pImg0 = src + j + ii * srcStride;
                pImg1 = pImg0 + srcStride;
                pImg2 = pImg0 - srcStride;
                pImg3 = pImg1 + srcStride;
                pImg4 = pImg2 - srcStride;

                pRec1 = pRec0 + j + ii * dstStride;

                const int y_vb = (i + ii) & (vbCtuHeight - 1);

                if (y_vb < vbPos && (y_vb >= vbPos - 2))   // above
                {
                    pImg1 = (y_vb == vbPos - 1) ? pImg0 : pImg1;
                    pImg3 = (y_vb >= vbPos - 2) ? pImg1 : pImg3;

                    pImg2 = (y_vb == vbPos - 1) ? pImg0 : pImg2;
                    pImg4 = (y_vb >= vbPos - 2) ? pImg2 : pImg4;
                }

                else if (y_vb >= vbPos && (y_vb <= vbPos + 1))   // bottom
                {
                    pImg2 = (y_vb == vbPos) ? pImg0 : pImg2;
                    pImg4 = (y_vb <= vbPos + 1) ? pImg2 : pImg4;

                    pImg1 = (y_vb == vbPos) ? pImg0 : pImg1;
                    pImg3 = (y_vb <= vbPos + 1) ? pImg1 : pImg3;
                }

                BOOL is_near_vb_above = y_vb < vbPos && (y_vb >= vbPos - 1);
                BOOL is_near_vb_below = y_vb >= vbPos && (y_vb <= vbPos);

                for (jj = 0; jj < clsSizeX; jj++)
                {
                    int sum = 0;
                    const int16_t curr = pImg0[+0];

                    sum += filterCoeff[0] * ( alfClip(filterClipp[0], curr, pImg3[+0], pImg4[+0]) );

                    sum += filterCoeff[1] * ( alfClip(filterClipp[1], curr, pImg1[+1], pImg2[-1]) );
                    sum += filterCoeff[2] * ( alfClip(filterClipp[2], curr, pImg1[+0], pImg2[+0]) );
                    sum += filterCoeff[3] * ( alfClip(filterClipp[3], curr, pImg1[-1], pImg2[+1]) );

                    sum += filterCoeff[4] * ( alfClip(filterClipp[4], curr, pImg0[+2], pImg0[-2]) );
                    sum += filterCoeff[5] * ( alfClip(filterClipp[5], curr, pImg0[+1], pImg0[-1]) );

                    if (!(is_near_vb_above || is_near_vb_below))
                    {
                        sum = (sum + offset) >> shift;
                    }
                    else
                    {
                        sum = (sum + (1 << ((shift + 3) - 1))) >> (shift + 3);
                    }

                    sum += curr;
                    pRec1[jj] = (PIXEL)XIN_CLIP(sum, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);

                    pImg0++;
                    pImg1++;
                    pImg2++;
                    pImg3++;
                    pImg4++;

                }
            }
        }

        pRec0 += dstStride2;
        pRec1 += dstStride2;

        src += srcStride2;
        dst += dstStride2;

    }

}


void Xin266AlfDeriveClass (
    xin_alf_class *alfClass,
    intptr_t      classStride,
    PIXEL         *src,
    intptr_t      srcStride,
    SINT32        blockWidth,
    SINT32        blockHeight,
    SINT32        blockPosY,
    SINT32        vbCtuHeight,
    SINT32        vbPos,
    SINT32        shift)
{
    static const int th[16] = { 0, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4 };
    int laplacian[XIN_ALF_DIR_NUM][XIN_ALF_CLS_BLK_SIZE + 5][XIN_ALF_CLS_BLK_SIZE + 5];

    const int max_activity = 15;

    int fl = 2;
    int fl_p1 = fl + 1;
    int fl2 = 2 * fl;

    int main_direction, secondary_direction, dir_temp_hv, dir_temp_d;
    int pix_y;

    int height = blockHeight + fl2;
    int width = blockWidth + fl2;
    int start_height = - fl_p1;

    for (int i = 0; i < height; i += 2)
    {
        intptr_t yoffset = (i + 1 + start_height) * srcStride - fl_p1;
        const PIXEL *src0 = &src[yoffset - srcStride];
        const PIXEL *src1 = &src[yoffset];
        const PIXEL *src2 = &src[yoffset + srcStride];
        const PIXEL *src3 = &src[yoffset + srcStride * 2];

        const int y = blockPosY - 2 + i;

        if (y > 0 && (y & (vbCtuHeight - 1)) == vbPos - 2)
        {
            src3 = &src[yoffset + srcStride];
        }
        else if (y > 0 && (y & (vbCtuHeight - 1)) == vbPos)
        {
            src0 = &src[yoffset];
        }

        int *p_y_ver = laplacian[XIN_ALF_VER][i];
        int *p_y_hor = laplacian[XIN_ALF_HOR][i];
        int *p_y_dig_0 = laplacian[XIN_ALF_DIAG0][i];
        int *p_y_dig_1 = laplacian[XIN_ALF_DIAG1][i];

        for (int j = 0; j < width; j += 2)
        {
            pix_y = j + 1;
            const PIXEL *p_y = src1 + pix_y;
            const PIXEL *p_y_down = src0 + pix_y;
            const PIXEL *p_y_up = src2 + pix_y;
            const PIXEL *p_y_up2 = src3 + pix_y;

            const int16_t y0 = p_y[0] << 1;
            const int16_t y_up1 = p_y_up[1] << 1;

            p_y_ver[j] = XIN_ABS(y0 - p_y_down[0] - p_y_up[0]) + XIN_ABS(y_up1 - p_y[1] - p_y_up2[1]);
            p_y_hor[j] = XIN_ABS(y0 - p_y[1] - p_y[-1]) + XIN_ABS(y_up1 - p_y_up[2] - p_y_up[0]);
            p_y_dig_0[j] = XIN_ABS(y0 - p_y_down[-1] - p_y_up[1]) + XIN_ABS(y_up1 - p_y[0] - p_y_up2[2]);
            p_y_dig_1[j] = XIN_ABS(y0 - p_y_up[-1] - p_y_down[1]) + XIN_ABS(y_up1 - p_y_up2[0] - p_y[2]);

            if (j > 4 && (j - 6) % 4 == 0)
            {
                int j_m_6 = j - 6;
                int j_m_4 = j - 4;
                int j_m_2 = j - 2;

                p_y_ver[j_m_6] += p_y_ver[j_m_4] + p_y_ver[j_m_2] + p_y_ver[j];
                p_y_hor[j_m_6] += p_y_hor[j_m_4] + p_y_hor[j_m_2] + p_y_hor[j];
                p_y_dig_0[j_m_6] += p_y_dig_0[j_m_4] + p_y_dig_0[j_m_2] + p_y_dig_0[j];
                p_y_dig_1[j_m_6] += p_y_dig_1[j_m_4] + p_y_dig_1[j_m_2] + p_y_dig_1[j];
            }
        }
    }

    // classification block size
    const int cls_size_y = 4;
    const int cls_size_x = 4;

    //for (int i = 0; i < blk.height; i += cls_size_y)
    for (int i = 0; i < blockHeight; i += cls_size_y)
    {
        int* p_y_ver = laplacian[XIN_ALF_VER][i];
        int* p_y_ver2 = laplacian[XIN_ALF_VER][i + 2];
        int* p_y_ver4 = laplacian[XIN_ALF_VER][i + 4];
        int* p_y_ver6 = laplacian[XIN_ALF_VER][i + 6];

        int* p_y_hor = laplacian[XIN_ALF_HOR][i];
        int* p_y_hor2 = laplacian[XIN_ALF_HOR][i + 2];
        int* p_y_hor4 = laplacian[XIN_ALF_HOR][i + 4];
        int* p_y_hor6 = laplacian[XIN_ALF_HOR][i + 6];

        int* p_y_dig0 = laplacian[XIN_ALF_DIAG0][i];
        int* p_y_dig02 = laplacian[XIN_ALF_DIAG0][i + 2];
        int* p_y_dig04 = laplacian[XIN_ALF_DIAG0][i + 4];
        int* p_y_dig06 = laplacian[XIN_ALF_DIAG0][i + 6];

        int* p_y_dig1 = laplacian[XIN_ALF_DIAG1][i];
        int* p_y_dig12 = laplacian[XIN_ALF_DIAG1][i + 2];
        int* p_y_dig14 = laplacian[XIN_ALF_DIAG1][i + 4];
        int* p_y_dig16 = laplacian[XIN_ALF_DIAG1][i + 6];

        //for (int j = 0; j < blk.width; j += cls_size_x)
        for (int j = 0; j < blockWidth; j += cls_size_x)
        {
            int sum_v = 0;
            int sum_h = 0;
            int sum_d0 = 0;
            int sum_d1 = 0;

            if (((i + blockPosY) % vbCtuHeight) == (vbPos - 4))
            {
                sum_v = p_y_ver[j] + p_y_ver2[j] + p_y_ver4[j];
                sum_h = p_y_hor[j] + p_y_hor2[j] + p_y_hor4[j];
                sum_d0 = p_y_dig0[j] + p_y_dig02[j] + p_y_dig04[j];
                sum_d1 = p_y_dig1[j] + p_y_dig12[j] + p_y_dig14[j];
            }
            else if (((i + blockPosY) % vbCtuHeight) == vbPos)
            {
                sum_v = p_y_ver2[j] + p_y_ver4[j] + p_y_ver6[j];
                sum_h = p_y_hor2[j] + p_y_hor4[j] + p_y_hor6[j];
                sum_d0 = p_y_dig02[j] + p_y_dig04[j] + p_y_dig06[j];
                sum_d1 = p_y_dig12[j] + p_y_dig14[j] + p_y_dig16[j];
            }
            else
            {
                sum_v = p_y_ver[j] + p_y_ver2[j] + p_y_ver4[j] + p_y_ver6[j];
                sum_h = p_y_hor[j] + p_y_hor2[j] + p_y_hor4[j] + p_y_hor6[j];
                sum_d0 = p_y_dig0[j] + p_y_dig02[j] + p_y_dig04[j] + p_y_dig06[j];
                sum_d1 = p_y_dig1[j] + p_y_dig12[j] + p_y_dig14[j] + p_y_dig16[j];
            }

            int temp_act = sum_v + sum_h;
            int activity = 0;

            const int y = (i + blockPosY) & (vbCtuHeight - 1);
            if (y == vbPos - 4 || y == vbPos)
            {
                activity = XIN_CLIP((temp_act * 96) >> shift, 0, max_activity);
            }
            else
            {
                activity = XIN_CLIP((temp_act * 64) >> shift, 0, max_activity);
            }

            int class_idx = th[activity];

            int hv1, hv0, d1, d0, hvd1, hvd0;

            if (sum_v > sum_h)
            {
                hv1 = sum_v;
                hv0 = sum_h;
                dir_temp_hv = 1;
            }
            else
            {
                hv1 = sum_h;
                hv0 = sum_v;
                dir_temp_hv = 3;
            }
            if (sum_d0 > sum_d1)
            {
                d1 = sum_d0;
                d0 = sum_d1;
                dir_temp_d = 0;
            }
            else
            {
                d1 = sum_d1;
                d0 = sum_d0;
                dir_temp_d = 2;
            }
            if ((uint32_t)d1 * (uint32_t)hv0 > (uint32_t)hv1 * (uint32_t)d0)
            {
                hvd1 = d1;
                hvd0 = d0;
                main_direction = dir_temp_d;
                secondary_direction = dir_temp_hv;
            }
            else
            {
                hvd1 = hv1;
                hvd0 = hv0;
                main_direction = dir_temp_hv;
                secondary_direction = dir_temp_d;
            }

            int direction_strength = 0;
            if (hvd1 > 2 * hvd0)
            {
                direction_strength = 1;
            }
            if (hvd1 * 2 > 9 * hvd0)
            {
                direction_strength = 2;
            }

            if (direction_strength)
            {
                class_idx += (((main_direction & 0x1) << 1) + direction_strength) * 5;
            }

            static const int transpose_table[8] = { 0, 1, 0, 2, 2, 3, 1, 3 };
            int transpose_idx = transpose_table[main_direction * 2 + (secondary_direction >> 1)];


            alfClass[(i >> 2)*classStride + (j >> 2)].classIdx     = (UINT8)class_idx;
            alfClass[(i >> 2)*classStride + (j >> 2)].transposeIdx = (UINT8)transpose_idx;

        }
    }

}

void Xin266FilterBlockCcAlf (
    PIXEL         *luma,
    intptr_t      lumaStride,
    PIXEL         *chroma,
    intptr_t      chromaStride,
    SINT16        *filterSet,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight)
{
    SINT32  clsSizeY;
    SINT32  clsSizeX;

    clsSizeY = 4;
    clsSizeX = 4;

    for( int i = 0; i < height; i += clsSizeY )
    {
        for( int j = 0; j < width; j += clsSizeX )
        {
            for( int ii = 0; ii < clsSizeY; ii++ )
            {
                int row        = ii;
                int col        = j;
                PIXEL *srcSelf = chroma + col + row * chromaStride;

                intptr_t offset1 = lumaStride;
                intptr_t offset2 = -lumaStride;
                intptr_t offset3 = 2 * lumaStride;
                row <<= 1;
                col <<= 1;
                PIXEL *srcCross = luma + col + row * lumaStride;

                int pos = ((i + ii) << 1) & (vbCtuHeight - 1);

                if (pos == (vbPos - 2) || pos == (vbPos + 1))
                {
                    offset3 = offset1;
                }
                else if (pos == (vbPos - 1) || pos == vbPos)
                {
                    offset1 = 0;
                    offset2 = 0;
                    offset3 = 0;
                }

                for (int jj = 0; jj < clsSizeX; jj++)
                {
                    const int jj2     = (jj << 1);
                    const int offset0 = 0;

                    int sum = 0;

                    sum += filterSet[0] * (srcCross[offset2 + jj2    ] - srcCross[offset0 + jj2]);
                    sum += filterSet[1] * (srcCross[offset0 + jj2 - 1] - srcCross[offset0 + jj2]);
                    sum += filterSet[2] * (srcCross[offset0 + jj2 + 1] - srcCross[offset0 + jj2]);
                    sum += filterSet[3] * (srcCross[offset1 + jj2 - 1] - srcCross[offset0 + jj2]);
                    sum += filterSet[4] * (srcCross[offset1 + jj2    ] - srcCross[offset0 + jj2]);
                    sum += filterSet[5] * (srcCross[offset1 + jj2 + 1] - srcCross[offset0 + jj2]);
                    sum += filterSet[6] * (srcCross[offset3 + jj2    ] - srcCross[offset0 + jj2]);

                    sum = (sum + ((1 << 7 ) >> 1)) >> 7;
                    const int offset = 1<<(XIN_INTERNAL_BIT_DEPTH - 1);
                    sum = XIN_CLIP(sum + offset, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE) - offset;
                    sum += srcSelf[jj];
                    srcSelf[jj] = (PIXEL)XIN_CLIP(sum, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
                    
                }
            }
        }

        chroma += chromaStride * clsSizeY;
        luma   += lumaStride * clsSizeY << 1;
    }

}


