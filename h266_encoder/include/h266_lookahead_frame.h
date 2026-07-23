/***************************************************************************//**
 *
 * @file          h266_lookahead_frame.h
 * @brief         This file contains h266 lookahead subroutine declarations.
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
#ifndef _h266_lookahead_frame_h_
#define _h266_lookahead_frame_h_

void Xin266LowerFrameInit (
    xin_seq_struct    *seqSet,
    xin_input_picture *inputPicture);

void Xin266LowerFrameCost (
    xin_la_struct    *laSet);

void Xin266LowerFrameIntraCost (
    xin_la_struct    *laSet);

void Xin266UnitTree (
    xin_seq_struct *seqSet);

#endif

