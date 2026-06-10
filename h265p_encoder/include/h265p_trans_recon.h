/***************************************************************************//**
 *
 * @file          h265p_trans_recon.h
 * @brief         This file declares transform or reconstruction subroutines.
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

