/***************************************************************************//**
 *
 * @file          h266_scene_cut_struct.h
 * @brief         This file contains h266 scene cut related structures.
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
#ifndef _h266_scene_cut_struct_h_
#define _h266_scene_cut_struct_h_

typedef struct xin_sc_struct
{
    xin_seq_struct    *seqSet;
    xin_func_struct   *funcSet;
    xin_input_picture *pictureWrite;
    xin_input_picture *pictureRead;
    SINT32            lastSceneCut;
    xin_job_desc      *jobSceneCut;
    BOOL              isBusy;
}xin_sc_struct;

#endif