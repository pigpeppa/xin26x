/***************************************************************************//**
 *
 * @file          h266_enc_init.h
 * @brief         This file declares frame, section, CTU and
 *                CU level initialization subroutines.
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
#ifndef _h266_enc_init_h_
#define _h266_enc_init_h_

void Xin266CtuInit (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266CtuPostInit (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266CuInit (
    xin_sec_struct *secSet,
    xin_cu_struct  *parentCu,
    UINT32         splitType,
    UINT32         partIdx);

void Xin266CalcCuContext (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu);

void Xin266CuPostInit (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu);

void Xin266SectionInit (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu);

void Xin266SaoSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266AlfStatSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266DeriveAlfFrame (
    xin_pic_struct *picSet);

void Xin266AlfSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266CcAlfSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266CcAlfStatSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266DeriveCcAlfFrame (
    xin_pic_struct *picSet);

void Xin266CuWrapUp (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu);

void Xin266FramePreInit (
    xin_pic_struct    *picSet,
    xin_input_picture *inputPicture,
    xin_ref_picture   *pictureWrite);

void Xin266PreFrame (
    xin_pic_struct *picSet);

void Xin266FrameProInit (
    xin_pic_struct *picSet);

void Xin266ProFrame (
    xin_pic_struct *picSet);

void Xin266EncodeSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266LpfSection (
    xin_pic_struct *picSet,
    UINT32         sectionIdx);

void Xin266WriteSection (
    xin_sec_struct *secSet);

void Xin266EncodeSectionWrapper (
    void *inputArg,
    void *localMem);

void Xin266DeblockSaoSection (
    xin_sec_struct * secSet);

void Xin266CuInfoUpdate (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu);

void Xin266MctfStage (
    xin266_encoder_struct *h266Encoder);

void Xin266SceneCutStage (
    xin266_encoder_struct *h266Encoder);

void Xin266LookaheadStage (
    xin266_encoder_struct *h266Encoder);

void Xin266ContructPictureRps (
    xin_pic_struct  *picSet,
    xin_ref_picture *pictureWrite,
    xin_rps_struct  *inputRps);

void Xin266ConstructPictureRead (
    xin_pic_struct  *picSet,
    xin_ref_picture *pictureWrite);

SINT32  Xin266CreateModeChunk (
    xin_sec_struct *secSet,
    UINT32         chunkNum);

SINT32  Xin266CreateCuChunk (
    xin_pic_struct *picSet,
    UINT32         tileIdx);

SINT32  Xin266CreateTuChunk (
    xin_pic_struct *picSet,
    UINT32         tileIdx);


#endif
