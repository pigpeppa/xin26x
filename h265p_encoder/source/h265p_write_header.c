/***************************************************************************//**
 *
 * @file          h265p_write_header.c
 * @brief         av1 sequence and slice header VLC coding.
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
#include "h265p_entropy_manipulate.h"
#include "h265p_intra_prediction.h"
#include "h265p_md_buf_manipulate.h"
#include "h26x_compute_dist.h"
#include "h265p_common_data.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_bit_stream.h"

static inline UINT32 Xin265pUlebSizeInByte (
    UINT32  value)
{
    UINT32  size;

    size = 0;

    do
    {
        ++size;
    }
    while ((value >>= 7) != 0);

    return size;
}

void Xin265pGenerateUleb (
    UINT32  value,
    UINT8   *codedValue,
    UINT8   *codedSize)
{
    UINT32  lebSize;
    UINT32  idx;
    UINT32  codedByte;

    lebSize = Xin265pUlebSizeInByte (value);

    if ((value > XIN_MAX_U32) || (lebSize > 8) || (lebSize > 4) || (codedValue == NULL) || (codedSize == NULL))
    {
        return;
    }

    for (idx = 0; idx < lebSize; ++idx)
    {
        codedByte = value & 0x7f;
        value   >>= 7;

        if (value != 0)
        {
            codedByte |= 0x80; // Signal that more bytes follow.
        }

        *(codedValue + idx) = (UINT8)codedByte;
    }

    *codedSize = (UINT8)lebSize;

}

void Xin265pGenObuHeaderAndUleb (
    UINT8   *obuValue,
    UINT8   *ulebValue,
    UINT8   *ulebLength,
    UINT32  obuType,
    UINT32  obuSize)
{
    UINT32  header;

    header  = 0;
    header |= 0 << 7;           // obu_forbidden_bit
    header |= obuType << 3;     // obu_type
    header |= 0 << 2;           // obu_extension_flag
    header |= 1 << 1;           // obu_has_size_field
    header |= 0 << 0;           // obu_reserved_1bit

    Xin265pGenerateUleb (
        obuSize,
        ulebValue,
        ulebLength);

    *obuValue = (UINT8)header;
}

static BOOL XinDoesLevelMatch (
    SINT32  width,
    SINT32  height,
    double  fps,
    SINT32  lvlWidth,
    SINT32  lvlHeight,
    double  lvlFps,
    SINT32  lvlDimMult)
{
    SINT64  lvlLumaPels;
    double  lvlDisplaySampleRate;
    SINT64  lumaPels;
    double  displaySampleRate;

    lvlLumaPels          = lvlWidth * lvlHeight;
    lvlDisplaySampleRate = lvlLumaPels * lvlFps;
    lumaPels             = width * height;
    displaySampleRate    = lumaPels * fps;

    return lumaPels <= lvlLumaPels &&
           displaySampleRate <= lvlDisplaySampleRate &&
           width <= lvlWidth * lvlDimMult &&
           height <= lvlHeight * lvlDimMult;

}

static void XinSetLevelTier (
    xin_seq_struct *seqSet)
{
    // TODO(any): This is a placeholder function that only addresses dimensions
    // and max display sample rates.
    // Need to add checks for max bit rate, max decoded luma sample rate, header
    // rate, etc. that are not covered by this function.
    UINT32          level;
    xin265p_cfg_api *config;

    level  = XIN_SEQ_LEVEL_MAX;
    config = &seqSet->config;

    if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 512, 288, 30.0, 4))
    {
        level = XIN_SEQ_LEVEL_2_0;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 704, 396, 30.0, 4))
    {
        level = XIN_SEQ_LEVEL_2_1;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 1088, 612, 30.0, 4))
    {
        level = XIN_SEQ_LEVEL_3_0;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 1376, 774, 30.0, 4))
    {
        level = XIN_SEQ_LEVEL_3_1;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 2048, 1152, 30.0, 3))
    {
        level = XIN_SEQ_LEVEL_4_0;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 2048, 1152, 60.0, 3))
    {
        level = XIN_SEQ_LEVEL_4_1;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 4096, 2176, 30.0, 2))
    {
        level = XIN_SEQ_LEVEL_5_0;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 4096, 2176, 60.0, 2))
    {
        level = XIN_SEQ_LEVEL_5_1;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 4096, 2176, 120.0, 2))
    {
        level = XIN_SEQ_LEVEL_5_2;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 8192, 4352, 30.0, 2))
    {
        level = XIN_SEQ_LEVEL_6_0;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 8192, 4352, 60.0, 2))
    {
        level = XIN_SEQ_LEVEL_6_1;
    }
    else if (XinDoesLevelMatch(config->inputWidth, config->inputHeight, config->frameRate, 8192, 4352, 120.0, 2))
    {
        level = XIN_SEQ_LEVEL_6_2;
    }

    seqSet->config.level = level;

}

void XinWriteColorConfig (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet)
{
    (void)seqSet;

    // high_bitdepth
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // mono_chrome
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // color_description_present_flag
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // color_range
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // chroma_sample_position
    Xin265pWriteBits (
        bitstream,
        0,
        2);

    // separate_uv_delta_q
    Xin265pWriteBits (
        bitstream,
        0,
        1);

}

void Xin265pGenIvfFileHeader (
    xin_seq_struct *seqSet,
    UINT8          *ivfFileHeader)
{
    UINT32 ivfSig;
    UINT32 av14Cc;

    ivfSig = 0x46494B44;
    av14Cc = 0x31305641;

    // ivf signature
    *((UINT32 *)(ivfFileHeader + 0x00)) = ivfSig;
   
    // version
    *((UINT16 *)(ivfFileHeader + 0x04)) = 0;

    // header size
    *((UINT16 *)(ivfFileHeader + 0x06)) = 32;

    // fourcc
    *((UINT32 *)(ivfFileHeader + 0x08)) = av14Cc;

    // width
    *((UINT16 *)(ivfFileHeader + 0x0c)) = (UINT16)seqSet->config.inputWidth;

    // height
    *((UINT16 *)(ivfFileHeader + 0x0e)) = (UINT16)seqSet->config.inputHeight;

    // rate
    *((UINT32 *)(ivfFileHeader + 0x10)) = (UINT32)(seqSet->config.frameRate);
    
    // scale
    *((UINT32 *)(ivfFileHeader + 0x14)) = 1;

    // frame count
    *((UINT32 *)(ivfFileHeader + 0x18)) = XIN_MAX (seqSet->config.frameToBeEncoded, 1);

    *((UINT32 *)(ivfFileHeader + 0x1c)) = 0;
    
}

void Xin265pGenIvfFrameHeader (
    xin_seq_struct *seqSet,
    UINT8          *ivfFrameHeader,
    UINT32         frameSize,
    UINT64         poc)
{
    (void)seqSet;

    // frame size
    *((UINT32 *)(ivfFrameHeader + 0x00)) = frameSize;

    // pts
    *((UINT32 *)(ivfFrameHeader + 0x04)) = (UINT32)poc;

    *((UINT32 *)(ivfFrameHeader + 0x08)) = 0;
}

void Xin265pWriteSeqHeader (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet)
{
    XinSetLevelTier (
        seqSet);

    // seq_profile
    Xin265pWriteBits (
        bitstream,
        seqSet->config.profile,
        XIN_PROFILE_BIT_NUM);

    // still_picture
    Xin265pWriteBits (
        bitstream,
        seqSet->config.stillPicture,
        1);

    // reduced_still_picture_header
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // timing_info_present_flag
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // initial_display_delay_present_flag
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // operating_points_cnt_minus_1
    Xin265pWriteBits (
        bitstream,
        0,
        5);

    // operating_point_idc[i]
    Xin265pWriteBits (
        bitstream,
        0,
        12);

    // seq_level_idx[i]
    Xin265pWriteBits (
        bitstream,
        seqSet->config.level,
        XIN_LEVEL_BIT_NUM);

    // seq_tier
    if (seqSet->config.level >= XIN_SEQ_LEVEL_4_0)
    {
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }

    // frame_width_bits_minus_1
    Xin265pWriteBits (
        bitstream,
        16 - 1,
        4);

    // frame_height_bits_minus_1
    Xin265pWriteBits (
        bitstream,
        16 - 1,
        4);

    // max_frame_width_minus_1
    Xin265pWriteBits (
        bitstream,
        seqSet->config.inputWidth - 1,
        16);

    // max_frame_height_minus_1
    Xin265pWriteBits (
        bitstream,
        seqSet->config.inputHeight - 1,
        16);

    // frame_id_numbers_present_flag
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // use_128x128_superblock
    Xin265pWriteBits (
        bitstream,
        seqSet->config.sbSize == 128,
        1);

    // enable_filter_intra
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableFilterIntra,
        1);

    // enable_intra_edge_filter
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableIntraEdgeFilter,
        1);

    // enable_interintra_compound
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableInterIntraCompound,
        1);

    // enable_masked_compound
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableMaskedCompound,
        1);

    // enable_warped_motion
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableWarpedMotion,
        1);

    // enable_dual_filter
    Xin265pWriteBits(
        bitstream,
        seqSet->config.enableDualFilter,
        1);

    // enable_order_hint
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableOrderHint,
        1);

    if (seqSet->config.enableOrderHint)
    {
        // enable_jnt_comp
        Xin265pWriteBits (
            bitstream,
            0,
            1);

        // enable_ref_frame_mvs
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }

    // seq_choose_screen_content_tools
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // seq_force_screen_content_tools
    Xin265pWriteBits(
        bitstream,
        0,
        1);

    if ( seqSet->config.enableOrderHint )
    {
        // order_hint_bits_minus_1
        Xin265pWriteBits (
            bitstream,
            seqSet->config.orderHintBits,
            3);
    }

    // enable_superres
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // enable_cdef
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableCdef,
        1);

    // enable_restoration
    Xin265pWriteBits (
        bitstream,
        seqSet->config.enableRestoration,
        1);

    // color config
    XinWriteColorConfig (
        bitstream,
        seqSet);

    // film_grain_params_present
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    Xin265pWriteFlush (
        bitstream);

}

void Xin265pFrameSize (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}

void Xin265pRenderSize (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)picSet;

    // render_and_frame_size_different
    Xin265pWriteBits (
        bitstream,
        0,
        1);
}

void Xin265pWriteTileInfo (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    xin_seq_struct  *seqSet;
    UINT32          logTileCol;
    UINT32          logTileRow;

    seqSet     = picSet->seqSet;
    logTileCol = calcLog2[seqSet->config.numTileCols];
    logTileRow = calcLog2[seqSet->config.numTileRows];

    // uniform_tile_spacing_flag
    Xin265pWriteBits (
        bitstream,
        1,
        1);

    if (logTileCol)
    {
        do
        {
            Xin265pWriteBits (
                bitstream,
                1,
                1);

            logTileCol >>= 1;

        }
        while(logTileCol);
    }

    Xin265pWriteBits (
        bitstream,
        0,
        1);

    if (logTileRow)
    {
        do
        {
            Xin265pWriteBits (
                bitstream,
                1,
                1);

            logTileRow >>= 1;

        }
        while(logTileRow);
    }

    Xin265pWriteBits (
        bitstream,
        0,
        1);

}

void Xin265pWriteQuantParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    // base_q_idx
    Xin265pWriteBits (
        bitstream,
        picSet->baseQIdx,
        8);

    if (picSet->deltaQDc[PLANE_LUMA])
    {
        Xin265pWriteSu (
            bitstream,
            picSet->deltaQDc[PLANE_LUMA],
            6);
    }
    else
    {
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }

    if (picSet->deltaQDc[PLANE_CHROMA])
    {
        Xin265pWriteSu (
            bitstream,
            picSet->deltaQDc[PLANE_CHROMA],
            6);
    }
    else
    {
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }

    if (picSet->deltaQAc[PLANE_CHROMA])
    {
        Xin265pWriteSu (
            bitstream,
            picSet->deltaQDc[PLANE_CHROMA],
            6);
    }
    else
    {
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }

    // using_qmatrix
    Xin265pWriteBits (
        bitstream,
        0,
        1);

}

void Xin265pWriteSegmentationParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)picSet;

    // segmentation_enabled
    Xin265pWriteBits (
        bitstream,
        0,
        1);
}

void Xin265pWriteDeltaQParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    if ( picSet->baseQIdx > 0 )
    {
        // delta_q_present
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }
}

void Xin265pWriteDeltaLfParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}

void Xin265pWriteLoopFilterParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    xin_ref_picture *pictureWrite;

    pictureWrite = picSet->pictureWrite;

    // loop_filter_level[ 0 ]
    Xin265pWriteBits (
        bitstream,
        pictureWrite->fltLvl[0],
        6);

    // loop_filter_level[ 1 ]
    Xin265pWriteBits (
        bitstream,
        pictureWrite->fltLvl[1],
        6);

    if (pictureWrite->fltLvl[0] || pictureWrite->fltLvl[1])
    {
        // loop_filter_level[ 2 ]
        Xin265pWriteBits (
            bitstream,
            pictureWrite->fltLvlU,
            6);

        // loop_filter_level[ 3 ]
        Xin265pWriteBits (
            bitstream,
            pictureWrite->fltLvlV,
            6);
    }

    // loop_filter_sharpness
    Xin265pWriteBits (
        bitstream,
        0,
        3);

    // loop_filter_delta_enabled
    Xin265pWriteBits (
        bitstream,
        0,
        1);

}

void Xin265pWriteCdefParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}

void Xin265pWriteLrParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}


void Xin265pWriteTxMode (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)picSet;

    // tx_mode_select
    Xin265pWriteBits (
        bitstream,
        0,
        1);
}

void Xin265pWriteFrameReferenceMode (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet,
    BOOL           frameIsIntra)
{
    (void)picSet;

    if (!frameIsIntra)
    {
        // tx_mode_select
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }
}

void Xin265pWriteSkipModeParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}

void Xin265pWriteGlobalMotionParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}

void Xin265pWriteFilmGrainParam (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    (void)bitstream;
    (void)picSet;
}

void Xin265pWriteSliceHeader (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    UINT32          orderHint;
    xin_frame_type  frameType;
    BOOL            frameIsIntra;
    UINT32          frameIdx;

    seqSet       = picSet->seqSet;
    pictureWrite = picSet->pictureWrite;
    frameIsIntra = (pictureWrite->frameType >= XIN_I_FRAME);
    frameType    = frameIsIntra ? XIN_KEY_FRAME : XIN_INTER_FRAME;

    // show_existing_frame
    Xin265pWriteBits (
        bitstream,
        picSet->showExistingFrame,
        1);

    if (picSet->showExistingFrame)
    {
        return;
    }

    // frame_type
    Xin265pWriteBits (
        bitstream,
        frameType,
        2);

    // show_frame
    Xin265pWriteBits (
        bitstream,
        pictureWrite->showFrame,
        1);

    if (!pictureWrite->showFrame)
    {
        // showable_frame
        Xin265pWriteBits (
            bitstream,
            pictureWrite->showableFrame,
            1);
    }

    if (!(frameType == XIN_KEY_FRAME && pictureWrite->showFrame))
    {
        // error_resilient_mode
        Xin265pWriteBits (
            bitstream,
            0,
            1);
    }

    // disable_cdf_update
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    // frame_size_override_flag
    Xin265pWriteBits (
        bitstream,
        0,
        1);

    if (seqSet->config.orderHintBits > 0)
    {
        orderHint = (pictureWrite->framePoc + (1 << seqSet->config.orderHintBits)) & ((1 << seqSet->config.orderHintBits)-1);

        // order_hint
        Xin265pWriteBits (
            bitstream,
            orderHint,
            seqSet->config.orderHintBits);
    }

    // primary_ref_frame
    if (!frameIsIntra)
    {
        Xin265pWriteBits (
            bitstream,
            XIN_PRIMARY_REF_NONE,
            3);
    }

    if (((frameType == XIN_KEY_FRAME) && (!pictureWrite->showFrame)) || (frameType == XIN_INTER_FRAME))
    {
        // refresh_frame_flags
        Xin265pWriteBits (
            bitstream,
            pictureWrite->refreshFrameFlags,
            8);
    }

    if (frameIsIntra)
    {
        Xin265pFrameSize (
            bitstream,
            picSet);
    }
    else
    {
        // frame_refs_short_signaling
        Xin265pWriteBits (
            bitstream,
            0,
            1);

        for ( frameIdx = 0; frameIdx < XIN_REF_FRAME_NUM; frameIdx++ )
        {
            // ref_frame_idx
            Xin265pWriteBits (
                bitstream,
                picSet->refFrameIdx[frameIdx],
                3);
        }
    }

    Xin265pRenderSize (
        bitstream,
        picSet);

    // disable_frame_end_update_cdf
    Xin265pWriteBits (
        bitstream,
        1,
        1);

    // tile_info
    Xin265pWriteTileInfo (
        bitstream,
        picSet);

    // quantization_params
    Xin265pWriteQuantParam (
        bitstream,
        picSet);

    // segmentation_params
    Xin265pWriteSegmentationParam (
        bitstream,
        picSet);

    // delta_q_params
    Xin265pWriteDeltaQParam (
        bitstream,
        picSet);

    // delta_lf_params
    Xin265pWriteDeltaLfParam (
        bitstream,
        picSet);

    // loop_filter_param
    Xin265pWriteLoopFilterParam (
        bitstream,
        picSet);

    // cdef_params
    Xin265pWriteCdefParam (
        bitstream,
        picSet);

    // lr_params
    Xin265pWriteLrParam (
        bitstream,
        picSet);

    // read_tx_mode
    Xin265pWriteTxMode (
        bitstream,
        picSet);

    // frame_reference_mode
    Xin265pWriteFrameReferenceMode (
        bitstream,
        picSet,
        frameIsIntra);

    // skip_mode_params
    Xin265pWriteSkipModeParam (
        bitstream,
        picSet);

    // reduced_tx_set
    Xin265pWriteBits (
        bitstream,
        1,
        1);

    // global_motion_params
    Xin265pWriteGlobalMotionParam (
        bitstream,
        picSet);

    // film_grain_params
    Xin265pWriteFilmGrainParam (
        bitstream,
        picSet);

    Xin265pWriteByteAlign (
        bitstream);

}
