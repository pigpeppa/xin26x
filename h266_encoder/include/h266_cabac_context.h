/***************************************************************************//**
 *
 * @file          h266_cabac_context.h
 * @brief         This file defines CABAC context related tables.
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
#ifndef _h266_cabac_context_h_
#define _h266_cabac_context_h_

void Xin266InitCabacContext (
    xin_prob_model *proModel,
    SINT32         sliceIdx,
    SINT32         qpIdx);

void Xin266CabacContextInit (
    xin_cabac_context *cabacSet,
    xin_prob_model    *context,
    SINT32            sliceIdx,
    SINT32            qpIdx,
    BOOL              resetContext);
    
#endif

