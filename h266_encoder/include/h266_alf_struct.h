/***************************************************************************//**
 *
 * @file          h266_alf_struct.h
 * @brief         This file contains h266 data structures related to ALF.
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
#ifndef _h26x_alf_struct_h_
#define _h26x_alf_struct_h_

#include "h266_alf_context.h"
#include "h266_ct_unit_struct.h"
#include "h266_alf_func_struct.h"

typedef struct xin_alf_param
{
    BOOL           alfEnabled[PLANE_NUM];     // alf_slice_enable_flag, alf_chroma_idc
    BOOL           nonLinearFlag[PLANE_TYPE]; // alf_[luma/chroma]_clip_flag
    SINT16         lumaCoeff[XIN_ALF_MAX_CLS_NUM * XIN_ALF_MAX_LUMA_COEF_NUM]; // alf_coeff_luma_delta[i][j]
    SINT16         lumaClipp[XIN_ALF_MAX_CLS_NUM * XIN_ALF_MAX_LUMA_COEF_NUM]; // alf_clipp_luma_[i][j]
    SINT32         numAltChroma;                                                  // alf_chroma_num_alts_minus_one + 1
    SINT16         chromaCoeff[XIN_ALF_MAX_CHROMA_ALT_NUM][XIN_ALF_MAX_CHROMA_COEF_NUM]; // alf_coeff_chroma[i]
    SINT16         chromaClipp[XIN_ALF_MAX_CHROMA_ALT_NUM][XIN_ALF_MAX_CHROMA_COEF_NUM]; // alf_clipp_chroma[i]
    SINT16         filterCoeffDeltaIdx[XIN_ALF_MAX_CLS_NUM];                // filter_coeff_delta[i]
    UINT8          alfLumaCoeffFlag[XIN_ALF_MAX_CLS_NUM];                   // alf_luma_coeff_flag[i]
    SINT32         numLumaFilters;                                          // number_of_filters_minus1 + 1
    BOOL           alfLumaCoeffDeltaFlag;                                   // alf_luma_coeff_delta_flag
    BOOL           newFilterFlag[PLANE_TYPE];
    xin_alf_filter *alfFilter[PLANE_NUM];
    
} xin_alf_param;

typedef struct xin_cc_alf_param
{
    BOOL    ccAlfFilterEnabled[2];
    BOOL    ccAlfFilterIdxEnabled[2][XIN_CC_ALF_MAX_FILTER_NUM];
    UINT8   ccAlfFilterCount[2];
    SINT16  ccAlfCoeff[2][XIN_CC_ALF_MAX_FILTER_NUM][XIN_CC_ALF_MAX_COEFF_NUM];
    SINT32  newCcAlfFilter[2];
    SINT32  numberValidComponents;
}xin_cc_alf_param;

typedef struct xin_alf_aps
{
    UINT32           apsId;                    // adaptation_parameter_set_id
    SINT32           temporalId;
    SINT32           layerId;
    UINT32           apsType;                  // aps_params_type
    xin_alf_param    alfParam;
    xin_cc_alf_param ccAlfParam;
    BOOL             hasPrefixNalUnitType;
    BOOL             chromaPresent;
    BOOL             changed;
}xin_alf_aps;

typedef struct xin_alf_struct
{
    PIXEL             *filterBuf[3];
    xin_frame_struct  *filterFrame;
    UINT32            temporalId;
    SINT32            maxNumAlfAltChroma;
    BOOL              useNonLinearAlfChroma;
    BOOL              useNonLinearAlfLuma;
    xin_alf_cov       *alfCovarianceY;          // [ctbAddr][classIdx]
    xin_alf_cov       *alfCovarianceU;
    xin_alf_cov       *alfCovarianceV;
    xin_alf_cov       alfCovarianceFrameLuma[XIN_ALF_MAX_CLS_NUM];   // [CHANNEL][shapeIdx][lumaClassIdx/chromaAltIdx]
    xin_alf_cov       alfCovarianceFrameChroma[XIN_ALF_MAX_CHROMA_ALT_NUM];
    UINT8             *ctuEnableFlagTmp[PLANE_NUM];
    UINT8             *ctuAlternativeTmp[PLANE_NUM];
    SINT16            coeffFinal[XIN_ALF_MAX_CLS_NUM*XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT16            clippFinal[XIN_ALF_MAX_CLS_NUM*XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT16            chromaClippFinal[XIN_ALF_MAX_CHROMA_ALT_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT16            alfClippingValues[PLANE_TYPE][XIN_ALF_CLIP_NUM];
    SINT32            fixedFilterSetCoeff[XIN_ALF_FIXED_FILTER_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32            clipDefaultEnc[XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32            filterTmp[XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT32            clipTmp[XIN_ALF_MAX_LUMA_COEF_NUM];

    SINT32            ctuSize;

    SINT16            fixedFilterSetCoeffDec[XIN_ALF_FIXED_FLT_SET_NUM][XIN_ALF_MAX_CLS_NUM * XIN_ALF_MAX_LUMA_COEF_NUM];
    SINT16            clipDefault[XIN_ALF_MAX_CLS_NUM * XIN_ALF_MAX_LUMA_COEF_NUM];
    
    xin_alf_param     alfParamTemp;
    xin_alf_param     alfParamTempNL;

    xin_alf_func      *funcSet;
    
    xin_alf_cov       alfCovarianceMerged[XIN_ALF_MAX_CLS_NUM + 2];
    int               alfClipMerged[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    xin_prob_model    *context;
    xin_prob_model    *contextOrg;
    short             filterIndices[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_CLS_NUM];
    int               filterCoeffSet[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    int               filterClippSet[XIN_ALF_MAX_CLS_NUM][XIN_ALF_MAX_LUMA_COEF_NUM];
    int**             diffFilterCoeff;
    UINT8             *ctuEnableFlag[PLANE_NUM];
    UINT8             *ctuAlternative[PLANE_NUM];
    UINT8             *alfCtbFilterSetIndex;
    UINT8             *alfCtbFilterSetIndexTmp;
    FLOAT32           *ctuUnfilterDist[PLANE_NUM];

    SINT16            chromaCoeffFinal[XIN_ALF_MAX_CHROMA_ALT_NUM][XIN_ALF_MAX_CHROMA_COEF_NUM];

    UINT32            bitsNewFilter[PLANE_TYPE];
    FLOAT32           lambda[PLANE_TYPE];
    SINT32            frameSizeInCtu;
    UINT32            frameWidthInCtu;
    xin_ctu_struct    *ctu;
    xin_alf_class     *alfClass;
    intptr_t          alfClassStride;
    xin_alf_filter    alfFilter[PLANE_TYPE];

    UINT32            apsNum;
    UINT32            lumaApsId;
    UINT32            chromaApsId;
    xin_alf_aps       alfAps[XIN_ALF_CTB_MAX_APS_NUM];   

    BOOL              alfEnabled[PLANE_NUM];
    UINT32            ccAlfCbApsId;
    UINT32            ccAlfCrApsId;
    BOOL              ccAlfCbEnabled;
    BOOL              ccAlfCrEnabled;
    xin_alf_cov       *alfCovarianceCcAlf[2];
    xin_alf_cov       alfCovarianceFrameCcAlf[2][XIN_CC_ALF_MAX_FILTER_NUM];
    UINT8             *bestFilterControl;
    UINT8             *filterControl;
    UINT8             *ccAlfFilterControl[2];
    BOOL              bestFilterIdxEnabled[XIN_CC_ALF_MAX_FILTER_NUM];
    SINT32            bestFilterCount;
    SINT16            bestFilterCoeffSet[XIN_CC_ALF_MAX_FILTER_NUM][XIN_CC_ALF_MAX_COEFF_NUM];
    UINT8             *trainingCovControl;
    UINT64            *trainingDistortion[XIN_CC_ALF_MAX_FILTER_NUM];
    xin_cc_alf_param  ccAlfParam;
    
} xin_alf_struct;


#endif

