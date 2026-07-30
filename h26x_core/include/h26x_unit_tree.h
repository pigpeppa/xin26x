/***************************************************************************//**
 *
 * @file          h26x_unit_tree.h
 * @brief         This file declares unit tree related subroutines.
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
#ifndef _h26x_unit_tree_h_
#define _h26x_unit_tree_h_

void Xin26xPropagateList (
    SINT32        propIn,
    UINT32        interDir,
    SINT32        srcUnitX,
    SINT32        srcUnitY,
    SINT32        lgUnitSize,
    SINT32        wdtInUnit,
    SINT32        hgtInUnit,
    xin_mv_u      **mv,
    UINT16        **propOut);

void Xin26xUnitTreeFinish (
    xin_input_picture *inputPicture);

void Xin26xUnitPropagate (
    xin_input_picture *inputPicture,
    UINT32            laUnitSize);

#endif
