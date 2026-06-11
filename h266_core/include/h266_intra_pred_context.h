/***************************************************************************//**
 *
 * @file          h266_intra_pred_context.h
 * @brief         This file contains h266 constant definitions.
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
#ifndef _h266_intra_pred_context_h_
#define _h266_intra_pred_context_h_

#define XIN_INTRA_MPM_NUM           6
///< Planar + DC + 65 directional mode (4*16 + 1)
#define XIN_LUMA_MODE_NUM           67
#define XIN_LM_CHROMA_IDX           67
#define XIN_LM_CHROMA_L_IDX         68
#define XIN_LM_CHROMA_T_IDX         69

///< LMC + MDLM_T + MDLM_L
#define XIN_LMC_MODE_NUM            3
#define XIN_INTRA_MODE_NUM          (XIN_LUMA_MODE_NUM + XIN_LMC_MODE_NUM)
#define XIN_INTRA_DIR_NUM           (((XIN_LUMA_MODE_NUM - 3) >> 2) + 1)
#define XIN_PLANAR_IDX              0
#define XIN_DC_IDX                  1
#define XIN_HOR_IDX                 (1 * (XIN_INTRA_DIR_NUM - 1) + 2)
#define XIN_DIA_IDX                 (2 * (XIN_INTRA_DIR_NUM - 1) + 2)
#define XIN_VER_IDX                 (3 * (XIN_INTRA_DIR_NUM - 1) + 2)
#define XIN_VDIA_IDX                (4 * (XIN_INTRA_DIR_NUM - 1) + 2)
#define XIN_BDPCM_IDX               (5 * (XIN_INTRA_DIR_NUM - 1) + 2)
#define XIN_INTRA_MOD               (XIN_LUMA_MODE_NUM - 3)
#define XIN_INTRA_MOD_MASK          (XIN_INTRA_MOD - 1)
#define XIN_DM_CHROMA_IDX           XIN_INTRA_MODE_NUM
#define XIN_INTRA_CHROMA_CAND_NUM   (5 + XIN_LMC_MODE_NUM)

#endif

