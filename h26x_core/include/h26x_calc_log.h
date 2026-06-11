/***************************************************************************//**
 *
 * @file          h26x_calc_log.h
 * @brief         This file declares the log calculation subroutine.
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
#ifndef _h26x_calc_log_h_
#define _h26x_calc_log_h_

#define XIN_LOG_FRAC_BITS       8
#define XIN_LOG_ROUNDING        (1 << (XIN_LOG_FRAC_BITS - 1))
#define XIN_LOG_FRACTION        (1<<XIN_LOG_FRAC_BITS)
#define XIN_LOG_INT2FLOAT(X)    ((X)/((double)XIN_LOG_FRACTION))
UINT32 XinCalcLog (
    UINT32 value);

#endif
