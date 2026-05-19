/***************************************************************************//**
*
* @file          h265p_trans_recon.h
* @brief         This file delcare transform or reconstruction subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_trans_recon_h_
#define _h265p_trans_recon_h_

void Xin265pReconMb (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb);

void Xin265pTransformTx (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    xin_tu_struct   *tu,
    UINT32          planeIdx);

#endif

