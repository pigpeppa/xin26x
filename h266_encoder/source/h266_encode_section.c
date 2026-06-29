/***************************************************************************//**
 *
 * @file          h266_encode_section.c
 * @brief         Encode a picture section in a frame.
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
#include "string.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
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
#include "h266_enc_init.h"
#include "h266_encode_ctu.h"
#include "h26x_thread_wrapper.h"
#include "h266_encode_ctu.h"
#include "h266_sao_rdo.h"
#include "h266_rate_control.h"
#include "h266_encode_ctu.h"
#include "h26x_thread_wrapper.h"
#include "h26x_thread_pool.h"
#include "h266_alf_rdo.h"
#include "basic_macro.h"
#include "h266_bit_stream.h"
#include "h266_cabac_context.h"

void Xin266CtuEnc (
    void *opaque)
{
    xin_pic_struct *picSet;
    xin_seq_struct *seqSet;
    xin_sec_struct *secSet;
    xin_ctu_struct *ctu;

    ctu    = (xin_ctu_struct *)opaque;
    picSet = ctu->picSet;
    seqSet = picSet->seqSet;
    secSet = (xin_sec_struct *)Xin26xMemListPop (&seqSet->scratchMem);

    secSet->picSet     = picSet;
    secSet->seqSet     = seqSet;
    secSet->sectionIdx = ctu->sliceIndex;

    Xin266SectionInit (
        secSet,
        ctu);

    Xin266RcCtu (
        secSet,
        ctu);

    Xin266CtuInit (
        secSet,
        ctu);

    Xin266ReadInputCtu (
        secSet,
        ctu);

    Xin266EncodeCtu (
        secSet,
        ctu);

    Xin266CtuInfoUpdate (
        secSet,
        ctu);

    Xin266SaoRdoCtu (
        secSet,
        ctu);

    if (picSet->offlineMode)
    {
        Xin266StoreCtuData (
            secSet,
            ctu);
    }

    Xin266WriteCtu (
        secSet,
        ctu,
        !picSet->offlineMode);

    Xin266RcUpdateCtu (
        secSet,
        ctu);

    Xin266ComputeBsCtu (
        secSet,
        ctu);

    Xin266CtuPostInit (
        secSet,
        ctu);

    Xin26xMemListPush (
        &seqSet->scratchMem,
        (void *)secSet);

}

void Xin266CtuSao (
    void *opaque)
{
    xin_ctu_struct *ctu;

    ctu = (xin_ctu_struct *)opaque;

    Xin266SaoCtu (
        ctu);
}

void Xin266CtuAlfStat (
    void *opaque)
{
    xin_ctu_struct *ctu;

    ctu = (xin_ctu_struct *)opaque;

    Xin266AlfStatCtu (
        ctu);
}

void Xin266CtuLpf (
    void *opaque)
{
    xin_ctu_struct *ctu;

    ctu = (xin_ctu_struct *)opaque;

    Xin266DeblockCtu (
        ctu);
}

void Xin266CtuAlf (
    void *opaque)
{
    xin_ctu_struct *ctu;

    ctu = (xin_ctu_struct *)opaque;

    Xin266AlfCtu (
        ctu);
}

void Xin266CtuCcAlfStat (
    void *opaque)
{
    xin_ctu_struct *ctu;

    ctu = (xin_ctu_struct *)opaque;

    Xin266CcAlfStatCtu (
        ctu);
}

void Xin266CtuCcAlf (
    void *opaque)
{
    xin_ctu_struct *ctu;

    ctu = (xin_ctu_struct *)opaque;

    Xin266CcAlfCtu (
        ctu);
}

void Xin266DeriveAlf (
    void *opaque)
{
    xin_pic_struct *picSet;

    picSet = (xin_pic_struct *)opaque;

    Xin266DeriveAlfFilter (
        picSet);
}

void Xin266DeriveCcAlf (
    void *opaque)
{
    xin_pic_struct *picSet;

    picSet = (xin_pic_struct *)opaque;

    if (!picSet->enableCcAlf)
    {
        return;
    }

    Xin266DeriveCcAlfFilter (
        picSet->alfSet,
        PLANE_CHROMA_U);

    Xin266DeriveCcAlfFilter (
        picSet->alfSet,
        PLANE_CHROMA_V);

}

void Xin266EncodeSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobEnc;
    xin_thread_queue *threadQueue;
    xin_ref_picture  *pictureWrite;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;
    pictureWrite    = picSet->pictureWrite;

    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobEnc    = picSet->jobCtuEnc + ctuRsAddr;
        ctu       = picSet->ctu + ctuRsAddr;

        Xin26xJobInit (
            jobEnc,
            Xin266CtuEnc,
            (void *)ctu);

        if (ctu->availField & XIN_LFT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobEnc,
                jobEnc - 1);
        }

        if (ctu->availField & XIN_TOP_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobEnc,
                jobEnc - frameWidthInCtu);
        }

        if ((ctu->availField & XIN_TOP_CTU_AVAIL) && (ctu->availField & XIN_RGT_CTU_AVAIL) && (!seqSet->config.enableWpp))
        {
            Xin26xThreadJobDepAdd (
                jobEnc,
                jobEnc - frameWidthInCtu + 1);
        }

        if (ctu->ctuAddr == 0)
        {
            Xin26xThreadJobDepAdd (
                jobEnc,
                pictureWrite->jobPreFrame);
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobEnc);

    }

}

void Xin266LpfSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobEnc;
    xin_job_desc     *jobLpf;
    xin_thread_queue *threadQueue;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;

    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobEnc    = picSet->jobCtuEnc + ctuRsAddr;
        jobLpf    = picSet->jobCtuLpf + ctuRsAddr;
        ctu       = picSet->ctu + ctuRsAddr;

        Xin26xJobInit (
            jobLpf,
            Xin266CtuLpf,
            (void *)ctu);

        if (ctu->availField & XIN_LFT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobLpf,
                jobLpf - 1);
        }

        if (ctu->availField & XIN_TOP_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobLpf,
                jobLpf - frameWidthInCtu);
        }

        if (ctu->availField & XIN_BOT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobLpf,
                jobEnc + frameWidthInCtu);
        }

        if (ctu->availField & XIN_RGT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobLpf,
                jobEnc + 1);
        }

        if ((ctu->availField & XIN_RGT_CTU_AVAIL) && (ctu->availField & XIN_BOT_CTU_AVAIL))
        {
            Xin26xThreadJobDepAdd (
                jobLpf,
                jobEnc + frameWidthInCtu + 1);
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobLpf);

    }

}

void Xin266SaoSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobLpf;
    xin_job_desc     *jobSao;
    xin_thread_queue *threadQueue;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;

    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobLpf    = picSet->jobCtuLpf + ctuRsAddr;
        jobSao    = picSet->jobCtuSao + ctuRsAddr;
        ctu       = picSet->ctu + ctuRsAddr;

        Xin26xJobInit (
            jobSao,
            Xin266CtuSao,
            (void *)ctu);

        if (ctu->availField & XIN_LFT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobSao,
                jobSao - 1);
        }

        if (ctu->availField & XIN_TOP_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobSao,
                jobSao - frameWidthInCtu);
        }

        if ((ctu->availField & XIN_TOP_CTU_AVAIL) && (ctu->availField & XIN_RGT_CTU_AVAIL))
        {
            Xin26xThreadJobDepAdd (
                jobSao,
                jobSao - frameWidthInCtu + 1);
        }

        if (ctu->availField & XIN_BOT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobSao,
                jobLpf + frameWidthInCtu);
        }

        if (ctu->availField & XIN_RGT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobSao,
                jobLpf + 1);
        }

        if ((ctu->availField & XIN_RGT_CTU_AVAIL) && (ctu->availField & XIN_BOT_CTU_AVAIL))
        {
            Xin26xThreadJobDepAdd (
                jobSao,
                jobLpf + frameWidthInCtu + 1);
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobSao);

    }

}

void Xin266AlfStatSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobSao;
    xin_job_desc     *jobAlfStat;
    xin_thread_queue *threadQueue;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;

    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr  = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobAlfStat = picSet->jobCtuAlfStat + ctuRsAddr;
        jobSao     = picSet->jobCtuSao + ctuRsAddr;
        ctu        = picSet->ctu + ctuRsAddr;

        Xin26xJobInit (
            jobAlfStat,
            Xin266CtuAlfStat,
            (void *)ctu);

        Xin26xThreadJobDepAdd (
            jobAlfStat,
            jobSao);

        if (ctu->availField & XIN_BOT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobAlfStat,
                jobSao + frameWidthInCtu);
        }

        if (ctu->availField & XIN_RGT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobAlfStat,
                jobSao + 1);
        }

        if ((ctu->availField & XIN_RGT_CTU_AVAIL) && (ctu->availField & XIN_BOT_CTU_AVAIL))
        {
            Xin26xThreadJobDepAdd (
                jobAlfStat,
                jobSao + frameWidthInCtu + 1);
        }

        if ((ctu->availField & XIN_TOP_CTU_AVAIL) && (ctu->availField & XIN_RGT_CTU_AVAIL))
        {
            Xin26xThreadJobDepAdd (
                jobAlfStat,
                jobSao - frameWidthInCtu + 1);
        }

        if (ctu->availField & XIN_LFT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobAlfStat,
                jobAlfStat - 1);
        }

        if (ctu->availField & XIN_TOP_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobAlfStat,
                jobAlfStat - frameWidthInCtu);
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobAlfStat);

    }

}

void Xin266DeriveAlfFrame (
    xin_pic_struct *picSet)
{
    xin_job_desc     *jobDeriveAlf;
    xin_job_desc     *jobAlfStatLast;
    xin_seq_struct   *seqSet;
    xin_thread_queue *threadQueue;

    seqSet         = picSet->seqSet;
    jobDeriveAlf   = picSet->jobDeriveAlf;
    jobAlfStatLast = picSet->jobCtuAlfStat + seqSet->frameSizeInCtu - 1;
    threadQueue    = seqSet->threadQueue;

    Xin26xJobInit (
        jobDeriveAlf,
        Xin266DeriveAlf,
        (void *)picSet);

    Xin26xThreadJobDepAdd (
        jobDeriveAlf,
        jobAlfStatLast);

    Xin26xThreadSubmit (
        threadQueue,
        jobDeriveAlf);

}

void Xin266AlfSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobDeriveAlf;
    xin_job_desc     *jobAlf;
    xin_thread_queue *threadQueue;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;
    jobDeriveAlf    = picSet->jobDeriveAlf;

    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobAlf    = picSet->jobCtuAlf + ctuRsAddr;
        ctu       = picSet->ctu + ctuRsAddr;

        Xin26xJobInit (
            jobAlf,
            Xin266CtuAlf,
            (void *)ctu);

        if (ctuRsAddr == 0)
        {
            Xin26xThreadJobDepAdd (
                jobAlf,
                jobDeriveAlf);
        }
        else
        {
            if (ctu->availField & XIN_LFT_CTU_AVAIL)
            {
                Xin26xThreadJobDepAdd (
                    jobAlf,
                    jobAlf - 1);
            }

            if (ctu->availField & XIN_TOP_CTU_AVAIL)
            {
                Xin26xThreadJobDepAdd (
                    jobAlf,
                    jobAlf - frameWidthInCtu);
            }
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobAlf);

    }

}

void Xin266CcAlfStatSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobAlf;
    xin_job_desc     *jobCcAlfStat;
    xin_thread_queue *threadQueue;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;


    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr    = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobCcAlfStat = picSet->jobCtuCcAlfStat + ctuRsAddr;
        ctu          = picSet->ctu + ctuRsAddr;
        jobAlf       = picSet->jobCtuAlf + ctuRsAddr;

        Xin26xJobInit (
            jobCcAlfStat,
            Xin266CtuCcAlfStat,
            (void *)ctu);

        Xin26xThreadJobDepAdd (
            jobCcAlfStat,
            jobAlf);

        if (ctu->availField & XIN_LFT_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobCcAlfStat,
                jobCcAlfStat - 1);
        }

        if (ctu->availField & XIN_TOP_CTU_AVAIL)
        {
            Xin26xThreadJobDepAdd (
                jobCcAlfStat,
                jobCcAlfStat - frameWidthInCtu);
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobCcAlfStat);

    }

}

void Xin266DeriveCcAlfFrame (
    xin_pic_struct *picSet)
{
    xin_job_desc     *jobDeriveCcAlf;
    xin_job_desc     *jobCcAlfStatLast;
    xin_seq_struct   *seqSet;
    xin_thread_queue *threadQueue;

    seqSet           = picSet->seqSet;
    jobDeriveCcAlf   = picSet->jobDeriveCcAlf;
    jobCcAlfStatLast = picSet->jobCtuCcAlfStat + seqSet->frameSizeInCtu - 1;
    threadQueue      = seqSet->threadQueue;

    Xin26xJobInit (
        jobDeriveCcAlf,
        Xin266DeriveCcAlf,
        (void *)picSet);

    Xin26xThreadJobDepAdd (
        jobDeriveCcAlf,
        jobCcAlfStatLast);

    Xin26xThreadSubmit (
        threadQueue,
        jobDeriveCcAlf);

}

void Xin266CcAlfSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx)
{
    xin_seq_struct   *seqSet;
    xin266_tile_dim  *tileDim;
    UINT32           firstTsAddr;
    UINT32           ctuIdx;
    UINT32           ctuRsAddr;
    xin_ctu_struct   *ctu;
    xin_job_desc     *jobCcAlf;
    xin_thread_queue *threadQueue;
    SINT32           frameWidthInCtu;

    seqSet          = picSet->seqSet;
    tileDim         = seqSet->tileDim + sectionIdx;
    firstTsAddr     = tileDim->firstTsCtu;
    threadQueue     = seqSet->threadQueue;
    frameWidthInCtu = seqSet->frameWidthInCtu;

    for (ctuIdx = 0; ctuIdx < tileDim->ctuNumInTile; ctuIdx++)
    {
        ctuRsAddr = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIdx];
        jobCcAlf  = picSet->jobCtuCcAlf + ctuRsAddr;
        ctu       = picSet->ctu + ctuRsAddr;

        Xin26xJobInit (
            jobCcAlf,
            Xin266CtuCcAlf,
            (void *)ctu);

        if (ctuRsAddr == 0)
        {
            Xin26xThreadJobDepAdd (
                jobCcAlf,
                picSet->jobCtuAlf + seqSet->frameSizeInCtu - 1);

            Xin26xThreadJobDepAdd (
                jobCcAlf,
                picSet->jobDeriveCcAlf);
        }
        else
        {
            if (ctu->availField & XIN_LFT_CTU_AVAIL)
            {
                Xin26xThreadJobDepAdd (
                    jobCcAlf,
                    jobCcAlf - 1);
            }

            if (ctu->availField & XIN_TOP_CTU_AVAIL)
            {
                Xin26xThreadJobDepAdd (
                    jobCcAlf,
                    jobCcAlf - frameWidthInCtu);
            }
        }

        Xin26xThreadSubmit (
            threadQueue,
            jobCcAlf);

    }

}

void Xin266WriteSection (
    xin_sec_struct *secSet)
{
    xin_pic_struct    *picSet;
    xin_ref_picture   *pictureWrite;
    xin_seq_struct    *seqSet;
    xin266_tile_dim   *tileDim;
    xin_cabac_context *cabacSet;
    UINT32            firstTsAddr;
    UINT32            ctuNumInTile;
    UINT32            ctuIndex;
    UINT32            ctuRsAddr;
    UINT32            tileIndex;
    UINT32            ctuY;
    UINT32            frameType;
    BOOL              resetContext;
    xin_ctu_struct    *ctu;

    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    tileIndex    = secSet->sectionIdx;
    tileDim      = (!seqSet->config.enableTiles) && (!seqSet->config.enableWpp) ? &seqSet->frameDim : seqSet->tileDim + tileIndex;
    firstTsAddr  = tileDim->firstTsCtu;
    ctuNumInTile = tileDim->ctuNumInTile;
    cabacSet     = picSet->cabacSet[tileIndex];
    pictureWrite = picSet->pictureWrite;
    frameType    = XIN_MIN (pictureWrite->frameType, XIN_I_FRAME);
    resetContext = (!seqSet->config.enableWpp) || (!tileIndex);

    secSet->tileDim  = tileDim;
    secSet->qp       = picSet->picQp;
    secSet->cabacSet = cabacSet;

    Xin266InitBitstream (
        &cabacSet->cabac.bitstream);

    Xin266CabacContextInit (
        cabacSet,
        seqSet->cabacContext,
        frameType,
        secSet->qp,
        resetContext);

    for (ctuIndex = 0; ctuIndex < ctuNumInTile; ctuIndex++)
    {
        ctuRsAddr = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIndex];
        ctu       = picSet->ctu + ctuRsAddr;
        ctuY      = ctu->ctuY;

        secSet->codingDeltaQp = seqSet->config.enableCuQpDelta;
        secSet->ctu           = ctu;
        secSet->refQp         = picSet->ctuRowRefQp[ctuY];

        Xin266WriteCtu (
            secSet,
            ctu,
            TRUE);

        if (seqSet->config.enableWpp)
        {
            if ((ctuIndex == 0) && ((secSet->sectionIdx + 1) != seqSet->frameHeightInCtu))
            {
                memcpy (
                    picSet->cabacContext[secSet->sectionIdx + 1],
                    &secSet->cabacSet->context,
                    XIN_NUM_OF_CTX*sizeof(xin_prob_model));
            }
        }

        picSet->ctuRowRefQp[ctuY] = secSet->refQp;
        picSet->hmvpNum[ctuY]     = secSet->hmvpNum;

    }

}


