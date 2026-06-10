/***************************************************************************//**
 *
 * @file          h265p_func_init.c
 * @brief         Assign function pointers according to CPU features.
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
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_func_struct.h"
#include "h26x_block_utility.h"
#include "h26x_compute_dist.h"
#include "h265p_trans_context.h"
#include "h265p_forward_trans.h"
#include "h26x_cpu_detection.h"
#include "h265p_intra_pred_filter.h"
#include "h265p_tx_init_level.h"
#include "h265p_inverse_trans.h"
#include "h265p_quant_inv_quant.h"
#include "h265p_compute_dist.h"

void Xin265pFuncInit (
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

    funcSet->pfXinComputeSad[XIN_BLOCK_1xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_2xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_4xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_8xH]   = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_16xH]  = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_32xH]  = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_64xH]  = Xin26xComputeSad;
    funcSet->pfXinComputeSad[XIN_BLOCK_128xH] = Xin26xComputeSad;

    funcSet->pfXinComputeSatd[XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinComputeSatd[XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinComputeSatd[XIN_BLOCK_4xH]   = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_8xH]   = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_16xH]  = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_32xH]  = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_64xH]  = Xin26xComputeSatd;
    funcSet->pfXinComputeSatd[XIN_BLOCK_128xH] = Xin26xComputeSatd;

    funcSet->pfXinComputeAvgSad[XIN_BLOCK_1xH]   = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_2xH]   = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_4xH]   = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_8xH]   = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_16xH]  = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_32xH]  = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_64xH]  = Xin26xComputeAvgSad;
    funcSet->pfXinComputeAvgSad[XIN_BLOCK_128xH] = Xin26xComputeAvgSad;

    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_1xH]   = NULL;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_2xH]   = NULL;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_4xH]   = Xin26xComputeAvgSatd4x4;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_8xH]   = Xin26xComputeAvgSatdGt4x4;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_16xH]  = Xin26xComputeAvgSatdGt4x4;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_32xH]  = Xin26xComputeAvgSatdGt4x4;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_64xH]  = Xin26xComputeAvgSatdGt4x4;
    funcSet->pfXinComputeAvgSatd[XIN_BLOCK_128xH] = Xin26xComputeAvgSatdGt4x4;

    funcSet->pfXinComputeSsdFd[XIN_BLOCK_1xH]   = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_2xH]   = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_4xH]   = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_8xH]   = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_16xH]  = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_32xH]  = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_64xH]  = Xin265pComputeSsdFd;
    funcSet->pfXinComputeSsdFd[XIN_BLOCK_128xH] = Xin265pComputeSsdFd;

    funcSet->pfXinBlockSub[XIN_BLOCK_2xH]   = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_4xH]   = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_8xH]   = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_16xH]  = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_32xH]  = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_64xH]  = Xin26xBlockSub;
    funcSet->pfXinBlockSub[XIN_BLOCK_128xH] = Xin26xBlockSub;

    funcSet->pfXinBlockRecon[XIN_BLOCK_2xH]   = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_4xH]   = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_8xH]   = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_16xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_32xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_64xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_128xH] = Xin26xBlockRecon;

    funcSet->pfXinComputeSadx3[XIN_BLOCK_1xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_2xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_4xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_8xH]   = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_16xH]  = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_32xH]  = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_64xH]  = Xin26xComputeSadx3;
    funcSet->pfXinComputeSadx3[XIN_BLOCK_128xH] = Xin26xComputeSadx3;

    funcSet->pfXinIntraPredVer[XIN_BLOCK_4xH]  = Xin265pIntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_8xH]  = Xin265pIntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_16xH] = Xin265pIntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_32xH] = Xin265pIntraPredVer;
    funcSet->pfXinIntraPredVer[XIN_BLOCK_64xH] = Xin265pIntraPredVer;

    funcSet->pfXinIntraPredHor[XIN_BLOCK_4xH]  = Xin265pIntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_8xH]  = Xin265pIntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_16xH] = Xin265pIntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_32xH] = Xin265pIntraPredHor;
    funcSet->pfXinIntraPredHor[XIN_BLOCK_64xH] = Xin265pIntraPredHor;

    funcSet->pfXinIntraPredSmV[XIN_BLOCK_4xH]  = Xin265pIntraPredSmV;
    funcSet->pfXinIntraPredSmV[XIN_BLOCK_8xH]  = Xin265pIntraPredSmV;
    funcSet->pfXinIntraPredSmV[XIN_BLOCK_16xH] = Xin265pIntraPredSmV;
    funcSet->pfXinIntraPredSmV[XIN_BLOCK_32xH] = Xin265pIntraPredSmV;
    funcSet->pfXinIntraPredSmV[XIN_BLOCK_64xH] = Xin265pIntraPredSmV;

    funcSet->pfXinIntraPredSmH[XIN_BLOCK_4xH]  = Xin265pIntraPredSmH;
    funcSet->pfXinIntraPredSmH[XIN_BLOCK_8xH]  = Xin265pIntraPredSmH;
    funcSet->pfXinIntraPredSmH[XIN_BLOCK_16xH] = Xin265pIntraPredSmH;
    funcSet->pfXinIntraPredSmH[XIN_BLOCK_32xH] = Xin265pIntraPredSmH;
    funcSet->pfXinIntraPredSmH[XIN_BLOCK_64xH] = Xin265pIntraPredSmH;

    funcSet->pfXinIntraPredSm[XIN_BLOCK_4xH]  = Xin265pIntraPredSm;
    funcSet->pfXinIntraPredSm[XIN_BLOCK_8xH]  = Xin265pIntraPredSm;
    funcSet->pfXinIntraPredSm[XIN_BLOCK_16xH] = Xin265pIntraPredSm;
    funcSet->pfXinIntraPredSm[XIN_BLOCK_32xH] = Xin265pIntraPredSm;
    funcSet->pfXinIntraPredSm[XIN_BLOCK_64xH] = Xin265pIntraPredSm;

    funcSet->pfXinIntraPredPaeth[XIN_BLOCK_4xH]  = Xin265pIntraPredPaeth;
    funcSet->pfXinIntraPredPaeth[XIN_BLOCK_8xH]  = Xin265pIntraPredPaeth;
    funcSet->pfXinIntraPredPaeth[XIN_BLOCK_16xH] = Xin265pIntraPredPaeth;
    funcSet->pfXinIntraPredPaeth[XIN_BLOCK_32xH] = Xin265pIntraPredPaeth;
    funcSet->pfXinIntraPredPaeth[XIN_BLOCK_64xH] = Xin265pIntraPredPaeth;

    funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_4xH]  = Xin265pIntraPredDrZ2;
    funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_8xH]  = Xin265pIntraPredDrZ2;
    funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_16xH] = Xin265pIntraPredDrZ2;
    funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_32xH] = Xin265pIntraPredDrZ2;
    funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_64xH] = Xin265pIntraPredDrZ2;

    funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_4xH]  = Xin265pIntraPredDrZ1;
    funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_8xH]  = Xin265pIntraPredDrZ1;
    funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_16xH] = Xin265pIntraPredDrZ1;
    funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_32xH] = Xin265pIntraPredDrZ1;
    funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_64xH] = Xin265pIntraPredDrZ1;

    funcSet->pfXinQuantInvQuant[XIN_BLOCK_4xH]  = Xin265pQuantInvQuantB;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_8xH]  = Xin265pQuantInvQuantB;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_16xH] = Xin265pQuantInvQuantB;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_32xH] = Xin265pQuantInvQuantB;
    funcSet->pfXinQuantInvQuant[XIN_BLOCK_64xH] = Xin265pQuantInvQuantB;

    funcSet->pfXinForTrans2d[XIN_TX_4X4]   = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_8X8]   = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X16] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X32] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_64X64] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_4X8]   = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_8X4]   = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_8X16]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X8]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X32] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X16] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X64] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_64X32] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_4X16]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X4]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_8X32]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X8]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X32] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X16] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X64] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_64X32] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_4X16]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X4]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_8X32]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_32X8]  = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_16X64] = Xin265pFDctWxH;
    funcSet->pfXinForTrans2d[XIN_TX_64X16] = Xin265pFDctWxH;

    funcSet->pfXinInvTrans2d[XIN_TX_4X4]   = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_8X8]   = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X16] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X32] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_64X64] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_4X8]   = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_8X4]   = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_8X16]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X8]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X32] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X16] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X64] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_64X32] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_4X16]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X4]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_8X32]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X8]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X32] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X16] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X64] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_64X32] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_4X16]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X4]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_8X32]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_32X8]  = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_16X64] = Xin265pIDctWxH;
    funcSet->pfXinInvTrans2d[XIN_TX_64X16] = Xin265pIDctWxH;

    funcSet->pfXinTxInitLevel[XIN_BLOCK_4xH]  = Xin265pTxInitLevel;
    funcSet->pfXinTxInitLevel[XIN_BLOCK_8xH]  = Xin265pTxInitLevel;
    funcSet->pfXinTxInitLevel[XIN_BLOCK_16xH] = Xin265pTxInitLevel;
    funcSet->pfXinTxInitLevel[XIN_BLOCK_32xH] = Xin265pTxInitLevel;
    funcSet->pfXinTxInitLevel[XIN_BLOCK_64xH] = Xin265pTxInitLevel;

    funcSet->pfXinBlockRecon[XIN_BLOCK_2xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_4xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_8xH]  = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_16xH] = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_32xH] = Xin26xBlockRecon;
    funcSet->pfXinBlockRecon[XIN_BLOCK_64xH] = Xin26xBlockRecon;

#ifdef _X86_OPT_

    if (cpuFeature & XIN_CPU_SSE2)
    {
        funcSet->pfXinBlockCopy[XIN_BLOCK_4xH]   = Xin26xBlockCopy4xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_8xH]   = Xin26xBlockCopy8xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_16xH]  = Xin26xBlockCopy16xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_32xH]  = Xin26xBlockCopy32xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_64xH]  = Xin26xBlockCopy64xH_SSE2;
        funcSet->pfXinBlockCopy[XIN_BLOCK_128xH] = Xin26xBlockCopy128xH_SSE2;
    
        funcSet->pfXinForTrans2d[XIN_TX_4X4]  = Xin265pFDct4x4_SSE2;
        funcSet->pfXinForTrans2d[XIN_TX_8X4]  = Xin265pFDct8x4_SSE2;
        funcSet->pfXinForTrans2d[XIN_TX_4X8]  = Xin265pFDct4x8_SSE2;
        funcSet->pfXinForTrans2d[XIN_TX_8X8]  = Xin265pFDct8x8_SSE2;

        funcSet->pfXinComputeSad[XIN_BLOCK_4xH]  = Xin26xComputeSad4xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_8xH]  = Xin26xComputeSad8xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_16xH] = Xin26xComputeSad16xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_32xH] = Xin26xComputeSad32xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_64xH] = Xin26xComputeSad64xH_SSE2;
        funcSet->pfXinComputeSad[XIN_BLOCK_64xH] = Xin26xComputeSad128xH_SSE2;

        funcSet->pfXinBlockSub[XIN_BLOCK_8xH]  = Xin26xBlockSub8xH_SSE2;
        funcSet->pfXinBlockSub[XIN_BLOCK_16xH] = Xin26xBlockSub16xH_SSE2;

        funcSet->pfXinBlockRecon[XIN_BLOCK_8xH]  = Xin26xBlockRecon8xH_SSE2;
        funcSet->pfXinBlockRecon[XIN_BLOCK_16xH] = Xin26xBlockRecon16xH_SSE2;
        
    }

    if (cpuFeature & XIN_CPU_SSE42)
    {
        funcSet->pfXinInvTrans2d[XIN_TX_4X4]  = Xin265pIDct4x4_SSE4;
        funcSet->pfXinInvTrans2d[XIN_TX_4X8]  = Xin265pIDct4x8_SSE4;
        funcSet->pfXinInvTrans2d[XIN_TX_8X4]  = Xin265pIDct8x4_SSE4;
        funcSet->pfXinInvTrans2d[XIN_TX_8X8]  = Xin265pIDct8x8_SSE4;
        funcSet->pfXinInvTrans2d[XIN_TX_8X16] = Xin265pIDct8x8_SSE4;
        funcSet->pfXinInvTrans2d[XIN_TX_16X8] = Xin265pIDct8x8_SSE4;
        
        funcSet->pfXinIntraPredVer[XIN_BLOCK_4xH]  = Xin265pIntraPredVer4xH_SSE4;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_8xH]  = Xin265pIntraPredVer8xH_SSE4;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_16xH] = Xin265pIntraPredVer16xH_SSE4;

        funcSet->pfXinIntraPredHor[XIN_BLOCK_4xH]  = Xin265pIntraPredHor4xH_SSE4;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_8xH]  = Xin265pIntraPredHor8xH_SSE4;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_16xH] = Xin265pIntraPredHor16xH_SSE4;

        funcSet->pfXinIntraPredSmV[XIN_BLOCK_8xH] = Xin265pIntraPredSmV8xH_SSE4;
        funcSet->pfXinIntraPredSm[XIN_BLOCK_8xH]  = Xin265pIntraPredSm8xH_SSE4;
        funcSet->pfXinIntraPredSmH[XIN_BLOCK_8xH] = Xin265pIntraPredSmH8xH_SSE4;

        funcSet->pfXinIntraPredPaeth[XIN_BLOCK_8xH]  = Xin265pIntraPredPaeth8xH_SSE4;
        funcSet->pfXinIntraPredPaeth[XIN_BLOCK_16xH] = Xin265pIntraPredPaeth16xH_SSE4;
        
        funcSet->pfXinTxInitLevel[XIN_BLOCK_4xH]  = Xin265pTxInitLevel4xH_SSE4;
        funcSet->pfXinTxInitLevel[XIN_BLOCK_8xH]  = Xin265pTxInitLevel8xH_SSE4;
        funcSet->pfXinTxInitLevel[XIN_BLOCK_16xH] = Xin265pTxInitLevel16xH_SSE4;
        funcSet->pfXinTxInitLevel[XIN_BLOCK_32xH] = Xin265pTxInitLevel32xH_SSE4;

        funcSet->pfXinQuantInvQuant[XIN_BLOCK_4xH] = Xin265pQuantInvQuantB4xH_SSE4;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_8xH] = Xin265pQuantInvQuantB8xH_SSE4;
        
    }

    if (cpuFeature & XIN_CPU_AVX2)
    {
        funcSet->pfXinForTrans2d[XIN_TX_8X16]  = Xin265pFDct8x16_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_16X8]  = Xin265pFDct16x8_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_16X16] = Xin265pFDct16x16_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_32X32] = Xin265pFDct32x32_AVX2;
		funcSet->pfXinForTrans2d[XIN_TX_32X16] = Xin265pFDct32x16_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_16X32] = Xin265pFDct16x32_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_64X64] = Xin265pFDct64x64_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_64X32] = Xin265pFDct64x32_AVX2;
        funcSet->pfXinForTrans2d[XIN_TX_32X64] = Xin265pFDct32x64_AVX2;

        funcSet->pfXinInvTrans2d[XIN_TX_16X16] = Xin265pIDctGt8xGt8_AVX2;
        funcSet->pfXinInvTrans2d[XIN_TX_32X16] = Xin265pIDctGt8xGt8_AVX2;
        funcSet->pfXinInvTrans2d[XIN_TX_16X32] = Xin265pIDctGt8xGt8_AVX2;
        funcSet->pfXinInvTrans2d[XIN_TX_32X32] = Xin265pIDctGt8xGt8_AVX2;
        funcSet->pfXinInvTrans2d[XIN_TX_32X64] = Xin265pIDctGt8xGt8_AVX2;
        funcSet->pfXinInvTrans2d[XIN_TX_64X32] = Xin265pIDctGt8xGt8_AVX2;
        funcSet->pfXinInvTrans2d[XIN_TX_64X64] = Xin265pIDctGt8xGt8_AVX2;

        funcSet->pfXinComputeSad[XIN_BLOCK_32xH]  = Xin26xComputeSad32xH_AVX2;
        funcSet->pfXinComputeSad[XIN_BLOCK_64xH]  = Xin26xComputeSad64xH_AVX2;
        funcSet->pfXinComputeSad[XIN_BLOCK_128xH] = Xin26xComputeSad128xH_AVX2;

        funcSet->pfXinComputeSsdFd[XIN_BLOCK_32xH]  = Xin265pComputeSsdFdGt16xH_AVX2;
        funcSet->pfXinComputeSsdFd[XIN_BLOCK_64xH]  = Xin265pComputeSsdFdGt16xH_AVX2;
        funcSet->pfXinComputeSsdFd[XIN_BLOCK_128xH] = Xin265pComputeSsdFdGt16xH_AVX2;

        funcSet->pfXinIntraPredSmV[XIN_BLOCK_16xH] = Xin265pIntraPredSmV16xH_AVX2;
        funcSet->pfXinIntraPredSmV[XIN_BLOCK_32xH] = Xin265pIntraPredSmV32xH_AVX2;
        funcSet->pfXinIntraPredSmV[XIN_BLOCK_64xH] = Xin265pIntraPredSmV64xH_AVX2;

        funcSet->pfXinIntraPredSm[XIN_BLOCK_16xH]  = Xin265pIntraPredSm16xH_AVX2;
        funcSet->pfXinIntraPredSm[XIN_BLOCK_32xH]  = Xin265pIntraPredSm32xH_AVX2;
        funcSet->pfXinIntraPredSm[XIN_BLOCK_64xH]  = Xin265pIntraPredSm64xH_AVX2;

        funcSet->pfXinIntraPredSmH[XIN_BLOCK_16xH] = Xin265pIntraPredSmH16xH_AVX2;
        funcSet->pfXinIntraPredSmH[XIN_BLOCK_32xH] = Xin265pIntraPredSmH32xH_AVX2;
        funcSet->pfXinIntraPredSmH[XIN_BLOCK_64xH] = Xin265pIntraPredSmH64xH_AVX2;
        
        funcSet->pfXinIntraPredHor[XIN_BLOCK_32xH] = Xin265pIntraPredHor32xH_AVX2;
        funcSet->pfXinIntraPredHor[XIN_BLOCK_64xH] = Xin265pIntraPredHor64xH_AVX2;
        
        funcSet->pfXinIntraPredVer[XIN_BLOCK_32xH] = Xin265pIntraPredVer32xH_AVX2;
        funcSet->pfXinIntraPredVer[XIN_BLOCK_64xH] = Xin265pIntraPredVer64xH_AVX2;

        funcSet->pfXinIntraPredPaeth[XIN_BLOCK_32xH] = Xin265pIntraPredPaeth32xH_AVX2;
        funcSet->pfXinIntraPredPaeth[XIN_BLOCK_64xH] = Xin265pIntraPredPaeth64xH_AVX2;
    
        funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_4xH]  = Xin265pIntraPred4xHDrZ2_AVX2;
        funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_8xH]  = Xin265pIntraPred8xHDrZ2_AVX2;
        funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_16xH] = Xin265pIntraPredWxHDrZ2_AVX2;
        funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_32xH] = Xin265pIntraPredWxHDrZ2_AVX2;
        funcSet->pfXinIntraPredDrZ2[XIN_BLOCK_64xH] = Xin265pIntraPredWxHDrZ2_AVX2;

        funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_4xH]  = Xin265pIntraPred4xHDrZ1_AVX2;
        funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_8xH]  = Xin265pIntraPred8xHDrZ1_AVX2;
        funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_16xH] = Xin265pIntraPred16xHDrZ1_AVX2;
        funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_32xH] = Xin265pIntraPred32xHDrZ1_AVX2;
        funcSet->pfXinIntraPredDrZ1[XIN_BLOCK_64xH] = Xin265pIntraPred64xHDrZ1_AVX2;

        funcSet->pfXinQuantInvQuant[XIN_BLOCK_16xH] = Xin265pQuantInvQuantB16xH_AVX2;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_32xH] = Xin265pQuantInvQuantB32xH_AVX2;
        funcSet->pfXinQuantInvQuant[XIN_BLOCK_64xH] = Xin265pQuantInvQuantB32xH_AVX2;

        funcSet->pfXinBlockSub[XIN_BLOCK_32xH] = Xin26xBlockSub32xH_AVX2;
        funcSet->pfXinBlockSub[XIN_BLOCK_64xH] = Xin26xBlockSub64xH_AVX2;

        funcSet->pfXinBlockRecon[XIN_BLOCK_32xH] = Xin26xBlockRecon32xH_AVX2;
        funcSet->pfXinBlockRecon[XIN_BLOCK_64xH] = Xin26xBlockRecon64xH_AVX2;
        
    }
    
#endif

}

