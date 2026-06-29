/***************************************************************************//**
 *
 * @file          h266_encoder_create.c
 * @brief         h266 encoder Application Programming Interface.
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
#include "xin_typedef.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "xin26x_params.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_common_data.h"
#include "basic_macro.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_section_struct.h"
#include "h266_coding_unit_struct.h"
#include "h26x_cpu_detection.h"
#include "h26x_thread_wrapper.h"
#include "h26x_thread_pool.h"
#include "h266_enc_init.h"
#include "h266_cabac_context.h"
#include "h266_encoder_create.h"
#include "h26x_rate_control_struct.h"
#include "h266_rate_control.h"
#include "h26x_look_ahead_struct.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h266_lookahead_frame.h"
#include "h26x_motion_est.h"
#include "h26x_mctf_struct.h"
#include "h26x_mctf.h"
#include "h26x_look_ahead_struct.h"
#include "h26x_look_ahead.h"
#include "h266_alf_rdo.h"
#include "h266_dep_quant.h"
#include "h266_lmcs.h"
#include "h266_func_struct.h"

static xin_depth_range drConfigSlow[80] =
{
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
};

static xin_depth_range drConfigFast[80] =
{
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3},
    {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3},
    {1, 3}, {1, 3}, {1, 3}, {1, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}
};

static const UINT32 qMult[2][6]  =
{
    {26214, 23302, 20560, 18396, 16384, 14564},

    {18396, 16384, 14564, 13107, 11651, 10280}
};

static const UINT32 iqMult[2][6] =
{
    {40,   45,   51,   57,   64,   72},
    {57,   64,   72,   80,   90,  102}
};

static xin_rps_struct rpsLd1Layer[1] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    }

};

static xin_rps_struct rpsLd2Layer[2] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {2, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },

    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        1,                  // temporalId
        0,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },

};

static xin_rps_struct rpsLd3Layer[4] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {4, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },

    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        2,                  // temporalId
        0,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },

    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        1,                  // temporalId
        1,                  // isRefFrame
        {2, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },

    {
        2,                  // numberOfNegPics
        0,                  // numberOfPosPics
        2,                  // temporalId
        0,                  // isRefFrame
        {1, 2, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },
};

xin_rps_struct* xin266RpsLowDelay[XIN_MAX_TEMPORAL_LAYER+1] =
{
    NULL,
    rpsLd1Layer,
    rpsLd2Layer,
    rpsLd3Layer,
    NULL
};

static xin_rps_struct encoderRpsGop1[1] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },

};

static xin_rps_struct encoderRpsGop2[2] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {2, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    },
    {
        1,                  // numberOfNegPics
        1,                  // numberOfPosPics
        1,                  // temporalId
        0,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {1, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
    }
};

static xin_rps_struct encoderRpsGop4[4] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {4, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        2,                  // numberOfPosPics
        2,                  // temporalId
        0,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {1, 2, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        1,                  // numberOfPosPics
        1,                  // temporalId
        1,                  // isRefFrame
        {2, 0, 0, 0, 0, 0}, // deltaNegPos
        {2, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        1,                  // numberOfPosPics
        2,                  // temporalId
        0,                  // isRefFrame
        {1, 2, 0, 0, 0, 0}, // deltaNegPos
        {1, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    }
};

static xin_rps_struct encoderRpsGop8[8] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {8, 0, 0, 0, 0, 0}, // deltaNegPos
        {0, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        3,                  // numberOfPosPics
        3,                  // temporalId
        0,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {1, 2, 4, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        2,                  // numberOfPosPics
        2,                  // temporalId
        1,                  // isRefFrame
        {2, 0, 0, 0, 0, 0}, // deltaNegPos
        {2, 4, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        2,                  // numberOfPosPics
        3,                  // temporalId
        0,                  // isRefFrame
        {1, 2, 0, 0, 0, 0}, // deltaNegPos
        {1, 4, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        1,                  // numberOfPosPics
        1,                  // temporalId
        1,                  // isRefFrame
        {4, 0, 0, 0, 0, 0}, // deltaNegPos
        {4, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        2,                  // numberOfPosPics
        3,                  // temporalId
        0,                  // isRefFrame
        {1, 4, 0, 0, 0, 0}, // deltaNegPos
        {1, 2, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        1,                  // numberOfPosPics
        2,                  // temporalId
        1,                  // isRefFrame
        {2, 4, 0, 0, 0, 0}, // deltaNegPos
        {2, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        1,                  // numberOfPosPics
        3,                  // temporalId
        0,                  // isRefFrame
        {1, 6, 0, 0, 0, 0}, // deltaNegPos
        {1, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    }
};

static xin_rps_struct encoderRpsGop16[16] =
{
    {
        1,                  // numberOfNegPics
        0,                  // numberOfPosPics
        0,                  // temporalId
        1,                  // isRefFrame
        {16, 0, 0, 0, 0, 0}, // deltaNegPos
        {0,  0, 0, 0, 0, 0}, // deltaPosPos
        {1,  0, 0, 0, 0, 0}, // usedByNegPicFlag
        {0,  0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        4,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 0, 0, 0, 0, 0}, // deltaNegPos
        {1, 2, 4, 8, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        3,                  // numberOfPosPics
        3,                  // temporalId
        1,                  // isRefFrame
        {2, 0, 0, 0, 0, 0}, // deltaNegPos
        {2, 4, 8, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        3,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 2, 0, 0, 0, 0}, // deltaNegPos
        {1, 4, 8, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        1,                  // numberOfNegPics
        2,                  // numberOfPosPics
        2,                  // temporalId
        1,                  // isRefFrame
        {4, 0, 0, 0, 0, 0}, // deltaNegPos
        {4, 8, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        3,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 4, 0, 0, 0, 0}, // deltaNegPos
        {1, 2, 8, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        2,                  // numberOfPosPics
        3,                  // temporalId
        1,                  // isRefFrame
        {2, 4, 0, 0, 0, 0}, // deltaNegPos
        {2, 8, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        2,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 6, 0, 0, 0, 0}, // deltaNegPos
        {1, 8, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },

    {
        1,                  // numberOfNegPics
        1,                  // numberOfPosPics
        1,                  // temporalId
        1,                  // isRefFrame
        {8, 0, 0, 0, 0, 0}, // deltaNegPos
        {8, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        3,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 8, 0, 0, 0, 0}, // deltaNegPos
        {1, 2, 4, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        2,                  // numberOfPosPics
        3,                  // temporalId
        1,                  // isRefFrame
        {2, 8, 0, 0, 0, 0}, // deltaNegPos
        {2, 4, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        3,                  // numberOfNegPics
        2,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 2, 8, 0, 0, 0}, // deltaNegPos
        {1, 4, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        2,                  // numberOfNegPics
        1,                  // numberOfPosPics
        2,                  // temporalId
        1,                  // isRefFrame
        {4, 8, 0, 0, 0, 0}, // deltaNegPos
        {4, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        3,                  // numberOfNegPics
        2,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 4, 8, 0, 0, 0}, // deltaNegPos
        {1, 2, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        3,                  // numberOfNegPics
        1,                  // numberOfPosPics
        3,                  // temporalId
        1,                  // isRefFrame
        {2, 4, 8, 0, 0, 0}, // deltaNegPos
        {2, 0, 0, 0, 0, 0}, // deltaPosPos
        {1, 0, 0, 0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0, 0, 0, 0}, // usedByPosPicFlag
    },
    {
        3,                  // numberOfNegPics
        1,                  // numberOfPosPics
        4,                  // temporalId
        0,                  // isRefFrame
        {1, 2, 12, 0, 0, 0}, // deltaNegPos
        {1, 0, 0,  0, 0, 0}, // deltaPosPos
        {1, 0, 0,  0, 0, 0}, // usedByNegPicFlag
        {1, 0, 0,  0, 0, 0}, // usedByPosPicFlag
    }
};


xin_rps_struct* xin266RpsRandom[XIN_MAX_PRED_HIER_NUM] =
{
    encoderRpsGop1,
    encoderRpsGop2,
    encoderRpsGop4,
    encoderRpsGop8,
    encoderRpsGop16,
};

static UINT32 encoderOrder1[1] =
{
    1
};

static UINT32 encoderOrder2[2] =
{
    2, 1
};

static UINT32 encoderOrder4[4] =
{
    4, 2, 1, 3
};

static UINT32 encoderOrder8[8] =
{
    8, 4, 2, 1, 3, 6, 5, 7
};

static UINT32 encoderOrder16[16] =
{
    16, 8, 4, 2, 1, 3, 6, 5, 7, 12, 10, 9, 11, 14, 13, 15
};

UINT32* xin266encOrder[XIN_MAX_PRED_HIER_NUM] =
{
    encoderOrder1,
    encoderOrder2,
    encoderOrder4,
    encoderOrder8,
    encoderOrder16
};

const SINT32 xinAlfFixedCoeff[XIN_ALF_FIXED_FILTER_NUM][XIN_ALF_MAX_LUMA_COEF_NUM] =
{
    { 0,   0,   2,  -3,   1,  -4,   1,   7,  -1,   1,  -1,   5, 0 },
    { 0,   0,   0,   0,   0,  -1,   0,   1,   0,   0,  -1,   2, 0 },
    { 0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   0, 0 },
    { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  -1,   1, 0 },
    { 2,   2,  -7,  -3,   0,  -5,  13,  22,  12,  -3,  -3,  17,  0 },
    { -1,   0,   6,  -8,   1,  -5,   1,  23,   0,   2,  -5,  10,  0 },
    { 0,   0,  -1,  -1,   0,  -1,   2,   1,   0,   0,  -1,   4, 0 },
    { 0,   0,   3, -11,   1,   0,  -1,  35,   5,   2,  -9,   9,  0 },
    { 0,   0,   8,  -8,  -2,  -7,   4,   4,   2,   1,  -1,  25,  0 },
    { 0,   0,   1,  -1,   0,  -3,   1,   3,  -1,   1,  -1,   3, 0 },
    { 0,   0,   3,  -3,   0,  -6,   5,  -1,   2,   1,  -4,  21,  0 },
    { -7,   1,   5,   4,  -3,   5,  11,  13,  12,  -8,  11,  12,  0 },
    { -5,  -3,   6,  -2,  -3,   8,  14,  15,   2,  -7,  11,  16,  0 },
    { 2,  -1,  -6,  -5,  -2,  -2,  20,  14,  -4,   0,  -3,  25,  0 },
    { 3,   1,  -8,  -4,   0,  -8,  22,   5,  -3,   2, -10,  29,  0 },
    { 2,   1,  -7,  -1,   2, -11,  23,  -5,   0,   2, -10,  29,  0 },
    { -6,  -3,   8,   9,  -4,   8,   9,   7,  14,  -2,   8,   9,  0 },
    { 2,   1,  -4,  -7,   0,  -8,  17,  22,   1,  -1,  -4,  23,  0 },
    { 3,   0,  -5,  -7,   0,  -7,  15,  18,  -5,   0,  -5,  27,  0 },
    { 2,   0,   0,  -7,   1, -10,  13,  13,  -4,   2,  -7,  24,  0 },
    { 3,   3, -13,   4,  -2,  -5,   9,  21,  25,  -2,  -3,  12,  0 },
    { -5,  -2,   7,  -3,  -7,   9,   8,   9,  16,  -2,  15,  12,  0 },
    { 0,  -1,   0,  -7,  -5,   4,  11,  11,   8,  -6,  12,  21,  0 },
    { 3,  -2,  -3,  -8,  -4,  -1,  16,  15,  -2,  -3,   3,  26,  0 },
    { 2,   1,  -5,  -4,  -1,  -8,  16,   4,  -2,   1,  -7,  33,  0 },
    { 2,   1,  -4,  -2,   1, -10,  17,  -2,   0,   2, -11,  33,  0 },
    { 1,  -2,   7, -15, -16,  10,   8,   8,  20,  11,  14,  11,  0 },
    { 2,   2,   3, -13, -13,   4,   8,  12,   2,  -3,  16,  24,  0 },
    { 1,   4,   0,  -7,  -8,  -4,   9,   9,  -2,  -2,   8,  29,  0 },
    { 1,   1,   2,  -4,  -1,  -6,   6,   3,  -1,  -1,  -3,  30,  0 },
    { -7,   3,   2,  10,  -2,   3,   7,  11,  19,  -7,   8,  10, 0 },
    { 0,  -2,  -5,  -3,  -2,   4,  20,  15,  -1,  -3,  -1,  22,  0 },
    { 3,  -1,  -8,  -4,  -1,  -4,  22,   8,  -4,   2,  -8,  28,  0 },
    { 0,   3, -14,   3,   0,   1,  19,  17,   8,  -3,  -7,  20,  0 },
    { 0,   2,  -1,  -8,   3,  -6,   5,  21,   1,   1,  -9,  13,  0 },
    { -4,  -2,   8,  20,  -2,   2,   3,   5,  21,   4,   6,   1, 0 },
    { 2,  -2,  -3,  -9,  -4,   2,  14,  16,   3,  -6,   8,  24,  0 },
    { 2,   1,   5, -16,  -7,   2,   3,  11,  15,  -3,  11,  22,  0 },
    { 1,   2,   3, -11,  -2,  -5,   4,   8,   9,  -3,  -2,  26,  0 },
    { 0,  -1,  10,  -9,  -1,  -8,   2,   3,   4,   0,   0,  29,  0 },
    { 1,   2,   0,  -5,   1,  -9,   9,   3,   0,   1,  -7,  20,  0 },
    { -2,   8,  -6,  -4,   3,  -9,  -8,  45,  14,   2, -13,   7, 0 },
    { 1,  -1,  16, -19,  -8,  -4,  -3,   2,  19,   0,   4,  30,  0 },
    { 1,   1,  -3,   0,   2, -11,  15,  -5,   1,   2,  -9,  24,  0 },
    { 0,   1,  -2,   0,   1,  -4,   4,   0,   0,   1,  -4,   7,  0 },
    { 0,   1,   2,  -5,   1,  -6,   4,  10,  -2,   1,  -4,  10,  0 },
    { 3,   0,  -3,  -6,  -2,  -6,  14,   8,  -1,  -1,  -3,  31,  0 },
    { 0,   1,   0,  -2,   1,  -6,   5,   1,   0,   1,  -5,  13,  0 },
    { 3,   1,   9, -19, -21,   9,   7,   6,  13,   5,  15,  21,  0 },
    { 2,   4,   3, -12, -13,   1,   7,   8,   3,   0,  12,  26,  0 },
    { 3,   1,  -8,  -2,   0,  -6,  18,   2,  -2,   3, -10,  23,  0 },
    { 1,   1,  -4,  -1,   1,  -5,   8,   1,  -1,   2,  -5,  10,  0 },
    { 0,   1,  -1,   0,   0,  -2,   2,   0,   0,   1,  -2,   3,  0 },
    { 1,   1,  -2,  -7,   1,  -7,  14,  18,   0,   0,  -7,  21,  0 },
    { 0,   1,   0,  -2,   0,  -7,   8,   1,  -2,   0,  -3,  24,  0 },
    { 0,   1,   1,  -2,   2, -10,  10,   0,  -2,   1,  -7,  23,  0 },
    { 0,   2,   2, -11,   2,  -4,  -3,  39,   7,   1, -10,   9,  0 },
    { 1,   0,  13, -16,  -5,  -6,  -1,   8,   6,   0,   6,  29,  0 },
    { 1,   3,   1,  -6,  -4,  -7,   9,   6,  -3,  -2,   3,  33,  0 },
    { 4,   0, -17,  -1,  -1,   5,  26,   8,  -2,   3, -15,  30,  0 },
    { 0,   1,  -2,   0,   2,  -8,  12,  -6,   1,   1,  -6,  16,  0 },
    { 0,   0,   0,  -1,   1,  -4,   4,   0,   0,   0,  -3,  11,  0 },
    { 0,   1,   2,  -8,   2,  -6,   5,  15,   0,   2,  -7,   9,  0 },
    { 1,  -1,  12, -15,  -7,  -2,   3,   6,   6,  -1,   7,  30,  0 },
};

const SINT32 classToFilterMapping[XIN_ALF_FIXED_FLT_SET_NUM][XIN_ALF_MAX_CLS_NUM] =
{
    { 8,   2,   2,   2,   3,   4,  53,   9,   9,  52,   4,   4,   5,   9,   2,   8,  10,   9,   1,   3,  39,  39,  10,   9,  52 },
    { 11,  12,  13,  14,  15,  30,  11,  17,  18,  19,  16,  20,  20,   4,  53,  21,  22,  23,  14,  25,  26,  26,  27,  28,  10 },
    { 16,  12,  31,  32,  14,  16,  30,  33,  53,  34,  35,  16,  20,   4,   7,  16,  21,  36,  18,  19,  21,  26,  37,  38,  39 },
    { 35,  11,  13,  14,  43,  35,  16,   4,  34,  62,  35,  35,  30,  56,   7,  35,  21,  38,  24,  40,  16,  21,  48,  57,  39 },
    { 11,  31,  32,  43,  44,  16,   4,  17,  34,  45,  30,  20,  20,   7,   5,  21,  22,  46,  40,  47,  26,  48,  63,  58,  10 },
    { 12,  13,  50,  51,  52,  11,  17,  53,  45,   9,  30,   4,  53,  19,   0,  22,  23,  25,  43,  44,  37,  27,  28,  10,  55 },
    { 30,  33,  62,  51,  44,  20,  41,  56,  34,  45,  20,  41,  41,  56,   5,  30,  56,  38,  40,  47,  11,  37,  42,  57,   8 },
    { 35,  11,  23,  32,  14,  35,  20,   4,  17,  18,  21,  20,  20,  20,   4,  16,  21,  36,  46,  25,  41,  26,  48,  49,  58 },
    { 12,  31,  59,  59,   3,  33,  33,  59,  59,  52,   4,  33,  17,  59,  55,  22,  36,  59,  59,  60,  22,  36,  59,  25,  55 },
    { 31,  25,  15,  60,  60,  22,  17,  19,  55,  55,  20,  20,  53,  19,  55,  22,  46,  25,  43,  60,  37,  28,  10,  55,  52 },
    { 12,  31,  32,  50,  51,  11,  33,  53,  19,  45,  16,   4,   4,  53,   5,  22,  36,  18,  25,  43,  26,  27,  27,  28,  10 },
    { 5,   2,  44,  52,   3,   4,  53,  45,   9,   3,   4,  56,   5,   0,   2,   5,  10,  47,  52,   3,  63,  39,  10,   9,  52 },
    { 12,  34,  44,  44,   3,  56,  56,  62,  45,   9,  56,  56,   7,   5,   0,  22,  38,  40,  47,  52,  48,  57,  39,  10,   9 },
    { 35,  11,  23,  14,  51,  35,  20,  41,  56,  62,  16,  20,  41,  56,   7,  16,  21,  38,  24,  40,  26,  26,  42,  57,  39 },
    { 33,  34,  51,  51,  52,  41,  41,  34,  62,   0,  41,  41,  56,   7,   5,  56,  38,  38,  40,  44,  37,  42,  57,  39,  10 },
    { 16,  31,  32,  15,  60,  30,   4,  17,  19,  25,  22,  20,   4,  53,  19,  21,  22,  46,  25,  55,  26,  48,  63,  58,  55 },
};

static const UINT32 chromaWeightFactor[] =
{
    25, 32, 40, 51, 64, 81, 102, 128, 161, 203, 256, 323, 406, 512, 645, 813, 1024, 1290, 1625, 2048, 2580,
};

static SINT32 Xin266VerifyConfig (
    xin26x_params *config)
{
    UINT32 predGopSize;

#ifdef EVAL_VERSION

    if ((config->rcMode) && (!config->zeroLatency))
    {
        config->rcMode = XIN_RC_ABR;
    }

#endif

    if (config->zeroLatency)
    {
        config->bFrameNum   = 0;
        config->lookAhead   = 0;
        config->enableMctf  = FALSE;
        config->unitTree    = FALSE;
        config->rcMode      = XIN_MIN (config->rcMode, XIN_RC_CBR);
    }

    if (config->screenContentMode)
    {
        // h266 & h265
        config->transformSkipFlag = TRUE;
        config->motionSearchMode  = 2;
        config->enableMctf        = FALSE;

        // h266
        config->maxTrSkipSize     = config->maxTrSkipSize > 8 ? config->maxTrSkipSize : 8;
        config->minCuSize         = 4;
        config->minQtSize         = 4;
        config->enableCclm        = TRUE;
    }

    if (((config->inputWidth & 0x01) != 0)
            || ((config->inputHeight & 0x01) != 0)
            || (config->inputWidth <= XIN_MAX_CTU_SIZE)
            || (config->inputHeight <= XIN_MAX_CTU_SIZE)
            || (config->inputWidth > XIN_MAX_FRAME_WIDTH)
            || (config->inputHeight > XIN_MAX_FRAME_HEIGHT))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Input picture height or width is not correct.\n");

        return XIN_FAIL;
    }

    if ((config->ctuSize > XIN_MAX_CTU_SIZE) || (config->ctuSize < XIN_MIN_CTU_SIZE))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Coding tree unit size is not correct.\n");

        return XIN_FAIL;
    }

    if (config->minQtSize < XIN_MIN_CU_SIZE)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "minQtSize can not be lower than 4.\n");

        return XIN_FAIL;
    }

    if (config->minQtSize > config->ctuSize)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "minQtSize can not be great than ctuSize.\n");

        return XIN_FAIL;
    }

    if ((config->maxBtSize > config->ctuSize) || (config->maxBtSize < config->minQtSize))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "maxBtSize should be between ctusize and minQtSize.\n");

        return XIN_FAIL;
    }

    if ((config->maxTtSize > config->ctuSize) || (config->maxTtSize < config->minQtSize))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "maxTtSize should be between ctusize and minQtSize.\n");

        return XIN_FAIL;
    }

    if ((config->temporalLayerNum > XIN_MAX_TEMPORAL_LAYER) || (config->temporalLayerNum < XIN_MIN_TEMPORAL_LAYER))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Temporal layer num is not correct.\n");

        return XIN_FAIL;
    }

    if ((config->temporalLayerNum > 1) && (config->refFrameNum > 1))
    {
        _XIN_LOGGER (XIN_LOGGER_WARNING, "Only support one frame reference, when temporal scalability is on.\n");
    }

    if (config->qp > XIN_MAX_QP)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Qp parameter is out of range.\n");

        return XIN_FAIL;
    }

    if ((config->numTileCols < XIN_MIN_WIDTH_IN_TILE) || (config->numTileRows < XIN_MIN_HEIGHT_IN_TILE))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Picture at least contains one tile.\n");

        return XIN_FAIL;
    }

    if (((config->numTileCols > XIN_MAX_WIDTH_IN_TILE) || (config->numTileRows > XIN_MAX_HEIGHT_IN_TILE))
            && (config->enableWpp != TRUE))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "The max tile horizontal or vertical num is 2.\n");

        return XIN_FAIL;
    }

    if ((config->refFrameNum < XIN_MIN_REF_FRAMES) || (config->refFrameNum > XIN_MAX_REF_FRAMES))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Reference frame number is out of range.\n");

        return XIN_FAIL;
    }

    if ((config->bFrameNum != 1) && (config->bFrameNum != 3) && (config->bFrameNum != 7) && (config->bFrameNum != 15) && (config->bFrameNum != 0))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "B frame num is not correct.\n");

        return XIN_FAIL;
    }

    if (config->maxTtSize > 64)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "maxTtSize can not be bigger than 64.\n");

        return XIN_FAIL;
    }

    if (config->bFrameNum > 0)
    {
        predGopSize = config->bFrameNum + 1;
    }
    else
    {
        predGopSize = 1 << (config->temporalLayerNum - 1);
    }

    if ((config->intraPeriod != 0) && (config->intraPeriod < predGopSize))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Intra period should be at least a prediction gop size.\n");

        return XIN_FAIL;
    }

    if (config->threadNum > XIN_MAX_THREAD_POOL_NUM)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Thread num is exceed the max thread pool num.\n");

        return XIN_FAIL;
    }

    if ((config->maxTrSkipSize != 16) && (config->maxTrSkipSize != 8) && (config->maxTrSkipSize != 4) && (config->transformSkipFlag))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Max tranform skip size has to be 4, 8 or 16.\n");

        return XIN_FAIL;
    }

    if ((config->frameSkip) && (config->bFrameNum))
    {
        _XIN_LOGGER (XIN_LOGGER_WARNING, "Frame skip is not supported, when B frame is on.\n");
    }

    if ((config->lumaTrSize64 == FALSE) && (config->ctuSize > 64))
    {
        _XIN_LOGGER (XIN_LOGGER_WARNING, "Max luma transform size 32 works under ctu size less than 128.\n");
    }

    if (config->maxMttDepth > 1)
    {
        _XIN_LOGGER (XIN_LOGGER_WARNING, "maxMttDepth can not be great than 1.\n");
    }

    return XIN_SUCCESS;

}

static SINT32 Xin266ConstructInputPicBuf (
    xin_seq_struct *seqSet)
{
    xin_input_picture *inputPicture;
    UINT32            inputFrameNum;
    UINT32            lumaStride;
    UINT32            chromaStride;
    UINT32            lumaSize;
    UINT32            chromaSize;
    UINT32            frameIdx;
    UINT32            frameWidth;
    UINT32            frameHeight;
    UINT32            laWdtInBlock;
    UINT32            laHgtInBlock;
    UINT32            laUnitSize;
    UINT32            inputMarginX;
    UINT32            inputMarginY;
    UINT32            lowerMarginX;
    UINT32            lowerMarginY;
    UINT32            lowerSize;
    UINT32            lowerWidth;
    UINT32            lowerHeight;

    if (seqSet->config.twoPassEncoder)
    {
        inputFrameNum = seqSet->config.frameToBeEncoded;
    }
    else if (seqSet->config.lookAhead)
    {
        inputFrameNum  = seqSet->config.lookAhead + seqSet->config.bFrameNum + seqSet->config.frameThreadNum + 3;
        inputFrameNum += seqSet->config.enableMctf ? seqSet->config.mctfRefNum : 0;
    }
    else if (seqSet->config.zeroLatency)
    {
        inputFrameNum = 1;
    }
    else
    {
        inputFrameNum = (seqSet->config.bFrameNum) ?  (seqSet->config.bFrameNum + 1) : 1;
        inputFrameNum = (seqSet->config.frameThreadNum == 1) ? inputFrameNum : (inputFrameNum + seqSet->config.frameThreadNum - 1);
    }

    inputMarginX   = XIN_INPUT_PADDING_X + 16;
    inputMarginY   = XIN_INPUT_PADDING_X + 16;
    lumaStride     = seqSet->config.inputWidth + 2*inputMarginX;
    chromaStride   = lumaStride >> 1;
    lumaSize       = lumaStride*(seqSet->config.inputHeight + 2*inputMarginY);
    chromaSize     = chromaStride*((seqSet->config.inputHeight + 2*inputMarginY) >> 1);
    laUnitSize     = seqSet->laUnitSize;

    XIN_MALLOC_CHECK(seqSet->inputFrame, sizeof(xin_input_picture)*inputFrameNum);

    seqSet->inputFrameNum = inputFrameNum;

    if (inputFrameNum <= 1)
    {
        return XIN_SUCCESS;
    }

    lowerMarginX = XIN_LOWER_PADDING_X + 4;
    lowerMarginY = XIN_LOWER_PADDING_Y + 4;
    frameWidth   = seqSet->config.inputWidth;
    frameHeight  = seqSet->config.inputHeight;
    laWdtInBlock = seqSet->laWdtInUnit;
    laHgtInBlock = seqSet->laHgtInUnit;
    lowerWidth   = laUnitSize*seqSet->laWdtInUnit;
    lowerHeight  = laUnitSize*seqSet->laHgtInUnit;
    lowerSize    = (lowerWidth + 2 * lowerMarginX)*(lowerHeight + 2 * lowerMarginY);

    seqSet->lowerWidth  = lowerWidth;
    seqSet->lowerHeight = lowerHeight;

    for (frameIdx = 0; frameIdx < inputFrameNum; frameIdx++)
    {
        inputPicture = seqSet->inputFrame + frameIdx;

        XIN_MALLOC_CHECK (inputPicture->inputBuffer, (lumaSize + 2*chromaSize)*sizeof(PIXEL));

        inputPicture->inputBuf[PLANE_LUMA]     = inputPicture->inputBuffer + inputMarginY*lumaStride + inputMarginX;
        inputPicture->inputBuf[PLANE_CHROMA_U] = inputPicture->inputBuffer + lumaSize + ((inputMarginY*chromaStride + inputMarginX) >> 1);
        inputPicture->inputBuf[PLANE_CHROMA_V] = inputPicture->inputBuffer + lumaSize + chromaSize + ((inputMarginY*chromaStride + inputMarginX) >> 1);

        inputPicture->inputStride[0] = lumaStride;
        inputPicture->inputStride[1] = chromaStride;
        inputPicture->inputMarginX   = inputMarginX;
        inputPicture->inputMarginY   = inputMarginY;
        inputPicture->inputWidth     = frameWidth;
        inputPicture->inputHeight    = frameHeight;

        if (seqSet->config.lookAhead)
        {
            XIN_MALLOC_CHECK (inputPicture->lowerBuffer, lowerSize*sizeof(PIXEL));

            XIN_MALLOC_CHECK (inputPicture->intraCost, laWdtInBlock*laHgtInBlock*sizeof(UINT32));
            XIN_MALLOC_CHECK (inputPicture->interCost, laWdtInBlock*laHgtInBlock*sizeof(UINT32));
            XIN_MALLOC_CHECK (inputPicture->laMv[0],   laWdtInBlock*laHgtInBlock*sizeof(xin_mv_u));
            XIN_MALLOC_CHECK (inputPicture->laMv[1],   laWdtInBlock*laHgtInBlock*sizeof(xin_mv_u));
            XIN_MALLOC_CHECK (inputPicture->interDir,  laWdtInBlock*laHgtInBlock*sizeof(UINT8));

            memset (inputPicture->laMv[0], 0, laWdtInBlock*laHgtInBlock*sizeof(xin_mv_u));
            memset (inputPicture->laMv[1], 0, laWdtInBlock*laHgtInBlock*sizeof(xin_mv_u));

            XIN_MALLOC_CHECK (inputPicture->propCost, laWdtInBlock*laHgtInBlock*sizeof(UINT16));
            XIN_MALLOC_CHECK (inputPicture->qpOffset, laWdtInBlock*laHgtInBlock*sizeof(FLOAT64));

            inputPicture->lowerStride  = lowerWidth + 2 * lowerMarginX;
            inputPicture->lowerBuf     = inputPicture->lowerBuffer + lowerMarginY*inputPicture->lowerStride + lowerMarginX;
            inputPicture->laWdtInUnit  = laWdtInBlock;
            inputPicture->laHgtInUnit  = laHgtInBlock;
            inputPicture->laTotalUnit  = laWdtInBlock*laHgtInBlock;
            inputPicture->lowerMarginX = lowerMarginX;
            inputPicture->lowerMarginY = lowerMarginY;
            inputPicture->lowerWidth   = lowerWidth;
            inputPicture->lowerHeight  = lowerHeight;

        }

        XIN_MALLOC_CHECK (inputPicture->dqpMap, seqSet->frameSizeInCtu*sizeof(double));
        inputPicture->dqpMapStride = seqSet->frameWidthInCtu;

        inputPicture->bufStage = XIN_BUF_INVALID;

    }

    memset (seqSet->inputList, 0, XIN_MAX_FRAME_NUM*sizeof(xin_input_picture *));

    for (frameIdx = 0; frameIdx < inputFrameNum; frameIdx++)
    {
        seqSet->inputList[frameIdx] = seqSet->inputFrame + frameIdx;
    }

    seqSet->inputListNum = seqSet->inputFrameNum;

    return XIN_SUCCESS;

}


static void Xin266DestructInputPicBuf (
    xin_seq_struct *seqSet)
{
    xin_input_picture *inputPicture;
    UINT32            frameIdx;

    if (seqSet->inputFrameNum > 1)
    {
        for (frameIdx = 0; frameIdx < seqSet->inputFrameNum; frameIdx++)
        {
            inputPicture = seqSet->inputFrame + frameIdx;

            free (inputPicture->inputBuffer);

            if (seqSet->config.lookAhead)
            {

                free (inputPicture->lowerBuffer);
                free (inputPicture->intraCost);
                free (inputPicture->interCost);
                free (inputPicture->laMv[0]);
                free (inputPicture->laMv[1]);
                free (inputPicture->interDir);
                free (inputPicture->propCost);
                free (inputPicture->qpOffset);

            }

        }

    }

    free (seqSet->inputFrame);

}


SINT32 Xin266ConstructRefPicBuf (
    xin_seq_struct *seqSet)
{
    xin_ref_picture *refPicture;
    UINT32          refFrameNum;
    UINT32          frameIdx;
    UINT32          widthInBlock;
    UINT32          heightInBlock;
    UINT32          blockSetWidth;
    UINT32          blockSetHeight;
    UINT32          padWidth;
    UINT32          padHeight;
    UINT32          lumaStride;
    UINT32          luma1Stride;
    UINT32          luma2Stride;
    UINT32          chromaStride;
    UINT32          lumaSize;
    UINT32          luma1Size;
    UINT32          luma2Size;
    UINT32          chromaSize;

    refFrameNum = (seqSet->config.temporalLayerNum > 1) ? (seqSet->config.temporalLayerNum + 1) : (seqSet->config.refFrameNum + 1);
    refFrameNum = (seqSet->config.bFrameNum) ? (seqSet->config.bFrameNum + seqSet->config.refFrameNum + 1) : refFrameNum + 1;
    refFrameNum = (seqSet->config.frameThreadNum == 1) ? refFrameNum : (refFrameNum + seqSet->config.frameThreadNum);

    XIN_MALLOC_CHECK(seqSet->refFrame, sizeof(xin_ref_picture)*refFrameNum);

    seqSet->refFrameNum = refFrameNum;

    padWidth  = seqSet->ctuSize + XIN_PADDING_OFFSET_X;
    padHeight = seqSet->ctuSize + XIN_PADDING_OFFSET_Y;

    lumaStride   = seqSet->frameWidth + 2*padWidth;
    luma1Stride  = (lumaStride + 1) >> 1;
    luma2Stride  = (lumaStride + 2) >> 2;
    chromaStride = (seqSet->frameWidth + 2*padWidth) >> 1;

    lumaSize   = lumaStride*(seqSet->frameHeight + 2*padHeight);
    luma1Size  = luma1Stride*((seqSet->frameHeight + 2*padHeight + 1) >> 1);
    luma2Size  = luma2Stride*((seqSet->frameHeight + 2*padHeight + 2) >> 2);
    chromaSize = chromaStride*((seqSet->frameHeight + 2*padHeight) >> 1);

    widthInBlock  = seqSet->frameWidthInBlock;
    heightInBlock = seqSet->frameHeightInBlock;

    blockSetWidth  = seqSet->blockSetWidth;
    blockSetHeight = seqSet->blockSetHeight;

    for (frameIdx = 0; frameIdx < refFrameNum; frameIdx++)
    {
        refPicture = seqSet->refFrame + frameIdx;

        XIN_MALLOC_CHECK (refPicture->refBuffer,   (lumaSize + 2*chromaSize)*sizeof(PIXEL));
        XIN_MALLOC_CHECK (refPicture->blockSetMap, blockSetWidth * blockSetHeight * sizeof(xin_block_struct));

        Xin26xJobCreate (
            &refPicture->jobProFrame,
            1,
            XIN_MAX_DEP_JOB_NUM);

        Xin26xJobCreate (
            &refPicture->jobPreFrame,
            1,
            XIN_MAX_DEP_JOB_NUM);

        refPicture->refBuf[PLANE_LUMA]     = refPicture->refBuffer + padHeight*lumaStride + padWidth;
        refPicture->refBuf[PLANE_CHROMA_U] = refPicture->refBuffer + lumaSize + ((padHeight*chromaStride + padWidth) >> 1);
        refPicture->refBuf[PLANE_CHROMA_V] = refPicture->refBuffer + lumaSize + chromaSize + ((padHeight*chromaStride + padWidth) >> 1);

        if (seqSet->config.motionSearchMode == XIN_ME_HIER_SEARCH)
        {
            XIN_MALLOC_CHECK (refPicture->ref1Buffer, luma1Size*sizeof(PIXEL));
            XIN_MALLOC_CHECK (refPicture->ref2Buffer, luma2Size*sizeof(PIXEL));

            refPicture->ref1Stride = luma1Stride;
            refPicture->ref2Stride = luma2Stride;
            refPicture->ref1Buf    = refPicture->ref1Buffer + padHeight*luma1Stride/2 + padWidth/2;
            refPicture->ref2Buf    = refPicture->ref2Buffer + padHeight*luma2Stride/4 + padWidth/4;
        }

        refPicture->refStride[0]   = lumaStride;
        refPicture->refStride[1]   = chromaStride;
        refPicture->blockSetSize   = blockSetHeight*blockSetWidth;
        refPicture->blockSetWidth  = blockSetWidth;
        refPicture->blockSetHeight = blockSetHeight;

        refPicture->lumaWidth  = seqSet->frameWidth;
        refPicture->lumaHeight = seqSet->frameHeight;

        refPicture->inputWidth  = seqSet->config.inputWidth;
        refPicture->inputHeight = seqSet->config.inputHeight;

        refPicture->paddingWidth  = padWidth;
        refPicture->paddingHeight = padHeight;

        refPicture->widthInPel4 = seqSet->frameWidth >> 2;
        refPicture->widthInPel8 = seqSet->frameWidth >> 3;

        refPicture->heightInPel4 = seqSet->frameHeight >> 2;
        refPicture->heightInPel8 = seqSet->frameHeight >> 3;

        refPicture->blockSize    = seqSet->blockSize;
        refPicture->lgBlockSize  = seqSet->lgBlockSize;

        refPicture->widthInBlock  = (seqSet->lgBlockSize == XIN_LOG_BLOCK_SIZE) ? refPicture->widthInPel4 : refPicture->widthInPel8;
        refPicture->heightInBlock = (seqSet->lgBlockSize == XIN_LOG_BLOCK_SIZE) ? refPicture->heightInPel4 : refPicture->heightInPel8;

        XIN_MALLOC_CHECK (refPicture->cbfMap,   widthInBlock * heightInBlock);
        XIN_MALLOC_CHECK (refPicture->rps,      sizeof(xin_rps_struct));
        XIN_MALLOC_CHECK (refPicture->horBs,    heightInBlock * widthInBlock);
        XIN_MALLOC_CHECK (refPicture->verBs,    widthInBlock * heightInBlock);
        XIN_MALLOC_CHECK (refPicture->qpMap,    widthInBlock * heightInBlock);
        XIN_MALLOC_CHECK (refPicture->qpUvMap,  widthInBlock * heightInBlock);
        XIN_MALLOC_CHECK (refPicture->drMap,    seqSet->frameWidthInCtu*seqSet->frameHeightInCtu*sizeof(xin_depth_range));
        XIN_MALLOC_CHECK (refPicture->qpOffset, seqSet->frameWidthInCtu*seqSet->frameHeightInCtu*sizeof(FLOAT64));
        XIN_MALLOC_CHECK (refPicture->qpNum,    seqSet->frameWidthInCtu*seqSet->frameHeightInCtu*sizeof(UINT8));

        refPicture->colFromL0Flag = TRUE;

    }

    memset (seqSet->refList, 0, XIN_MAX_FRAME_NUM*sizeof(xin_ref_picture *));

    for (frameIdx = 0; frameIdx < refFrameNum; frameIdx++)
    {
        seqSet->refList[frameIdx] = seqSet->refFrame + frameIdx;
    }

    seqSet->refListNum = seqSet->refFrameNum;
    seqSet->dpbSize    = 0;

    return XIN_SUCCESS;

}

void Xin266DestructRefPicBuf (
    xin_seq_struct *seqSet)
{
    xin_ref_picture *refPicture;
    UINT32          frameIdx;

    for (frameIdx = 0; frameIdx < seqSet->refFrameNum; frameIdx++)
    {
        refPicture = seqSet->refFrame + frameIdx;

        free (refPicture->refBuffer);
        free (refPicture->blockSetMap);

        Xin26xJobDelete (
            refPicture->jobPreFrame,
            1);

        Xin26xJobDelete (
            refPicture->jobProFrame,
            1);

        if (seqSet->config.motionSearchMode == XIN_ME_HIER_SEARCH)
        {
            free (refPicture->ref1Buffer);
            free (refPicture->ref2Buffer);
        }

        free (refPicture->rps);
        free (refPicture->cbfMap);
        free (refPicture->horBs);
        free (refPicture->verBs);
        free (refPicture->qpMap);
        free (refPicture->qpUvMap);
        free (refPicture->drMap);
        free (refPicture->qpOffset);
        free (refPicture->qpNum);

    }

    free (seqSet->refFrame);

}

static void Xin266InitCtuTsRsAddrMaps (
    xin_seq_struct  *seqSet,
    xin266_tile_dim *tileDim,
    UINT32          *ctuTsToRsAddrMap)
{
    UINT32  colIdx;
    UINT32  rowIdx;
    UINT32  firstRsCtu;
    UINT32  frameWidthInCtu;
    UINT32  tileWidthInCtu;
    UINT32  tileHeightInCtu;

    firstRsCtu      = tileDim->firstRsCtu;
    frameWidthInCtu = seqSet->frameWidthInCtu;
    tileWidthInCtu  = tileDim->tileWidthInCtu;
    tileHeightInCtu = tileDim->tileHeightInCtu;

    for (rowIdx = 0; rowIdx < tileHeightInCtu; rowIdx++)
    {
        for (colIdx = 0; colIdx < tileWidthInCtu; colIdx++)
        {
            ctuTsToRsAddrMap[colIdx + rowIdx * tileWidthInCtu] = firstRsCtu + rowIdx * frameWidthInCtu + colIdx;
        }
    }

}

static void Xin266ContructTileCtu (
    xin_pic_struct  *picSet,
    xin266_tile_dim *tile)
{
    UINT32         colIdx;
    UINT32         rowIdx;
    UINT32         ctuIndex;
    UINT32         tileCol;
    UINT32         tileRow;
    UINT32         ctuAddr;
    UINT32         firstTsAddr;
    xin_seq_struct *seqSet;
    xin_ctu_struct *ctu;
    UINT32         widthInCtu;
    UINT32         heightInCtu;
    UINT32         availField;

    seqSet      = picSet->seqSet;
    firstTsAddr = tile->firstTsCtu;
    widthInCtu  = tile->tileWidthInCtu;
    heightInCtu = tile->tileHeightInCtu;
    tileCol     = tile->tileCtuX;
    tileRow     = tile->tileCtuY;

    if (seqSet->config.enableWpp || (!seqSet->config.enableTiles))
    {
        for (rowIdx = 0; rowIdx < heightInCtu; rowIdx++)
        {
            for (colIdx = 0; colIdx < widthInCtu; colIdx++)
            {
                ctuIndex = rowIdx*widthInCtu + colIdx;
                ctuAddr  = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIndex];
                ctu      = picSet->ctu + ctuAddr;

                ctu->ctuX = colIdx + tileCol;
                ctu->ctuY = rowIdx + tileRow;

                availField  = 0;
                availField |= (ctu->ctuX != 0) ? XIN_LFT_CTU_AVAIL : 0;
                availField |= (ctu->ctuY != 0) ? XIN_TOP_CTU_AVAIL : 0;
                availField |= ((ctu->ctuX + 1) != seqSet->frameWidthInCtu) ? XIN_RGT_CTU_AVAIL : 0;
                availField |= ((ctu->ctuY + 1) != seqSet->frameHeightInCtu) ? XIN_BOT_CTU_AVAIL : 0;

                ctu->availField = availField;
                ctu->ctuIndex   = seqSet->config.enableWpp ? ctuIndex : ctuAddr;
                ctu->ctuAddr    = ctuAddr;
                ctu->picSet     = picSet;

                ctu->ctuPelX = ctu->ctuX*seqSet->ctuSize;
                ctu->ctuPelY = ctu->ctuY*seqSet->ctuSize;

                ctu->width  = ((ctu->ctuX + 1) == seqSet->frameWidthInCtu) ? (seqSet->frameWidth - ctu->ctuPelX) : (seqSet->ctuSize);
                ctu->height = ((ctu->ctuY + 1) == seqSet->frameHeightInCtu) ? (seqSet->frameHeight - ctu->ctuPelY) : (seqSet->ctuSize);

                ctu->pixelNum   = ctu->width*ctu->height;
                ctu->lgWidth    = (ctu->width == seqSet->ctuSize) ? calcLog2[ctu->width] : 0;
                ctu->sliceIndex = tile->tileIndex;

            }
        }

    }
    else
    {
        for (rowIdx = 0; rowIdx < heightInCtu; rowIdx++)
        {
            for (colIdx = 0; colIdx < widthInCtu; colIdx++)
            {
                ctuIndex = rowIdx*widthInCtu + colIdx;
                ctuAddr  = seqSet->ctuTsToRsAddrMap[firstTsAddr + ctuIndex];
                ctu      = picSet->ctu + ctuAddr;

                availField  = 0;
                availField |= (colIdx) ? XIN_LFT_CTU_AVAIL : 0;
                availField |= (rowIdx) ? XIN_TOP_CTU_AVAIL : 0;
                availField |= ((colIdx + 1) != widthInCtu) ? XIN_RGT_CTU_AVAIL : 0;
                availField |= ((rowIdx + 1) != heightInCtu) ? XIN_BOT_CTU_AVAIL : 0;

                ctu->availField = availField;
                ctu->ctuIndex   = ctuIndex;
                ctu->ctuAddr    = ctuAddr;
                ctu->picSet     = picSet;

                ctu->ctuX = colIdx + tileCol;
                ctu->ctuY = rowIdx + tileRow;

                ctu->ctuPelX = ctu->ctuX*seqSet->ctuSize;
                ctu->ctuPelY = ctu->ctuY*seqSet->ctuSize;

                ctu->width  = ((ctu->ctuX + 1) == seqSet->frameWidthInCtu) ? (seqSet->frameWidth - ctu->ctuPelX) : (seqSet->ctuSize);
                ctu->height = ((ctu->ctuY + 1) == seqSet->frameHeightInCtu) ? (seqSet->frameHeight - ctu->ctuPelY) : (seqSet->ctuSize);

                ctu->lgWidth    = (ctu->width == seqSet->ctuSize) ? calcLog2[ctu->width] : 0;
                ctu->sliceIndex = tile->tileIndex;

            }
        }
    }

}

static void Xin266ConstructTileDim (
    xin_seq_struct *seqSet)
{
    UINT32          tileIdx;
    UINT32          colIdx;
    UINT32          rowIdx;
    UINT32          tileWidthInCtu;
    UINT32          tileHeightInCtu;
    UINT32          frameWidthInCtu;
    UINT32          frameHeightInCtu;
    UINT32          tileWidth;
    UINT32          tileHeight;
    UINT32          frameWidth;
    UINT32          frameHeight;
    xin266_tile_dim *tileDim;
    xin266_tile_dim *frameDim;
    UINT32          *ctuTsToRsAddrMap;
    UINT32          firstTsCtu;

    frameWidthInCtu  = seqSet->frameWidthInCtu;
    frameHeightInCtu = seqSet->frameHeightInCtu;
    tileWidthInCtu   = frameWidthInCtu / seqSet->config.numTileCols;
    tileHeightInCtu  = frameHeightInCtu / seqSet->config.numTileRows;
    ctuTsToRsAddrMap = seqSet->ctuTsToRsAddrMap;

    frameWidth  = seqSet->frameWidth;
    frameHeight = seqSet->frameHeight;
    tileWidth   = tileWidthInCtu * seqSet->ctuSize;
    tileHeight  = tileHeightInCtu * seqSet->ctuSize;
    tileIdx     = 0;
    firstTsCtu  = 0;
    frameDim    = &seqSet->frameDim;

    for (rowIdx = 0; rowIdx < seqSet->config.numTileRows; rowIdx++)
    {
        for (colIdx = 0; colIdx < seqSet->config.numTileCols; colIdx++)
        {
            tileDim = seqSet->tileDim + tileIdx;

            tileDim->firstRsCtu = rowIdx * tileHeightInCtu * frameWidthInCtu + colIdx * tileWidthInCtu;
            tileDim->firstTsCtu = firstTsCtu;
            tileDim->tileIndex  = tileIdx;
            tileDim->tilePelX   = colIdx * tileWidthInCtu * seqSet->ctuSize;
            tileDim->tilePelY   = rowIdx * tileHeightInCtu * seqSet->ctuSize;
            tileDim->tileCtuX   = colIdx * tileWidthInCtu;
            tileDim->tileCtuY   = rowIdx * tileHeightInCtu;

            if ((colIdx + 1) != seqSet->config.numTileCols)
            {
                tileDim->tileWidthInCtu = tileWidthInCtu;
                tileDim->tileWidth      = tileWidth;
            }
            else
            {
                tileDim->tileWidthInCtu = frameWidthInCtu - colIdx * tileWidthInCtu;
                tileDim->tileWidth      = frameWidth - colIdx * tileWidth;
            }

            if ((rowIdx + 1) != seqSet->config.numTileRows)
            {
                tileDim->tileHeightInCtu = tileHeightInCtu;
                tileDim->tileHeight      = tileHeight;
            }
            else
            {
                tileDim->tileHeightInCtu = frameHeightInCtu - rowIdx * tileHeightInCtu;
                tileDim->tileHeight      = frameHeight - rowIdx * tileHeight;
            }

            tileDim->ctuNumInTile = tileDim->tileHeightInCtu * tileDim->tileWidthInCtu;

            Xin266InitCtuTsRsAddrMaps (
                seqSet,
                tileDim,
                ctuTsToRsAddrMap);

            firstTsCtu       += tileDim->ctuNumInTile;
            ctuTsToRsAddrMap += tileDim->ctuNumInTile;
            tileIdx++;

        }
    }

    memset (frameDim, 0, sizeof(xin266_tile_dim));

    frameDim->tileWidthInCtu  = frameWidthInCtu;
    frameDim->tileHeightInCtu = frameHeightInCtu;
    frameDim->tileWidth       = frameWidth;
    frameDim->tileHeight      = frameHeight;
    frameDim->ctuNumInTile    = seqSet->frameSizeInCtu;

}

static SINT32 Xin266ConstructCabacContext (
    xin_seq_struct *seqSet)
{
    UINT32          sliceIdx;
    UINT32          qpIdx;
    xin_prob_model  *cabacContext;

    XIN_MALLOC_CHECK (seqSet->cabacContext, (XIN_FRAME_TYPE - 1)*XIN_QP_NUM*XIN_NUM_OF_CTX*sizeof(xin_prob_model));

    for (sliceIdx = 0; sliceIdx < (XIN_FRAME_TYPE - 1); sliceIdx++)
    {
        for (qpIdx = XIN_MIN_QP; qpIdx <= XIN_MAX_QP; qpIdx++)
        {
            cabacContext = seqSet->cabacContext + sliceIdx*XIN_QP_NUM*XIN_NUM_OF_CTX + qpIdx*XIN_NUM_OF_CTX;

            Xin266InitCabacContext (
                cabacContext,
                sliceIdx,
                qpIdx);

        }
    }

    return XIN_SUCCESS;

}

void Xin266ConstructRps (
    xin_seq_struct *seqSet)
{
    xin_rps_struct *rpsSet;
    xin_rps_struct *rpsInit;
    SINT32         tLNum;
    SINT32         refNum;
    SINT32         refNum0;
    SINT32         bFrameNum;
    SINT32         gopSize;
    UINT32         hierSize;
    UINT32         rpsSize;
    UINT32         rpsIdx;
    UINT32         rpl0Idx;
    UINT32         rpl1Idx;
    SINT32         refIdx;
    SINT32         refNumNeg;
    SINT32         refNumPos;
    SINT32         refNumExt;
    SINT32         valRefNum;

    tLNum     = seqSet->config.temporalLayerNum;
    bFrameNum = seqSet->config.bFrameNum;
    rpsSet    = seqSet->rpsSet;
    refNum    = seqSet->config.refFrameNum;
    refNum0   = bFrameNum ? XIN_REF_PRED0_NUM : refNum;
    refNum0   = XIN_MAX (refNum, refNum0);

    if (bFrameNum)
    {
        gopSize  = bFrameNum + 1;
        hierSize = calcLog2[gopSize];
        rpsInit  = xin266RpsRandom[hierSize];
        rpsSize  = gopSize;
    }
    else
    {
        gopSize = 1 << (tLNum - 1);
        rpsInit = xin266RpsLowDelay[tLNum];
        rpsSize = gopSize;
    }

    memcpy (rpsSet, rpsInit, sizeof(xin_rps_struct)*rpsSize);

    seqSet->rpsSize     = rpsSize;
    seqSet->predGopSize = gopSize;
    seqSet->initGopSize = gopSize;

    for (rpsIdx = 0; rpsIdx < rpsSize; rpsIdx++)
    {
        if (rpsIdx == 0)
        {
            refNumNeg = rpsSet[rpsIdx].numOfNegPics;
            refNumExt = refNum0 - refNumNeg;

            for (refIdx = 0; refIdx < refNumExt; refIdx++)
            {
                rpsSet[rpsIdx].deltaNegPos[refIdx + refNumNeg]      = gopSize;
                rpsSet[rpsIdx].usedByNegPicFlag[refIdx + refNumNeg] = TRUE;
            }

            rpsSet[rpsIdx].numOfNegPics = refNumNeg + refNumExt;

        }
        else
        {
            // List 0
            refNumNeg = XIN_MIN ((SINT32)rpsSet[rpsIdx].numOfNegPics, refNum);
            refNumPos = bFrameNum ? XIN_MIN ((SINT32)rpsSet[rpsIdx].numOfPosPics, refNum) : 0;

            for (refIdx = 0; refIdx < refNumNeg; refIdx++)
            {
                rpsSet[rpsIdx].usedByNegPicFlag[refIdx] = TRUE;
            }

            refNumNeg = rpsSet[rpsIdx].numOfNegPics;
            refNumExt = refNum - refNumNeg;
            refNumExt = XIN_MAX (refNumExt, XIN_MAX(refNum0 - 2, 0));
            valRefNum = XIN_MIN (refNumNeg + refNumExt, XIN_MAX_DPB_FRAMES - refNumPos);
            valRefNum = XIN_MIN (valRefNum, refNum);

            for (refIdx = 0; refIdx < refNumExt; refIdx++)
            {
                rpsSet[rpsIdx].deltaNegPos[refIdx + refNumNeg]      = gopSize;
                rpsSet[rpsIdx].usedByNegPicFlag[refIdx + refNumNeg] = (refNumNeg + refIdx) < valRefNum;
            }

            rpsSet[rpsIdx].numOfNegPics = refNumNeg + refNumExt;

            // List 1
            for (refIdx = 0; refIdx < refNumPos; refIdx++)
            {
                rpsSet[rpsIdx].usedByPosPicFlag[refIdx] = TRUE;
            }

        }

    }

    rpl0Idx = 0;
    rpl1Idx = 0;

    memset (seqSet->rplList[XIN_LIST_0], 0, sizeof(xin_rpl_struct)*rpsSize);
    memset (seqSet->rplList[XIN_LIST_1], 0, sizeof(xin_rpl_struct)*rpsSize);

    for (rpsIdx = 0; rpsIdx < rpsSize; rpsIdx++)
    {
        seqSet->rplList[XIN_LIST_0][rpl0Idx].numOfPics = rpsSet[rpsIdx].numOfNegPics;

        for (refIdx = 0; refIdx < rpsSet[rpsIdx].numOfNegPics; refIdx++)
        {
            if (rpsSet[rpsIdx].usedByNegPicFlag)
            {
                seqSet->rplList[XIN_LIST_0][rpl0Idx].deltaPos[refIdx] = rpsSet[rpsIdx].deltaNegPos[refIdx];
            }
        }

        rpl0Idx++;

        if (seqSet->config.bFrameNum)
        {
            if (rpsIdx == 0)
            {
                seqSet->rplList[XIN_LIST_1][rpl1Idx].numOfPics  = rpsSet[rpsIdx].numOfNegPics;

                for (refIdx = 0; refIdx < rpsSet[rpsIdx].numOfNegPics; refIdx++)
                {
                    if (rpsSet[rpsIdx].usedByNegPicFlag)
                    {
                        seqSet->rplList[XIN_LIST_1][rpl1Idx].deltaPos[refIdx] = rpsSet[rpsIdx].deltaNegPos[refIdx];
                    }
                }
            }
            else
            {
                seqSet->rplList[XIN_LIST_1][rpl1Idx].numOfPics = rpsSet[rpsIdx].numOfPosPics;

                for (refIdx = 0; refIdx < rpsSet[rpsIdx].numOfPosPics; refIdx++)
                {
                    if (rpsSet[rpsIdx].usedByPosPicFlag)
                    {
                        seqSet->rplList[XIN_LIST_1][rpl1Idx].deltaPos[refIdx] = -rpsSet[rpsIdx].deltaPosPos[refIdx];
                    }
                }
            }

            rpl1Idx++;

        }

    }

    seqSet->rplNum[XIN_LIST_0] = rpl0Idx;
    seqSet->rplNum[XIN_LIST_1] = rpl1Idx;

    if ((seqSet->config.temporalLayerNum > 1) || (seqSet->config.bFrameNum))
    {
        seqSet->rcGopSize = seqSet->predGopSize;
    }

    if (seqSet->config.intraPeriod)
    {
        seqSet->config.intraPeriod  = (seqSet->config.intraPeriod + seqSet->predGopSize - 1) & (~(seqSet->predGopSize - 1));
        seqSet->config.intraPeriod += (seqSet->config.refreshType == XIN_IDR_REFRESH) && (seqSet->config.bFrameNum > 0);
    }

}

void Xin266GenChromaQpMap (
    xin_seq_struct *seqSet)
{
    UINT32  qpIdx;
    UINT32  numPtsIdx;
    UINT32  *qpInVal;
    UINT32  *qpOutVal;
    SINT32  sh;
    SINT32  mIdx;
    SINT32  deltaQpInValMinus1[6];

    qpInVal  = seqSet->config.qpInVal;
    qpOutVal = seqSet->config.qpOutVal;

    for (qpIdx = 0; qpIdx <= qpInVal[0]; qpIdx++)
    {
        seqSet->chromaQpMap[qpIdx] = qpIdx;
    }

    for (numPtsIdx = 0; numPtsIdx < seqSet->config.numPtsInQpTable; numPtsIdx++)
    {
        deltaQpInValMinus1[numPtsIdx] = qpInVal[numPtsIdx + 1] - qpInVal[numPtsIdx] - 1;
    }

    for (numPtsIdx = 0; numPtsIdx < seqSet->config.numPtsInQpTable; numPtsIdx++)
    {
        sh   = (deltaQpInValMinus1[numPtsIdx] + 1) >> 1;
        mIdx = 1;

        for (qpIdx = qpInVal[numPtsIdx] + 1; qpIdx <= qpInVal[numPtsIdx+1]; qpIdx++)
        {
            seqSet->chromaQpMap[qpIdx] = seqSet->chromaQpMap[qpInVal[numPtsIdx]] + ((qpOutVal[numPtsIdx+1] - qpOutVal[numPtsIdx]) * mIdx + sh) / (deltaQpInValMinus1[numPtsIdx] + 1);

            mIdx++;
        }
    }

    for (qpIdx = qpInVal[seqSet->config.numPtsInQpTable] + 1; qpIdx <= XIN_MAX_QP; qpIdx++)
    {
        seqSet->chromaQpMap[qpIdx] = seqSet->chromaQpMap[qpIdx - 1] + 1;
        seqSet->chromaQpMap[qpIdx] = XIN_CLIP (seqSet->chromaQpMap[qpIdx], XIN_MIN_QP, XIN_MAX_QP);
    }

    seqSet->chromaWeight = chromaWeightFactor + XIN_MAX_UV_QP_DIF;

}

static SINT32 Xin266ConstructQuantParam (
    xin_seq_struct *seqSet)
{
    UINT32          qp;
    UINT32          qpRem;
    UINT32          qpPer;
    xin_quant_param *quantParam;

    XIN_MALLOC_CHECK (seqSet->quantParam[0], (XIN_QP_NUM + XIN_QP_SHIFT)*sizeof(xin_quant_param));
    XIN_MALLOC_CHECK (seqSet->quantParam[1], (XIN_QP_NUM + XIN_QP_SHIFT)*sizeof(xin_quant_param));

    for (qp = XIN_MIN_QP; qp <= XIN_MAX_QP + XIN_QP_SHIFT; qp++)
    {
        quantParam = seqSet->quantParam[0] + qp;

        qpRem = qp % 6;
        qpPer = qp / 6;

        quantParam->qMult   = qMult[0][qpRem];
        quantParam->iqMult  = iqMult[0][qpRem] << qpPer;
        quantParam->qShift  = 14 + (15 - XIN_INTERNAL_BIT_DEPTH) + qpPer;
        quantParam->iqShift = 6 - (15 - XIN_INTERNAL_BIT_DEPTH);

        quantParam = seqSet->quantParam[1] + qp;

        quantParam->qMult   = qMult[1][qpRem];
        quantParam->iqMult  = iqMult[1][qpRem] << qpPer;
        quantParam->qShift  = 14 + (15 - XIN_INTERNAL_BIT_DEPTH) + qpPer;
        quantParam->iqShift = 6 - (15 - XIN_INTERNAL_BIT_DEPTH);

    }

    return XIN_SUCCESS;

}

SINT32 Xin266SeqCreate (
    xin_seq_struct *seqSet,
    xin26x_params  *config)
{
    xin_lb_struct  *outputBuf;

    seqSet->config.inputWidth   = config->inputWidth;
    seqSet->config.inputHeight  = config->inputHeight;
    seqSet->config.minCuSize    = config->minCuSize;
    seqSet->config.minQtSize    = config->minQtSize;
    seqSet->config.maxBtSize    = config->maxBtSize;
    seqSet->config.maxTtSize    = config->maxTtSize;
    seqSet->config.minBtSize    = config->minCuSize;
    seqSet->config.minTtSize    = config->minCuSize;
    seqSet->config.maxMttDepth  = !!config->maxMttDepth;
    seqSet->config.lumaTrSize64 = (config->ctuSize == 128) ? TRUE : config->lumaTrSize64;
    seqSet->config.maxTrSize    = (seqSet->config.lumaTrSize64) ? XIN_MAX_TU_SIZE : 32;
    seqSet->config.ctuSize      = (seqSet->config.lumaTrSize64) ? config->ctuSize : XIN_MIN (64, config->ctuSize);
    seqSet->config.enableCclm   = config->enableCclm;
    seqSet->config.offlineMode  = config->enableAlf;
    seqSet->config.enableAlf    = config->enableAlf;
    seqSet->config.enableCcAlf  = config->enableAlf;
    seqSet->config.enableLmcs   = FALSE;
    seqSet->config.enableAmvr   = config->enableAmvr;
    seqSet->config.zeroLatency  = config->zeroLatency;
    seqSet->config.enableBcw    = config->enableBcw;

    seqSet->frameWidth  = (config->inputWidth + XIN_MIN_CU_SIZE - 1) & (~(XIN_MIN_CU_SIZE - 1));
    seqSet->frameHeight = (config->inputHeight + XIN_MIN_CU_SIZE - 1) & (~(XIN_MIN_CU_SIZE - 1));
    seqSet->ctuSize     = seqSet->config.ctuSize;
    seqSet->lgCtuSize   = calcLog2[seqSet->ctuSize];
    seqSet->rcGopSize   = 1;
    seqSet->laUnitSize  = (config->bFrameNum) ? 8 : 16;
    seqSet->fpUnitSize  = 16;

    seqSet->frameWidthInCtu  = (seqSet->frameWidth + (seqSet->ctuSize - 1)) / seqSet->ctuSize;
    seqSet->frameHeightInCtu = (seqSet->frameHeight + (seqSet->ctuSize - 1)) / seqSet->ctuSize;
    seqSet->frameSizeInCtu   = seqSet->frameWidthInCtu * seqSet->frameHeightInCtu;

    seqSet->laWdtInUnit = ((seqSet->config.inputWidth >> 1) + seqSet->laUnitSize - 1) / seqSet->laUnitSize;
    seqSet->laHgtInUnit = ((seqSet->config.inputHeight >> 1) + seqSet->laUnitSize - 1) / seqSet->laUnitSize;
    seqSet->laTotalUnit = seqSet->laWdtInUnit * seqSet->laHgtInUnit;

    seqSet->bitsForPOC         = 8;
    seqSet->config.bitDepth    = 8;
    seqSet->config.qp          = config->qp;
    seqSet->config.numTileRows = 1;
    seqSet->config.numTileCols = 1;
    seqSet->config.refFrameNum = 1;
    seqSet->config.rdoqThrVal  = config->enableRdoq ? 8 : 0;

    seqSet->config.unitTreeStrength     = XIN_CLIP (config->unitTreeStrength, 1.5, 3.0);
    seqSet->config.frameThreadNum       = config->enableFpp && config->bFrameNum ? calcLog2[config->bFrameNum + 1] + 1 : 1;
    seqSet->config.temporalLayerNum     = config->bFrameNum ? 1 : config->temporalLayerNum;
    seqSet->config.enableSignDataHiding = config->enableSignDataHiding;
    seqSet->config.enableCuQpDelta      = config->enableCuQpDelta;
    seqSet->config.disableDeblock       = !config->enableDeblock;
    seqSet->config.rateControlMode      = config->rcMode;
    seqSet->config.enableFrameSkip      = config->bFrameNum ? FALSE : config->frameSkip;
    seqSet->config.unfoldRefList0       = FALSE;
    seqSet->config.lookAhead            = (seqSet->config.rateControlMode == 0) ? 0 : config->lookAhead;
    seqSet->config.frameToBeEncoded     = config->frameToBeEncoded;
    seqSet->config.transSkipFlag        = config->transformSkipFlag;
    seqSet->config.screenContentMode    = config->screenContentMode;
    seqSet->config.maxTrSkipLgSize      = config->transformSkipFlag ? calcLog2[config->maxTrSkipSize] : 0;
    seqSet->config.motionSearchMode     = config->motionSearchMode;
    seqSet->config.intraLgDec           = 3;
    seqSet->config.enableMctf           = config->enableMctf != 0;
    seqSet->config.inputBitDepth        = XIN_8_BIT_DEPTH;
    seqSet->config.internalBitDepth     = sizeof(PIXEL) > 1 ? XIN_10_BIT_DEPTH : XIN_8_BIT_DEPTH;
    seqSet->config.disableBigInter      = TRUE;
    seqSet->config.maxAffineMergeCand   = 5;
    seqSet->config.mctfUnitSize         = (seqSet->config.inputWidth*seqSet->config.inputHeight < 1920*1080) ? 8 : 16;
    seqSet->config.enableDepQuant       = config->enableDepQuant;
    seqSet->config.enableSignDataHiding = seqSet->config.enableDepQuant ? FALSE : config->enableSignDataHiding;
    seqSet->config.enableSceneCut       = config->enableSceneCut;
    seqSet->config.chromaQpOffset       = 0;
    seqSet->config.quitSkipDepths       = 6;

    seqSet->config.searchRange  = config->searchRange;
    seqSet->config.enableSao    = config->enableSao;
    seqSet->config.enableTMvp   = config->enableTMvp;
    seqSet->config.enableWpp    = config->enableWpp;
    seqSet->config.bitRate      = config->bitRate;
    seqSet->config.crf          = config->crf;
    seqSet->config.vbvBufSize   = config->vbvBufSize;
    seqSet->config.vbvMaxRate   = config->vbvMaxRate;
    seqSet->config.frameRate    = config->frameRate;
    seqSet->config.minQp        = config->minQp;
    seqSet->config.maxQp        = config->maxQp;
    seqSet->config.intraPeriod  = config->intraPeriod;
    seqSet->config.encoderMode  = config->encoderMode;
    seqSet->config.bFrameNum    = config->bFrameNum;
    seqSet->config.refFrameNum  = (seqSet->config.temporalLayerNum > 1) ? 1 : config->refFrameNum;
    seqSet->config.calcPsnr     = config->calcPsnr;
    seqSet->config.refreshType  = config->refreshType;
    seqSet->config.maxMdCandNum = 1;
    seqSet->config.enableRdoq   = config->enableRdoq;
    seqSet->config.fastSubMe    = 2;
    seqSet->config.unitTree     = (seqSet->config.rateControlMode == 0) ? FALSE : config->unitTree;
    seqSet->config.laSubMe      = config->bFrameNum ? TRUE : FALSE;
    seqSet->config.maxBtSize    = seqSet->config.maxMttDepth ? seqSet->config.maxBtSize : seqSet->config.minCuSize;
    seqSet->config.maxTtSize    = seqSet->config.maxMttDepth ? seqSet->config.maxTtSize : seqSet->config.minCuSize;
    seqSet->config.enableDmvr   = config->enableDmvr;
    seqSet->config.threadNum    = config->threadNum ? XIN_MIN (config->threadNum, seqSet->cpuCoreNum) : seqSet->cpuCoreNum;
    seqSet->config.needRecon    = config->needRecon;
    seqSet->config.fastNonRef   = TRUE;
    seqSet->config.fastNonQt    = TRUE;
    seqSet->config.mctfMode     = 1;
    seqSet->config.alfMode      = 1;
    seqSet->config.affineType   = XIN_AFFINE_MODEL_6PARAM;
    seqSet->config.enableSbtMvp = config->enableSbTmvp;
    seqSet->config.enableAffine = config->enableAffine;
    seqSet->config.enableMts    = config->enableMts;
    seqSet->config.mctfRefNum   = XIN_TF_MAX_REF_NUM >> 1;
    seqSet->config.enableGpb    = config->enableGpb;
    seqSet->config.enableBim    = config->enableBim;

    if (seqSet->config.encoderMode == 0)
    {
        seqSet->config.satdMd           = FALSE;
        seqSet->config.fastRateEst      = TRUE;
        seqSet->config.fastSao          = TRUE;
        seqSet->config.cuDepthPred      = TRUE;
        seqSet->config.cuDepthQuit      = TRUE;
        seqSet->config.cuModeQuit       = TRUE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = TRUE;
        seqSet->config.skipIntraMode    = TRUE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.gradientFastQtbt = TRUE;
        seqSet->config.fastIntraMd      = 2;
        seqSet->config.intraLgDec       = 3;
        seqSet->config.maxMergeCand     = 3;
        seqSet->config.intraRdoNum      = 1;
        seqSet->config.maxMdCandNum     = 1;
        seqSet->config.qtbttSpeedUp     = 3;
        seqSet->config.fastTTSplit      = 1;
        seqSet->config.tuCoefDepthQuit  = 2;
        seqSet->config.mctfMode         = 2;
        seqSet->config.alfMode          = 3;
        seqSet->config.quitSkipDepths   = 1;

    }
    else if (seqSet->config.encoderMode == 1)
    {
        seqSet->config.satdMd           = FALSE;
        seqSet->config.fastRateEst      = TRUE;
        seqSet->config.fastSao          = TRUE;
        seqSet->config.cuDepthPred      = TRUE;
        seqSet->config.cuDepthQuit      = TRUE;
        seqSet->config.cuModeQuit       = TRUE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = TRUE;
        seqSet->config.skipIntraMode    = TRUE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.gradientFastQtbt = TRUE;
        seqSet->config.biPredMe         = FALSE;
        seqSet->config.fastIntraMd      = 2;
        seqSet->config.intraLgDec       = 3;
        seqSet->config.maxMergeCand     = 5;
        seqSet->config.intraRdoNum      = 1;
        seqSet->config.maxMdCandNum     = 1;
        seqSet->config.qtbttSpeedUp     = 3;
        seqSet->config.fastTTSplit      = 1;
        seqSet->config.tuCoefDepthQuit  = 2;
        seqSet->config.mctfMode         = 2;
        seqSet->config.alfMode          = 3;
        seqSet->config.quitSkipDepths   = 1;

    }
    else if (seqSet->config.encoderMode == 2)
    {
        seqSet->config.satdMd           = TRUE;
        seqSet->config.fastRateEst      = TRUE;
        seqSet->config.fastSao          = TRUE;
        seqSet->config.cuDepthPred      = TRUE;
        seqSet->config.cuDepthQuit      = TRUE;
        seqSet->config.cuModeQuit       = TRUE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = TRUE;
        seqSet->config.skipIntraMode    = TRUE;
        seqSet->config.gradientFastQtbt = TRUE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.fastIntraMd      = 1;
        seqSet->config.intraLgDec       = 3;
        seqSet->config.maxMergeCand     = 5;
        seqSet->config.intraRdoNum      = 2;
        seqSet->config.maxMdCandNum     = 2;
        seqSet->config.qtbttSpeedUp     = 3;
        seqSet->config.fastTTSplit      = 1;
        seqSet->config.tuCoefDepthQuit  = 1;
        seqSet->config.mctfMode         = 2;
        seqSet->config.alfMode          = 3;
        seqSet->config.quitSkipDepths   = 1;

    }
    else if (seqSet->config.encoderMode == 3)
    {
        seqSet->config.satdMd           = TRUE;
        seqSet->config.fastRateEst      = TRUE;
        seqSet->config.fastSao          = TRUE;
        seqSet->config.cuDepthPred      = FALSE;
        seqSet->config.cuDepthQuit      = FALSE;
        seqSet->config.cuModeQuit       = FALSE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = TRUE;
        seqSet->config.skipIntraMode    = TRUE;
        seqSet->config.gradientFastQtbt = TRUE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.fastIntraMd      = 1;
        seqSet->config.intraLgDec       = 2;
        seqSet->config.maxMergeCand     = 5;
        seqSet->config.intraRdoNum      = 2;
        seqSet->config.maxMdCandNum     = 3;
        seqSet->config.qtbttSpeedUp     = 2;
        seqSet->config.fastTTSplit      = 1;
        seqSet->config.tuCoefDepthQuit  = 1;
        seqSet->config.mctfMode         = 2;
        seqSet->config.alfMode          = 2;
        seqSet->config.quitSkipDepths   = 1;

    }
    else if (seqSet->config.encoderMode == 4)
    {
        seqSet->config.satdMd           = TRUE;
        seqSet->config.fastRateEst      = FALSE;
        seqSet->config.fastSao          = FALSE;
        seqSet->config.cuDepthPred      = FALSE;
        seqSet->config.cuDepthQuit      = FALSE;
        seqSet->config.cuModeQuit       = FALSE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = FALSE;
        seqSet->config.skipIntraMode    = FALSE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.gradientFastQtbt = FALSE;
        seqSet->config.fastIntraMd      = 1;
        seqSet->config.intraLgDec       = 2;
        seqSet->config.maxMergeCand     = 5;
        seqSet->config.maxMdCandNum     = 3;
        seqSet->config.intraRdoNum      = 3;
        seqSet->config.biPredMe         = TRUE;
        seqSet->config.mctfMode         = 2;
        seqSet->config.fastTTSplit      = 1;
        seqSet->config.qtbttSpeedUp     = 3;
        seqSet->config.tuCoefDepthQuit  = 0;
        seqSet->config.quitSkipDepths   = 2;
        seqSet->config.mctfRefNum       = XIN_TF_MAX_REF_NUM;

    }
    else if (seqSet->config.encoderMode == 5)
    {
        seqSet->config.satdMd           = TRUE;
        seqSet->config.fastRateEst      = FALSE;
        seqSet->config.fastSao          = FALSE;
        seqSet->config.cuDepthPred      = FALSE;
        seqSet->config.cuDepthQuit      = FALSE;
        seqSet->config.cuModeQuit       = FALSE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = FALSE;
        seqSet->config.skipIntraMode    = FALSE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.gradientFastQtbt = FALSE;
        seqSet->config.fastIntraMd      = 1;
        seqSet->config.intraLgDec       = 1;
        seqSet->config.maxMergeCand     = 6;
        seqSet->config.maxMdCandNum     = 4;
        seqSet->config.intraRdoNum      = 3;
        seqSet->config.biPredMe         = TRUE;
        seqSet->config.qtbttSpeedUp     = 2;
        seqSet->config.fastTTSplit      = 0;
        seqSet->config.tuCoefDepthQuit  = 0;
        seqSet->config.quitSkipDepths   = 2;
        seqSet->config.mctfRefNum       = XIN_TF_MAX_REF_NUM;

    }
    else if (seqSet->config.encoderMode == 6)
    {
        seqSet->config.satdMd           = TRUE;
        seqSet->config.fastRateEst      = FALSE;
        seqSet->config.fastSao          = FALSE;
        seqSet->config.cuDepthPred      = FALSE;
        seqSet->config.cuDepthQuit      = FALSE;
        seqSet->config.cuModeQuit       = FALSE;
        seqSet->config.cuEarlySkip      = TRUE;
        seqSet->config.enableSkipMe     = FALSE;
        seqSet->config.skipIntraMode    = FALSE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.gradientFastQtbt = FALSE;
        seqSet->config.fastIntraMd      = 1;
        seqSet->config.intraLgDec       = 1;
        seqSet->config.maxMergeCand     = 6;
        seqSet->config.maxMdCandNum     = 4;
        seqSet->config.intraRdoNum      = 4;
        seqSet->config.fastSubMe        = 1;
        seqSet->config.biPredMe         = TRUE;
        seqSet->config.qtbttSpeedUp     = 1;
        seqSet->config.fastTTSplit      = 0;
        seqSet->config.tuCoefDepthQuit  = 0;
        seqSet->config.quitSkipDepths   = 3;
        seqSet->config.mctfRefNum       = XIN_TF_MAX_REF_NUM;

    }
    else
    {
        seqSet->config.satdMd           = TRUE;
        seqSet->config.fastRateEst      = FALSE;
        seqSet->config.fastSao          = FALSE;
        seqSet->config.cuDepthPred      = FALSE;
        seqSet->config.cuDepthQuit      = FALSE;
        seqSet->config.cuModeQuit       = FALSE;
        seqSet->config.cuEarlySkip      = FALSE;
        seqSet->config.enableSkipMe     = FALSE;
        seqSet->config.skipIntraMode    = FALSE;
        seqSet->config.chromaFastCost   = FALSE;
        seqSet->config.gradientFastQtbt = FALSE;
        seqSet->config.fastIntraMd      = 1;
        seqSet->config.intraLgDec       = 1;
        seqSet->config.maxMergeCand     = 6;
        seqSet->config.maxMdCandNum     = 8;
        seqSet->config.intraRdoNum      = 5;
        seqSet->config.fastSubMe        = 1;
        seqSet->config.biPredMe         = TRUE;
        seqSet->config.qtbttSpeedUp     = 0;
        seqSet->config.fastNonRef       = FALSE;
        seqSet->config.tuCoefDepthQuit  = 0;
        seqSet->config.mctfRefNum       = XIN_TF_MAX_REF_NUM;

    }

    seqSet->config.constrainedIntraPredFlag   = FALSE;
    seqSet->config.enableStrongIntraSmoothing = FALSE;

    seqSet->config.oneDimMe           = seqSet->config.screenContentMode ? TRUE : FALSE;
    seqSet->config.motionSearchMode   = seqSet->config.screenContentMode ? XIN_ME_HIER_SEARCH : seqSet->config.motionSearchMode;
    seqSet->config.satdMd             = seqSet->config.screenContentMode ? FALSE : seqSet->config.satdMd;
    seqSet->config.skipIntraMode      = seqSet->config.screenContentMode ? FALSE : seqSet->config.skipIntraMode;
    seqSet->config.transSkipFlag      = seqSet->config.screenContentMode ? TRUE : seqSet->config.transSkipFlag;
    seqSet->config.maxTrSkipLgSize    = seqSet->config.transSkipFlag ? XIN_MAX (seqSet->config.maxTrSkipLgSize, calcLog2[8]) : 0;
    seqSet->config.maxAffineMergeCand = seqSet->config.enableAffine ? seqSet->config.maxAffineMergeCand : (seqSet->config.enableSbtMvp ? 1 : 0);
    seqSet->config.laSatdMd           = seqSet->config.satdMd;

    if ((seqSet->config.bFrameNum == 0) && (seqSet->config.bitRate * 50 < seqSet->frameHeight*seqSet->frameWidth*seqSet->config.frameRate) && (seqSet->config.unitTreeStrength > 1.0))
    {
        seqSet->config.unitTreeStrength = 1.0;
    }

    if (seqSet->config.rateControlMode == XIN_RC_ABR)
    {
        seqSet->config.lookAhead = XIN_MAX (seqSet->config.lookAhead, seqSet->config.bFrameNum + 2);
    }

    seqSet->config.numTileRows    = config->numTileRows;
    seqSet->config.numTileCols    = config->numTileCols;
    seqSet->config.enableTiles    = ((config->numTileRows > 1) || (config->numTileCols > 1)) && (!config->enableWpp);
    seqSet->config.numTileCols    = (config->enableWpp || (!seqSet->config.enableTiles)) ? 1 : seqSet->config.numTileCols;
    seqSet->config.numTileRows    = (config->enableWpp || (!seqSet->config.enableTiles)) ? seqSet->frameHeightInCtu : seqSet->config.numTileRows;
    seqSet->config.lookAhead      = XIN_MIN (seqSet->config.frameToBeEncoded, seqSet->config.lookAhead);
    seqSet->config.lookAhead      = seqSet->config.lookAhead ? XIN_MAX (seqSet->config.bFrameNum + 1, seqSet->config.lookAhead) : seqSet->config.lookAhead;
    seqSet->config.adaptiveBFrame = (config->bFrameNum && seqSet->config.lookAhead) ? config->adaptiveBFrame : FALSE;
    seqSet->config.offlineMode   |= (!seqSet->config.enableTiles) && (!config->enableWpp);

    seqSet->tileNum  = seqSet->config.numTileRows*seqSet->config.numTileCols;
    seqSet->drConfig = seqSet->config.cuDepthPred ? drConfigFast : drConfigSlow;

    seqSet->cuNumInCtu         = ((seqSet->ctuSize == 128) || (seqSet->config.minCuSize == XIN_MIN_CU_SIZE)) ? 1000 : 200;
    seqSet->blockSize          = ((seqSet->config.minCuSize == 8) && (!seqSet->config.enableAffine)) ? 8 : 4;
    seqSet->lgBlockSize        = calcLog2[seqSet->blockSize];
    seqSet->blockSetWidth      = (seqSet->frameWidthInCtu + 1)*seqSet->ctuSize/seqSet->blockSize;
    seqSet->blockSetHeight     = (seqSet->frameHeightInCtu + 1)*seqSet->ctuSize/seqSet->blockSize;
    seqSet->frameWidthInBlock  = (seqSet->frameWidth + (seqSet->blockSize - 1)) / seqSet->blockSize;
    seqSet->frameHeightInBlock = (seqSet->frameHeight + (seqSet->blockSize - 1)) / seqSet->blockSize;
    seqSet->encodeFinished     = FALSE;
    seqSet->flushIdx           = -1;
    seqSet->maxTemporalId      = calcLog2[config->bFrameNum + 1];

    seqSet->config.qpInVal[0] = 26;//17;//33;
    seqSet->config.qpInVal[1] = 27;//22;//34;
    seqSet->config.qpInVal[2] = 0;//34; //36;
    seqSet->config.qpInVal[3] = 0;//42; //38;
    seqSet->config.qpInVal[4] = 0; //40;
    seqSet->config.qpInVal[5] = 0;

    seqSet->config.qpOutVal[0] = 26;//17;//33;
    seqSet->config.qpOutVal[1] = 26;//23;//33;
    seqSet->config.qpOutVal[2] = 0;//35; //34;
    seqSet->config.qpOutVal[3] = 0;//39; //35;
    seqSet->config.qpOutVal[4] = 0; //36;
    seqSet->config.qpOutVal[5] = 0;

    seqSet->config.numPtsInQpTable = 1;

    Xin266GenChromaQpMap (
        seqSet);

    XIN_MALLOC_CHECK (seqSet->ctuTsToRsAddrMap,    seqSet->frameSizeInCtu*sizeof(UINT32));
    XIN_MALLOC_CHECK (seqSet->rpsSet,              sizeof(xin_rps_struct)*XIN_MAX_RPS_NUM);
    XIN_MALLOC_CHECK (seqSet->rplList[XIN_LIST_0], sizeof(xin_rpl_struct)*XIN_MAX_RPL_NUM);
    XIN_MALLOC_CHECK (seqSet->rplList[XIN_LIST_1], sizeof(xin_rpl_struct)*XIN_MAX_RPL_NUM);

    Xin266ConstructTileDim (
        seqSet);

    if (Xin266ConstructCabacContext (seqSet))
    {
        return XIN_FAIL;
    }

    Xin266ConstructRps (
        seqSet);

    if (Xin266ConstructInputPicBuf (seqSet))
    {
        return XIN_FAIL;
    }

    if (Xin266ConstructRefPicBuf (seqSet))
    {
        return XIN_FAIL;
    }

    if (Xin266ContructScanOrder (seqSet))
    {
        return XIN_FAIL;
    }

    if (Xin266ConstructQuantParam (seqSet))
    {
        return XIN_FAIL;
    }

    seqSet->unitListLock = Xin26xConstructLock ();

    // Malloc linear buffer memory
    XIN_MALLOC_CHECK (outputBuf,       sizeof(xin_lb_struct));
    XIN_MALLOC_CHECK (outputBuf->base, seqSet->frameWidth*seqSet->frameHeight*3/2);

    outputBuf->size   = seqSet->frameWidth*seqSet->frameHeight*3/2;
    outputBuf->index  = 0;
    seqSet->outputBuf = outputBuf;

    return XIN_SUCCESS;

}

void Xin266SeqDelete (
    xin_seq_struct *seqSet)
{
    free (seqSet->ctuTsToRsAddrMap);
    free (seqSet->rpsSet);
    free (seqSet->rplList[0]);
    free (seqSet->rplList[1]);
    free (seqSet->cabacContext);

    Xin266DestructInputPicBuf (
        seqSet);

    Xin266DestructRefPicBuf (
        seqSet);

    Xin266DestructScanOrder (
        seqSet);

    Xin26xDestructLock (
        seqSet->unitListLock);

    free (seqSet->quantParam[0]);
    free (seqSet->quantParam[1]);
    free (seqSet->outputBuf->base);
    free (seqSet->outputBuf);
    free (seqSet);

}

static SINT32 Xin266ContructPicCtu (
    xin_pic_struct *picSet)
{
    xin_seq_struct  *seqSet;
    UINT32          colIdx;
    UINT32          rowIdx;
    UINT32          tileIdx;
    xin266_tile_dim *tileDim;

    seqSet  = picSet->seqSet;
    tileIdx = 0;

    XIN_MALLOC_CHECK (picSet->ctu, seqSet->frameSizeInCtu * sizeof(xin_ctu_struct));

    for (rowIdx = 0; rowIdx < seqSet->config.numTileRows; rowIdx++)
    {
        for (colIdx = 0; colIdx < seqSet->config.numTileCols; colIdx++)
        {
            tileDim = seqSet->tileDim + tileIdx;

            Xin266ContructTileCtu (
                picSet,
                tileDim);

            tileIdx++;
        }
    }

    return XIN_SUCCESS;

}

SINT32 Xin266ContructPicCoef (
    xin_pic_struct *picSet)
{
    xin_seq_struct *seqSet;
    xin_ctu_struct *ctu;
    UINT32         ctuIdx;
    UINT32         ctuSize;
    UINT32         lumaSize;
    UINT32         chromaSize;
    UINT32         lumaCGNum;
    UINT32         chromaCGNum;

    seqSet      = picSet->seqSet;
    ctuSize     = seqSet->ctuSize;
    lumaSize    = ctuSize*ctuSize;
    chromaSize  = ctuSize*ctuSize/4;
    lumaCGNum   = lumaSize / 16;
    chromaCGNum = chromaSize / 16;

    XIN_MALLOC_CHECK (picSet->coeffBuf[PLANE_LUMA],     seqSet->frameSizeInCtu*lumaSize*sizeof(COEFF));
    XIN_MALLOC_CHECK (picSet->coeffBuf[PLANE_CHROMA_U], seqSet->frameSizeInCtu*chromaSize*sizeof(COEFF));
    XIN_MALLOC_CHECK (picSet->coeffBuf[PLANE_CHROMA_V], seqSet->frameSizeInCtu*chromaSize*sizeof(COEFF));

    XIN_MALLOC_CHECK (picSet->gt0Buf[PLANE_LUMA],     seqSet->frameSizeInCtu*lumaCGNum*sizeof(UINT16));
    XIN_MALLOC_CHECK (picSet->gt0Buf[PLANE_CHROMA_U], seqSet->frameSizeInCtu*chromaCGNum*sizeof(UINT16));
    XIN_MALLOC_CHECK (picSet->gt0Buf[PLANE_CHROMA_V], seqSet->frameSizeInCtu*chromaCGNum*sizeof(UINT16));

    for (ctuIdx = 0; ctuIdx < seqSet->frameSizeInCtu; ctuIdx++)
    {
        ctu = picSet->ctu + ctuIdx;

        ctu->coeffBuf[PLANE_LUMA]     = picSet->coeffBuf[PLANE_LUMA] + lumaSize*ctuIdx;
        ctu->coeffBuf[PLANE_CHROMA_U] = picSet->coeffBuf[PLANE_CHROMA_U] + chromaSize*ctuIdx;
        ctu->coeffBuf[PLANE_CHROMA_V] = picSet->coeffBuf[PLANE_CHROMA_V] + chromaSize*ctuIdx;

        ctu->coeffStride[PLANE_LUMA]   = ctuSize;
        ctu->coeffStride[PLANE_CHROMA] = ctuSize/2;

        ctu->gt0Buf[PLANE_LUMA]     = picSet->gt0Buf[PLANE_LUMA] + lumaCGNum*ctuIdx;
        ctu->gt0Buf[PLANE_CHROMA_U] = picSet->gt0Buf[PLANE_CHROMA_U] + chromaCGNum*ctuIdx;
        ctu->gt0Buf[PLANE_CHROMA_V] = picSet->gt0Buf[PLANE_CHROMA_V] + chromaCGNum*ctuIdx;
    }

    return XIN_SUCCESS;

}

static void Xin266DestructPicCoef (
    xin_pic_struct *picSet)
{
    free (picSet->coeffBuf[PLANE_LUMA]);
    free (picSet->coeffBuf[PLANE_CHROMA_U]);
    free (picSet->coeffBuf[PLANE_CHROMA_V]);

    free (picSet->gt0Buf[PLANE_LUMA]);
    free (picSet->gt0Buf[PLANE_CHROMA_U]);
    free (picSet->gt0Buf[PLANE_CHROMA_V]);

}

static SINT32 Xin266ConstructEntropy (
    xin_pic_struct *picSet)
{
    xin266_tile_dim   *tileDim;
    xin_cabac_context *cabacSet;
    xin_seq_struct    *seqSet;
    xin_cabac_struct  *cabac;
    UINT8             *bitBuf;
    UINT32            buf0Size;
    UINT32            bufSize;
    UINT32            sectionIdx;

    seqSet   = picSet->seqSet;
    tileDim  = seqSet->tileDim;
    buf0Size = (!seqSet->config.enableWpp) && (!seqSet->config.enableTiles) ? seqSet->frameWidth*seqSet->frameHeight * 3 / 2 : tileDim->tileWidth*tileDim->tileHeight * 3 / 2;
    bufSize  = tileDim->tileWidth*tileDim->tileHeight * 3 / 2;

    for (sectionIdx = 0; sectionIdx < seqSet->tileNum; sectionIdx++)
    {
        XIN_MALLOC_CHECK (cabacSet, sizeof(xin_cabac_context));

        if (sectionIdx == 0)
        {
            XIN_MALLOC_CHECK (bitBuf, buf0Size);
        }
        else
        {
            XIN_MALLOC_CHECK (bitBuf, bufSize);
        }

        cabac = &cabacSet->cabac;

        cabac->bitstream.base = bitBuf;
        cabac->bitstream.size = sectionIdx ? bufSize : buf0Size;

        picSet->cabacSet[sectionIdx]     = cabacSet;
        picSet->cabacContext[sectionIdx] = cabacSet->context;

    }

    return XIN_SUCCESS;

}

SINT32 Xin266AlfCreate (
    xin_alf_struct *alfSet,
    xin_seq_struct *seqSet)
{
    UINT32           widthInBlock;
    UINT32           heightInBlock;
    UINT32           frameSizeInCtu;
    UINT32           fltSetIdx;
    UINT32           coeffIdx;
    UINT32           classIdx;
    SINT32           filterIdx;
    SINT32           planeIdx;
    xin_frame_struct *filterFrame;
    intptr_t         frameStrideLuma;
    intptr_t         frameSizeLuma;
    intptr_t         frameStrideChroma;
    intptr_t         frameSizeChroma;
    SINT32           shift;

    widthInBlock      = seqSet->frameWidth >> 2;
    heightInBlock     = seqSet->frameHeight >> 2;
    frameSizeInCtu    = seqSet->frameSizeInCtu;
    frameStrideLuma   = seqSet->frameWidth + XIN_ALF_FRAME_PADDING*2;
    frameStrideChroma = (seqSet->frameWidth >> 1) + XIN_ALF_FRAME_PADDING*2;
    frameSizeLuma     = frameStrideLuma*(seqSet->frameHeight + XIN_ALF_FRAME_PADDING*2);
    frameSizeChroma   = frameStrideChroma*((seqSet->frameHeight >> 1) + XIN_ALF_FRAME_PADDING*2);

    memset (alfSet, 0, sizeof(xin_alf_struct));

    XIN_MALLOC_CHECK (alfSet->alfClass,       widthInBlock*heightInBlock*sizeof(xin_alf_class));
    XIN_MALLOC_CHECK (alfSet->alfCovarianceY, frameSizeInCtu*XIN_ALF_MAX_CLS_NUM*sizeof(xin_alf_cov));
    XIN_MALLOC_CHECK (alfSet->alfCovarianceU, frameSizeInCtu*sizeof(xin_alf_cov));
    XIN_MALLOC_CHECK (alfSet->alfCovarianceV, frameSizeInCtu*sizeof(xin_alf_cov));

    XIN_MALLOC_CHECK (alfSet->ctuEnableFlagTmp[PLANE_LUMA],     frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ctuEnableFlagTmp[PLANE_CHROMA_U], frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ctuEnableFlagTmp[PLANE_CHROMA_V], frameSizeInCtu*sizeof(UINT8));

    XIN_MALLOC_CHECK (alfSet->ctuEnableFlag[PLANE_LUMA],     frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ctuEnableFlag[PLANE_CHROMA_U], frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ctuEnableFlag[PLANE_CHROMA_V], frameSizeInCtu*sizeof(UINT8));

    XIN_MALLOC_CHECK (alfSet->alfCtbFilterSetIndex,    frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->alfCtbFilterSetIndexTmp, frameSizeInCtu*sizeof(UINT8));

    XIN_MALLOC_CHECK (alfSet->ctuUnfilterDist[PLANE_LUMA],     frameSizeInCtu*sizeof(FLOAT32));
    XIN_MALLOC_CHECK (alfSet->ctuUnfilterDist[PLANE_CHROMA_U], frameSizeInCtu*sizeof(FLOAT32));
    XIN_MALLOC_CHECK (alfSet->ctuUnfilterDist[PLANE_CHROMA_V], frameSizeInCtu*sizeof(FLOAT32));

    XIN_MALLOC_CHECK (alfSet->context,    XIN_NUM_OF_CTX*sizeof(xin_prob_model));
    XIN_MALLOC_CHECK (alfSet->contextOrg, XIN_NUM_OF_CTX*sizeof(xin_prob_model));

    XIN_MALLOC_CHECK (filterFrame,          sizeof(xin_frame_struct));
    XIN_MALLOC_CHECK (alfSet->filterBuf[0], frameSizeLuma*sizeof(PIXEL));
    XIN_MALLOC_CHECK (alfSet->filterBuf[1], frameSizeChroma*sizeof(PIXEL));
    XIN_MALLOC_CHECK (alfSet->filterBuf[2], frameSizeChroma*sizeof(PIXEL));

    XIN_MALLOC_CHECK (alfSet->ctuAlternative[1], frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ctuAlternative[2], frameSizeInCtu*sizeof(UINT8));

    XIN_MALLOC_CHECK (alfSet->ctuAlternativeTmp[1], frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ctuAlternativeTmp[2], frameSizeInCtu*sizeof(UINT8));

    XIN_MALLOC_CHECK (alfSet->alfCovarianceCcAlf[0], frameSizeInCtu*sizeof(xin_alf_cov));
    XIN_MALLOC_CHECK (alfSet->alfCovarianceCcAlf[1], frameSizeInCtu*sizeof(xin_alf_cov));

    XIN_MALLOC_CHECK (alfSet->bestFilterControl,  frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->trainingCovControl, frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->filterControl,      frameSizeInCtu*sizeof(UINT8));

    XIN_MALLOC_CHECK (alfSet->ccAlfFilterControl[0], frameSizeInCtu*sizeof(UINT8));
    XIN_MALLOC_CHECK (alfSet->ccAlfFilterControl[1], frameSizeInCtu*sizeof(UINT8));

    for (filterIdx = 0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
    {
        XIN_MALLOC_CHECK (alfSet->trainingDistortion[filterIdx],  frameSizeInCtu*sizeof(UINT64));
    }

    XIN_MALLOC_CHECK (alfSet->funcSet, sizeof(xin_alf_func));

    filterFrame->lumaStride   = frameStrideLuma;
    filterFrame->chromaStride = frameStrideChroma;
    filterFrame->yuvBuf[0]    = alfSet->filterBuf[0] + XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*frameStrideLuma;
    filterFrame->yuvBuf[1]    = alfSet->filterBuf[1] + XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*frameStrideChroma;
    filterFrame->yuvBuf[2]    = alfSet->filterBuf[2] + XIN_ALF_FRAME_PADDING + XIN_ALF_FRAME_PADDING*frameStrideChroma;
    filterFrame->lumaWidth    = seqSet->frameWidth;
    filterFrame->lumaHeight   = seqSet->frameHeight;

    alfSet->alfClassStride  = widthInBlock;
    alfSet->frameSizeInCtu  = seqSet->frameSizeInCtu;
    alfSet->frameWidthInCtu = seqSet->frameWidthInCtu;
    alfSet->filterFrame     = filterFrame;
    alfSet->ctuSize         = seqSet->ctuSize;

    alfSet->useNonLinearAlfLuma   = FALSE;
    alfSet->useNonLinearAlfChroma = FALSE;
    alfSet->maxNumAlfAltChroma    = 8;

    alfSet->alfFilter[PLANE_LUMA].filterLength = XIN_ALF_LUMA_FILTER_LENGTH;
    alfSet->alfFilter[PLANE_LUMA].filterType   = XIN_ALF_FILTER_7;
    alfSet->alfFilter[PLANE_LUMA].filterSize   = XIN_ALF_LUMA_FILTER_LENGTH*XIN_ALF_LUMA_FILTER_LENGTH/2 + 1;
    alfSet->alfFilter[PLANE_LUMA].numCoeff     = XIN_ALF_LUMA_FILTER_LENGTH*XIN_ALF_LUMA_FILTER_LENGTH/4 + 1;

    alfSet->alfFilter[PLANE_CHROMA].filterLength = XIN_ALF_CHROMA_FILTER_LENGTH;
    alfSet->alfFilter[PLANE_CHROMA].filterType   = XIN_ALF_FILTER_5;
    alfSet->alfFilter[PLANE_CHROMA].filterSize   = XIN_ALF_CHROMA_FILTER_LENGTH*XIN_ALF_CHROMA_FILTER_LENGTH/2 + 1;
    alfSet->alfFilter[PLANE_CHROMA].numCoeff     = XIN_ALF_CHROMA_FILTER_LENGTH*XIN_ALF_CHROMA_FILTER_LENGTH/4 + 1;

    for (fltSetIdx = 0; fltSetIdx < XIN_ALF_FIXED_FILTER_NUM; fltSetIdx++)
    {
        for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_LUMA_COEF_NUM; coeffIdx++)
        {
            alfSet->fixedFilterSetCoeff[fltSetIdx][coeffIdx] = xinAlfFixedCoeff[fltSetIdx][coeffIdx];
        }
    }

    for (fltSetIdx = 0; fltSetIdx < XIN_ALF_FIXED_FLT_SET_NUM; fltSetIdx++)
    {
        for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
        {
            for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_LUMA_COEF_NUM; coeffIdx++)
            {
                filterIdx = classToFilterMapping[fltSetIdx][classIdx];

                alfSet->fixedFilterSetCoeffDec[fltSetIdx][classIdx*XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx] = (SINT16)xinAlfFixedCoeff[filterIdx][coeffIdx];
            }
        }
    }

    for (planeIdx = 0; planeIdx < PLANE_TYPE; planeIdx++)
    {
        alfSet->alfClippingValues[planeIdx][0] = 1 << XIN_INTERNAL_BIT_DEPTH;

        shift = XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH;

        for (classIdx = 1; classIdx < XIN_ALF_CLIP_NUM; classIdx++)
        {
            alfSet->alfClippingValues[planeIdx][classIdx] = 1 << (7 - 2 * classIdx + shift);
        }
    }

    for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
    {
        for (coeffIdx = 0; coeffIdx < XIN_ALF_MAX_LUMA_COEF_NUM; coeffIdx++)
        {
            alfSet->clipDefault[classIdx*XIN_ALF_MAX_LUMA_COEF_NUM + coeffIdx] = 1 << XIN_INTERNAL_BIT_DEPTH;
        }
    }

    Xin266ALfFuncInit (
        alfSet,
        seqSet->cpuFeature);

    return XIN_SUCCESS;

}

void Xin266AlfDelete (
    xin_alf_struct *alfSet,
    xin_seq_struct *seqSet)
{
    UINT32      filterIdx;

    (void)seqSet;

    free (alfSet->alfClass);
    free (alfSet->alfCovarianceY);
    free (alfSet->alfCovarianceU);
    free (alfSet->alfCovarianceV);

    free (alfSet->ctuEnableFlagTmp[PLANE_LUMA]);
    free (alfSet->ctuEnableFlagTmp[PLANE_CHROMA_U]);
    free (alfSet->ctuEnableFlagTmp[PLANE_CHROMA_V]);

    free (alfSet->ctuEnableFlag[PLANE_LUMA]);
    free (alfSet->ctuEnableFlag[PLANE_CHROMA_U]);
    free (alfSet->ctuEnableFlag[PLANE_CHROMA_V]);

    free (alfSet->alfCtbFilterSetIndex);
    free (alfSet->alfCtbFilterSetIndexTmp);

    free (alfSet->ctuUnfilterDist[PLANE_LUMA]);
    free (alfSet->ctuUnfilterDist[PLANE_CHROMA_U]);
    free (alfSet->ctuUnfilterDist[PLANE_CHROMA_V]);

    free (alfSet->context);
    free (alfSet->contextOrg);

    free (alfSet->filterFrame);
    free (alfSet->filterBuf[0]);
    free (alfSet->filterBuf[1]);
    free (alfSet->filterBuf[2]);

    free (alfSet->ctuAlternative[1]);
    free (alfSet->ctuAlternative[2]);

    free (alfSet->ctuAlternativeTmp[1]);
    free (alfSet->ctuAlternativeTmp[2]);

    free (alfSet->alfCovarianceCcAlf[0]);
    free (alfSet->alfCovarianceCcAlf[1]);

    free (alfSet->bestFilterControl);
    free (alfSet->trainingCovControl);
    free (alfSet->filterControl);

    free (alfSet->ccAlfFilterControl[0]);
    free (alfSet->ccAlfFilterControl[1]);

    for (filterIdx = 0; filterIdx < XIN_CC_ALF_MAX_FILTER_NUM; filterIdx++)
    {
        free (alfSet->trainingDistortion[filterIdx]);
    }

    free (alfSet->funcSet);
    free (alfSet);

}

static SINT32 Xin266ConstructPicJob (
    xin_pic_struct *picSet)
{
    xin_seq_struct *seqSet;
    xin_job_desc   *jobEnc;
    xin_job_desc   *jobSao;
    xin_job_desc   *jobLpf;
    xin_job_desc   *jobAlfStat;
    xin_job_desc   *jobAlf;
    xin_job_desc   *jobCcAlfStat;
    xin_job_desc   *jobCcAlf;
    xin_job_desc   *jobDeriveAlf;
    xin_job_desc   *jobDeriveCcAlf;

    seqSet         = picSet->seqSet;
    jobEnc         = NULL;
    jobSao         = NULL;
    jobLpf         = NULL;
    jobAlfStat     = NULL;
    jobAlf         = NULL;
    jobCcAlfStat   = NULL;
    jobCcAlf       = NULL;
    jobDeriveAlf   = NULL;
    jobDeriveCcAlf = NULL;

    Xin26xJobCreate (
        &jobEnc,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobSao,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobLpf,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobAlfStat,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobAlf,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobCcAlfStat,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobCcAlf,
        seqSet->frameSizeInCtu,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobDeriveAlf,
        1,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobDeriveCcAlf,
        1,
        XIN_MAX_DEP_JOB_NUM);

    picSet->jobCtuEnc       = jobEnc;
    picSet->jobCtuSao       = jobSao;
    picSet->jobCtuLpf       = jobLpf;
    picSet->jobCtuAlfStat   = jobAlfStat;
    picSet->jobCtuAlf       = jobAlf;
    picSet->jobCtuCcAlfStat = jobCcAlfStat;
    picSet->jobCtuCcAlf     = jobCcAlf;
    picSet->jobDeriveAlf    = jobDeriveAlf;
    picSet->jobDeriveCcAlf  = jobDeriveCcAlf;

    return XIN_SUCCESS;

}

static void Xin266DestructPicJob (
    xin_pic_struct *picSet)
{
    xin_seq_struct *seqSet;
    xin_job_desc   *jobEnc;
    xin_job_desc   *jobSao;
    xin_job_desc   *jobLpf;
    xin_job_desc   *jobAlfStat;
    xin_job_desc   *jobAlf;
    xin_job_desc   *jobCcAlfStat;
    xin_job_desc   *jobCcAlf;
    xin_job_desc   *jobDeriveAlf;
    xin_job_desc   *jobDeriveCcAlf;

    seqSet         = picSet->seqSet;
    jobEnc         = picSet->jobCtuEnc;
    jobSao         = picSet->jobCtuSao;
    jobLpf         = picSet->jobCtuLpf;
    jobAlfStat     = picSet->jobCtuAlfStat;
    jobAlf         = picSet->jobCtuAlf;
    jobCcAlfStat   = picSet->jobCtuCcAlfStat;
    jobCcAlf       = picSet->jobCtuCcAlf;
    jobDeriveAlf   = picSet->jobDeriveAlf;
    jobDeriveCcAlf = picSet->jobDeriveCcAlf;

    Xin26xJobDelete (
        jobEnc,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobSao,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobLpf,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobAlfStat,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobAlf,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobCcAlfStat,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobCcAlf,
        seqSet->frameSizeInCtu);

    Xin26xJobDelete (
        jobDeriveAlf,
        1);

    Xin26xJobDelete (
        jobDeriveCcAlf,
        1);

}

SINT32 Xin266PicCreate (
    xin_pic_struct **dblPicSet,
    xin_seq_struct *seqSet)
{
    xin_pic_struct    *picSet;
    xin_bs_struct     *bitstream;
    xin_mv_u          *mvdMap;
    xin_affine_mv     *affineMvMap;
    UINT32            rowIdx;

    XIN_MALLOC_CHECK (picSet, sizeof(xin_pic_struct));

    *dblPicSet = picSet;

    memset (picSet, 0, sizeof(xin_pic_struct));

    picSet->seqSet = seqSet;

    if (Xin266ContructPicCtu (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (Xin266ConstructPicJob (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (Xin266CreateLmcs (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (seqSet->config.offlineMode)
    {
        Xin266ContructPicCoef (
            picSet);

        XIN_MALLOC_CHECK (picSet->secSet, sizeof(xin_sec_struct));

    }

    if (Xin266ConstructEntropy (picSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    // Malloc bitstream memory for vps sps and pps
    XIN_MALLOC_CHECK (bitstream,       sizeof(xin_bs_struct));
    XIN_MALLOC_CHECK (bitstream->base, XIN_ENTROPY_HEADER_SIZE);

    // Malloc sub mvd map
    XIN_MALLOC_CHECK (mvdMap, sizeof(xin_mv_u)*seqSet->blockSetHeight*seqSet->blockSetWidth);

    bitstream->size      = XIN_ENTROPY_HEADER_SIZE;
    picSet->bitstream    = bitstream;
    picSet->subMvdMap    = mvdMap;
    picSet->subMvdStride = seqSet->blockSetWidth;

    XIN_MALLOC_CHECK (picSet->saoTopBuffer[PLANE_LUMA],     (seqSet->frameHeightInCtu*seqSet->frameWidth + 2)*sizeof(PIXEL));
    XIN_MALLOC_CHECK (picSet->saoTopBuffer[PLANE_CHROMA_U], (seqSet->frameHeightInCtu*(seqSet->frameWidth>>1) + 2)*sizeof(PIXEL));
    XIN_MALLOC_CHECK (picSet->saoTopBuffer[PLANE_CHROMA_V], (seqSet->frameHeightInCtu*(seqSet->frameWidth>>1) + 2)*sizeof(PIXEL));

    picSet->saoTopBuf[PLANE_LUMA]     = picSet->saoTopBuffer[PLANE_LUMA] + 1;
    picSet->saoTopBuf[PLANE_CHROMA_U] = picSet->saoTopBuffer[PLANE_CHROMA_U] + 1;
    picSet->saoTopBuf[PLANE_CHROMA_V] = picSet->saoTopBuffer[PLANE_CHROMA_V] + 1;

    picSet->firstCuChunk   = NULL;
    picSet->lastCuChunk    = NULL;
    picSet->firstTuChunk   = NULL;
    picSet->lastTuChunk    = NULL;

    picSet->listLock = Xin26xConstructLock ();

    for (rowIdx = 0; rowIdx < seqSet->tileNum; rowIdx++)
    {
        picSet->cuList[rowIdx]     = NULL;
        picSet->cuListSize[rowIdx] = 0;
        picSet->cuListIdx[rowIdx]  = 0;

        picSet->tuList[rowIdx]     = NULL;
        picSet->tuListSize[rowIdx] = 0;
        picSet->tuListIdx[rowIdx]  = 0;
    }

    // Sao backup memory
    for (rowIdx = 0; rowIdx < seqSet->frameHeightInCtu; rowIdx++)
    {
        XIN_MALLOC_CHECK (picSet->saoLftBuf[rowIdx][PLANE_LUMA],     seqSet->frameWidthInCtu*(seqSet->ctuSize+1)*sizeof(PIXEL));
        XIN_MALLOC_CHECK (picSet->saoLftBuf[rowIdx][PLANE_CHROMA_U], seqSet->frameWidthInCtu*((seqSet->ctuSize>>1) + 1)*sizeof(PIXEL));
        XIN_MALLOC_CHECK (picSet->saoLftBuf[rowIdx][PLANE_CHROMA_V], seqSet->frameWidthInCtu*((seqSet->ctuSize>>1) + 1)*sizeof(PIXEL));
    }

    if (seqSet->config.enableAlf)
    {
        XIN_MALLOC_CHECK (picSet->alfSet, sizeof(xin_alf_struct));

        if (Xin266AlfCreate ( picSet->alfSet, seqSet))
        {
            return XIN_FAIL;
        }
    }

    if (seqSet->config.enableAffine)
    {
        XIN_MALLOC_CHECK (affineMvMap, sizeof(xin_affine_mv)*seqSet->blockSetHeight*seqSet->blockSetWidth);

        picSet->affineMvMap    = affineMvMap;
        picSet->affineMvStride = seqSet->blockSetWidth;
    }

    picSet->funcSet = seqSet->funcSet;

    return XIN_SUCCESS;

}

void Xin266DeleteCuChunk (
    xin_pic_struct *picSet)
{
    xin_cu_list *currCuChunk;
    xin_cu_list *nextCuChunk;

    currCuChunk = picSet->firstCuChunk;

    for (; currCuChunk != NULL;)
    {
        nextCuChunk = currCuChunk->nextList;

        free (currCuChunk->cuBuf);
        free (currCuChunk);

        currCuChunk = nextCuChunk;
    }

}

void Xin266DeleteTuChunk (
    xin_pic_struct *picSet)
{
    xin_tu_list *currTuChunk;
    xin_tu_list *nextTuChunk;

    currTuChunk = picSet->firstTuChunk;

    for (; currTuChunk != NULL;)
    {
        nextTuChunk = currTuChunk->nextList;

        free (currTuChunk->tuBuf);
        free (currTuChunk);

        currTuChunk = nextTuChunk;
    }

}

void Xin266PicDelete (
    xin_pic_struct *picSet)
{
    xin_seq_struct *seqSet;
    UINT32         tileIdx;

    seqSet = picSet->seqSet;

    free (picSet->ctu);

    // Malloc bitstream memory for vps sps and pps
    free (picSet->bitstream->base);
    free (picSet->bitstream);

    free (picSet->subMvdMap);

    free (picSet->saoTopBuffer[PLANE_LUMA]);
    free (picSet->saoTopBuffer[PLANE_CHROMA_U]);
    free (picSet->saoTopBuffer[PLANE_CHROMA_V]);

    for (tileIdx = 0; tileIdx < seqSet->frameHeightInCtu; tileIdx++)
    {
        free (picSet->saoLftBuf[tileIdx][PLANE_LUMA]);
        free (picSet->saoLftBuf[tileIdx][PLANE_CHROMA_U]);
        free (picSet->saoLftBuf[tileIdx][PLANE_CHROMA_V]);
    }

    for (tileIdx = 0; tileIdx < seqSet->tileNum; tileIdx++)
    {
        free (picSet->cabacSet[tileIdx]->cabac.bitstream.base);
        free (picSet->cabacSet[tileIdx]);
    }

    if (seqSet->config.enableAlf)
    {
        Xin266AlfDelete (
            picSet->alfSet,
            seqSet);
    }

    if (seqSet->config.enableLmcs)
    {
        free (picSet->lmcsSet->tempBuffer);
        free (picSet->lmcsSet);
    }

    if (seqSet->config.enableAffine)
    {
        free (picSet->affineMvMap);
    }

    if (seqSet->config.offlineMode)
    {
        Xin266DestructPicCoef (
            picSet);

        free (picSet->secSet);
    }

    Xin266DestructPicJob (
        picSet);

    Xin266DeleteCuChunk (
        picSet);

    Xin266DeleteTuChunk (
        picSet);

    Xin26xDestructLock (
        picSet->listLock);

    for (tileIdx = 0; tileIdx < seqSet->tileNum; tileIdx++)
    {
        free (picSet->cuList[tileIdx]);

        free (picSet->tuList[tileIdx]);
    }

    free (picSet);

}

static SINT32 Xin266GetSearchRange (
    SINT32 frameWidth,
    SINT32 frameHeight)
{
    SINT32 searchRange;
    SINT32 minSize;

    searchRange = 0;
    minSize = XIN_MIN(frameWidth, frameHeight);

    while ((minSize << searchRange) < 1023)
    {
        ++searchRange;
    }

    return searchRange;
}

static SINT32 Xin266LookaheadMeCreate (
    xin_la_section *laSection,
    xin_seq_struct *seqSet)
{
    SINT32        stageIndex;
    SINT16        radius;
    SINT16        tanRadius;
    SINT32        searchPts;
    PIXEL         *halfPelH;
    PIXEL         *halfPelV;
    PIXEL         *halfPelHv;
    PIXEL         *integPel;
    xin_me_struct *meSet;

    XIN_MALLOC_CHECK (meSet, sizeof(xin_me_struct));
    XIN_MALLOC_CHECK (meSet->interpBuf, sizeof(PIXEL)*XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE * 4);
    XIN_MALLOC_CHECK (meSet->input1,    sizeof(PIXEL)*XIN_MAX_CTU_SIZE*XIN_MAX_CTU_SIZE / 4);
    XIN_MALLOC_CHECK (meSet->input2,    sizeof(PIXEL)*XIN_MAX_CTU_SIZE*XIN_MAX_CTU_SIZE / 16);

    Xin26xMeFuncInit (
        meSet,
        seqSet->cpuFeature);

    meSet->input1Stride = XIN_MAX_CTU_SIZE / 2;
    meSet->input2Stride = XIN_MAX_CTU_SIZE / 4;
    meSet->interpStride = XIN_ME_BUF_STRIDE;

    meSet->biMe           = FALSE;
    meSet->iMvMode        = 0;
    meSet->width          = seqSet->laUnitSize;
    meSet->height         = seqSet->laUnitSize;
    meSet->oneDimMe       = FALSE;
    meSet->skippedSR      = 4;
    meSet->refinedSR      = Xin266GetSearchRange(seqSet->frameWidth, seqSet->frameHeight);
    stageIndex            = XIN_MAX_SEARCH_ROUND - 1;
    meSet->numSearchRound = XIN_MAX_SEARCH_ROUND;
    meSet->searchHeight   = 64;
    meSet->searchWidth    = 64;

    for (radius = XIN_MAX_FIRST_STEP; radius > 0; radius /= 2)
    {
        // Generate offsets for 8 search sites per step.
        tanRadius = XIN_MAX((SINT16)(0.41 * radius), 1);
        searchPts = (radius == 1) ? 8 : 12;

        meSet->searchOffset[stageIndex][0].mvY = 0;
        meSet->searchOffset[stageIndex][0].mvX = 0;
        meSet->searchOffset[stageIndex][1].mvY = -radius;
        meSet->searchOffset[stageIndex][1].mvX = 0;
        meSet->searchOffset[stageIndex][2].mvY = radius;
        meSet->searchOffset[stageIndex][2].mvX = 0;
        meSet->searchOffset[stageIndex][3].mvY = 0;
        meSet->searchOffset[stageIndex][3].mvX = -radius;
        meSet->searchOffset[stageIndex][4].mvY = 0;
        meSet->searchOffset[stageIndex][4].mvX = radius;
        meSet->searchOffset[stageIndex][5].mvY = -radius;
        meSet->searchOffset[stageIndex][5].mvX = -tanRadius;
        meSet->searchOffset[stageIndex][6].mvY = radius;
        meSet->searchOffset[stageIndex][6].mvX = tanRadius;
        meSet->searchOffset[stageIndex][7].mvY = -tanRadius;
        meSet->searchOffset[stageIndex][7].mvX = radius;
        meSet->searchOffset[stageIndex][8].mvY = tanRadius;
        meSet->searchOffset[stageIndex][8].mvX = -radius;
        meSet->searchOffset[stageIndex][9].mvY = -radius;
        meSet->searchOffset[stageIndex][9].mvX = tanRadius;
        meSet->searchOffset[stageIndex][10].mvY = radius;
        meSet->searchOffset[stageIndex][10].mvX = -tanRadius;
        meSet->searchOffset[stageIndex][11].mvY = tanRadius;
        meSet->searchOffset[stageIndex][11].mvX = radius;
        meSet->searchOffset[stageIndex][12].mvY = -tanRadius;
        meSet->searchOffset[stageIndex][12].mvX = -radius;

        meSet->searchPoints[stageIndex] = searchPts;
        meSet->searchRadius[stageIndex] = radius;

        --stageIndex;

    }

    integPel  = meSet->interpBuf;
    halfPelH  = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE;
    halfPelV  = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE * 2;
    halfPelHv = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE * 3;

    meSet->integPel  = integPel;
    meSet->halfPelH  = halfPelH;
    meSet->halfPelV  = halfPelV;
    meSet->halfPelHv = halfPelHv;

    halfPelH += (XIN_ME_FILTER_TAP >> 1)*XIN_ME_BUF_STRIDE;
    halfPelV += 1;
    integPel += XIN_ME_BUF_STRIDE + 1;

    meSet->halfPel[LFT_POS]     = halfPelH;
    meSet->halfPel[RGT_POS]     = halfPelH + 1;
    meSet->halfPel[TOP_POS]     = halfPelV;
    meSet->halfPel[BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->halfPel[TOP_LFT_POS] = halfPelHv;
    meSet->halfPel[TOP_RGT_POS] = halfPelHv + 1;
    meSet->halfPel[BOT_LFT_POS] = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->halfPel[BOT_RGT_POS] = halfPelHv + XIN_ME_BUF_STRIDE + 1;

    // The following is seting quarter pixel motion estimation buffer
    meSet->qBufA[LFT_POS][LFT_POS]     = integPel - 1;
    meSet->qBufB[LFT_POS][LFT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][RGT_POS]     = integPel;
    meSet->qBufB[LFT_POS][RGT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][TOP_POS]     = halfPelHv;
    meSet->qBufB[LFT_POS][TOP_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufB[LFT_POS][BOT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][TOP_LFT_POS] = halfPelV - 1;
    meSet->qBufB[LFT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufB[LFT_POS][TOP_RGT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[LFT_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[LFT_POS][BOT_RGT_POS] = halfPelH;

    meSet->qBufA[RGT_POS][LFT_POS]     = integPel;
    meSet->qBufB[RGT_POS][LFT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][RGT_POS]     = integPel + 1;
    meSet->qBufB[RGT_POS][RGT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_POS]     = halfPelHv + 1;
    meSet->qBufB[RGT_POS][TOP_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[RGT_POS][BOT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[RGT_POS][TOP_LFT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_RGT_POS] = halfPelV + 1;
    meSet->qBufB[RGT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[RGT_POS][BOT_LFT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[RGT_POS][BOT_RGT_POS] = halfPelH + 1;

    meSet->qBufA[TOP_POS][LFT_POS]     = halfPelHv;
    meSet->qBufB[TOP_POS][LFT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][RGT_POS]     = halfPelHv + 1;
    meSet->qBufB[TOP_POS][RGT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][TOP_POS]     = integPel - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_POS][TOP_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][BOT_POS]     = integPel;
    meSet->qBufB[TOP_POS][BOT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufB[TOP_POS][BOT_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][BOT_RGT_POS] = halfPelH + 1;
    meSet->qBufB[TOP_POS][BOT_RGT_POS] = halfPelV;

    meSet->qBufA[BOT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_POS]     = integPel;
    meSet->qBufB[BOT_POS][TOP_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_POS]     = integPel + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufB[BOT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufB[BOT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_RGT_POS] = halfPelH + 1 + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;

    meSet->qBufA[TOP_LFT_POS][LFT_POS]     = halfPelV - 1;
    meSet->qBufB[TOP_LFT_POS][LFT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][RGT_POS]     = halfPelV;
    meSet->qBufB[TOP_LFT_POS][RGT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][TOP_POS]     = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_LFT_POS][TOP_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][BOT_POS]     = halfPelH;
    meSet->qBufB[TOP_LFT_POS][BOT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][TOP_LFT_POS] = halfPelV - 1;
    meSet->qBufB[TOP_LFT_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufA[TOP_LFT_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_LFT_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufA[TOP_LFT_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufB[TOP_LFT_POS][BOT_LFT_POS] = halfPelV - 1;
    meSet->qBufA[TOP_LFT_POS][BOT_RGT_POS] = halfPelV;
    meSet->qBufB[TOP_LFT_POS][BOT_RGT_POS] = halfPelH;

    meSet->qBufA[TOP_RGT_POS][LFT_POS]     = halfPelV;
    meSet->qBufB[TOP_RGT_POS][LFT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][RGT_POS]     = halfPelV + 1;
    meSet->qBufB[TOP_RGT_POS][RGT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_POS]     = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_RGT_POS][TOP_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][BOT_POS]     = halfPelH + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[TOP_RGT_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_RGT_POS][TOP_RGT_POS] = halfPelV + 1;
    meSet->qBufA[TOP_RGT_POS][BOT_LFT_POS] = halfPelH + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_RGT_POS][BOT_RGT_POS] = halfPelV + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_RGT_POS] = halfPelH + 1;

    meSet->qBufA[BOT_LFT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[BOT_LFT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][TOP_POS]     = halfPelH;
    meSet->qBufB[BOT_LFT_POS][TOP_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][BOT_POS]     = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[BOT_LFT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[BOT_LFT_POS][TOP_RGT_POS] = halfPelH;
    meSet->qBufB[BOT_LFT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufA[BOT_LFT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_RGT_POS] = halfPelH + XIN_ME_BUF_STRIDE;

    meSet->qBufA[BOT_RGT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_RGT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_POS]     = halfPelH + 1;
    meSet->qBufB[BOT_RGT_POS][TOP_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][BOT_POS]     = halfPelH + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_RGT_POS][TOP_LFT_POS] = halfPelH + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufB[BOT_RGT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_RGT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_RGT_POS] = halfPelH + XIN_ME_BUF_STRIDE + 1;

    meSet->qBufA[CEN_POS][LFT_POS]     = halfPelH;
    meSet->qBufB[CEN_POS][LFT_POS]     = integPel;
    meSet->qBufA[CEN_POS][RGT_POS]     = halfPelH + 1;
    meSet->qBufB[CEN_POS][RGT_POS]     = integPel;
    meSet->qBufA[CEN_POS][TOP_POS]     = halfPelV;
    meSet->qBufB[CEN_POS][TOP_POS]     = integPel;
    meSet->qBufA[CEN_POS][BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_POS]     = integPel;
    meSet->qBufA[CEN_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[CEN_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[CEN_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufB[CEN_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufA[CEN_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufA[CEN_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_RGT_POS] = halfPelH + 1;

    laSection->meSet = meSet;

    return XIN_SUCCESS;

}

SINT32 Xin266LookaheadCreate (
    xin266_encoder_struct *h266Encoder,
    xin_seq_struct        *seqSet)
{
    xin_la_struct  *laSet;
    xin_la_section *laSection;
    UINT32         laUnitSize;
    UINT32         lgUnitSize;
    UINT32         sectionIdx;
    xin_job_desc   *jobSubSample;
    xin_job_desc   *jobLookahead;
    xin_job_desc   *jobPostInit;

    jobSubSample = NULL;
    jobLookahead = NULL;
    jobPostInit  = NULL;

    XIN_MALLOC_CHECK (laSet, sizeof(xin_la_struct));

    memset(laSet, 0, sizeof(xin_la_struct));

    h266Encoder->laSet = laSet;

    XIN_MALLOC_CHECK (laSet->funcSet, sizeof(xin_la_func));

    Xin26xJobCreate (
        &jobSubSample,
        1,
        XIN_MAX_DEP_JOB_NUM);

    Xin26xJobCreate (
        &jobPostInit,
        1,
        XIN_MAX_DEP_JOB_NUM);

    laUnitSize = seqSet->laUnitSize;
    lgUnitSize = calcLog2[seqSet->laUnitSize];

    laSet->jobSubSample = jobSubSample;
    laSet->jobPostInit  = jobPostInit;
    laSet->frameHeight  = seqSet->lowerHeight;
    laSet->frameWidth   = seqSet->lowerWidth;
    laSet->frameStride  = laSet->frameWidth + (XIN_LOWER_PADDING_X + 4)*2;
    laSet->wdtInUnit    = laSet->frameWidth >> lgUnitSize;
    laSet->hgtInUnit    = laSet->frameHeight >> lgUnitSize;
    laSet->totalUnit    = laSet->wdtInUnit * laSet->hgtInUnit;
    laSet->lgUnitSize   = lgUnitSize;
    laSet->laUnitSize   = laUnitSize;
    laSet->sectionNum   = 2;
    laSet->laSatdMd     = seqSet->config.laSatdMd;
    laSet->laSubMe      = seqSet->config.laSubMe;
    laSet->threadQueue  = seqSet->threadQueue;

    for (sectionIdx = 0; sectionIdx < laSet->sectionNum; sectionIdx++)
    {
        XIN_MALLOC_CHECK (laSection, sizeof(xin_la_section));

        Xin26xJobCreate (
            &jobLookahead,
            1,
            XIN_MAX_DEP_JOB_NUM);

        laSection->laSet = laSet;

        laSet->laSection[sectionIdx]    = laSection;
        laSet->jobLookahead[sectionIdx] = jobLookahead;

        if (Xin266LookaheadMeCreate(laSection, seqSet))
        {
            return XIN_FAIL;
        }
    }

    Xin26xLookaheadFuncInit (
        laSet,
        seqSet->cpuFeature);

    return XIN_SUCCESS;

}

void Xin266LookaheadDelete (
    xin_la_struct *laSet)
{
    UINT32         sectionIdx;
    xin_la_section *laSection;
    xin_job_desc   *jobSubSample;
    xin_job_desc   *jobPostInit;
    xin_job_desc   *jobLookahead;

    jobSubSample = laSet->jobSubSample;
    jobPostInit  = laSet->jobPostInit;

    Xin26xJobDelete (
        jobSubSample,
        1);

    Xin26xJobDelete (
        jobPostInit,
        1);

    free (laSet->funcSet);

    for (sectionIdx = 0; sectionIdx < laSet->sectionNum; sectionIdx++)
    {
        laSection    = laSet->laSection[sectionIdx];
        jobLookahead = laSet->jobLookahead[sectionIdx];

        Xin26xJobDelete (
            jobLookahead,
            1);

        free (laSection->meSet->interpBuf);
        free (laSection->meSet->input1);
        free (laSection->meSet->input2);
        free (laSection->meSet);

        free(laSection);
    }

    free(laSet);

}

static void Xin266FuncUpdate (
    xin_func_struct *funcSet,
    xin_seq_struct  *seqSet)
{
    if (seqSet->config.satdMd)
    {
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_1xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_2xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_4xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_8xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_16xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_32xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_64xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeAvgSatd[XIN_BLOCK_128xH];

        funcSet->pfXinComputeDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_1xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_2xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_4xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_8xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeSatd[XIN_BLOCK_16xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeSatd[XIN_BLOCK_32xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeSatd[XIN_BLOCK_64xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeSatd[XIN_BLOCK_128xH];

        funcSet->pfXinComputeWeightDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_1xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_2xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_4xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_8xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_16xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_32xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_64xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeWeightSatd[XIN_BLOCK_128xH];

    }
    else
    {
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeAvgSad[XIN_BLOCK_1xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeAvgSad[XIN_BLOCK_2xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeAvgSad[XIN_BLOCK_4xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeAvgSad[XIN_BLOCK_8xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeAvgSad[XIN_BLOCK_16xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeAvgSad[XIN_BLOCK_32xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeAvgSad[XIN_BLOCK_64xH];
        funcSet->pfXinComputeAvgDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeAvgSad[XIN_BLOCK_128xH];

        funcSet->pfXinComputeDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_1xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_2xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_4xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_8xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeSad[XIN_BLOCK_16xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeSad[XIN_BLOCK_32xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeSad[XIN_BLOCK_64xH];
        funcSet->pfXinComputeDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeSad[XIN_BLOCK_128xH];

        funcSet->pfXinComputeWeightDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeWeightSad[XIN_BLOCK_1xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeWeightSad[XIN_BLOCK_2xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeWeightSad[XIN_BLOCK_4xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeWeightSad[XIN_BLOCK_8xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeWeightSad[XIN_BLOCK_16xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeWeightSad[XIN_BLOCK_32xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeWeightSad[XIN_BLOCK_64xH];
        funcSet->pfXinComputeWeightDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeWeightSad[XIN_BLOCK_128xH];

    }

    if (seqSet->config.laSatdMd)
    {
        funcSet->pfXinLaComputeDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_1xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_2xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_4xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeSatd[XIN_BLOCK_8xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeSatd[XIN_BLOCK_16xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeSatd[XIN_BLOCK_32xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeSatd[XIN_BLOCK_64xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeSatd[XIN_BLOCK_128xH];
    }
    else
    {
        funcSet->pfXinLaComputeDist[XIN_BLOCK_1xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_1xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_2xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_2xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_4xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_4xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_8xH]   = funcSet->pfXinComputeSad[XIN_BLOCK_8xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_16xH]  = funcSet->pfXinComputeSad[XIN_BLOCK_16xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_32xH]  = funcSet->pfXinComputeSad[XIN_BLOCK_32xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_64xH]  = funcSet->pfXinComputeSad[XIN_BLOCK_64xH];
        funcSet->pfXinLaComputeDist[XIN_BLOCK_128xH] = funcSet->pfXinComputeSad[XIN_BLOCK_128xH];
    }

    if (seqSet->config.fastRateEst)
    {
        funcSet->pfXinEstimateTuCoeff = Xin266EstimateFastTuCoeff;
    }
    else
    {
        funcSet->pfXinEstimateTuCoeff = Xin266EstimateFullTuCoeff;
    }

}

static SINT32 Xin266MctfCreate (
    xin266_encoder_struct *h266Encoder,
    xin_seq_struct        *seqSet)
{
    UINT32            refIdx;
    UINT32            sectionIdx;
    SINT32            frameWidth;
    SINT32            frameHeight;
    SINT32            frameL1Width;
    SINT32            frameL1Height;
    SINT32            frameL2Width;
    SINT32            frameL2Height;
    SINT32            wdtInUnit;
    SINT32            hgtInUnit;
    SINT32            wdtL0InUnit;
    SINT32            wdtL1InUnit;
    SINT32            wdtL2InUnit;
    SINT32            hgtL0InUnit;
    SINT32            hgtL1InUnit;
    SINT32            hgtL2InUnit;
    intptr_t          l1BufSize;
    intptr_t          l2BufSize;
    intptr_t          l1BufStride;
    intptr_t          l2BufStride;
    SINT32            marginX;
    SINT32            marginY;
    xin_mctf_struct   *mctfSet;
    xin_mctf_mc       *mctfMc;
    xin_mctf_picture  *mctfPicture;
    xin_job_desc      *jobSubSample;
    xin_job_desc      *jobMctfMe;
    xin_job_desc      *jobMctfMc;
    xin_job_desc      *jobMctfVar;
    xin_job_desc      *jobMctfBim; 
    UINT32            sectionNum;
    UINT32            searchIdx;
    SINT32            mctfUnitSize;

    if (!seqSet->config.enableMctf)
    {
        return XIN_SUCCESS;
    }

    mctfUnitSize  = seqSet->config.mctfUnitSize;
    frameWidth    = seqSet->frameWidth;
    frameHeight   = seqSet->frameHeight;
    marginX       = XIN_INPUT_PADDING_X + 16;
    marginY       = XIN_INPUT_PADDING_X + 16;
    frameL1Width  = frameWidth >> 1;
    frameL1Height = frameHeight >> 1;
    frameL2Width  = frameL1Width >> 1;
    frameL2Height = frameL1Height >> 1;
    wdtInUnit     = frameWidth / mctfUnitSize + 1;
    hgtInUnit     = frameHeight / mctfUnitSize + 1;
    wdtL0InUnit   = frameWidth / (mctfUnitSize*2) + 1;
    hgtL0InUnit   = frameHeight / (mctfUnitSize*2) + 1;
    wdtL1InUnit   = frameWidth / (mctfUnitSize*4);
    hgtL1InUnit   = frameL1Height / (mctfUnitSize*2);
    wdtL2InUnit   = frameWidth / (mctfUnitSize*8);
    hgtL2InUnit   = frameL2Height / (mctfUnitSize*2);
    l1BufStride   = (frameL1Width + marginX*2);
    l2BufStride   = (frameL2Width + marginX*2);
    l1BufSize     = l1BufStride*(frameL1Height + marginY*2);
    l2BufSize     = l2BufStride*(frameL2Height + marginY*2);
    sectionNum    = (frameWidth*frameHeight >= 1920 * 1080) ? 4 : 2;
    jobMctfMe     = NULL;
    jobMctfMc     = NULL;
    jobSubSample  = NULL;
    jobMctfVar    = NULL;
    jobMctfBim    = NULL;

    XIN_MALLOC_CHECK (mctfSet, sizeof(xin_mctf_struct));

    memset (mctfSet, 0, sizeof(xin_mctf_struct));

    XIN_MALLOC_CHECK (mctfSet->funcSet, sizeof(xin_mctf_func));

    for (refIdx = 0; refIdx < seqSet->config.mctfRefNum; refIdx++)
    {
        for (searchIdx = 0; searchIdx < XIN_TF_ME_LEVEL_NUM; searchIdx++)
        {
            for (sectionIdx = 0; sectionIdx < XIN_TF_MAX_SECTION_NUM; sectionIdx++)
            {
                XIN_MALLOC_CHECK (mctfSet->mctfMe[refIdx][searchIdx][sectionIdx], sizeof(xin_mctf_me));

                Xin26xJobCreate (
                    &jobMctfMe,
                    1,
                    XIN_MAX_DEP_JOB_NUM);

                mctfSet->jobMctfMe[refIdx][searchIdx][sectionIdx] = jobMctfMe;

            }
        }
    }

    for (sectionIdx = 0; sectionIdx < XIN_TF_MAX_SECTION_NUM; sectionIdx++)
    {
        XIN_MALLOC_CHECK (mctfMc, sizeof(xin_mctf_mc));

        for (refIdx = 0; refIdx < seqSet->config.mctfRefNum; refIdx++)
        {
            XIN_MALLOC_CHECK (mctfMc->filter[refIdx][0], sizeof(PIXEL)*mctfUnitSize*mctfUnitSize*2);
            XIN_MALLOC_CHECK (mctfMc->filter[refIdx][1], sizeof(PIXEL)*mctfUnitSize*mctfUnitSize*2 / 4);
            XIN_MALLOC_CHECK (mctfMc->filter[refIdx][2], sizeof(PIXEL)*mctfUnitSize*mctfUnitSize*2 / 4);
        }

        mctfSet->mctfMc[sectionIdx] = mctfMc;
    }

    for (refIdx = 0; refIdx < seqSet->config.mctfRefNum; refIdx++)
    {
        XIN_MALLOC_CHECK (mctfSet->mv[refIdx],    wdtInUnit*hgtInUnit*sizeof(xin_mv32_u));
        XIN_MALLOC_CHECK (mctfSet->l0Mv[refIdx],  wdtL0InUnit*hgtL0InUnit*sizeof(xin_mv32_u));
        XIN_MALLOC_CHECK (mctfSet->l1Mv[refIdx],  wdtL1InUnit*hgtL1InUnit*sizeof(xin_mv32_u));
        XIN_MALLOC_CHECK (mctfSet->l2Mv[refIdx],  wdtL2InUnit*hgtL2InUnit*sizeof(xin_mv32_u));
        XIN_MALLOC_CHECK (mctfSet->error[refIdx], wdtInUnit*hgtInUnit*sizeof(SINT32));

        memset (mctfSet->mv[refIdx],    0,  wdtInUnit*hgtInUnit*sizeof(xin_mv32_u));
        memset (mctfSet->l0Mv[refIdx],  0,  wdtL0InUnit*hgtL0InUnit*sizeof(xin_mv32_u));
        memset (mctfSet->l1Mv[refIdx],  0,  wdtL1InUnit*hgtL1InUnit*sizeof(xin_mv32_u));
        memset (mctfSet->l2Mv[refIdx],  0,  wdtL2InUnit*hgtL2InUnit*sizeof(xin_mv32_u));
        memset (mctfSet->error[refIdx], 0,  wdtInUnit*hgtInUnit*sizeof(SINT32));
    }

    for (sectionIdx = 0; sectionIdx < sectionNum; sectionIdx++)
    {
        Xin26xJobCreate (
            &jobMctfMc,
            1,
            XIN_MAX_DEP_JOB_NUM);

        mctfSet->jobMctfMc[sectionIdx] = jobMctfMc;
    }

    XIN_MALLOC_CHECK (mctfPicture, sizeof(xin_mctf_picture));

    XIN_MALLOC_CHECK (mctfPicture->inputSub2Buf, l1BufSize*sizeof(PIXEL));

    XIN_MALLOC_CHECK (mctfPicture->inputSub4Buf, l2BufSize*sizeof(PIXEL));

    XIN_MALLOC_CHECK (mctfSet->variance, wdtInUnit*hgtInUnit*sizeof(SINT32));

    XIN_MALLOC_CHECK (mctfSet->unitDqpMap, wdtInUnit*hgtInUnit*sizeof(double));

    Xin26xJobCreate (
        &jobSubSample,
        1,
        XIN_MAX_DEP_JOB_NUM*2);

    Xin26xJobCreate (
        &jobMctfVar,
        1,
        XIN_MAX_DEP_JOB_NUM*2);

    Xin26xJobCreate (
        &jobMctfBim,
        1,
        XIN_MAX_DEP_JOB_NUM);

    mctfPicture->mctfSet         = mctfSet;
    mctfPicture->marginX         = marginX;
    mctfPicture->marginY         = marginY;
    mctfPicture->inputSub2       = mctfPicture->inputSub2Buf + marginX + marginY*l1BufStride;
    mctfPicture->inputSub4       = mctfPicture->inputSub4Buf + marginX + marginY*l2BufStride;
    mctfPicture->inputWidth      = frameWidth;
    mctfPicture->inputHeight     = frameHeight;
    mctfPicture->inputSub2Width  = frameL1Width;
    mctfPicture->inputSub2Height = frameL1Height;
    mctfPicture->inputSub2Stride = l1BufStride;
    mctfPicture->inputSub4Width  = frameL2Width;
    mctfPicture->inputSub4Height = frameL2Height;
    mctfPicture->inputSub4Stride = l2BufStride;
    mctfPicture->jobSubSample    = jobSubSample;

    mctfSet->pictureWrite = mctfPicture;
    mctfSet->threadQueue  = seqSet->threadQueue;
    mctfSet->mctfRefNum   = seqSet->config.mctfRefNum;
    mctfSet->mctfUnitSize = seqSet->config.mctfUnitSize;
    mctfSet->ctuSize      = seqSet->ctuSize;

    for (refIdx = 0; refIdx < seqSet->config.mctfRefNum; refIdx++)
    {
        XIN_MALLOC_CHECK (mctfPicture, sizeof(xin_mctf_picture));

        XIN_MALLOC_CHECK (mctfPicture->inputSub2Buf, l1BufSize*sizeof(PIXEL));

        XIN_MALLOC_CHECK (mctfPicture->inputSub4Buf, l2BufSize*sizeof(PIXEL));

        Xin26xJobCreate (
            &jobSubSample,
            1,
            XIN_MAX_DEP_JOB_NUM*2);

        mctfSet->pictureRead[refIdx] = mctfPicture;
        mctfPicture->inputSub2       = mctfPicture->inputSub2Buf + marginX + marginY*l1BufStride;
        mctfPicture->inputSub4       = mctfPicture->inputSub4Buf + marginX + marginY*l2BufStride;
        mctfPicture->marginX         = marginX;
        mctfPicture->marginY         = marginY;
        mctfPicture->inputWidth      = frameWidth;
        mctfPicture->inputHeight     = frameHeight;
        mctfPicture->inputSub2Width  = frameL1Width;
        mctfPicture->inputSub2Height = frameL1Height;
        mctfPicture->inputSub2Stride = l1BufStride;
        mctfPicture->inputSub4Width  = frameL2Width;
        mctfPicture->inputSub4Height = frameL2Height;
        mctfPicture->inputSub4Stride = l2BufStride;
        mctfPicture->jobSubSample    = jobSubSample;
        mctfPicture->mctfSet         = mctfSet;

    }

    mctfSet->mvStride         = wdtInUnit;
    mctfSet->l0MvStride       = wdtL0InUnit;
    mctfSet->l1MvStride       = wdtL1InUnit;
    mctfSet->l2MvStride       = wdtL2InUnit;
    mctfSet->errorStride      = wdtInUnit;
    mctfSet->sectionNum       = sectionNum;
    mctfSet->mctfMode         = seqSet->config.mctfMode;
    mctfSet->inputWidth       = seqSet->config.inputWidth;
    mctfSet->inputHeight      = seqSet->config.inputHeight;
    mctfSet->varianceStride   = wdtInUnit;
    mctfSet->unitDqpMapStride = wdtInUnit;
    mctfSet->jobMctfVar       = jobMctfVar;
    mctfSet->jobMctfBim       = jobMctfBim;
    mctfSet->bimEnabled       = seqSet->config.enableBim;

    Xin26xMctfFuncInit (
        mctfSet,
        seqSet->cpuFeature);

    h266Encoder->mctfSet = mctfSet;

    return XIN_SUCCESS;

}

static SINT32 Xin266MctfDelete (
    xin_mctf_struct *mctfSet)
{
    UINT32           refIdx;
    UINT32           sectionIdx;
    SINT32           searchIdx;
    xin_mctf_picture *mctfPicture;
    xin_job_desc     *jobSubSample;
    xin_job_desc     *jobMctfMc;
    xin_job_desc     *jobMctfMe;
    xin_job_desc     *jobMctfVar;
    xin_mctf_mc      *mctfMc;

    for (refIdx = 0; refIdx < mctfSet->mctfRefNum; refIdx++)
    {
        mctfPicture  = mctfSet->pictureRead[refIdx];
        jobSubSample = mctfPicture->jobSubSample;

        Xin26xJobDelete (
            jobSubSample,
            1);

        free (mctfPicture->inputSub4Buf);
        free (mctfPicture->inputSub2Buf);
        free (mctfPicture);

    }

    mctfPicture  = mctfSet->pictureWrite;
    jobSubSample = mctfPicture->jobSubSample;
    jobMctfVar   = mctfSet->jobMctfVar;

    Xin26xJobDelete (
        jobSubSample,
        1);

    Xin26xJobDelete (
        jobMctfVar,
        1);

    free (mctfPicture->inputSub4Buf);
    free (mctfPicture->inputSub2Buf);
    free (mctfPicture);

    for (sectionIdx = 0; sectionIdx < mctfSet->sectionNum; sectionIdx++)
    {
        jobMctfMc = mctfSet->jobMctfMc[sectionIdx];

        Xin26xJobDelete (
            jobMctfMc,
            1);

        mctfMc = mctfSet->mctfMc[sectionIdx];

        for (refIdx = 0; refIdx < mctfSet->mctfRefNum; refIdx++)
        {
            free (mctfMc->filter[refIdx][0]);
            free (mctfMc->filter[refIdx][1]);
            free (mctfMc->filter[refIdx][2]);
        }

        free (mctfMc);

    }

    for (refIdx = 0; refIdx < mctfSet->mctfRefNum; refIdx++)
    {
        free (mctfSet->l2Mv[refIdx]);
        free (mctfSet->l1Mv[refIdx]);
        free (mctfSet->l0Mv[refIdx]);
        free (mctfSet->mv[refIdx]);
        free (mctfSet->error[refIdx]);
    }

    for (refIdx = 0; refIdx < mctfSet->mctfRefNum; refIdx++)
    {
        for (searchIdx = 0; searchIdx < XIN_TF_ME_LEVEL_NUM; searchIdx++)
        {
            for (sectionIdx = 0; sectionIdx < XIN_TF_MAX_SECTION_NUM; sectionIdx++)
            {
                jobMctfMe = mctfSet->jobMctfMe[refIdx][searchIdx][sectionIdx];

                Xin26xJobDelete (
                    jobMctfMe,
                    1);

                free (mctfSet->mctfMe[refIdx][searchIdx][sectionIdx]);
            }
        }
    }

    free (mctfSet->variance);
    free (mctfSet->unitDqpMap);
    free (mctfSet->funcSet);
    free (mctfSet);

    return XIN_SUCCESS;

}

static SINT32 Xin266SceneCutCreate (
    xin266_encoder_struct *h266Encoder,
    xin_seq_struct        *seqSet)
{
    xin_sc_struct *scSet;
    xin_job_desc  *jobSceneCut;

    XIN_MALLOC_CHECK (scSet, sizeof(xin_sc_struct));

    memset(scSet, 0, sizeof(xin_sc_struct));

    jobSceneCut = NULL;

    Xin26xJobCreate (
        &jobSceneCut,
        1,
        XIN_MAX_DEP_JOB_NUM);

    scSet->funcSet     = seqSet->funcSet;
    scSet->seqSet      = seqSet;
    scSet->jobSceneCut = jobSceneCut;
    h266Encoder->scSet = scSet;

    return XIN_SUCCESS;

}

static void Xin266SceneCutDelete (
    xin_sc_struct *scSet)
{
    xin_job_desc  *jobSceneCut;

    jobSceneCut = scSet->jobSceneCut;

    Xin26xJobDelete (
        jobSceneCut,
        1);

    free (scSet);
}

SINT32 Xin266ContructReconFrame (
    xin_frame_struct *reconFrame,
    xin_seq_struct   *seqSet)
{
    SINT32  frameWidth;
    SINT32  frameHeight;

    frameWidth  = seqSet->config.inputWidth;
    frameHeight = seqSet->config.inputHeight;

    XIN_MALLOC_CHECK (reconFrame->yuvBuf[0], sizeof(PIXEL)*frameWidth*frameHeight);
    XIN_MALLOC_CHECK (reconFrame->yuvBuf[1], sizeof(PIXEL)*frameWidth*frameHeight/4);
    XIN_MALLOC_CHECK (reconFrame->yuvBuf[2], sizeof(PIXEL)*frameWidth*frameHeight/4);

    reconFrame->lumaWidth    = frameWidth;
    reconFrame->lumaHeight   = frameHeight;
    reconFrame->lumaStride   = frameWidth;
    reconFrame->chromaStride = frameWidth/2;

    return XIN_SUCCESS;

}

SINT32 Xin266EncoderCreate (
    XIN_HANDLE     *encoderHandle,
    xin26x_params  *config)
{
    xin266_encoder_struct *h266Encoder;
    xin_seq_struct        *seqSet;
    xin_pic_struct        *picSet;
    xin_func_struct       *funcSet;
    xin_thread_queue      *threadQueue;
    UINT32                sectionIdx;
    UINT32                frameIdx;

    if (Xin266VerifyConfig(config) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    // malloc memory for encoder instance
    XIN_MALLOC_CHECK (h266Encoder, sizeof(xin266_encoder_struct));

    *encoderHandle = (XIN_HANDLE)h266Encoder;
    memset (h266Encoder, 0, sizeof(xin266_encoder_struct));

    XIN_MALLOC_CHECK (funcSet, sizeof(xin_func_struct));
    memset (funcSet, 0, sizeof(xin_func_struct));
    h266Encoder->funcSet = funcSet;

    XIN_MALLOC_CHECK (threadQueue, sizeof(xin_thread_queue));
    memset (threadQueue, 0, sizeof(xin_thread_queue));
    h266Encoder->threadQueue = threadQueue;

    XIN_MALLOC_CHECK (seqSet, sizeof(xin_seq_struct));
    memset (seqSet, 0, sizeof(xin_seq_struct));
    h266Encoder->seqSet = seqSet;

    Xin26xCpuDetection (
        &seqSet->cpuFeature,
        &seqSet->cpuCoreNum);

    _XIN_LOGGER (XIN_LOGGER_STATUS, "Xin26x lib build date: %s %s", __DATE__, __TIME__);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "yuv size %dx%d fps:%3.3f frame num:%d", config->inputWidth, config->inputHeight, config->frameRate, config->frameToBeEncoded);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "algorithm:%d encoder mode:%d rdoq:%d screen mode:%d", config->algorithmMode, config->encoderMode, config->enableRdoq, config->screenContentMode);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "smp:%d mctf:%d sao:%d deblock:%d alf:%d", config->enableSmp, config->enableMctf, config->enableSao, config->enableDeblock, config->enableAlf);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "tmvp:%d scenecut:%d cclm:%d dmvr:%d", config->enableTMvp, config->enableSceneCut, config->enableCclm, config->enableDmvr);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "sbTmvp:%d affine:%d mts:%d sign bit hidden:%d", config->enableSbTmvp, config->enableAffine, config->enableMts, config->enableSignDataHiding);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "ctuSize:%d, minCuSize:%d, minQtSize:%d, maxBtSize:%d, maxTtSize:%d", config->ctuSize, config->minCuSize, config->minQtSize, config->maxBtSize, config->maxTtSize);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "rc mode:%d target bit:%d", config->rcMode, config->bitRate);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "b frame num:%d ref num:%d", config->bFrameNum, config->refFrameNum);
    _XIN_LOGGER (XIN_LOGGER_STATUS, "thread num:%d wpp:%d fpp:%d", config->threadNum > 0 ? XIN_MIN (config->threadNum, seqSet->cpuCoreNum) : seqSet->cpuCoreNum, config->enableWpp, config->enableFpp);

    Xin266FuncInit (
        funcSet,
        seqSet->cpuFeature);

    Xin266SeqCreate (
        seqSet,
        config);

    Xin266FuncUpdate (
        funcSet,
        seqSet);

    seqSet->funcSet     = funcSet;
    h266Encoder->seqSet = seqSet;

    if (Xin266CreateDepQuantRom (seqSet))
    {
        return XIN_FAIL;
    }

    for (sectionIdx = 0; sectionIdx < seqSet->config.threadNum; sectionIdx++)
    {
        if (Xin266SecCreate (h266Encoder->secSet + sectionIdx, seqSet) == XIN_FAIL)
        {
            return XIN_FAIL;
        }
    }

    Xin26xMemListCreate (
        &seqSet->scratchMem,
        (void **)h266Encoder->secSet,
        seqSet->config.threadNum);

    Xin26xThreadQueueCreate (
        threadQueue,
        seqSet->config.threadNum);

    seqSet->threadQueue = threadQueue;

    if (Xin266LookaheadCreate (h266Encoder, seqSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (Xin266MctfCreate (h266Encoder, seqSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (Xin266SceneCutCreate (h266Encoder, seqSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    for (frameIdx = 0; frameIdx < seqSet->config.frameThreadNum; frameIdx++)
    {
        if (Xin266PicCreate (h266Encoder->picSet + frameIdx, seqSet) == XIN_FAIL)
        {
            return XIN_FAIL;
        }

        picSet = h266Encoder->picSet[frameIdx];
    }

    Xin266RcCreate (
        seqSet);

    if (seqSet->config.needRecon)
    {
        Xin266ContructReconFrame (
            &h266Encoder->reconFrame,
            seqSet);
    }

    seqSet->secSet = h266Encoder->secSet;

    return XIN_SUCCESS;

}

void Xin266EncoderDelete (
    XIN_HANDLE encoderHandle)
{
    xin266_encoder_struct *h266Encoder;
    xin_pic_struct        *picSet;
    xin_seq_struct        *seqSet;
    UINT32                frameIdx;
    UINT32                sectionIdx;

    h266Encoder = (xin266_encoder_struct *)encoderHandle;
    seqSet      = h266Encoder->seqSet;

    Xin266RcDelete (
        seqSet);

    for (frameIdx = 0; frameIdx < seqSet->config.frameThreadNum; frameIdx++)
    {
        picSet = h266Encoder->picSet[frameIdx];

        Xin266PicDelete (
            picSet);
    }

    for (sectionIdx = 0; sectionIdx < seqSet->config.threadNum; sectionIdx++)
    {
        Xin266SecDelete (
            h266Encoder->secSet[sectionIdx],
            seqSet);
    }

    Xin266DeleteDepQuantRom (seqSet);

    Xin266SceneCutDelete (
        h266Encoder->scSet);

    Xin266LookaheadDelete (
        h266Encoder->laSet);

    if (seqSet->config.enableMctf)
    {
        Xin266MctfDelete (
            h266Encoder->mctfSet);
    }

    Xin26xTheadQueueStop (
        h266Encoder->threadQueue);

    Xin26xThreadQueueDelete (
        h266Encoder->threadQueue);

    Xin26xMemListDelete (
        &seqSet->scratchMem);

    if (seqSet->config.needRecon)
    {
        free(h266Encoder->reconFrame.yuvBuf[0]);
        free(h266Encoder->reconFrame.yuvBuf[1]);
        free(h266Encoder->reconFrame.yuvBuf[2]);
    }

    Xin266SeqDelete (
        seqSet);

    free (h266Encoder->rcSet);
    free (h266Encoder->funcSet);
    free (h266Encoder->threadQueue);
    free (h266Encoder);

}

