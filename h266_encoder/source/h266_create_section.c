/***************************************************************************//**
 *
 * @file          h266_create_section.c
 * @brief         Section level memory allocation and initialization.
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
#include "string.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_constant.h"
#include "h266_definition.h"
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
#include "h266_coding_unit_struct.h"
#include "basic_macro.h"
#include "h266_common_data.h"
#include "h26x_motion_est.h"
#include "h266_dep_quant.h"
#include "h266_enc_init.h"
#include "assert.h"

static SINT32 Xin266ConstructMe (
    xin_sec_struct *secSet)
{
    xin_me_struct  *meSet;
    xin_seq_struct *seqSet;
    UINT32         ctuSize;
    PIXEL          *halfPelH;
    PIXEL          *halfPelV;
    PIXEL          *halfPelHv;
    PIXEL          *integPel;

    seqSet  = secSet->seqSet;
    ctuSize = seqSet->ctuSize;

    XIN_MALLOC_CHECK (meSet, sizeof(xin_me_struct));
    XIN_MALLOC_CHECK (meSet->interpBuf, sizeof(PIXEL)*XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE*4);
    XIN_MALLOC_CHECK (meSet->input1,    sizeof(PIXEL)*ctuSize*ctuSize/4);
    XIN_MALLOC_CHECK (meSet->input2,    sizeof(PIXEL)*ctuSize*ctuSize/16);

    Xin26xMeFuncInit (
        meSet,
        seqSet->cpuFeature);

    meSet->input1Stride = ctuSize/2;
    meSet->input2Stride = ctuSize/4;
    meSet->interpStride = XIN_ME_BUF_STRIDE;
    meSet->oneDimMe     = seqSet->config.oneDimMe;

    integPel    = meSet->interpBuf;
    halfPelH    = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE;
    halfPelV    = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE*2;
    halfPelHv   = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE*3;

    meSet->integPel  = integPel;
    meSet->halfPelH  = halfPelH;
    meSet->halfPelV  = halfPelV;
    meSet->halfPelHv = halfPelHv;

    halfPelH   += (XIN_ME_FILTER_TAP>>1)*XIN_ME_BUF_STRIDE;
    halfPelV   += 1;
    integPel   += XIN_ME_BUF_STRIDE + 1;

    meSet->halfPel[LFT_POS]     = halfPelH;
    meSet->halfPel[RGT_POS]     = halfPelH + 1;
    meSet->halfPel[TOP_POS]     = halfPelV;
    meSet->halfPel[BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->halfPel[TOP_LFT_POS] = halfPelHv;
    meSet->halfPel[TOP_RGT_POS] = halfPelHv + 1;
    meSet->halfPel[BOT_LFT_POS] = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->halfPel[BOT_RGT_POS] = halfPelHv + XIN_ME_BUF_STRIDE + 1;

    // The following is seting quarter pixel motion estimation buffer
    meSet->qBufA[LFT_POS][LFT_POS]     = integPel - 1;
    meSet->qBufB[LFT_POS][LFT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][RGT_POS]     = integPel;
    meSet->qBufB[LFT_POS][RGT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][TOP_POS]     = halfPelHv;
    meSet->qBufB[LFT_POS][TOP_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufB[LFT_POS][BOT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][TOP_LFT_POS] = halfPelV - 1;
    meSet->qBufB[LFT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufB[LFT_POS][TOP_RGT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[LFT_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[LFT_POS][BOT_RGT_POS] = halfPelH;

    meSet->qBufA[RGT_POS][LFT_POS]     = integPel;
    meSet->qBufB[RGT_POS][LFT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][RGT_POS]     = integPel + 1;
    meSet->qBufB[RGT_POS][RGT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_POS]     = halfPelHv + 1;
    meSet->qBufB[RGT_POS][TOP_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[RGT_POS][BOT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[RGT_POS][TOP_LFT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_RGT_POS] = halfPelV + 1;
    meSet->qBufB[RGT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[RGT_POS][BOT_LFT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[RGT_POS][BOT_RGT_POS] = halfPelH + 1;

    meSet->qBufA[TOP_POS][LFT_POS]     = halfPelHv;
    meSet->qBufB[TOP_POS][LFT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][RGT_POS]     = halfPelHv + 1;
    meSet->qBufB[TOP_POS][RGT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][TOP_POS]     = integPel - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_POS][TOP_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][BOT_POS]     = integPel;
    meSet->qBufB[TOP_POS][BOT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufB[TOP_POS][BOT_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][BOT_RGT_POS] = halfPelH + 1;
    meSet->qBufB[TOP_POS][BOT_RGT_POS] = halfPelV;

    meSet->qBufA[BOT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_POS]     = integPel;
    meSet->qBufB[BOT_POS][TOP_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_POS]     = integPel + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufB[BOT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufB[BOT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_RGT_POS] = halfPelH + 1 + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;

    meSet->qBufA[TOP_LFT_POS][LFT_POS]     = halfPelV - 1;
    meSet->qBufB[TOP_LFT_POS][LFT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][RGT_POS]     = halfPelV;
    meSet->qBufB[TOP_LFT_POS][RGT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][TOP_POS]     = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_LFT_POS][TOP_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][BOT_POS]     = halfPelH;
    meSet->qBufB[TOP_LFT_POS][BOT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][TOP_LFT_POS] = halfPelV - 1;
    meSet->qBufB[TOP_LFT_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufA[TOP_LFT_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_LFT_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufA[TOP_LFT_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufB[TOP_LFT_POS][BOT_LFT_POS] = halfPelV - 1;
    meSet->qBufA[TOP_LFT_POS][BOT_RGT_POS] = halfPelV;
    meSet->qBufB[TOP_LFT_POS][BOT_RGT_POS] = halfPelH;

    meSet->qBufA[TOP_RGT_POS][LFT_POS]     = halfPelV;
    meSet->qBufB[TOP_RGT_POS][LFT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][RGT_POS]     = halfPelV + 1;
    meSet->qBufB[TOP_RGT_POS][RGT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_POS]     = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_RGT_POS][TOP_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][BOT_POS]     = halfPelH + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[TOP_RGT_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_RGT_POS][TOP_RGT_POS] = halfPelV + 1;
    meSet->qBufA[TOP_RGT_POS][BOT_LFT_POS] = halfPelH + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_RGT_POS][BOT_RGT_POS] = halfPelV + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_RGT_POS] = halfPelH + 1;

    meSet->qBufA[BOT_LFT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[BOT_LFT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][TOP_POS]     = halfPelH;
    meSet->qBufB[BOT_LFT_POS][TOP_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][BOT_POS]     = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[BOT_LFT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[BOT_LFT_POS][TOP_RGT_POS] = halfPelH;
    meSet->qBufB[BOT_LFT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufA[BOT_LFT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_RGT_POS] = halfPelH + XIN_ME_BUF_STRIDE;

    meSet->qBufA[BOT_RGT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_RGT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_POS]     = halfPelH + 1;
    meSet->qBufB[BOT_RGT_POS][TOP_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][BOT_POS]     = halfPelH + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_RGT_POS][TOP_LFT_POS] = halfPelH + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufB[BOT_RGT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_RGT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_RGT_POS] = halfPelH + XIN_ME_BUF_STRIDE + 1;

    meSet->qBufA[CEN_POS][LFT_POS]     = halfPelH;
    meSet->qBufB[CEN_POS][LFT_POS]     = integPel;
    meSet->qBufA[CEN_POS][RGT_POS]     = halfPelH + 1;
    meSet->qBufB[CEN_POS][RGT_POS]     = integPel;
    meSet->qBufA[CEN_POS][TOP_POS]     = halfPelV;
    meSet->qBufB[CEN_POS][TOP_POS]     = integPel;
    meSet->qBufA[CEN_POS][BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_POS]     = integPel;
    meSet->qBufA[CEN_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[CEN_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[CEN_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufB[CEN_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufA[CEN_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufA[CEN_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_RGT_POS] = halfPelH + 1;

    secSet->meSet = meSet;

    return XIN_SUCCESS;

}

static SINT32 Xin266ConstructMdBuf (
    xin_seq_struct  *seqSet,
    xin_fast_md_buf **dblFastBuf,
    UINT32          fastBufNum,
    xin_full_md_buf **dblFullBuf,
    UINT32          fullBufNum,
    UINT32          ctuSize)
{
    xin_fast_md_buf *fastBuf;
    xin_full_md_buf *fullBuf;
    void            *bufBase;
    UINT32          bufIdx;
    UINT32          mtsIdx;
    UINT32          lumaSize;
    UINT32          chromaSize;
    UINT32          predBufSize;
    UINT32          qCoefSize;
    UINT32          tCoefSize;
    UINT32          rCoefSize;
    UINT32          rPixelSize;
    UINT32          coefSignSize;
    UINT32          gt0MapSize;

    (void)seqSet;
    lumaSize      = ctuSize*ctuSize;
    chromaSize    = ctuSize*ctuSize/4;
    predBufSize   = (lumaSize + chromaSize*2);
    qCoefSize     = (lumaSize + chromaSize*2);
    tCoefSize     = (lumaSize + chromaSize*2);
    rCoefSize     = (lumaSize + chromaSize*2);
    rPixelSize    = (lumaSize + chromaSize*2);
    coefSignSize  = (lumaSize/16 + chromaSize*2/16);
    gt0MapSize    = (lumaSize/16 + chromaSize*2/16);

    XIN_MALLOC_CHECK (fastBuf, fastBufNum*sizeof(xin_fast_md_buf));

    for (bufIdx = 0; bufIdx < fastBufNum; bufIdx++)
    {
        XIN_MALLOC_CHECK (bufBase, predBufSize*sizeof(PIXEL));

        fastBuf[bufIdx].bufBase                 = bufBase;
        fastBuf[bufIdx].predBuf[PLANE_LUMA]     = bufBase;
        fastBuf[bufIdx].predBuf[PLANE_CHROMA_U] = (PIXEL *)bufBase + lumaSize;
        fastBuf[bufIdx].predBuf[PLANE_CHROMA_V] = (PIXEL *)bufBase + (lumaSize + chromaSize);

        fastBuf[bufIdx].lumaStride   = ctuSize;
        fastBuf[bufIdx].chromaStride = ctuSize/2;
    }

    XIN_MALLOC_CHECK (fullBuf, fullBufNum*sizeof(xin_full_md_buf));

    for (bufIdx = 0; bufIdx < fullBufNum; bufIdx++)
    {
        for (mtsIdx = 0; mtsIdx < XIN_MTS_IDX_NUM; mtsIdx++)
        {
            if ((mtsIdx == 0) || ((mtsIdx == XIN_MTS_SKIP) && seqSet->config.transSkipFlag))
            {
                XIN_MALLOC_CHECK (bufBase, qCoefSize*sizeof(COEFF) + tCoefSize*sizeof(COEFF) + rCoefSize*sizeof(COEFF) + rPixelSize*sizeof(PIXEL) + coefSignSize*sizeof(UINT16) + gt0MapSize*sizeof(UINT16));

                fullBuf[bufIdx].bufBase[mtsIdx] = bufBase;

                fullBuf[bufIdx].qCoefBuf[mtsIdx][PLANE_LUMA]     = bufBase;
                fullBuf[bufIdx].qCoefBuf[mtsIdx][PLANE_CHROMA_U] = (COEFF *)bufBase + lumaSize;
                fullBuf[bufIdx].qCoefBuf[mtsIdx][PLANE_CHROMA_V] = (COEFF *)bufBase + (lumaSize + chromaSize);

                bufBase = (COEFF *)bufBase + qCoefSize;

                fullBuf[bufIdx].tCoefBuf[mtsIdx][PLANE_LUMA]     = bufBase;
                fullBuf[bufIdx].tCoefBuf[mtsIdx][PLANE_CHROMA_U] = (COEFF *)bufBase + lumaSize;
                fullBuf[bufIdx].tCoefBuf[mtsIdx][PLANE_CHROMA_V] = (COEFF *)bufBase + (lumaSize + chromaSize);

                bufBase = (COEFF *)bufBase + tCoefSize;

                fullBuf[bufIdx].rCoefBuf[mtsIdx][PLANE_LUMA]     = bufBase;
                fullBuf[bufIdx].rCoefBuf[mtsIdx][PLANE_CHROMA_U] = (COEFF *)bufBase + lumaSize;
                fullBuf[bufIdx].rCoefBuf[mtsIdx][PLANE_CHROMA_V] = (COEFF *)bufBase + (lumaSize + chromaSize);

                bufBase = (COEFF *)bufBase + rCoefSize;

                fullBuf[bufIdx].reconBuf[mtsIdx][PLANE_LUMA]     = bufBase;
                fullBuf[bufIdx].reconBuf[mtsIdx][PLANE_CHROMA_U] = (PIXEL *)bufBase + lumaSize;
                fullBuf[bufIdx].reconBuf[mtsIdx][PLANE_CHROMA_V] = (PIXEL *)bufBase + (lumaSize + chromaSize);

                bufBase = (PIXEL *)bufBase + rPixelSize;

                fullBuf[bufIdx].coeffSign[mtsIdx][PLANE_LUMA]     = bufBase;
                fullBuf[bufIdx].coeffSign[mtsIdx][PLANE_CHROMA_U] = (UINT16 *)bufBase + (lumaSize / 16);
                fullBuf[bufIdx].coeffSign[mtsIdx][PLANE_CHROMA_V] = (UINT16 *)bufBase + (lumaSize / 16 + chromaSize / 16);

                bufBase = (UINT16 *)bufBase + coefSignSize;

                fullBuf[bufIdx].gt0BitMap[mtsIdx][PLANE_LUMA]     = bufBase;
                fullBuf[bufIdx].gt0BitMap[mtsIdx][PLANE_CHROMA_U] = (UINT16 *)bufBase + (lumaSize / 16);
                fullBuf[bufIdx].gt0BitMap[mtsIdx][PLANE_CHROMA_V] = (UINT16 *)bufBase + (lumaSize / 16 + chromaSize / 16);

            }
            else
            {
                fullBuf[bufIdx].bufBase[mtsIdx] = NULL;
            }

        }

        fullBuf[bufIdx].coeffStride[0] = ctuSize;
        fullBuf[bufIdx].coeffStride[1] = ctuSize / 2;

    }

    *dblFastBuf = fastBuf;
    *dblFullBuf = fullBuf;


    return XIN_SUCCESS;

}

static void Xin266DestructMdBuf (
    xin_fast_md_buf *fastBuf,
    UINT32          fastBufNum,
    xin_full_md_buf *fullBuf,
    UINT32          fullBufNum)
{
    UINT32  bufIdx;
    UINT32  mtsIdx;

    if (fastBufNum)
    {
        for (bufIdx = 0; bufIdx < fastBufNum; bufIdx++)
        {
            free (fastBuf[bufIdx].bufBase);
        }

        free (fastBuf);
    }

    if (fullBufNum)
    {
        for (bufIdx = 0; bufIdx < fullBufNum; bufIdx++)
        {
            for (mtsIdx = 0; mtsIdx < XIN_MTS_IDX_NUM; mtsIdx++)
            {
                if (fullBuf[bufIdx].bufBase[mtsIdx])
                {
                    free (fullBuf[bufIdx].bufBase[mtsIdx]);
                }
            }

        }

        free (fullBuf);
    }

}

SINT32 Xin266SecCreate (
    xin_sec_struct **dblSecSet,
    xin_seq_struct *seqSet)
{
    xin_sec_struct  *secSet;
    UINT32          qtDepthRange;
    UINT32          depthIdx;
    UINT32          ctuSize;
    UINT32          ctuSizeInBlock;
    UINT32          mdBufferWidth;
    UINT32          intraBufNum;
    UINT32          interBufNum;
    UINT32          maxTrSize2;

    XIN_MALLOC_CHECK (secSet, sizeof(xin_sec_struct));
    
    *dblSecSet     = secSet;
    secSet->seqSet = seqSet;
    ctuSize        = seqSet->ctuSize;
    ctuSizeInBlock = seqSet->ctuSize >> seqSet->lgBlockSize;
    qtDepthRange   = calcLog2[ctuSize] - calcLog2[seqSet->config.minQtSize] + 1;
    intraBufNum    = seqSet->config.intraRdoNum  + 1;
    interBufNum    = seqSet->config.maxMdCandNum + 1;
    mdBufferWidth  = intraBufNum + interBufNum;
    maxTrSize2     = seqSet->config.maxTrSize*2;

    for (depthIdx = 0; depthIdx < qtDepthRange*2; depthIdx++)
    {
        Xin266ConstructMdBuf (
            seqSet,
            &secSet->fastMdBuf[depthIdx],
            mdBufferWidth,
            &secSet->fullMdBuf[depthIdx],
            seqSet->config.maxMdCandNum + 1,
            ctuSize);
    }

    Xin266ConstructMdBuf (
        seqSet,
        &secSet->fastUvMdBuf,
        XIN_INTRA_CHROMA_CAND_NUM,
        &secSet->fullUvMdBuf,
        XIN_INTRA_CHROMA_CAND_NUM,
        ctuSize);

    secSet->fullUvMdBufNum  = XIN_INTRA_CHROMA_CAND_NUM;
    secSet->fastUvMdBufNum  = XIN_INTRA_CHROMA_CAND_NUM;
    secSet->interFastBufNum = interBufNum;
    secSet->intraFastBufNum = intraBufNum;
    secSet->fastMdBufNum    = mdBufferWidth;
    secSet->fullMdBufNum    = seqSet->config.maxMdCandNum + 1;

    XIN_MALLOC_CHECK (secSet->inputCtu[PLANE_LUMA],     ctuSize*ctuSize*sizeof(PIXEL));
    XIN_MALLOC_CHECK (secSet->inputCtu[PLANE_CHROMA_U], (ctuSize*ctuSize/4)*sizeof(PIXEL));
    XIN_MALLOC_CHECK (secSet->inputCtu[PLANE_CHROMA_V], (ctuSize*ctuSize/4)*sizeof(PIXEL));

    if (seqSet->config.enableLmcs)
    {
        XIN_MALLOC_CHECK (secSet->reshapeCtuY,     ctuSize*ctuSize*sizeof(PIXEL));
    }

    secSet->inputYStride  = ctuSize;
    secSet->inputUvStride = ctuSize/2;

    secSet->reconYStride  = seqSet->refFrame->refStride[0];
    secSet->reconUvStride = seqSet->refFrame->refStride[1];

    if (seqSet->config.enableAffine)
    {
        XIN_MALLOC_CHECK (secSet->tempBuffer, XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*8*sizeof(COEFF) + sizeof(xin_mv32_u)*1024*10);
    }
    else
    {
        XIN_MALLOC_CHECK (secSet->tempBuffer, XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*8*sizeof(COEFF));
    }
    
    XIN_MALLOC_CHECK (secSet->predBuffer, 2*2*XIN_MAX_REF_FRAMES*PRED_BUF_SIZE*sizeof(PIXEL));
    XIN_MALLOC_CHECK (secSet->biMeInput,  XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(PIXEL));

    secSet->firstModeChunk = NULL;
    secSet->lastModeChunk  = NULL;
    secSet->modeList       = NULL;
    secSet->modeListSize   = 0;
    secSet->modeListIdx    = 0;

    // Intra neighbour buffer
    secSet->nIntraBufY    = secSet->nIntraBuf + maxTrSize2*2;
    secSet->nIntraFltBufY = secSet->nIntraBufY + maxTrSize2*5 + 1 + maxTrSize2;
    secSet->nIntraBufU    = secSet->nIntraFltBufY + maxTrSize2*5 + 1 + maxTrSize2;
    secSet->nIntraBufV    = secSet->nIntraBufU + maxTrSize2*5 + 1 + maxTrSize2;

    if (Xin266ConstructMe (secSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    secSet->funcSet = seqSet->funcSet;

    for (depthIdx = 0; depthIdx <= qtDepthRange; depthIdx++)
    {
        XIN_MALLOC_CHECK (secSet->reconData[depthIdx][PLANE_LUMA],     ctuSize*ctuSize*sizeof(PIXEL));
        XIN_MALLOC_CHECK (secSet->reconData[depthIdx][PLANE_CHROMA_U], ctuSize*ctuSize*sizeof(PIXEL)/4);
        XIN_MALLOC_CHECK (secSet->reconData[depthIdx][PLANE_CHROMA_V], ctuSize*ctuSize*sizeof(PIXEL)/4);

        XIN_MALLOC_CHECK (secSet->blockData[depthIdx], ctuSizeInBlock*ctuSizeInBlock*sizeof(xin_block_struct));
    }

    secSet->reconDataStride[PLANE_LUMA]   = ctuSize;
    secSet->reconDataStride[PLANE_CHROMA] = ctuSize / 2;

    secSet->blockDataStride = ctuSizeInBlock;

    if (seqSet->config.enableAffine)
    {
        secSet->affMvBuf[0][XIN_LIST_0] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*0*1024);
        secSet->affMvBuf[0][XIN_LIST_1] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*1*1024);
        secSet->affMvBuf[1][XIN_LIST_0] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*2*1024);
        secSet->affMvBuf[1][XIN_LIST_1] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*3*1024);
        secSet->affMvBuf[2][XIN_LIST_0] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*4*1024);
        secSet->affMvBuf[2][XIN_LIST_1] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*5*1024);
        secSet->affMvBuf[3][XIN_LIST_0] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*6*1024);
        secSet->affMvBuf[3][XIN_LIST_1] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*7*1024);
        secSet->affMvBuf[4][XIN_LIST_0] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*8*1024);
        secSet->affMvBuf[4][XIN_LIST_1] = (xin_mv32_u *)(secSet->tempBuffer + XIN_MAX_CU_SIZE*XIN_MAX_CU_SIZE*sizeof(SINT16)*8 + sizeof(xin_mv32_u)*9*1024);
    }

    if (seqSet->config.enableDepQuant)
    {
        Xin266CreateDepQuant (
            secSet);
    }

    if (Xin266CreateModeChunk (secSet, 1))
    {
        return XIN_FAIL;
    }

    return XIN_SUCCESS;

}

void Xin266DeleteModeChunk (
    xin_sec_struct *secSet)
{
    xin_mode_list *currModeChunk;
    xin_mode_list *nextModeChunk;

    currModeChunk = secSet->firstModeChunk;

    for (; currModeChunk != NULL;)
    {
        nextModeChunk = currModeChunk->nextList;

        free (currModeChunk->modeBuf);
        free (currModeChunk);

        currModeChunk = nextModeChunk;
    }

    free (secSet->modeList);

}

void Xin266SecDelete (
    xin_sec_struct *secSet,
    xin_seq_struct *seqSet)
{
    UINT32          qtDepthRange;
    UINT32          depthIdx;
    UINT32          ctuSize;

    ctuSize        = seqSet->ctuSize;
    qtDepthRange   = calcLog2[ctuSize] - calcLog2[seqSet->config.minQtSize] + 1;

    for (depthIdx = 0; depthIdx < qtDepthRange*2; depthIdx++)
    {
        Xin266DestructMdBuf (
            secSet->fastMdBuf[depthIdx],
            secSet->fastMdBufNum,
            secSet->fullMdBuf[depthIdx],
            secSet->fullMdBufNum);
    }

    Xin266DestructMdBuf (
        secSet->fastUvMdBuf,
        XIN_INTRA_CHROMA_CAND_NUM,
        secSet->fullUvMdBuf,
        XIN_INTRA_CHROMA_CAND_NUM);

    free (secSet->inputCtu[PLANE_LUMA]);
    free (secSet->inputCtu[PLANE_CHROMA_U]);
    free (secSet->inputCtu[PLANE_CHROMA_V]);

    if (seqSet->config.enableLmcs)
    {
        free (secSet->reshapeCtuY);
    }

    free (secSet->tempBuffer);
    free (secSet->predBuffer);
    
    free (secSet->biMeInput);

    free (secSet->meSet->interpBuf);
    free (secSet->meSet->input1);
    free (secSet->meSet->input2);
    free (secSet->meSet);

    for (depthIdx = 0; depthIdx <= qtDepthRange; depthIdx++)
    {
        free (secSet->reconData[depthIdx][PLANE_LUMA]);
        free (secSet->reconData[depthIdx][PLANE_CHROMA_U]);
        free (secSet->reconData[depthIdx][PLANE_CHROMA_V]);

        free (secSet->blockData[depthIdx]);
    }

    if (seqSet->config.enableDepQuant)
    {
        free (secSet->depQuant);
    }

    Xin266DeleteModeChunk (
        secSet);

    free (secSet);

}

