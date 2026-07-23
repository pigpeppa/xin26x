/***************************************************************************//**
 *
 * @file          h266_scan_order.h
 * @brief         This file declares tables for coefficient scan order.
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
#ifndef _h266_scan_order_h_
#define _h266_scan_order_h_

extern xin_scan_pos *coeffScanOrder[XIN_MAX_LG_CG_SIZE][XIN_MAX_LG_CG_SIZE];
extern xin_scan_pos *coeffScanOrderCG[XIN_MAX_LG_TU_SIZE][XIN_MAX_LG_TU_SIZE];

#endif

