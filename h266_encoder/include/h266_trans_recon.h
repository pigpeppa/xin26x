/***************************************************************************//**
 *
 * @file          h266_trans_recon.h
 * @brief         This file declares transform and reconstruction subroutines.
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
#ifndef _h266_trans_recon_h_
#define _h266_trans_recon_h_

void Xin266Transform (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    UINT32          planeIdx);

void Xin266ReconCu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu);

void Xin266ReconTu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    UINT32          planeIdx);

#endif

