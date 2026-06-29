/***************************************************************************//**
 *
 * @file          h266_write_header.c
 * @brief         h266 VPS, SPS, PPS and slice header VLC coding.
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
#include "h26x_definition.h"
#include "h266_definition.h"
#include "assert.h"
#include "xin_typedef.h"
#include "basic_macro.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h266_seq_struct.h"
#include "h266_bit_stream_struct.h"
#include "h266_bit_stream.h"
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
#include "h266_pic_struct.h"
#include "video_macro.h"
#include "h26x_common_data.h"

void Xin266WriteNalUnitHeader (
    xin_bs_struct *bitstream,
    UINT32        nalType,
    UINT32        tempId)
{
    // forbidden_zero_bit
    Xin266WriteOneBit (
        bitstream,
        0);

    // nuh_reserved_zero_bit
    Xin266WriteOneBit (
        bitstream,
        0);

    // nuh_layer_id
    Xin266WriteBits (
        bitstream,
        0,
        6);

    // nal_unit_type
    Xin266WriteBits (
        bitstream,
        nalType,
        5);

    // nuh_temporal_id_plus1
    Xin266WriteBits (
        bitstream,
        tempId+1,
        3);

}

void Xin266CodeProfileTierLevel (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet)
{
    (void)seqSet;

    // general_profile_idc
    Xin266WriteBits (
        bitstream,
        6,
        7);

    // general_tier_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // num_sub_profiles
    Xin266WriteBits (
        bitstream,
        0,
        8);

    // general_progressive_source_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // general_interlaced_source_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // general_non_packed_constraint_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // general_frame_only_constraint_flag
    Xin266WriteOneBit (
        bitstream,
        0);

}

void Xin266WriteVps (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet)
{

    Xin266WriteNalUnitHeader (
        bitstream,
        NAL_UNIT_VPS,
        0);

    // vps_video_parameter_set_id
    Xin266WriteBits (
        bitstream,
        seqSet->vpsId,
        4);

    // vps_max_layers_minus1
    Xin266WriteBits (
        bitstream,
        seqSet->config.temporalLayerNum - 1,
        6);

    // Flush the bitstream
    Xin266WriteFlush (
        bitstream);

}

static void Xin266CodeRefPicList (
    xin_bs_struct  *bitstream,
    xin_rpl_struct *rpl)
{
    SINT32  refIdx;
    SINT32  deltaValue;
    SINT32  absDelta;

    // num_ref_entries[ listIdx ][ rplsIdx ]
    Xin266WriteUvlc (
        bitstream,
        rpl->numOfPics);

    for (refIdx = 0; refIdx < rpl->numOfPics; ++refIdx)
    {
        deltaValue = rpl->deltaPos[refIdx];
        absDelta   = (deltaValue < 0) ? 0 - deltaValue : deltaValue;

        // abs_delta_poc_st[ listIdx ][ rplsIdx ][ i ]
        Xin266WriteUvlc (
            bitstream,
            absDelta - 1);

        if (absDelta > 0)
        {
            // strp_entry_sign_flag[ listIdx ][ rplsIdx ][ i ]
            Xin266WriteOneBit (
                bitstream,
                (deltaValue < 0) ? 0 : 1);  //0  means negative delta POC : 1 means positive
        }

    }

}

void Xin266WriteChromaQp (
    xin_seq_struct *seqSet,
    xin_bs_struct  *bitstream)
{
    UINT32  *qpInVal;
    UINT32  *qpOutVal;
    UINT32  numPtsIdx;
    SINT32  deltaQpInValMinus1[6];
    SINT32  deltaQpOutVal[6];

    qpInVal  = seqSet->config.qpInVal;
    qpOutVal = seqSet->config.qpOutVal;

    for (numPtsIdx = 0; numPtsIdx < seqSet->config.numPtsInQpTable; numPtsIdx++)
    {
        deltaQpInValMinus1[numPtsIdx] = qpInVal[numPtsIdx + 1] - qpInVal[numPtsIdx] - 1;
        deltaQpOutVal[numPtsIdx]      = qpOutVal[numPtsIdx + 1] - qpOutVal[numPtsIdx];
    }

    // same_qp_table_for_chroma
    Xin266WriteOneBit (
        bitstream,
        1);

    // qp_table_start_minus26[ i ]
    Xin266WriteSvlc (
        bitstream,
        (SINT32)qpInVal[0] - 26);

    // num_points_in_qp_table_minus1
    Xin266WriteUvlc (
        bitstream,
        seqSet->config.numPtsInQpTable - 1);

    for (numPtsIdx = 0; numPtsIdx < seqSet->config.numPtsInQpTable; numPtsIdx++)
    {
        // delta_qp_in_val_minus1[ i ][ j ]
        Xin266WriteUvlc (
            bitstream,
            deltaQpInValMinus1[numPtsIdx]);

        // delta_qp_diff_val[ i ][ j ]
        Xin266WriteUvlc (
            bitstream,
            deltaQpOutVal[numPtsIdx] ^ deltaQpInValMinus1[numPtsIdx]);
    }

}

// Write the video parameter set in the bitstream.
void Xin266WriteSps (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet)
{
    UINT32  listIdx;
    UINT32  rplIdx;

    assert (seqSet->config.minCuSize >= XIN_MIN_CU_SIZE);

    Xin266WriteNalUnitHeader (
        bitstream,
        NAL_UNIT_SPS,
        0);

    // sps_seq_parameter_set_id
    Xin266WriteBits (
        bitstream,
        seqSet->spsId,
        4);

    // sps_video_parameter_set_id
    Xin266WriteBits (
        bitstream,
        seqSet->vpsId,
        4);

    // sps_max_sub_layers_minus1
    Xin266WriteBits (
        bitstream,
        seqSet->config.temporalLayerNum - 1,
        3);

    // sps_chroma_format_idc
    Xin266WriteBits (
        bitstream,
        1,
        2);

    // sps_log2_ctu_size_minus5
    Xin266WriteBits (
        bitstream,
        seqSet->lgCtuSize - 5,
        2);

    if (seqSet->vpsId == 0)
    {
        // sps_ptl_dpb_hrd_params_present_flag
        Xin266WriteOneBit (
            bitstream,
            TRUE);

        // general_profile_idc
        Xin266WriteBits (
            bitstream,
            1,
            7);

        // general_tier_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);

        // general_level_idc
        Xin266WriteBits (
            bitstream,
            64,
            8);

        // ptl_frame_only_constraint_flag
        Xin266WriteOneBit (
            bitstream,
            TRUE);

        // ptl_multilayer_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);

        // gci_present_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);

        // gci_alignment_zero_bit
        if (bitstream->bitsLeft & 0x7)
        {
            Xin266WriteBits(
                bitstream,
                0,
                bitstream->bitsLeft);
        }

        // ptl_num_sub_profiles
        Xin266WriteBits (
            bitstream,
            0,
            8);

    }
    else
    {
        // sps_ptl_dpb_hrd_params_present_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }

    // sps_gdr_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_ref_pic_resampling_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // pic_width_max_in_luma_samples
    Xin266WriteUvlc (
        bitstream,
        seqSet->frameWidth);

    // pic_height_max_in_luma_samples
    Xin266WriteUvlc (
        bitstream,
        seqSet->frameHeight);

    if ((seqSet->frameWidth == seqSet->config.inputWidth) && (seqSet->frameHeight == seqSet->config.inputHeight))
    {
        // conformance_window_flag
        Xin266WriteOneBit (
            bitstream,
            0);
    }
    else
    {
        // conformance_window_flag
        Xin266WriteOneBit (
            bitstream,
            1);

        // conf_win_left_offset
        Xin266WriteUvlc (
            bitstream,
            0);

        // conf_win_right_offset
        Xin266WriteUvlc (
            bitstream,
            (seqSet->frameWidth - seqSet->config.inputWidth)/2);

        // conf_win_top_offset
        Xin266WriteUvlc (
            bitstream,
            0);

        // conf_win_bottom_offset
        Xin266WriteUvlc (
            bitstream,
            (seqSet->frameHeight - seqSet->config.inputHeight)/2);

    }

    // sps_subpic_info_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // bit_depth_minus8
    Xin266WriteUvlc (
        bitstream,
        seqSet->config.internalBitDepth - 8);

    // sps_entropy_coding_sync_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableWpp);

    // sps_wpp_entry_point_offsets_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // log2_max_pic_order_cnt_lsb_minus4
    Xin266WriteBits (
        bitstream,
        seqSet->bitsForPOC - 4,
        4);

    // sps_poc_msb_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // num_extra_ph_bits_bytes
    Xin266WriteBits (
        bitstream,
        0,
        2);

    // num_extra_sh_bits_bytes
    Xin266WriteBits (
        bitstream,
        0,
        2);

    // max_dec_pic_buffering_minus1
    Xin266WriteUvlc (
        bitstream,
        9);

    // max_num_reorder_pics
    Xin266WriteUvlc (
        bitstream,
        4);

    // max_latency_increase_plus1
    Xin266WriteUvlc (
        bitstream,
        0);

    // log2_min_luma_coding_block_size_minus2
    Xin266WriteUvlc (
        bitstream,
        calcLog2[seqSet->config.minCuSize] - 2);

    // partition_constraints_override_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_log2_diff_min_qt_min_cb_intra_slice_luma
    Xin266WriteUvlc (
        bitstream,
        calcLog2[seqSet->config.minQtSize] - calcLog2[seqSet->config.minCuSize]);

    // sps_max_mtt_hierarchy_depth_intra_slice_luma
    Xin266WriteUvlc (
        bitstream,
        seqSet->config.maxMttDepth);

    if (seqSet->config.maxMttDepth)
    {
        // sps_log2_diff_max_bt_min_qt_intra_slice_luma
        Xin266WriteUvlc (
            bitstream,
            calcLog2[seqSet->config.maxBtSize] - calcLog2[seqSet->config.minQtSize]);

        // sps_log2_diff_max_tt_min_qt_intra_slice_luma
        Xin266WriteUvlc (
            bitstream,
            calcLog2[seqSet->config.maxTtSize] - calcLog2[seqSet->config.minQtSize]);
    }

    // qtbtt_dual_tree_intra_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_log2_diff_min_qt_min_cb_inter_slice
    Xin266WriteUvlc (
        bitstream,
        calcLog2[seqSet->config.minQtSize] - calcLog2[seqSet->config.minCuSize]);

    // sps_max_mtt_hierarchy_depth_inter_slice
    Xin266WriteUvlc (
        bitstream,
        seqSet->config.maxMttDepth);

    if (seqSet->config.maxMttDepth)
    {
        // sps_log2_diff_max_bt_min_qt_inter_slice
        Xin266WriteUvlc (
            bitstream,
            calcLog2[seqSet->config.maxBtSize] - calcLog2[seqSet->config.minQtSize]);

        // sps_log2_diff_max_tt_min_qt_inter_slice
        Xin266WriteUvlc (
            bitstream,
            calcLog2[seqSet->config.maxTtSize] - calcLog2[seqSet->config.minQtSize]);
    }

    // sps_max_luma_transform_size_64_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.lumaTrSize64);

    // sps_transform_skip_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.transSkipFlag);

    if (seqSet->config.transSkipFlag)
    {
        // sps_log2_transform_skip_max_size_minus2
        Xin266WriteUvlc (
            bitstream,
            seqSet->config.maxTrSkipLgSize - 2);

        // sps_bdpcm_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }

    // sps_mts_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableMts);

    if (seqSet->config.enableMts)
    {
        // sps_explicit_mts_intra_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            0);

        // sps_explicit_mts_inter_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            0);
    }

    // sps_lfnst_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_joint_cbcr_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    Xin266WriteChromaQp (
        seqSet,
        bitstream);

    // sps_sao_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableSao);

    // sps_alf_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableAlf);

    if (seqSet->config.enableAlf)
    {
        // sps_ccalf_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            seqSet->config.enableCcAlf);
    }

    // sps_lmcs_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableLmcs);

    // sps_weighted_pred_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_weighted_bipred_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // long_term_ref_pics_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_idr_rpl_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // rpl1_copy_from_rpl0_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    for (listIdx = 0; listIdx < 2; listIdx++)
    {
        // num_ref_pic_lists_in_sps[ i ]
        Xin266WriteUvlc (
            bitstream,
            seqSet->rplNum[listIdx]);

        for (rplIdx = 0; rplIdx < seqSet->rplNum[listIdx]; rplIdx++)
        {
            Xin266CodeRefPicList (
                bitstream,
                seqSet->rplList[listIdx] + rplIdx);
        }

    }

    // sps_ref_wraparound_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_temporal_mvp_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableTMvp);

    if (seqSet->config.enableTMvp)
    {
        // sps_sbtmvp_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            seqSet->config.enableSbtMvp);
    }

    // sps_amvr_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableAmvr);

    // sps_bdof_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_smvd_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_dmvr_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableDmvr);

    if (seqSet->config.enableDmvr)
    {
        // sps_dmvr_pic_present_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }

    // sps_mmvd_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // six_minus_max_num_merge_cand
    Xin266WriteUvlc(
        bitstream,
        XIN_MAX_MERGE_MV_NUM - seqSet->config.maxMergeCand);

    // sps_sbt_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_affine_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableAffine);

    if (seqSet->config.enableAffine)
    {
        // sps_five_minus_max_num_subblock_merge_cand
        Xin266WriteUvlc (
            bitstream,
            XIN_MAX_AFF_MERGE_MV_NUM - seqSet->config.maxAffineMergeCand);

        // sps_6param_affine_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            seqSet->config.affineType);

        
        if (seqSet->config.enableAmvr)
        {
            // sps_affine_amvr_enabled_flag
            Xin266WriteOneBit (
                bitstream,
                0);
        }

        // sps_affine_prof_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            0);
        
    }

    // sps_bcw_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableBcw);

    // sps_ciip_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    if (seqSet->config.maxMergeCand > 2)
    {
        // sps_gpm_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            0);
    }

    // log2_parallel_merge_level_minus2
    Xin266WriteUvlc (
        bitstream,
        0);

    // sps_isp_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_mrl_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_mip_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_cclm_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableCclm);

    // sps_chroma_horizontal_collocated_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_chroma_vertical_collocated_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_palette_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    if (seqSet->config.transSkipFlag)
    {
        // sps_min_qp_prime_ts
        Xin266WriteUvlc (
            bitstream,
            0);
    }

    // sps_ibc_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_ladf_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_explicit_scaling_list_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_dep_quant_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableDepQuant);

    // sps_sign_data_hiding_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableSignDataHiding);

    // sps_virtual_boundaries_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_general_hrd_params_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // field_seq_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // vui_parameters_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // sps_extension_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    Xin266WriteFlush (
        bitstream);

}

// Write the video parameter set in the bitstream.
void Xin266WritePps (
    xin_bs_struct  *bitstream,
    xin_seq_struct *seqSet)
{
    UINT32  rpsIdx;
    SINT32  refNumActive;

    Xin266WriteNalUnitHeader (
        bitstream,
        NAL_UNIT_PPS,
        0);

    // pps_pic_parameter_set_id
    Xin266WriteBits (
        bitstream,
        seqSet->ppsId,
        6);

    // pps_seq_parameter_set_id
    Xin266WriteBits (
        bitstream,
        seqSet->spsId,
        4);

    // mixed_nalu_types_in_pic_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // pic_width_in_luma_samples
    Xin266WriteUvlc (
        bitstream,
        seqSet->frameWidth);

    // pic_height_in_luma_samples
    Xin266WriteUvlc (
        bitstream,
        seqSet->frameHeight);

    if ((seqSet->frameWidth == seqSet->config.inputWidth) && (seqSet->frameHeight == seqSet->config.inputHeight))
    {
        // conformance_window_flag
        Xin266WriteOneBit (
            bitstream,
            0);
    }
    else
    {
        // conformance_window_flag
        Xin266WriteOneBit (
            bitstream,
            1);

        // conf_win_left_offset
        Xin266WriteUvlc (
            bitstream,
            0);

        // conf_win_right_offset
        Xin266WriteUvlc (
            bitstream,
            (seqSet->frameWidth - seqSet->config.inputWidth)/2);

        // conf_win_top_offset
        Xin266WriteUvlc (
            bitstream,
            0);

        // conf_win_bottom_offset
        Xin266WriteUvlc (
            bitstream,
            (seqSet->frameHeight - seqSet->config.inputHeight)/2);

    }

    // scaling_window_explicit_signalling_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // output_flag_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // no_pic_partition_flag
    Xin266WriteOneBit (
        bitstream,
        1);

    // subpic_id_mapping_in_pps_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // cabac_init_present_flag
    Xin266WriteOneBit (
        bitstream,
        FALSE);

    refNumActive = 1;

    for (rpsIdx = 0; rpsIdx < seqSet->rpsSize; rpsIdx++)
    {
        refNumActive = (seqSet->rpsSet[rpsIdx].numOfNegPics > refNumActive) ? seqSet->rpsSet[rpsIdx].numOfNegPics : refNumActive;
    }

    // num_ref_idx_default_active_minus1[ i ]
    Xin266WriteUvlc (
        bitstream,
        refNumActive - 1);

    // num_ref_idx_default_active_minus1[ i ]
    Xin266WriteUvlc (
        bitstream,
        refNumActive - 1);

    // rpl1_idx_present_flag
    Xin266WriteOneBit (
        bitstream,
        1);

    // pps_weighted_pred_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // pps_weighted_bipred_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // pps_ref_wraparound_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // init_qp_minus26
    Xin266WriteSvlc (
        bitstream,
        0);

    // cu_qp_delta_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.enableCuQpDelta);

    // pps_chroma_tool_offsets_present_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.chromaQpOffset != 0);
    
    if (seqSet->config.chromaQpOffset)
    {
        // pps_cb_qp_offset
        Xin266WriteSvlc (
            bitstream,
            seqSet->config.chromaQpOffset);

        // pps_cr_qp_offset
        Xin266WriteSvlc (
            bitstream,
            seqSet->config.chromaQpOffset);

        // pps_joint_cbcr_qp_offset_present_flag
        Xin266WriteOneBit (
            bitstream,
            0);

        // pps_slice_chroma_qp_offsets_present_flag
        Xin266WriteOneBit (
            bitstream,
            0);

        // pps_cu_chroma_qp_offset_list_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            0);
        
    }
    
    // deblocking_filter_control_present_flag
    Xin266WriteOneBit (
        bitstream,
        1);

    // deblocking_filter_override_enabled_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // pps_deblocking_filter_disabled_flag
    Xin266WriteOneBit (
        bitstream,
        seqSet->config.disableDeblock);

    if (!seqSet->config.disableDeblock)
    {
        // pps_beta_offset_div2
        Xin266WriteSvlc (
            bitstream,
            0);

        // pps_tc_offset_div2
        Xin266WriteSvlc (
            bitstream,
            0);

        if (seqSet->config.chromaQpOffset)
        {
            Xin266WriteSvlc (
                bitstream,
                0);

            Xin266WriteSvlc (
                bitstream,
                0);

            Xin266WriteSvlc (
                bitstream,
                0);

            Xin266WriteSvlc (
                bitstream,
                0);
        }

    }

    // picture_header_extension_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // slice_header_extension_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    // pps_extension_present_flag
    Xin266WriteOneBit (
        bitstream,
        0);

    Xin266WriteFlush (
        bitstream);

}

static void Xin266WriteRefPicLists (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    UINT32          listIdx;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    UINT32          numRefEntry[XIN_LIST_NUM];

    pictureWrite = picSet->pictureWrite;
    seqSet       = picSet->seqSet;

    for (listIdx = 0; listIdx < XIN_LIST_NUM; listIdx++)
    {
        if (seqSet->rplNum[listIdx] > 0)
        {
            // rpl_sps_flag[ i ]
            Xin266WriteOneBit (
                bitstream,
                pictureWrite->useRpsInSps[listIdx]);
        }

        if (pictureWrite->useRpsInSps[listIdx])
        {
            if (seqSet->rplNum[listIdx] > 0)
            {
                if (seqSet->rplNum[listIdx] > 1)
                {
                    // rpl_idx[i]
                    Xin266WriteBits(
                        bitstream,
                        pictureWrite->predIdxInGop,
                        calcLog2[seqSet->rplNum[listIdx] - 1] + 1);
                }
            }
            else
            {
                // num_ref_entries[ listIdx ][ rplsIdx ]
                Xin266WriteUvlc(
                    bitstream,
                    0);
            }

        }
        else
        {
            Xin266CodeRefPicList (
                bitstream,
                pictureWrite->rpl + listIdx);
        }

    }

    if (pictureWrite->frameType >= XIN_I_FRAME)
    {
        return;
    }

    if (pictureWrite->useRpsInSps[XIN_LIST_0])
    {
        numRefEntry[XIN_LIST_0] = seqSet->rplList[XIN_LIST_0][pictureWrite->predIdxInGop].numOfPics;
    }
    else
    {
        numRefEntry[XIN_LIST_0] = pictureWrite->refPicNum[XIN_LIST_0];
    }

    if (pictureWrite->frameType == XIN_B_FRAME)
    {
        if (pictureWrite->useRpsInSps[XIN_LIST_1])
        {
            numRefEntry[XIN_LIST_1] = seqSet->rplList[XIN_LIST_1][pictureWrite->predIdxInGop].numOfPics;
        }
        else
        {
            numRefEntry[XIN_LIST_1] = pictureWrite->refPicNum[XIN_LIST_1];
        }
    }
    else
    {
        numRefEntry[XIN_LIST_1] = 0;
    }

    if ((numRefEntry[XIN_LIST_0] > 1) || (numRefEntry[XIN_LIST_1] > 1))
    {
        if ((pictureWrite->numOfRefs[XIN_LIST_0] != numRefEntry[XIN_LIST_0])
                || ((pictureWrite->numOfRefs[XIN_LIST_1] != numRefEntry[XIN_LIST_1]) && (numRefEntry[XIN_LIST_1] > 0)))
        {
            // num_ref_idx_active_override_flag
            Xin266WriteOneBit (
                bitstream,
                TRUE);

            if (numRefEntry[XIN_LIST_0] > 1)
            {
                // num_ref_idx_l0_active_minus1
                Xin266WriteUvlc (
                    bitstream,
                    pictureWrite->numOfRefs[XIN_LIST_0] - 1);
            }


            if (numRefEntry[XIN_LIST_1] > 1)
            {
                // num_ref_idx_l1_active_minus1
                Xin266WriteUvlc (
                    bitstream,
                    pictureWrite->numOfRefs[XIN_LIST_1] - 1);
            }
        }
        else
        {
            // num_ref_idx_active_override_flag
            Xin266WriteOneBit (
                bitstream,
                FALSE);
        }

    }

}

void Xin266WriteSliceHeader (
    UINT32         firstCtuAddr,
    SINT32         sliceHeaderQp,
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_alf_struct  *alfSet;
    xin_lmcs_struct *lmcsSet;
    UINT32          nalType;
    UINT32          frameType;
    UINT32          pocNumber;

    (void)firstCtuAddr;
    seqSet       = picSet->seqSet;
    alfSet       = picSet->alfSet;
    lmcsSet      = picSet->lmcsSet;
    pictureWrite = picSet->pictureWrite;
    nalType      = pictureWrite->nalType;
    frameType    = pictureWrite->frameType;
    pocNumber    = (pictureWrite->framePoc + (1 << seqSet->bitsForPOC)) & ((1 << seqSet->bitsForPOC)-1);

    Xin266WriteNalUnitHeader (
        bitstream,
        nalType,
        pictureWrite->temporalId);

    // picture_header_in_slice_header_flag
    Xin266WriteOneBit (
        bitstream,
        1);

    // ph_gdr_or_irap_pic_flag
    Xin266WriteOneBit (
        bitstream,
        frameType >= XIN_I_FRAME);

    // ph_non_reference_picture_flag
    Xin266WriteOneBit (
        bitstream,
        !pictureWrite->isReferenced);

    if (frameType >= XIN_I_FRAME)
    {
        // ph_gdr_pic_flag
        Xin266WriteOneBit (
            bitstream,
            0);

        // ph_pic_inter_slice_allowed_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }
    else
    {
        // ph_pic_inter_slice_allowed_flag
        Xin266WriteOneBit (
            bitstream,
            TRUE);

        // ph_pic_intra_slice_allowed_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }

    // ph_pic_parameter_set_id
    Xin266WriteUvlc (
        bitstream,
        seqSet->ppsId);

    // ph_pic_order_cnt_lsb
    Xin266WriteBits (
        bitstream,
        pocNumber,
        seqSet->bitsForPOC);

    if (seqSet->config.enableLmcs)
    {
        // ph_lmcs_enabled_flag
        Xin266WriteOneBit (
            bitstream,
            lmcsSet->lmcsParam.sliceReshaperEnabled);

        if (lmcsSet->lmcsParam.sliceReshaperEnabled)
        {
            // ph_lmcs_aps_id
            Xin266WriteBits (
                bitstream,
                0,
                2);
            
            // ph_chroma_residual_scale_flag
            Xin266WriteOneBit (
                bitstream,
                lmcsSet->lmcsParam.enableChromaAdj);

        }
    }

    if (seqSet->config.enableCuQpDelta)
    {
        // ph_cu_qp_delta_subdiv_intra_slice
        Xin266WriteUvlc (
            bitstream,
            0);
    }

    if (frameType >= XIN_I_FRAME)
    {
        // ph_no_output_of_prior_pics_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }

    if (frameType < XIN_I_FRAME)
    {
        if (seqSet->config.enableTMvp)
        {
            // ph_temporal_mvp_enabled_flag
            Xin266WriteOneBit (
                bitstream,
                TRUE);
        }

        // mvd_l1_zero_flag
        Xin266WriteOneBit (
            bitstream,
            pictureWrite->mvdL1Zero);

        // slice_type
        Xin266WriteUvlc (
            bitstream,
            frameType);

    }

    if (seqSet->config.enableAlf)
    {

        Xin266WriteOneBit (
            bitstream,
            alfSet->alfEnabled[PLANE_LUMA] && picSet->enableAlf);

        if (alfSet->alfEnabled[PLANE_LUMA] && picSet->enableAlf)
        {
            Xin266WriteBits (
                bitstream,
                alfSet->apsNum,
                3);

            if (alfSet->apsNum)
            {
                Xin266WriteBits (
                    bitstream,
                    alfSet->lumaApsId,
                    3);
            }

            Xin266WriteOneBit (
                bitstream,
                alfSet->alfEnabled[PLANE_CHROMA_U]);


            Xin266WriteOneBit (
                bitstream,
                alfSet->alfEnabled[PLANE_CHROMA_V]);

            if (alfSet->alfEnabled[PLANE_CHROMA_U] || alfSet->alfEnabled[PLANE_CHROMA_V])
            {
                Xin266WriteBits (
                    bitstream,
                    alfSet->chromaApsId,
                    3);
            }

            if (seqSet->config.enableCcAlf)
            {
                Xin266WriteOneBit (
                    bitstream,
                    alfSet->ccAlfCbEnabled);

                if (alfSet->ccAlfCbEnabled)
                {
                    // write CC ALF Cb APS ID
                    Xin266WriteBits (
                        bitstream,
                        alfSet->ccAlfCbApsId,
                        3);
                }

                // Cr
                Xin266WriteOneBit (
                    bitstream,
                    alfSet->ccAlfCrEnabled);

                if (alfSet->ccAlfCrEnabled)
                {
                    // write CC ALF Cr APS ID
                    Xin266WriteBits (
                        bitstream,
                        alfSet->ccAlfCrApsId,
                        3);
                }

            }

        }

    }

    if (frameType <= XIN_I_FRAME)
    {
        Xin266WriteRefPicLists (
            bitstream,
            picSet);
    }

    if (seqSet->config.enableTMvp && (pictureWrite->frameType < XIN_I_FRAME))
    {
        if (pictureWrite->frameType == XIN_B_FRAME)
        {
            // collocated_from_l0_flag
            Xin266WriteOneBit (
                bitstream,
                pictureWrite->colFromL0Flag);
        }

        if ((pictureWrite->colFromL0Flag && (pictureWrite->numOfRefs[XIN_LIST_0] > 1))
                || ((!pictureWrite->colFromL0Flag) && (pictureWrite->numOfRefs[XIN_LIST_1] > 1)))
        {
            // collocated_ref_idx
            Xin266WriteUvlc (
                bitstream,
                0);
        }
    }

    // slice_qp_delta
    Xin266WriteSvlc (
        bitstream,
        sliceHeaderQp - 26);

    if (seqSet->config.enableSao)
    {
        // "slice_sao_luma_flag"
        Xin266WriteOneBit(
            bitstream,
            picSet->saoEnabledFlag[0]);

        // "slice_sao_chroma_flag"
        Xin266WriteOneBit(
            bitstream,
            picSet->saoEnabledFlag[1]);
    }

    if (seqSet->config.enableDepQuant)
    {
        // sh_dep_quant_used_flag
        Xin266WriteOneBit (
            bitstream,
            TRUE);
    }

    if (seqSet->config.enableSignDataHiding && !seqSet->config.enableDepQuant)
    {
        // sh_sign_data_hiding_used_flag
        Xin266WriteOneBit (
            bitstream,
            TRUE);
    }

    if ((seqSet->config.transSkipFlag) && (!seqSet->config.enableSignDataHiding) && !seqSet->config.enableDepQuant)
    {
        // sh_ts_residual_coding_disabled_flag
        Xin266WriteOneBit (
            bitstream,
            FALSE);
    }

    Xin266WriteFlush (
        bitstream);

}

void Xin266WriteApsAlf (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    xin_alf_aps     *alfAps;
    xin_ref_picture *pictureWrite;
    UINT32          filterIdx;
    UINT32          bitLength;
    UINT32          filterNum;
    UINT32          numAltChroma;
    UINT32          coeffIdx;
    UINT32          altIdx;
    UINT32          ccIdx;

    alfAps       = picSet->alfSet->alfAps;
    pictureWrite = picSet->pictureWrite;

    Xin266WriteNalUnitHeader (
        bitstream,
        NAL_UNIT_PREFIX_APS,
        pictureWrite->temporalId);

    // aps_params_type
    Xin266WriteBits (
        bitstream,
        XIN_ALF_APS,
        3);

    // adaptation_parameter_set_id
    Xin266WriteBits (
        bitstream,
        0,
        5);

    // aps_chroma_present_flag
    Xin266WriteBits (
        bitstream,
        1,
        1);

    // alf_luma_filter_signal_flag
    Xin266WriteBits (
        bitstream,
        alfAps->alfParam.newFilterFlag[0],
        1);

    // alf_chroma_filter_signal_flag
    Xin266WriteBits (
        bitstream,
        alfAps->alfParam.newFilterFlag[1],
        1);

    // alf_cc_cb_filter_signal_flag
    Xin266WriteBits (
        bitstream,
        alfAps->ccAlfParam.newCcAlfFilter[0],
        1);

    // alf_cc_cr_filter_signal_flag
    Xin266WriteBits (
        bitstream,
        alfAps->ccAlfParam.newCcAlfFilter[1],
        1);

    if (alfAps->alfParam.newFilterFlag[0])
    {
        // alf_luma_clip_flag
        Xin266WriteBits (
            bitstream,
            alfAps->alfParam.nonLinearFlag[0],
            1);

        filterNum = alfAps->alfParam.numLumaFilters;

        // alf_luma_num_filters_signalled_minus1
        Xin266WriteUvlc (
            bitstream,
            filterNum - 1);

        if (filterNum > 1)
        {
            bitLength = calcLog2[filterNum] + ((filterNum & (filterNum - 1)) ? 1 : 0);

            for (filterIdx = 0; filterIdx < XIN_ALF_MAX_CLS_NUM; filterIdx++)
            {
                // alf_luma_coeff_delta_idx
                Xin266WriteBits (
                    bitstream,
                    alfAps->alfParam.filterCoeffDeltaIdx[filterIdx],
                    bitLength);
            }
        }

        // Filter coefficients
        for (filterIdx = 0; filterIdx < filterNum; filterIdx++)
        {
            for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_LUMA_COEF_NUM - 1; coeffIdx++)
            {
                // alf_luma_coeff_abs
                Xin266WriteUvlc (
                    bitstream,
                    XIN_ABS (alfAps->alfParam.lumaCoeff[filterIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx]));

                if (XIN_ABS (alfAps->alfParam.lumaCoeff[filterIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx]))
                {
                    // alf_luma_coeff_sign
                    Xin266WriteBits (
                        bitstream,
                        (alfAps->alfParam.lumaCoeff[filterIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx] < 0) ? 1 : 0,
                        1);
                }
            }
        }

        // Clipping values coding
        if (alfAps->alfParam.nonLinearFlag[0])
        {
            for (filterIdx = 0; filterIdx < filterNum; filterIdx++)
            {
                for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_LUMA_COEF_NUM - 1; coeffIdx++)
                {
                    // alf_luma_clip_idx
                    Xin266WriteBits (
                        bitstream,
                        alfAps->alfParam.lumaClipp[filterIdx * XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx],
                        2);
                }
            }
        }

    }

    if (alfAps->alfParam.newFilterFlag[1])
    {
        // alf_chroma_clip_flag
        Xin266WriteBits (
            bitstream,
            alfAps->alfParam.nonLinearFlag[1],
            1);

        numAltChroma = alfAps->alfParam.numAltChroma;

        // alf_chroma_num_alt_filters_minus1
        Xin266WriteUvlc (
            bitstream,
            numAltChroma - 1);

        for (altIdx = 0; altIdx < numAltChroma; altIdx++)
        {
            for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_CHROMA_COEF_NUM - 1; coeffIdx++)
            {
                // alf_chroma_coeff_abs
                Xin266WriteUvlc (
                    bitstream,
                    XIN_ABS (alfAps->alfParam.chromaCoeff[altIdx][coeffIdx]));

                if (XIN_ABS (alfAps->alfParam.chromaCoeff[altIdx][coeffIdx]))
                {
                    // alf_chroma_coeff_sign
                    Xin266WriteBits (
                        bitstream,
                        (alfAps->alfParam.chromaCoeff[altIdx][coeffIdx] < 0) ? 1 : 0,
                        1);
                }
            }

            if (alfAps->alfParam.nonLinearFlag[1])
            {
                for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_CHROMA_COEF_NUM - 1; coeffIdx++)
                {
                    // alf_luma_clip_idx
                    Xin266WriteBits (
                        bitstream,
                        alfAps->alfParam.chromaClipp[altIdx][coeffIdx],
                        2);
                }
            }

        }

    }

    for (ccIdx = 0; ccIdx < 2; ccIdx++)
    {
        if (alfAps->ccAlfParam.newCcAlfFilter[ccIdx])
        {
            Xin266WriteUvlc (
                bitstream,
                alfAps->ccAlfParam.ccAlfFilterCount[ccIdx] - 1);

            for (filterIdx = 0; filterIdx < alfAps->ccAlfParam.ccAlfFilterCount[ccIdx]; filterIdx++)
            {
                // Filter coefficients
                for (coeffIdx = 0; coeffIdx < XIN_CC_ALF_MAX_COEFF_NUM - 1; coeffIdx++)
                {
                    if (alfAps->ccAlfParam.ccAlfCoeff[ccIdx][filterIdx][coeffIdx] == 0)
                    {
                        Xin266WriteBits (
                            bitstream,
                            0,
                            CCALF_BITS_PER_COEFF_LEVEL);
                    }
                    else
                    {
                        Xin266WriteBits (
                            bitstream,
                            1 + calcLog2[XIN_ABS(alfAps->ccAlfParam.ccAlfCoeff[ccIdx][filterIdx][coeffIdx])],
                            CCALF_BITS_PER_COEFF_LEVEL);

                        Xin266WriteBits (
                            bitstream,
                            alfAps->ccAlfParam.ccAlfCoeff[ccIdx][filterIdx][coeffIdx] < 0 ? 1 : 0,
                            1);
                    }

                }
            }
        }
    }

    // aps_extension_flag
    Xin266WriteBits (
        bitstream,
        0,
        1);

    Xin266WriteFlush (
        bitstream);

}

void Xin266WriteApsLmcs (
    xin_bs_struct  *bitstream,
    xin_pic_struct *picSet)
{
    xin_lmcs_struct *lmcsSet;
    xin_ref_picture *pictureWrite;
    SINT32          idx;
    SINT32          deltaCW;
    SINT32          signCW;
    SINT32          absCW;
    SINT32          deltaCRS;
    SINT32          signCRS;
    SINT32          absCRS;

    pictureWrite = picSet->pictureWrite;
    lmcsSet      = picSet->lmcsSet;

    Xin266WriteNalUnitHeader (
        bitstream,
        NAL_UNIT_PREFIX_APS,
        pictureWrite->temporalId);

    // aps_params_type
    Xin266WriteBits (
        bitstream,
        XIN_LMCS_APS,
        3);

    // adaptation_parameter_set_id
    Xin266WriteBits (
        bitstream,
        0,
        5);

    // aps_chroma_present_flag
    Xin266WriteBits (
        bitstream,
        1,
        1);

    // lmcs_min_bin_idx
    Xin266WriteUvlc (
        bitstream,
        lmcsSet->lmcsParam.reshaperModelMinBinIdx);

    // lmcs_delta_max_bin_idx
    Xin266WriteUvlc (
        bitstream,
        XIN_LMCS_ENCODE_CW_BINS - 1 - lmcsSet->lmcsParam.reshaperModelMaxBinIdx);

    // lmcs_delta_cw_prec_minus1
    Xin266WriteUvlc (
        bitstream,
        lmcsSet->lmcsParam.maxNbitsNeededDeltaCW - 1);

    for (idx = lmcsSet->lmcsParam.reshaperModelMinBinIdx; idx <= lmcsSet->lmcsParam.reshaperModelMaxBinIdx; idx++)
    {
        deltaCW = lmcsSet->lmcsParam.reshaperModelBinCWDelta[idx];
        signCW  = (deltaCW < 0) ? 1 : 0;
        absCW   = (deltaCW < 0) ? (-deltaCW) : deltaCW;

        Xin266WriteBits (
            bitstream,
            absCW,
            lmcsSet->lmcsParam.maxNbitsNeededDeltaCW);

        if (absCW > 0)
        {
            Xin266WriteOneBit (
                bitstream,
                signCW);
        }
    }

    deltaCRS = lmcsSet->lmcsParam.chrResScalingOffset;
    signCRS  = (deltaCRS < 0) ? 1 : 0;
    absCRS   = (deltaCRS < 0) ? (-deltaCRS) : deltaCRS;

    // lmcs_delta_abs_crs
    Xin266WriteBits (
        bitstream,
        absCRS,
        3);

    if (absCRS > 0)
    {
        // lmcs_delta_sign_crs_flag
        Xin266WriteOneBit (
            bitstream,
            signCRS);
    }

    // aps_extension_flag
    Xin266WriteBits (
        bitstream,
        0,
        1);

    Xin266WriteFlush (
        bitstream);

}

