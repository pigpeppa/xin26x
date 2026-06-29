/***************************************************************************//**
 *
 * @file          h266_func_init.c
 * @brief         Subroutines to assign function pointers according to CPU features.
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
#include "h26x_compute_dist.h"
#include "h266_motion_comp.h"
#include "h26x_block_utility.h"
#include "h266_quant_inv_quant.h"
#include "h266_sub_dct.h"
#include "h26x_cpu_detection.h"
#include "h266_idct_add.h"
#include "h266_intra_pred_filter.h"
#include "h266_sao_rdo.h"
#include "h266_loop_filter.h"
#include "h26x_sao.h"
#include "h26x_downscale_subs.h"
#include "h26x_hierarchic_motion_search.h"
#include "h26x_construct_bi_me_input.h"
#include "h26x_one_dim_motion_search.h"
#include "h26x_forward_1d_trans.h"
#include "h26x_forward_2d_trans.h"
#include "h266_compute_dist.h"
#include "h266_inverse_trans.h"
#include "h266_entropy_manipulate.h"
#include "h266_loop_filter.h"
#include "h26x_compute_frame_act.h"
#include "h266_copy_and_pad.h"
#include "h26x_forward_1d_trans.h"
#include "h26x_forward_2d_trans.h"
#include "h26x_picture_copy.h"
#include "h266_compute_gradient.h"
#include "h266_construct_weight_input.h"
#include "h266_dep_quant_struct.h"
#include "h266_dep_quant.h"
#include "h26x_block_transpose.h"
#include "h266_func_struct.h"
#include "h266_intra_prediction.h"
#include "h26x_inverse_1d_trans.h"

static void Xin266InitDctSub (
    xin_func_struct *funcSet)
{
    UINT32  colIdx, rowIdx;

    for (rowIdx = 0; rowIdx < XIN_BLOCK_128xH; rowIdx++)
    {
        for (colIdx = 0; colIdx < XIN_BLOCK_128xH; colIdx++)
        {
            funcSet->pfXinFor2dDct2[colIdx][rowIdx] = Xin26xFDct2WxH;
            funcSet->pfXinInv2dDct2[colIdx][rowIdx] = Xin266IDct2WxH;
        }
    }
}

void Xin266FuncInit (
    xin_func_struct *funcSet,
    UINT32          cpuFeature)
{
    funcSet->pfXinBlockCopy[XIN_BLOCK_1xH]   = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_2xH]   = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_4xH]   = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_8xH]   = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_16xH]  = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_32xH]  = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_64xH]  = Xin26xBlockCopy;
    funcSet->pfXinBlockCopy[XIN_BLOCK_128xH] = Xin26xBlockCopy;

    funcSet->pfXinBlockAvg[XIN_BLOCK_1xH]   = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_2xH]   = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_4xH]   = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_8xH]   = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_16xH]  = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_32xH]  = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_64xH]  = Xin26xBlockAverage;
    funcSet->pfXinBlockAvg[XIN_BLOCK_128xH] = Xin26xBlockAverage;

    funcSet->pfXinComputeSad[XIN_BLOCK_1xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_2xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_4xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_8xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_16xH]  = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_32xH]  = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_64xH]  = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_128xH] = Xin26xComputeSad;

    funcSet->pfXinComputeSadS16[XIN_BLOCK_1xH]   = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_2xH]   = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_4xH]   = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_8xH]   = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_16xH]  = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_32xH]  = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_64xH]  = Xin26xComputeSadS16;
    funcSet->pfXinComputeSadS16[XIN_BLOCK_128xH] = Xin26xComputeSadS16;

    funcSet->pfXinComputeSsd[XIN_BLOCK_1xH]   = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_2xH]   = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_4xH]   = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_8xH]   = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_16xH]  = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_32xH]  = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_64xH]  = Xin26xComputeSsd;
    funcSet->pfXinComputeSsd[XIN_BLOCK_128xH] = Xin26xComputeSsd;

    funcSet->pfXinComputeSsdFd[XIN_BLOCK_1xH]   = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_2xH]   = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_4xH]   = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_8xH]   = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_16xH]  = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_32xH]  = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_64xH]  = Xin266ComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_128xH] = Xin266ComputeSsdFd;

    funcSet->pfXinBlockSub[XIN_BLOCK_2xH]   = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_4xH]   = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_8xH]   = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_16xH]  = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_32xH]  = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_64xH]  = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_128xH] = Xin26xBlockSub;

    funcSet->pfXinBlockSubForDct[XIN_BLOCK_2xH]   = Xin26xBlockSubForDct;
    funcSet->pfXinBlockSubForDct[XIN_BLOCK_4xH]   = Xin26xBlockSubForDct;
    funcSet->pfXinBlockSubForDct[XIN_BLOCK_8xH]   = Xin26xBlockSubForDct;
    funcSet->pfXinBlockSubForDct[XIN_BLOCK_16xH]  = Xin26xBlockSubForDct;
    funcSet->pfXinBlockSubForDct[XIN_BLOCK_32xH]  = Xin26xBlockSubForDct;
    funcSet->pfXinBlockSubForDct[XIN_BLOCK_64xH]  = Xin26xBlockSubForDct;
    funcSet->pfXinBlockSubForDct[XIN_BLOCK_128xH] = Xin26xBlockSubForDct;

    funcSet->pfXinBlockAddForDct[XIN_BLOCK_2xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockAddForDct[XIN_BLOCK_4xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockAddForDct[XIN_BLOCK_8xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockAddForDct[XIN_BLOCK_16xH] = Xin26xBlockRecon;
    funcSet->pfXinBlockAddForDct[XIN_BLOCK_32xH] = Xin26xBlockRecon;
    funcSet->pfXinBlockAddForDct[XIN_BLOCK_64xH] = Xin26xBlockRecon;

    funcSet->pfXinLumaIntraFilter[XIN_BLOCK_2xH]  = Xin266LumaIntraFilter;
    funcSet->pfXinLumaIntraFilter[XIN_BLOCK_4xH]  = Xin266LumaIntraFilter;
    funcSet->pfXinLumaIntraFilter[XIN_BLOCK_8xH]  = Xin266LumaIntraFilter;
    funcSet->pfXinLumaIntraFilter[XIN_BLOCK_16xH] = Xin266LumaIntraFilter;
    funcSet->pfXinLumaIntraFilter[XIN_BLOCK_32xH] = Xin266LumaIntraFilter;
    funcSet->pfXinLumaIntraFilter[XIN_BLOCK_64xH] = Xin266LumaIntraFilter;

    funcSet->pfXinChromaIntraFilter[XIN_BLOCK_2xH]  = Xin266ChromaIntraFilter;
    funcSet->pfXinChromaIntraFilter[XIN_BLOCK_4xH]  = Xin266ChromaIntraFilter;
    funcSet->pfXinChromaIntraFilter[XIN_BLOCK_8xH]  = Xin266ChromaIntraFilter;
    funcSet->pfXinChromaIntraFilter[XIN_BLOCK_16xH] = Xin266ChromaIntraFilter;
    funcSet->pfXinChromaIntraFilter[XIN_BLOCK_32xH] = Xin266ChromaIntraFilter;
    funcSet->pfXinChromaIntraFilter[XIN_BLOCK_64xH] = Xin266ChromaIntraFilter;

    funcSet->pfXinComputeAvgSad[XIN_BLOCK_4xH]   = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_8xH]   = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_16xH]  = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_32xH]  = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_64xH]  = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_128xH] = Xin26xComputeAvgSad;

    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_4xH]   = Xin26xComputeAvgSatd;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_8xH]   = Xin26xComputeAvgSatd;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_16xH]  = Xin26xComputeAvgSatd;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_32xH]  = Xin26xComputeAvgSatd;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_64xH]  = Xin26xComputeAvgSatd;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_128xH] = Xin26xComputeAvgSatd;

    funcSet->pfXinComputeWeightSad[XIN_BLOCK_4xH]   = Xin26xComputeWeightSad;
    funcSet->pfXinComputeWeightSad[XIN_BLOCK_8xH]   = Xin26xComputeWeightSad;
    funcSet->pfXinComputeWeightSad[XIN_BLOCK_16xH]  = Xin26xComputeWeightSad;
    funcSet->pfXinComputeWeightSad[XIN_BLOCK_32xH]  = Xin26xComputeWeightSad;
    funcSet->pfXinComputeWeightSad[XIN_BLOCK_64xH]  = Xin26xComputeWeightSad;
    funcSet->pfXinComputeWeightSad[XIN_BLOCK_128xH] = Xin26xComputeWeightSad;

    funcSet->pfXinComputeWeightSatd[XIN_BLOCK_4xH]   = Xin26xComputeWeightSatd;
    funcSet->pfXinComputeWeightSatd[XIN_BLOCK_8xH]   = Xin26xComputeWeightSatd;
    funcSet->pfXinComputeWeightSatd[XIN_BLOCK_16xH]  = Xin26xComputeWeightSatd;
    funcSet->pfXinComputeWeightSatd[XIN_BLOCK_32xH]  = Xin26xComputeWeightSatd;
    funcSet->pfXinComputeWeightSatd[XIN_BLOCK_64xH]  = Xin26xComputeWeightSatd;
    funcSet->pfXinComputeWeightSatd[XIN_BLOCK_128xH] = Xin26xComputeWeightSatd;

    funcSet->pfXinComputeSadx8[XIN_BLOCK_1xH]   = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_2xH]   = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_4xH]   = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_8xH]   = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_16xH]  = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_32xH]  = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_64xH]  = Xin26xComputeSadx8;
    funcSet->pfXinComputeSadx8[XIN_BLOCK_128xH] = Xin26xComputeSadx8;

    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_1xH]   = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_2xH]   = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_4xH]   = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_8xH]   = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_16xH]  = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_32xH]  = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_64xH]  = Xin26xComputeAvgSadx8;
    funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_128xH] = Xin26xComputeAvgSadx8;

    funcSet->pfXinComputeSadx3[XIN_BLOCK_1xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_2xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_4xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_8xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_16xH]  = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_32xH]  = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_64xH]  = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_128xH] = Xin26xComputeSadx3;

    funcSet->pfXinComputeSadx5[XIN_BLOCK_1xH]   = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_2xH]   = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_4xH]   = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_8xH]   = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_16xH]  = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_32xH]  = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_64xH]  = Xin26xComputeSadx5;
    funcSet->pfXinComputeSadx5[XIN_BLOCK_128xH] = Xin26xComputeSadx5;

    funcSet->pfXinQuantInvQuant[XIN_BLOCK_1xH]  = Xin266QuantInvQuant;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_2xH]  = Xin266QuantInvQuant;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_4xH]  = Xin266QuantInvQuant;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_8xH]  = Xin266QuantInvQuant;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_16xH] = Xin266QuantInvQuant;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_32xH] = Xin266QuantInvQuant;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_64xH] = Xin266QuantInvQuant;

    funcSet->pfXinIntraPredDc[XIN_BLOCK_1xH]   = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_2xH]   = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_4xH]   = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_8xH]   = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_16xH]  = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_32xH]  = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_64xH]  = Xin266IntraPredDc;
    funcSet->pfXinIntraPredDc[XIN_BLOCK_128xH] = Xin266IntraPredDc;

    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_1xH]   = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_2xH]   = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_4xH]   = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_8xH]   = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_16xH]  = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_32xH]  = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_64xH]  = Xin266IntraPredPlanar;
    funcSet->pfXinIntraPredPlanar[XIN_BLOCK_128xH] = Xin266IntraPredPlanar;

    funcSet->pfXinIntraPredHor[XIN_BLOCK_1xH]   = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_2xH]   = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_4xH]   = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_8xH]   = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_16xH]  = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_32xH]  = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_64xH]  = Xin266IntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_128xH] = Xin266IntraPredHor;

    funcSet->pfXinIntraPredVer[XIN_BLOCK_1xH]   = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_2xH]   = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_4xH]   = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_8xH]   = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_16xH]  = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_32xH]  = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_64xH]  = Xin266IntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_128xH] = Xin266IntraPredVer;

    funcSet->pfXinApplyPDPC[XIN_BLOCK_1xH]   = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_2xH]   = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_4xH]   = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_8xH]   = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_16xH]  = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_32xH]  = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_64xH]  = Xin266ApplyPDPC;
    funcSet->pfXinApplyPDPC[XIN_BLOCK_128xH] = Xin266ApplyPDPC;

    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_1xH]  = Xin266ApplyPDPCHor;
    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_2xH]  = Xin266ApplyPDPCHor;
    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_4xH]  = Xin266ApplyPDPCHor;
    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_8xH]  = Xin266ApplyPDPCHor;
    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_16xH] = Xin266ApplyPDPCHor;
    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPCHor;
    funcSet->pfXinApplyHorPDPC[XIN_BLOCK_64xH] = Xin266ApplyPDPCHor;

    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_1xH]  = Xin266ApplyPDPCVer;
    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_2xH]  = Xin266ApplyPDPCVer;
    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_4xH]  = Xin266ApplyPDPCVer;
    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_8xH]  = Xin266ApplyPDPCVer;
    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_16xH] = Xin266ApplyPDPCVer;
    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPCVer;
    funcSet->pfXinApplyVerPDPC[XIN_BLOCK_64xH] = Xin266ApplyPDPCVer;

    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_1xH]  = Xin266ApplyAngPDPCHor;
    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_2xH]  = Xin266ApplyAngPDPCHor;
    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_4xH]  = Xin266ApplyAngPDPCHor;
    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_8xH]  = Xin266ApplyAngPDPCHor;
    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_16xH] = Xin266ApplyAngPDPCHor;
    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_32xH] = Xin266ApplyAngPDPCHor;
    funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_64xH] = Xin266ApplyAngPDPCHor;

    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_1xH]  = Xin266ApplyAngPDPCVert;
    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_2xH]  = Xin266ApplyAngPDPCVert;
    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_4xH]  = Xin266ApplyAngPDPCVert;
    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_8xH]  = Xin266ApplyAngPDPCVert;
    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_16xH] = Xin266ApplyAngPDPCVert;
    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_32xH] = Xin266ApplyAngPDPCVert;
    funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_64xH] = Xin266ApplyAngPDPCVert;

    funcSet->pfXinComputeGradient[XIN_BLOCK_1xH]   = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_2xH]   = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_4xH]   = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_8xH]   = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_16xH]  = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_32xH]  = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_64xH]  = Xin266ComputeGradient;
    funcSet->pfXinComputeGradient[XIN_BLOCK_128xH] = Xin266ComputeGradient;

    funcSet->pfXinLinearTransform[XIN_BLOCK_1xH]   = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_2xH]   = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_4xH]   = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_8xH]   = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_16xH]  = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_32xH]  = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_64xH]  = Xin266LinearTransform;
    funcSet->pfXinLinearTransform[XIN_BLOCK_128xH] = Xin266LinearTransform;

    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_2xH]   = Xin26xFDct2P2;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_4xH]   = Xin26xFDct2P4;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_8xH]   = Xin26xFDct2P8;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_16xH]  = Xin26xFDct2P16;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_32xH]  = Xin26xFDct2P32;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_64xH]  = Xin26xFDct2P64;
    funcSet->pfXinFor1dDct[XIN_DCT2][XIN_BLOCK_128xH] = NULL;

    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_4xH]   = Xin26xFDct8P4;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_8xH]   = Xin26xFDct8P8;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_16xH]  = Xin26xFDct8P16;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_32xH]  = Xin26xFDct8P32;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_64xH]  = NULL;
    funcSet->pfXinFor1dDct[XIN_DCT8][XIN_BLOCK_128xH] = NULL;

    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_4xH]   = Xin26xFDst7P4;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_8xH]   = Xin26xFDst7P8;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_16xH]  = Xin26xFDst7P16;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_32xH]  = Xin26xFDst7P32;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_64xH]  = NULL;
    funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_128xH] = NULL;

    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_2xH]   = Xin26xIDct2P2;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_4xH]   = Xin26xIDct2P4;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_8xH]   = Xin26xIDct2P8;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_16xH]  = Xin26xIDct2P16;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_32xH]  = Xin26xIDct2P32;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_64xH]  = Xin26xIDct2P64;
    funcSet->pfXinInv1dDct[XIN_DCT2][XIN_BLOCK_128xH] = NULL;

    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_4xH]   = Xin26xIDct8P4;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_8xH]   = Xin26xIDct8P8;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_16xH]  = Xin26xIDct8P16;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_32xH]  = Xin26xIDct8P32;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_64xH]  = NULL;
    funcSet->pfXinInv1dDct[XIN_DCT8][XIN_BLOCK_128xH] = NULL;

    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_4xH]   = Xin26xIDst7P4;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_8xH]   = Xin26xIDst7P8;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_16xH]  = Xin26xIDst7P16;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_32xH]  = Xin26xIDst7P32;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_64xH]  = NULL;
    funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_128xH] = NULL;

    funcSet->pfXinFor2dSkip[XIN_BLOCK_128xH] = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_64xH]  = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_32xH]  = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_16xH]  = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_8xH]   = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_4xH]   = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_2xH]   = Xin26xFSkipWxH;
    funcSet->pfXinFor2dSkip[XIN_BLOCK_1xH]   = Xin26xFSkipWxH;

    funcSet->pfXinInv2dSkip[XIN_BLOCK_128xH] = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_64xH]  = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_32xH]  = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_16xH]  = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_8xH]   = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_4xH]   = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_2xH]   = Xin266ISkipWxH;
    funcSet->pfXinInv2dSkip[XIN_BLOCK_1xH]   = Xin266ISkipWxH;

    Xin266InitDctSub (
        funcSet);

    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_1xH]   = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_2xH]   = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopy;
    funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_128xH] = Xin266InterpCopy;

    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_1xH]   = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_2xH]   = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_4xH]   = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_8xH]   = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_16xH]  = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_32xH]  = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_64xH]  = Xin266LumaInterpHor;
    funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_128xH] = Xin266LumaInterpHor;

    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_1xH]   = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_2xH]   = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_4xH]   = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_8xH]   = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_16xH]  = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_32xH]  = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_64xH]  = Xin266LumaInterpVet;
    funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_128xH] = Xin266LumaInterpVet;

    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_1xH]   = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_2xH]   = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_4xH]   = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_8xH]   = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_16xH]  = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_32xH]  = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_64xH]  = Xin266LumaInterpHorVet;
    funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_128xH] = Xin266LumaInterpHorVet;

    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_1xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_2xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopyU8S16;
    funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_128xH] = Xin266InterpCopyU8S16;

    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_1xH]   = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_2xH]   = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_4xH]   = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_8xH]   = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_16xH]  = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_32xH]  = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_64xH]  = Xin266LumaInterpHorU8S16;
    funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_128xH] = Xin266LumaInterpHorU8S16;

    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_1xH]   = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_2xH]   = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_4xH]   = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_8xH]   = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_16xH]  = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_32xH]  = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_64xH]  = Xin266LumaInterpVetU8S16;
    funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_128xH] = Xin266LumaInterpVetU8S16;

    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_1xH]   = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_2xH]   = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_4xH]   = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_8xH]   = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_16xH]  = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_32xH]  = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_64xH]  = Xin266LumaInterpHorVetU8S16;
    funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_128xH] = Xin266LumaInterpHorVetU8S16;

    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_1xH]   = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_2xH]   = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_4xH]   = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_8xH]   = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_16xH]  = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_32xH]  = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_64xH]  = Xin266BiliInterpCopy;
    funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_128xH] = Xin266BiliInterpCopy;

    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_1xH]   = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_2xH]   = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_4xH]   = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_8xH]   = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_16xH]  = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_32xH]  = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_64xH]  = Xin266BiliInterpHor;
    funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_128xH] = Xin266BiliInterpHor;

    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_1xH]   = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_2xH]   = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_4xH]   = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_8xH]   = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_16xH]  = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_32xH]  = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_64xH]  = Xin266BiliInterpVet;
    funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_128xH] = Xin266BiliInterpVet;

    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_1xH]   = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_2xH]   = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_4xH]   = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_8xH]   = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_16xH]  = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_32xH]  = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_64xH]  = Xin266BiliInterpHorVet;
    funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_128xH] = Xin266BiliInterpHorVet;

    funcSet->pfXinInterpAvg[XIN_BLOCK_1xH]   = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_2xH]   = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_4xH]   = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_8xH]   = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_16xH]  = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_32xH]  = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_64xH]  = Xin266InterpAvgS16U8;
    funcSet->pfXinInterpAvg[XIN_BLOCK_128xH] = Xin266InterpAvgS16U8;

    funcSet->pfXinInterpWeight[XIN_BLOCK_1xH]   = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_2xH]   = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_4xH]   = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_8xH]   = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_16xH]  = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_32xH]  = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_64xH]  = Xin266InterpWeightS16U8;
    funcSet->pfXinInterpWeight[XIN_BLOCK_128xH] = Xin266InterpWeightS16U8;

    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_1xH]   = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_2xH]   = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopy;
    funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_128xH] = Xin266InterpCopy;

    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_1xH]   = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_2xH]   = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_4xH]   = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_8xH]   = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_16xH]  = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_32xH]  = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_64xH]  = Xin266ChromaInterpHor;
    funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_128xH] = Xin266ChromaInterpHor;

    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_1xH]   = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_2xH]   = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_4xH]   = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpVet;
    funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_128xH] = Xin266ChromaInterpVet;

    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_1xH]   = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_2xH]   = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_4xH]   = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorVet;
    funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_128xH] = Xin266ChromaInterpHorVet;

    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_1xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_2xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopyU8S16;
    funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_128xH] = Xin266InterpCopyU8S16;
    
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_1xH]   = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_2xH]   = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_4xH]   = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_8xH]   = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorU8S16;
    funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_128xH] = Xin266ChromaInterpHorU8S16;

    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_1xH]   = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_2xH]   = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_4xH]   = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpVetU8S16;
    funcSet->pfXinChromaInterpS16[0][1][XIN_BLOCK_128xH] = Xin266ChromaInterpVetU8S16;

    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_1xH]   = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_2xH]   = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_4xH]   = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorVetU8S16;
    funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_128xH] = Xin266ChromaInterpHorVetU8S16;

    funcSet->pfXinConstructBiMeInput[XIN_BLOCK_4xH]   = Xin26xConstructBiMeInput;
    funcSet->pfXinConstructBiMeInput[XIN_BLOCK_8xH]   = Xin26xConstructBiMeInput;
    funcSet->pfXinConstructBiMeInput[XIN_BLOCK_16xH]  = Xin26xConstructBiMeInput;
    funcSet->pfXinConstructBiMeInput[XIN_BLOCK_32xH]  = Xin26xConstructBiMeInput;
    funcSet->pfXinConstructBiMeInput[XIN_BLOCK_64xH]  = Xin26xConstructBiMeInput;
    funcSet->pfXinConstructBiMeInput[XIN_BLOCK_128xH] = Xin26xConstructBiMeInput;

    funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_4xH]   = Xin266ConstructWeightBiMeInput;
    funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_8xH]   = Xin266ConstructWeightBiMeInput;
    funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_16xH]  = Xin266ConstructWeightBiMeInput;
    funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_32xH]  = Xin266ConstructWeightBiMeInput;
    funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_64xH]  = Xin266ConstructWeightBiMeInput;
    funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_128xH] = Xin266ConstructWeightBiMeInput;

    funcSet->pfXinSaoStatEo[XIN_BLOCK_1xH]   = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_2xH]   = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_4xH]   = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_8xH]   = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_16xH]  = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_32xH]  = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_64xH]  = Xin26xSaoStatEo;
    funcSet->pfXinSaoStatEo[XIN_BLOCK_128xH] = Xin26xSaoStatEo;

    funcSet->pfXinCopyAndPad   = Xin266CopyAndPad;
    funcSet->pfXinCopyAndPadUv = Xin266CopyAndPadUv;

    funcSet->pfXinComputeSatd[XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinComputeSatd[XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinComputeSatd[XIN_BLOCK_4xH]   = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_8xH]   = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_16xH]  = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_32xH]  = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_64xH]  = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_128xH] = Xin26xComputeSatd;

    funcSet->pfXinCoeffScanCG[0] = Xin266CoeffScanCG;
    funcSet->pfXinCoeffScanCG[1] = Xin266CoeffScanCG;

    funcSet->pfXinGetBlockDeltaU[0] = Xin266GetBlockDeltaU;
    funcSet->pfXinGetBlockDeltaU[1] = Xin266GetBlockDeltaU;

    funcSet->pfXinCalcBlockDeltaU[0] = Xin266ComputeBlockDeltaU;
    funcSet->pfXinCalcBlockDeltaU[1] = Xin266ComputeBlockDeltaU;

    funcSet->pfXinSaoEo0[0] = Xin26xSaoEo0;
    funcSet->pfXinSaoEo0[1] = Xin26xSaoEo0;

    funcSet->pfXinSaoEo90[0] = Xin26xSaoEo90;
    funcSet->pfXinSaoEo90[1] = Xin26xSaoEo90;

    funcSet->pfXinSaoEo45[0] = Xin26xSaoEo45;
    funcSet->pfXinSaoEo45[1] = Xin26xSaoEo45;

    funcSet->pfXinSaoEo135[0] = Xin26xSaoEo135;
    funcSet->pfXinSaoEo135[1] = Xin26xSaoEo135;

    funcSet->pfXinComputeVar8x8    = Xin26xComputeVar8x8;
    funcSet->pfXinPictureCopy      = Xin26xBlockCopy;
    funcSet->pfXinPictureScaleCopy = Xin26xPictureScaleCopy;
    funcSet->pfXinDownscale2x2     = Xin26xDownscale2x2Taps4;
    funcSet->pfXinLumaLoopFilter   = Xin266LumaLoopFilter;
    funcSet->pfXinComputeBlockSsd  = Xin26xComputeBlockSsd;
    funcSet->pfXinComputeFrameAct  = Xin26xComputeFrameAct;
    funcSet->pfXinPreRdoq          = Xin266PreRdoq;
    funcSet->pfXinPreDepQuant      = Xin266PreDepQuant;
    funcSet->pfXinDepQuant         = Xin266DepQuant;
    funcSet->pfXinBlockTranspose   = Xin26xBlockTranspose;
    funcSet->pfXinExtendIntraRef   = Xin266ExtendIntraRef;
    funcSet->pfXinFilterIntraNB    = Xin266FilterIntraNB;

#if defined(_X86_OPT_)

    if (cpuFeature & XIN_CPU_SSE2)
    {
        funcSet->pfXinBlockCopy[XIN_BLOCK_4xH]   = Xin26xBlockCopy4xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_8xH]   = Xin26xBlockCopy8xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_16xH]  = Xin26xBlockCopy16xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_32xH]  = Xin26xBlockCopy32xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_64xH]  = Xin26xBlockCopy64xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_128xH] = Xin26xBlockCopy128xH_SSE2;

        funcSet->pfXinBlockAvg[XIN_BLOCK_8xH] = Xin26xBlockAverage8xH_SSE2;

        funcSet->pfXinComputeSad[XIN_BLOCK_4xH]   = Xin26xComputeSad4xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_8xH]   = Xin26xComputeSad8xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_16xH]  = Xin26xComputeSad16xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_32xH]  = Xin26xComputeSad32xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_64xH]  = Xin26xComputeSad64xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_128xH] = Xin26xComputeSad128xH_SSE2;

        funcSet->pfXinComputeSsdFd[XIN_BLOCK_4xH]  = Xin266ComputeSsdFd4xH_SSE2;
        funcSet->pfXinComputeSsdFd[XIN_BLOCK_8xH]  = Xin266ComputeSsdFd8xH_SSE2;
        funcSet->pfXinComputeSsdFd[XIN_BLOCK_16xH] = Xin266ComputeSsdFd16xH_SSE2;

        funcSet->pfXinBlockSub[XIN_BLOCK_8xH]  = Xin26xBlockSub8xH_SSE2;
        funcSet->pfXinBlockSub[XIN_BLOCK_16xH] = Xin26xBlockSub16xH_SSE2;

        funcSet->pfXinBlockSubForDct[XIN_BLOCK_8xH]  = Xin26xBlockSub8xH_SSE2;
        funcSet->pfXinBlockSubForDct[XIN_BLOCK_16xH] = Xin26xBlockSub16xH_SSE2;

        funcSet->pfXinBlockAddForDct[XIN_BLOCK_8xH]  = Xin26xBlockRecon8xH_SSE2;
        funcSet->pfXinBlockAddForDct[XIN_BLOCK_16xH] = Xin26xBlockRecon16xH_SSE2;

        funcSet->pfXinComputeAvgSad[XIN_BLOCK_4xH]   = Xin26xComputeAvgSad4xH_SSE2;
        funcSet->pfXinComputeAvgSad[XIN_BLOCK_8xH]   = Xin26xComputeAvgSad8xH_SSE2;
        funcSet->pfXinComputeAvgSad[XIN_BLOCK_16xH]  = Xin26xComputeAvgSad16xH_SSE2;
        funcSet->pfXinComputeAvgSad[XIN_BLOCK_32xH]  = Xin26xComputeAvgSad32xH_SSE2;
        funcSet->pfXinComputeAvgSad[XIN_BLOCK_64xH]  = Xin26xComputeAvgSad64xH_SSE2;
        funcSet->pfXinComputeAvgSad[XIN_BLOCK_128xH] = Xin26xComputeAvgSad128xH_SSE2;

        funcSet->pfXinComputeSadx8[XIN_BLOCK_8xH]   = Xin26xComputeSad8xHx8_SSE2;
        funcSet->pfXinComputeSadx8[XIN_BLOCK_16xH]  = Xin26xComputeSad16xHx8_SSE2;
        funcSet->pfXinComputeSadx8[XIN_BLOCK_32xH]  = Xin26xComputeSadGt16xHx8_SSE2;
        funcSet->pfXinComputeSadx8[XIN_BLOCK_64xH]  = Xin26xComputeSadGt16xHx8_SSE2;
        funcSet->pfXinComputeSadx8[XIN_BLOCK_128xH] = Xin26xComputeSadGt16xHx8_SSE2;

        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_8xH]   = Xin26xComputeAvgSad8xHx8_SSE2;
        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_16xH]  = Xin26xComputeAvgSad16xHx8_SSE2;
        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_32xH]  = Xin26xComputeAvgSadGt16xHx8_SSE2;
        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_64xH]  = Xin26xComputeAvgSadGt16xHx8_SSE2;
        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_128xH] = Xin26xComputeAvgSadGt16xHx8_SSE2;

        funcSet->pfXinComputeSadx5[XIN_BLOCK_8xH]  = Xin26xComputeSad8xHx5_SSE2;
        funcSet->pfXinComputeSadx5[XIN_BLOCK_16xH] = Xin26xComputeSad16xHx5_SSE2;

        funcSet->pfXinComputeSadx3[XIN_BLOCK_8xH]   = Xin26xComputeSad8xHx3_SSE2;
        funcSet->pfXinComputeSadx3[XIN_BLOCK_16xH]  = Xin26xComputeSad16xHx3_SSE2;
        funcSet->pfXinComputeSadx3[XIN_BLOCK_32xH]  = Xin26xComputeSad32xHx3_SSE2;
        funcSet->pfXinComputeSadx3[XIN_BLOCK_64xH]  = Xin26xComputeSad64xHx3_SSE2;
        funcSet->pfXinComputeSadx3[XIN_BLOCK_128xH] = Xin26xComputeSad128xHx3_SSE2;

        funcSet->pfXinComputeSsd[XIN_BLOCK_4xH]   = Xin26xComputeSsd4xH_SSE2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_8xH]   = Xin26xComputeSsd8xH_SSE2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_16xH]  = Xin26xComputeSsdGt8xH_SSE2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_32xH]  = Xin26xComputeSsdGt8xH_SSE2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_64xH]  = Xin26xComputeSsdGt8xH_SSE2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_128xH] = Xin26xComputeSsdGt8xH_SSE2;

        funcSet->pfXinConstructBiMeInput[XIN_BLOCK_8xH]   = Xin26xConstructBiMeInput8xH_SSE2;
        funcSet->pfXinConstructBiMeInput[XIN_BLOCK_16xH]  = Xin26xConstructBiMeInput16xH_SSE2;
        funcSet->pfXinConstructBiMeInput[XIN_BLOCK_32xH]  = Xin26xConstructBiMeInputGt16xH_SSE2;
        funcSet->pfXinConstructBiMeInput[XIN_BLOCK_64xH]  = Xin26xConstructBiMeInputGt16xH_SSE2;
        funcSet->pfXinConstructBiMeInput[XIN_BLOCK_128xH] = Xin26xConstructBiMeInputGt16xH_SSE2;

        funcSet->pfXinFor2dSkip[XIN_BLOCK_8xH] = Xin26xFSkip8xH_SSE2;
        funcSet->pfXinFor2dSkip[XIN_BLOCK_4xH] = Xin26xFSkip4xH_SSE2;

        funcSet->pfXinPictureCopy     = Xin26xPictureCopy_SSE2;
        funcSet->pfXinComputeVar8x8   = Xin26xComputeVar8x8_SSE2;
        funcSet->pfXinDownscale2x2    = Xin26xDownscale2x2_AVX2;
        funcSet->pfXinComputeBlockSsd = Xin26xComputeBlockSsd_SSE2;
        funcSet->pfXinBlockTranspose  = Xin26xBlockTranspose_SSE2;

    }

    if (cpuFeature & XIN_CPU_SSSE3)
    {
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_8xH]   = Xin26xComputeAvgSatdGt4x4_SSSE3;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_16xH]  = Xin26xComputeAvgSatdGt4x4_SSSE3;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_32xH]  = Xin26xComputeAvgSatdGt4x4_SSSE3;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_64xH]  = Xin26xComputeAvgSatdGt4x4_SSSE3;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_128xH] = Xin26xComputeAvgSatdGt4x4_SSSE3;

        funcSet->pfXinIntraPredDc[XIN_BLOCK_4xH]  = Xin266IntraPredDc4xH_SSSE3;
        funcSet->pfXinIntraPredDc[XIN_BLOCK_8xH]  = Xin266IntraPredDc8xH_SSSE3;
        funcSet->pfXinIntraPredDc[XIN_BLOCK_16xH] = Xin266IntraPredDc16xH_SSSE3;
        funcSet->pfXinIntraPredDc[XIN_BLOCK_32xH] = Xin266IntraPredDc32xH_SSSE3;
        funcSet->pfXinIntraPredDc[XIN_BLOCK_64xH] = Xin266IntraPredDc64xH_SSSE3;

        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_4xH]  = Xin266IntraPredPlanar4xH_SSSE3;
        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_8xH]  = Xin266IntraPredPlanar_SSSE3;
        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_16xH] = Xin266IntraPredPlanar_SSSE3;
        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_32xH] = Xin266IntraPredPlanar_SSSE3;
        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_64xH] = Xin266IntraPredPlanar_SSSE3;

        funcSet->pfXinIntraPredHor[XIN_BLOCK_4xH]  = Xin266IntraPredHor4xH_SSSE3;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_8xH]  = Xin266IntraPredHor8xH_SSSE3;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_16xH] = Xin266IntraPredHor16xH_SSSE3;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_32xH] = Xin266IntraPredHor32xH_SSSE3;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_64xH] = Xin266IntraPredHor64xH_SSSE3;

        funcSet->pfXinApplyPDPC[XIN_BLOCK_4xH]  = Xin266ApplyPDPC4xH_SSSE3;
        funcSet->pfXinApplyPDPC[XIN_BLOCK_8xH]  = Xin266ApplyPDPC8xH_SSSE3;
        funcSet->pfXinApplyPDPC[XIN_BLOCK_16xH] = Xin266ApplyPDPC16xH_SSSE3;
        funcSet->pfXinApplyPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPC32xH_SSSE3;
        funcSet->pfXinApplyPDPC[XIN_BLOCK_64xH] = Xin266ApplyPDPC64xH_SSSE3;

        funcSet->pfXinApplyHorPDPC[XIN_BLOCK_4xH]  = Xin266ApplyPDPCHor4xH_SSSE3;
        funcSet->pfXinApplyHorPDPC[XIN_BLOCK_8xH]  = Xin266ApplyPDPCHor8xH_SSSE3;
        funcSet->pfXinApplyHorPDPC[XIN_BLOCK_16xH] = Xin266ApplyPDPCHor16xH_SSSE3;

        funcSet->pfXinLumaIntraFilter[XIN_BLOCK_8xH]  = Xin266LumaIntraFilter8xH_SSSE3;
        funcSet->pfXinLumaIntraFilter[XIN_BLOCK_16xH] = Xin266LumaIntraFilterGt8xH_SSSE3;
        funcSet->pfXinLumaIntraFilter[XIN_BLOCK_32xH] = Xin266LumaIntraFilterGt8xH_SSSE3;
        funcSet->pfXinLumaIntraFilter[XIN_BLOCK_64xH] = Xin266LumaIntraFilterGt8xH_SSSE3;

        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_4xH]  = Xin266ChromaIntraFilter4xH_SSSE3;
        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_8xH]  = Xin266ChromaIntraFilterGt4xH_SSSE3;
        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_16xH] = Xin266ChromaIntraFilterGt4xH_SSSE3;
        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_32xH] = Xin266ChromaIntraFilterGt4xH_SSSE3;
        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_64xH] = Xin266ChromaIntraFilterGt4xH_SSSE3;

        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopy4xH_SSSE3;
        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopy8xH_SSSE3;
        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopy16xH_SSSE3;
        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopy32xH_SSSE3;
        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopy64xH_SSSE3;

        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_4xH]   = Xin266LumaInterpVet4xH_SSSE3;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_8xH]   = Xin266LumaInterpVet8xH_SSSE3;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_16xH]  = Xin266LumaInterpVet16xH_SSSE3;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_32xH]  = Xin266LumaInterpVetGt16xH_SSSE3;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_64xH]  = Xin266LumaInterpVetGt16xH_SSSE3;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_128xH] = Xin266LumaInterpVetGt16xH_SSSE3;

        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_4xH]   = Xin266LumaInterpHor4xH_SSSE3;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_8xH]   = Xin266LumaInterpHor8xH_SSSE3;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_16xH]  = Xin266LumaInterpHor16xH_SSSE3;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_32xH]  = Xin266LumaInterpHorGt16xH_SSSE3;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_64xH]  = Xin266LumaInterpHorGt16xH_SSSE3;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_128xH] = Xin266LumaInterpHorGt16xH_SSSE3;

        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_4xH]   = Xin266LumaInterpHorVet4xH_SSSE3;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_8xH]   = Xin266LumaInterpHorVet8xH_SSSE3;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_16xH]  = Xin266LumaInterpHorVetGt8xH_SSSE3;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_32xH]  = Xin266LumaInterpHorVetGt8xH_SSSE3;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_64xH]  = Xin266LumaInterpHorVetGt8xH_SSSE3;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_128xH] = Xin266LumaInterpHorVetGt8xH_SSSE3;

        funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopy4xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopy8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopyGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopyGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopyGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][0][XIN_BLOCK_128xH] = Xin266InterpCopyGt8xHU8S16_SSSE3;

        funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_4xH]   = Xin266LumaInterpVet4xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_8xH]   = Xin266LumaInterpVet8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_16xH]  = Xin266LumaInterpVet16xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_32xH]  = Xin266LumaInterpVetGt16xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_64xH]  = Xin266LumaInterpVetGt16xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[0][1][XIN_BLOCK_128xH] = Xin266LumaInterpVetGt16xHU8S16_SSSE3;

        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_4xH]   = Xin266LumaInterpHor4xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_8xH]   = Xin266LumaInterpHor8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_16xH]  = Xin266LumaInterpHorGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_32xH]  = Xin266LumaInterpHorGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_64xH]  = Xin266LumaInterpHorGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_128xH] = Xin266LumaInterpHorGt8xHU8S16_SSSE3;

        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_4xH]   = Xin266LumaInterpHorVet4xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_8xH]   = Xin266LumaInterpHorVet8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_16xH]  = Xin266LumaInterpHorVetGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_32xH]  = Xin266LumaInterpHorVetGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_64xH]  = Xin266LumaInterpHorVetGt8xHU8S16_SSSE3;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_128xH] = Xin266LumaInterpHorVetGt8xHU8S16_SSSE3;

        funcSet->pfXinInterpAvg[XIN_BLOCK_8xH]   = Xin266InterpAvg8xHS16U8_SSSE3;
        funcSet->pfXinInterpAvg[XIN_BLOCK_16xH]  = Xin266InterpAvgGt8xHS16U8_SSSE3;
        funcSet->pfXinInterpAvg[XIN_BLOCK_32xH]  = Xin266InterpAvgGt8xHS16U8_SSSE3;
        funcSet->pfXinInterpAvg[XIN_BLOCK_64xH]  = Xin266InterpAvgGt8xHS16U8_SSSE3;
        funcSet->pfXinInterpAvg[XIN_BLOCK_128xH] = Xin266InterpAvgGt8xHS16U8_SSSE3;

        funcSet->pfXinInterpWeight[XIN_BLOCK_8xH]   = Xin266InterpWeightGt4xHS16U8_SSSE3;
        funcSet->pfXinInterpWeight[XIN_BLOCK_16xH]  = Xin266InterpWeightGt4xHS16U8_SSSE3;
        funcSet->pfXinInterpWeight[XIN_BLOCK_32xH]  = Xin266InterpWeightGt4xHS16U8_SSSE3;
        funcSet->pfXinInterpWeight[XIN_BLOCK_64xH]  = Xin266InterpWeightGt4xHS16U8_SSSE3;
        funcSet->pfXinInterpWeight[XIN_BLOCK_128xH] = Xin266InterpWeightGt4xHS16U8_SSSE3;

        funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopy4xH_SSSE3;
        funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopy8xH_SSSE3;
        funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopy16xH_SSSE3;
        funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopy32xH_SSSE3;
        funcSet->pfXinChromaInterp[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopy64xH_SSSE3;

        funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_8xH]   = Xin266ChromaInterpHor8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][0][XIN_BLOCK_128xH] = Xin266ChromaInterpHorGt8xH_SSSE3;

        funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpVet8xH_SSSE3;
        funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpVetGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpVetGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpVetGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[0][1][XIN_BLOCK_128xH] = Xin266ChromaInterpVetGt8xH_SSSE3;

        funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpHorVet8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorVetGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorVetGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorVetGt8xH_SSSE3;
        funcSet->pfXinChromaInterp[1][1][XIN_BLOCK_128xH] = Xin266ChromaInterpHorVetGt8xH_SSSE3;

        funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_4xH]   = Xin266InterpCopy4xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_8xH]   = Xin266InterpCopy8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_16xH]  = Xin266InterpCopyGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopyGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopyGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[0][0][XIN_BLOCK_128xH] = Xin266InterpCopyGt8xHU8S16_SSSE3;

        funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_8xH]   = Xin266ChromaInterpHor8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][0][XIN_BLOCK_128xH] = Xin266ChromaInterpHorGt8xHU8S16_SSSE3;

        funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_8xH]   = Xin266ChromaInterpHorVet8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_16xH]  = Xin266ChromaInterpHorVetGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_32xH]  = Xin266ChromaInterpHorVetGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_64xH]  = Xin266ChromaInterpHorVetGt8xHU8S16_SSSE3;
        funcSet->pfXinChromaInterpS16[1][1][XIN_BLOCK_128xH] = Xin266ChromaInterpHorVetGt8xHU8S16_SSSE3;

    }
    
    if (cpuFeature & XIN_CPU_SSE42)
    {
        funcSet->pfXinFor2dDct2[XIN_BLOCK_4xH][XIN_BLOCK_4xH] = Xin26xFDct2W4H4_SSE4;

        funcSet->pfXinInv2dSkip[XIN_BLOCK_4xH]  = Xin266ISkip4xH_SSE4;
        funcSet->pfXinInv2dSkip[XIN_BLOCK_8xH]  = Xin266ISkip8xH_SSE4;
        
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_4xH]  = Xin266QuantInvQuant4xH_SSE4;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_8xH]  = Xin266QuantInvQuantGt4x4_SSE4;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_16xH] = Xin266QuantInvQuantGt4x4_SSE4;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_32xH] = Xin266QuantInvQuantGt4x4_SSE4;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_64xH] = Xin266QuantInvQuantGt4x4_SSE4;

        funcSet->pfXinLinearTransform[XIN_BLOCK_8xH] = Xin266LinearTransform8xH_SSE4;

        funcSet->pfXinIntraPredVer[XIN_BLOCK_4xH]   = Xin266IntraPredVer4xH_SSE4;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_8xH]   = Xin266IntraPredVer8xH_SSE4;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_16xH]  = Xin266IntraPredVer16xH_SSE4;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_32xH]  = Xin266IntraPredVer32xH_SSE4;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_64xH]  = Xin266IntraPredVer64xH_SSE4;

        funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_4xH] = Xin266ApplyAngPDPCHor4xH_SSE4;
        funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_8xH] = Xin266ApplyAngPDPCHor8xH_SSE4;

        funcSet->pfXinApplyVerPDPC[XIN_BLOCK_4xH]  = Xin266ApplyPDPCVer4xH_SSE4;
        funcSet->pfXinApplyVerPDPC[XIN_BLOCK_8xH]  = Xin266ApplyPDPCVer8xH_SSE4;
        funcSet->pfXinApplyVerPDPC[XIN_BLOCK_16xH] = Xin266ApplyPDPCVer16xH_SSE4;
        funcSet->pfXinApplyVerPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPCVer16xH_SSE4;
        funcSet->pfXinApplyVerPDPC[XIN_BLOCK_64xH] = Xin266ApplyPDPCVer16xH_SSE4;
        
        funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_4xH]  = Xin266BiliInterpCopy4xH_SSE4;
        funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_8xH]  = Xin266BiliInterpCopy8xH_SSE4;
        funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_16xH] = Xin266BiliInterpCopy16xH_SSE4;

        funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_4xH]  = Xin266BiliInterpHor4xH_SSE4;
        funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_8xH]  = Xin266BiliInterpHor8xH_SSE4;

        funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_4xH]  = Xin266BiliInterpVet4xH_SSE4;
        funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_8xH]  = Xin266BiliInterpVet8xH_SSE4;

        funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_4xH]  = Xin266BiliInterpHorVet4xH_SSE4;
        funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_8xH]  = Xin266BiliInterpHorVet8xH_SSE4;

        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_8xH]   = Xin26xComputeWeightSatdGt4x4_SSE4;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_16xH]  = Xin26xComputeWeightSatdGt4x4_SSE4;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_32xH]  = Xin26xComputeWeightSatdGt4x4_SSE4;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_64xH]  = Xin26xComputeWeightSatdGt4x4_SSE4;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_128xH] = Xin26xComputeWeightSatdGt4x4_SSE4;

        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_8xH]   = Xin266ConstructWeightBiMeInputGt4xH_SSE4;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_16xH]  = Xin266ConstructWeightBiMeInputGt4xH_SSE4;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_32xH]  = Xin266ConstructWeightBiMeInputGt4xH_SSE4;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_64xH]  = Xin266ConstructWeightBiMeInputGt4xH_SSE4;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_128xH] = Xin266ConstructWeightBiMeInputGt4xH_SSE4;

        funcSet->pfXinCoeffScanCG[1]     = Xin266CoeffScan4x4_SSE4;
        funcSet->pfXinGetBlockDeltaU[1]  = Xin266GetBlockDeltaU_SSE4;
        funcSet->pfXinCalcBlockDeltaU[1] = Xin266ComputeBlockDeltaU_SSE4;
        funcSet->pfXinLumaLoopFilter     = Xin266LumaLoopFilter_SSE4;
        funcSet->pfXinPreRdoq            = Xin266PreRdoq_SSE4;
        funcSet->pfXinPreDepQuant        = Xin266PreDepQuant_SSE4;
        funcSet->pfXinCopyAndPadUv       = Xin266CopyAndPadUv_SSE4;

        funcSet->pfXinSaoEo0[1]   = Xin26xSaoEo0_SSE4;
        funcSet->pfXinSaoEo90[1]  = Xin26xSaoEo90_SSE4;
        funcSet->pfXinSaoEo45[1]  = Xin26xSaoEo45_SSE4;
        funcSet->pfXinSaoEo135[1] = Xin26xSaoEo135_SSE4;

    }

    if (cpuFeature & XIN_CPU_AVX2)
    {
        funcSet->pfXinLinearTransform[XIN_BLOCK_16xH]  = Xin266LinearTransformGt8xH_AVX2;
        funcSet->pfXinLinearTransform[XIN_BLOCK_32xH]  = Xin266LinearTransformGt8xH_AVX2;
        funcSet->pfXinLinearTransform[XIN_BLOCK_64xH]  = Xin266LinearTransformGt8xH_AVX2;
        funcSet->pfXinLinearTransform[XIN_BLOCK_128xH] = Xin266LinearTransformGt8xH_AVX2;
        
        funcSet->pfXinBlockCopy[XIN_BLOCK_32xH]  = Xin26xBlockCopy32xH_AVX2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_64xH]  = Xin26xBlockCopy64xH_AVX2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_128xH] = Xin26xBlockCopy128xH_AVX2;

        funcSet->pfXinComputeSad[XIN_BLOCK_32xH]  = Xin26xComputeSad32xH_AVX2;
        funcSet->pfXinComputeSad[XIN_BLOCK_64xH]  = Xin26xComputeSad64xH_AVX2;
        funcSet->pfXinComputeSad[XIN_BLOCK_128xH] = Xin26xComputeSad128xH_AVX2;

        funcSet->pfXinComputeSadS16[XIN_BLOCK_8xH]  = Xin26xComputeSadS168xH_AVX2;
        funcSet->pfXinComputeSadS16[XIN_BLOCK_16xH] = Xin26xComputeSadS1616xH_AVX2;

        funcSet->pfXinComputeSsdFd[XIN_BLOCK_32xH]  = Xin266ComputeSsdFd32xH_AVX2;
        funcSet->pfXinComputeSsdFd[XIN_BLOCK_64xH]  = Xin266ComputeSsdFd64xH_AVX2;
        funcSet->pfXinComputeSsdFd[XIN_BLOCK_128xH] = Xin266ComputeSsdFd128xH_AVX2;

        funcSet->pfXinComputeSsd[XIN_BLOCK_32xH]  = Xin26xComputeSsdGt16xH_AVX2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_64xH]  = Xin26xComputeSsdGt16xH_AVX2;
        funcSet->pfXinComputeSsd[XIN_BLOCK_128xH] = Xin26xComputeSsdGt16xH_AVX2;

        funcSet->pfXinBlockSub[XIN_BLOCK_32xH] = Xin26xBlockSub32xH_AVX2;
        funcSet->pfXinBlockSub[XIN_BLOCK_64xH] = Xin26xBlockSub64xH_AVX2;

        funcSet->pfXinBlockSubForDct[XIN_BLOCK_32xH] = Xin26xBlockSub32xH_AVX2;
        funcSet->pfXinBlockSubForDct[XIN_BLOCK_64xH] = Xin26xBlockSub64xH_AVX2;

        funcSet->pfXinBlockAddForDct[XIN_BLOCK_32xH] = Xin26xBlockRecon32xH_AVX2;
        funcSet->pfXinBlockAddForDct[XIN_BLOCK_64xH] = Xin26xBlockRecon64xH_AVX2;

        funcSet->pfXinComputeSadx5[XIN_BLOCK_32xH]  = Xin26xComputeSad32xHx5_AVX2;
        funcSet->pfXinComputeSadx5[XIN_BLOCK_64xH]  = Xin26xComputeSad64xHx5_AVX2;
        funcSet->pfXinComputeSadx5[XIN_BLOCK_128xH] = Xin26xComputeSad128xHx5_AVX2;

        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_32xH]  = Xin26xComputeAvgSadGt16xHx8_AVX2;
        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_64xH]  = Xin26xComputeAvgSadGt16xHx8_AVX2;
        funcSet->pfXinComputeAvgSadx8[XIN_BLOCK_128xH] = Xin26xComputeAvgSadGt16xHx8_AVX2;

        funcSet->pfXinComputeSadx8[XIN_BLOCK_32xH]  = Xin26xComputeSadGt16xHx8_AVX2;
        funcSet->pfXinComputeSadx8[XIN_BLOCK_64xH]  = Xin26xComputeSadGt16xHx8_AVX2;
        funcSet->pfXinComputeSadx8[XIN_BLOCK_128xH] = Xin26xComputeSadGt16xHx8_AVX2;

        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_16xH]  = Xin26xComputeAvgSatdGt8x8_AVX2;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_32xH]  = Xin26xComputeAvgSatdGt8x8_AVX2;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_64xH]  = Xin26xComputeAvgSatdGt8x8_AVX2;
        funcSet->pfXinComputeAvgSatd[XIN_BLOCK_128xH] = Xin26xComputeAvgSatdGt8x8_AVX2;

        funcSet->pfXinComputeSatd[XIN_BLOCK_8xH]   = Xin26xComputeSatd_AVX2;
        funcSet->pfXinComputeSatd[XIN_BLOCK_16xH]  = Xin26xComputeSatd_AVX2;
        funcSet->pfXinComputeSatd[XIN_BLOCK_32xH]  = Xin26xComputeSatd_AVX2;
        funcSet->pfXinComputeSatd[XIN_BLOCK_64xH]  = Xin26xComputeSatd_AVX2;
        funcSet->pfXinComputeSatd[XIN_BLOCK_128xH] = Xin26xComputeSatd_AVX2;

        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_16xH]  = Xin26xComputeWeightSatdGt8x8_AVX2;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_32xH]  = Xin26xComputeWeightSatdGt8x8_AVX2;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_64xH]  = Xin26xComputeWeightSatdGt8x8_AVX2;
        funcSet->pfXinComputeWeightSatd[XIN_BLOCK_128xH] = Xin26xComputeWeightSatdGt8x8_AVX2;

        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_16xH]  = Xin266ConstructWeightBiMeInputGt8xH_AVX2;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_32xH]  = Xin266ConstructWeightBiMeInputGt8xH_AVX2;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_64xH]  = Xin266ConstructWeightBiMeInputGt8xH_AVX2;
        funcSet->pfXinConstructWeightBiMeInput[XIN_BLOCK_128xH] = Xin266ConstructWeightBiMeInputGt8xH_AVX2;

        funcSet->pfXinFor2dSkip[XIN_BLOCK_64xH] = Xin26xFSkip64xH_AVX2;
        funcSet->pfXinFor2dSkip[XIN_BLOCK_32xH] = Xin26xFSkip32xH_AVX2;
        funcSet->pfXinFor2dSkip[XIN_BLOCK_16xH] = Xin26xFSkip16xH_AVX2;

        funcSet->pfXinInv2dSkip[XIN_BLOCK_64xH]  = Xin266ISkip64xH_AVX2;
        funcSet->pfXinInv2dSkip[XIN_BLOCK_32xH]  = Xin266ISkip32xH_AVX2;
        funcSet->pfXinInv2dSkip[XIN_BLOCK_16xH]  = Xin266ISkip16xH_AVX2;

        funcSet->pfXinInv2dDct2[XIN_BLOCK_8xH][XIN_BLOCK_8xH]   = Xin266IDct2W8H8_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_16xH] = Xin266IDct2W16H16_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_32xH] = Xin266IDct2W32H32_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_8xH]  = Xin266IDct2W16H8_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_8xH][XIN_BLOCK_16xH]  = Xin266IDct2W8H16_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_16xH] = Xin266IDct2W32H16_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_32xH] = Xin266IDct2W16H32_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_64xH][XIN_BLOCK_64xH] = Xin266IDct2W64H64_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_64xH] = Xin266IDct2W16H64_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_64xH][XIN_BLOCK_16xH] = Xin266IDct2W64H16_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_64xH][XIN_BLOCK_32xH] = Xin266IDct2W64H32_AVX2;
        funcSet->pfXinInv2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_64xH] = Xin266IDct2W32H64_AVX2;

        funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_8xH]  = Xin26xIDst7P8_AVX2;
        funcSet->pfXinInv1dDct[XIN_DST7][XIN_BLOCK_16xH] = Xin26xIDst7P16_AVX2;

        funcSet->pfXinFor2dDct2[XIN_BLOCK_8xH][XIN_BLOCK_8xH]   = Xin26xFDct2W8H8_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_16xH] = Xin26xFDct2W16H16_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_32xH] = Xin26xFDct2W32H32_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_8xH][XIN_BLOCK_16xH]  = Xin26xFDct2W8H16_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_8xH]  = Xin26xFDct2W16H8_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_16xH] = Xin26xFDct2W32H16_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_32xH] = Xin26xFDct2W16H32_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_8xH]  = Xin26xFDct2W32H8_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_8xH][XIN_BLOCK_32xH]  = Xin26xFDct2W8H32_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_64xH][XIN_BLOCK_64xH] = Xin26xFDct2W64H64_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_32xH][XIN_BLOCK_64xH] = Xin26xFDct2W32H64_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_64xH][XIN_BLOCK_32xH] = Xin26xFDct2W64H32_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_16xH][XIN_BLOCK_64xH] = Xin26xFDct2W16H64_AVX2;
        funcSet->pfXinFor2dDct2[XIN_BLOCK_64xH][XIN_BLOCK_16xH] = Xin26xFDct2W64H16_AVX2;

        funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_8xH]  = Xin26xFDst7P8_AVX2;
        funcSet->pfXinFor1dDct[XIN_DST7][XIN_BLOCK_16xH] = Xin26xFDst7P16_AVX2;

        funcSet->pfXinQuantInvQuant[XIN_BLOCK_16xH] = Xin266QuantInvQuantGt8xH_AVX2;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_32xH] = Xin266QuantInvQuantGt8xH_AVX2;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_64xH] = Xin266QuantInvQuantGt8xH_AVX2;

        funcSet->pfXinSaoStatEo[XIN_BLOCK_32xH]  = Xin26xSaoStatEo32xH_AVX2;
        funcSet->pfXinSaoStatEo[XIN_BLOCK_64xH]  = Xin26xSaoStatEo64xH_AVX2;
        funcSet->pfXinSaoStatEo[XIN_BLOCK_128xH] = Xin26xSaoStatEo128xH_AVX2;

        funcSet->pfXinApplyPDPC[XIN_BLOCK_16xH] = Xin266ApplyPDPC16xH_AVX2;
        //funcSet->pfXinApplyPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPC32xH_AVX2;
        //funcSet->pfXinApplyPDPC[XIN_BLOCK_64xH] = Xin266ApplyPDPCGt16xH_AVX2;

        funcSet->pfXinApplyHorPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPCHor32xH_AVX2;
        funcSet->pfXinApplyHorPDPC[XIN_BLOCK_32xH] = Xin266ApplyPDPCHor64xH_AVX2;

        funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_16xH] = Xin266ApplyAngPDPCHor16xH_AVX2;
        funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_32xH] = Xin266ApplyAngPDPCHorGt16xH_AVX2;
        funcSet->pfXinApplyAngPDPC[0][XIN_BLOCK_64xH] = Xin266ApplyAngPDPCHorGt16xH_AVX2;

        funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_4xH]  = Xin266ApplyAngPDPCVert4xH_AVX2;
        funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_8xH]  = Xin266ApplyAngPDPCVert8xH_AVX2;
        funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_16xH] = Xin266ApplyAngPDPCVertGt8xH_AVX2;
        funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_32xH] = Xin266ApplyAngPDPCVertGt8xH_AVX2;
        funcSet->pfXinApplyAngPDPC[1][XIN_BLOCK_64xH] = Xin266ApplyAngPDPCVertGt8xH_AVX2;

        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_16xH] = Xin266IntraPredPlanarGt8xH_AVX2;
        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_32xH] = Xin266IntraPredPlanarGt8xH_AVX2;
        funcSet->pfXinIntraPredPlanar[XIN_BLOCK_64xH] = Xin266IntraPredPlanarGt8xH_AVX2;

        funcSet->pfXinIntraPredHor[XIN_BLOCK_16xH] = Xin266IntraPredHor16xH_AVX2;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_32xH] = Xin266IntraPredHor32xH_AVX2;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_64xH] = Xin266IntraPredHor64xH_AVX2;

        funcSet->pfXinIntraPredVer[XIN_BLOCK_16xH] = Xin266IntraPredVer16xH_AVX2;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_32xH] = Xin266IntraPredVer32xH_AVX2;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_64xH] = Xin266IntraPredVer64xH_AVX2;

        funcSet->pfXinLumaIntraFilter[XIN_BLOCK_32xH] = Xin266LumaIntraFilterGt16xH_AVX2;
        funcSet->pfXinLumaIntraFilter[XIN_BLOCK_64xH] = Xin266LumaIntraFilterGt16xH_AVX2;

        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_16xH] = Xin266ChromaIntraFilterGt8xH_AVX2;
        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_32xH] = Xin266ChromaIntraFilterGt8xH_AVX2;
        funcSet->pfXinChromaIntraFilter[XIN_BLOCK_64xH] = Xin266ChromaIntraFilterGt8xH_AVX2;

        funcSet->pfXinComputeGradient[XIN_BLOCK_32xH]  = Xin266ComputeGradient32xH_AVX2;
        funcSet->pfXinComputeGradient[XIN_BLOCK_64xH]  = Xin266ComputeGradient64xH_AVX2;

        funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_32xH]  = Xin266BiliInterpCopy32xH_AVX2;
        funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_64xH]  = Xin266BiliInterpCopy64xH_AVX2;
        funcSet->pfXinBiliInterp[0][0][XIN_BLOCK_128xH] = Xin266BiliInterpCopy128xH_AVX2;
        
        funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_16xH]  = Xin266BiliInterpHorGt8xH_AVX2;
        funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_32xH]  = Xin266BiliInterpHorGt8xH_AVX2;
        funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_64xH]  = Xin266BiliInterpHorGt8xH_AVX2;
        funcSet->pfXinBiliInterp[1][0][XIN_BLOCK_128xH] = Xin266BiliInterpHorGt8xH_AVX2;

        funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_16xH]  = Xin266BiliInterpVetGt8xH_AVX2;
        funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_32xH]  = Xin266BiliInterpVetGt8xH_AVX2;
        funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_64xH]  = Xin266BiliInterpVetGt8xH_AVX2;
        funcSet->pfXinBiliInterp[0][1][XIN_BLOCK_128xH] = Xin266BiliInterpVetGt8xH_AVX2;

        funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_16xH]  = Xin266BiliInterpHorVetGt8xH_AVX2;
        funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_32xH]  = Xin266BiliInterpHorVetGt8xH_AVX2;
        funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_64xH]  = Xin266BiliInterpHorVetGt8xH_AVX2;
        funcSet->pfXinBiliInterp[1][1][XIN_BLOCK_128xH] = Xin266BiliInterpHorVetGt8xH_AVX2;

        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_32xH]  = Xin266InterpCopy32xH_AVX2;
        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_64xH]  = Xin266InterpCopy64xH_AVX2;
        funcSet->pfXinLumaInterp[0][0][XIN_BLOCK_128xH] = Xin266InterpCopy128xH_AVX2;

        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_32xH]  = Xin266LumaInterpVetGt16xH_AVX2;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_64xH]  = Xin266LumaInterpVetGt16xH_AVX2;
        funcSet->pfXinLumaInterp[0][1][XIN_BLOCK_128xH] = Xin266LumaInterpVetGt16xH_AVX2;
        
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_16xH]  = Xin266LumaInterpHorVetGt8xH_AVX2;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_32xH]  = Xin266LumaInterpHorVetGt8xH_AVX2;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_64xH]  = Xin266LumaInterpHorVetGt8xH_AVX2;
        funcSet->pfXinLumaInterp[1][1][XIN_BLOCK_128xH] = Xin266LumaInterpHorVetGt8xH_AVX2;

        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_32xH]  = Xin266LumaInterpHorGt16xH_AVX2;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_64xH]  = Xin266LumaInterpHorGt16xH_AVX2;
        funcSet->pfXinLumaInterp[1][0][XIN_BLOCK_128xH] = Xin266LumaInterpHorGt16xH_AVX2;

        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_16xH]  = Xin266LumaInterpHorGt8xHU8S16_AVX2;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_32xH]  = Xin266LumaInterpHorGt8xHU8S16_AVX2;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_64xH]  = Xin266LumaInterpHorGt8xHU8S16_AVX2;
        funcSet->pfXinLumaInterpS16[1][0][XIN_BLOCK_128xH] = Xin266LumaInterpHorGt8xHU8S16_AVX2;

        funcSet->pfXinInterpWeight[XIN_BLOCK_16xH]   = Xin266InterpWeightGt8xHS16U8_AVX2;
        funcSet->pfXinInterpWeight[XIN_BLOCK_32xH]   = Xin266InterpWeightGt8xHS16U8_AVX2;
        funcSet->pfXinInterpWeight[XIN_BLOCK_64xH]   = Xin266InterpWeightGt8xHS16U8_AVX2;
        funcSet->pfXinInterpWeight[XIN_BLOCK_128xH]  = Xin266InterpWeightGt8xHS16U8_AVX2;

        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_32xH]  = Xin266LumaInterpHorVetGt16xHU8S16_AVX2;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_64xH]  = Xin266LumaInterpHorVetGt16xHU8S16_AVX2;
        funcSet->pfXinLumaInterpS16[1][1][XIN_BLOCK_128xH] = Xin266LumaInterpHorVetGt16xHU8S16_AVX2;

        funcSet->pfXinComputeFrameAct = Xin26xComputeFrameAct_AVX2;
        funcSet->pfXinPictureCopy     = Xin26xPictureCopy_AVX2;
        funcSet->pfXinDepQuant        = Xin266DepQuant_AVX2;
        funcSet->pfXinExtendIntraRef  = Xin266ExtendIntraRef_AVX2;
        funcSet->pfXinFilterIntraNB   = Xin266FilterIntraNB_AVX2;
        funcSet->pfXinCopyAndPad      = Xin266CopyAndPad_AVX2;

    }
#endif
}

