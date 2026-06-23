/***************************************************************************//**
 *
 * @file          h266_dep_quant_avx2.c
 * @brief         h266 dependent quantization (AVX2).
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
#include "string.h"
#include "stdlib.h"
#include "assert.h"
#include "basic_macro.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
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
#include "h266_sub_dct.h"
#include "h266_quant_inv_quant.h"
#include "h266_dep_quant_struct.h"
#include "immintrin.h"
#include "h26x_common_data.h"
#include "h266_dep_quant.h"
#include "h266_func_struct.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

extern const UINT32 auiGoRiceParsCoeff[32];
extern const SINT32 goRiceBits[4][32];
extern const xin_decision startDec[2];

static void Xin266StateInit (
    xin_dp_simd_state *state,
    UINT32            stateId)
{
    state->rdCost[stateId]         = XIN_MAX_DEP_COST;
    state->coeffCtx.coeff[stateId] = 0;
    state->coeffCtx.sig[stateId]   = 0;
    state->numSig[stateId]         = 0;
    state->refSbbCtxId[stateId]    = -1;
    state->remRegBins[stateId]     = 4;
    state->coeffBitsCtxOffset      = 0;
    state->goRicePar[stateId]      = 0;
    state->goRiceZero[stateId]     = 0;
    state->sbbBits0[stateId]       = 0;
    state->sbbBits1[stateId]       = 0;

}

static void Xin266CheckRdCostSkipSbbZeroOut (
    xin_dp_simd_state *state,
    xin_decision      *decision,
    SINT32            statId)
{
    decision->rdCost[statId]   = state->rdCost[statId] + state->sbbBits0[statId];
    decision->absLevel[statId] = 0;
    decision->prevId[statId]   = (SINT8)(4 | statId);
}

static inline UINT32 auiGoRicePosCoeff0 (
    SINT32 st,
    UINT32 ricePar)
{
    return (st < 2 ? 1 : 2) << ricePar;
}

static void Xin266SetRiceParam (
    xin_dp_simd_state *state,
    xin_scan_info     *scanInfo,
    SINT32            stateId,
    BOOL              ge4)
{
    SINT32 sumAbs;
    SINT32 sumSub, sumAll;

    if (state->remRegBins[stateId] < 4 || ge4)
    {
        sumAbs  = state->sum1st[(scanInfo->insidePos << 2) + stateId];
        sumSub  = state->remRegBins[stateId] < 4 ? 0 : 4 * 5;
        sumAll  = XIN_MAX (XIN_MIN (31, sumAbs - sumSub ), 0);

        state->goRicePar[stateId] = (SINT8)auiGoRiceParsCoeff[sumAll];

        if (state->remRegBins[stateId] < 4)
        {
            state->goRiceZero[stateId] = (SINT8)auiGoRicePosCoeff0 (stateId, state->goRicePar[stateId]);
        }
    }
}


static void Xin266CheckRdCosts (
    xin_dp_simd_state *state,
    UINT32            spt,
    SINT32            stateId,
    xin_pq_data       *pqDataA,
    xin_pq_data       *pqDataB,
    xin_decision      *decisions,
    SINT32            idxAZ,
    SINT32            idxB)
{
    const SINT32  *goRiceTab;
    SINT64         rdCostA;
    SINT64         rdCostB;
    SINT64         rdCostZ;
    UINT32         value;

    goRiceTab = goRiceBits[state->goRicePar[stateId]];
    rdCostA   = state->rdCost[stateId] + pqDataA->deltaDist;
    rdCostB   = state->rdCost[stateId] + pqDataB->deltaDist;
    rdCostZ   = state->rdCost[stateId];

    if (state->remRegBins[stateId] >= 4)
    {
        if (pqDataA->absLevel < 4)
        {
            rdCostA += state->gtxFracBitsArray[state->coeffCtx.coeff[stateId]].intBits[pqDataA->absLevel];
        }
        else
        {
            value    = (pqDataA->absLevel - 4) >> 1;
            rdCostA += state->gtxFracBitsArray[state->coeffCtx.coeff[stateId]].intBits[pqDataA->absLevel - (value << 1)] + goRiceTab[XIN_MIN (value, 32 - 1)];
        }

        if (pqDataB->absLevel < 4)
        {
            rdCostB += state->gtxFracBitsArray[state->coeffCtx.coeff[stateId]].intBits[pqDataB->absLevel];
        }
        else
        {
            value    = (pqDataB->absLevel - 4) >> 1;
            rdCostB += state->gtxFracBitsArray[state->coeffCtx.coeff[stateId]].intBits[pqDataB->absLevel - (value << 1)] + goRiceTab[XIN_MIN (value, 32 - 1)];
        }

        if (spt == XIN_SCAN_ISCSBB)
        {
            rdCostA += state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[1];
            rdCostB += state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[1];
            rdCostZ += state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[0];
        }
        else if (spt == XIN_SCAN_SOCSBB)
        {
            rdCostA += state->sbbBits1[stateId] + state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[1];
            rdCostB += state->sbbBits1[stateId] + state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[1];
            rdCostZ += state->sbbBits1[stateId] + state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[0];
        }
        else if (state->numSig[stateId])
        {
            rdCostA += state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[1];
            rdCostB += state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[1];
            rdCostZ += state->sigFracBitsArray[stateId][state->coeffCtx.sig[stateId]].intBits[0];
        }
        else
        {
            rdCostZ = decisions->rdCost[idxAZ];
        }

    }
    else
    {
        rdCostA += (1 << 15) + goRiceTab[pqDataA->absLevel <= state->goRiceZero[stateId] ? pqDataA->absLevel - 1 : XIN_MIN(pqDataA->absLevel, 32 - 1)];
        rdCostB += (1 << 15) + goRiceTab[pqDataB->absLevel <= state->goRiceZero[stateId] ? pqDataB->absLevel - 1 : XIN_MIN(pqDataB->absLevel, 32 - 1)];
        rdCostZ += goRiceTab[state->goRiceZero[stateId]];
    }

    if (rdCostA < rdCostZ && rdCostA < decisions->rdCost[idxAZ])
    {
        decisions->rdCost[idxAZ]   = rdCostA;
        decisions->absLevel[idxAZ] = (SINT16)pqDataA->absLevel;
        decisions->prevId[idxAZ]   = (SINT8)stateId;
    }
    else if (rdCostZ < decisions->rdCost[idxAZ])
    {
        decisions->rdCost[idxAZ]   = rdCostZ;
        decisions->absLevel[idxAZ] = 0;
        decisions->prevId  [idxAZ] = (SINT8)stateId;
    }

    if (rdCostB < decisions->rdCost[idxB])
    {
        decisions->rdCost[idxB]   = rdCostB;
        decisions->absLevel[idxB] = (SINT16)pqDataB->absLevel;
        decisions->prevId[idxB]   = (SINT8)stateId;
    }
}

// has to be called as a first check, assumes no decision has been made yet
static void Xin266CheckAllRdCosts (
    xin_dp_simd_state *state,
    UINT32            spt,
    xin_pq_data       *pqData,
    xin_decision      *decisions)
{
    // State mapping
    // decision 0: either A from 0 (pq0), or B from 1 (pq2), or 0 from 0
    // decision 1: either A from 2 (pq3), or B from 3 (pq1), or 0 from 2
    // decision 2: either A from 1 (pq0), or B from 0 (pq2), or 0 from 1
    // decision 3: either A from 3 (pq3), or B from 2 (pq1), or 0 from 3

    __m128i mrd01 = _mm_loadu_si128 ((__m128i*)(&state->rdCost[0]));
    __m128i mrd23 = _mm_loadu_si128 ((__m128i*)(&state->rdCost[2]));

    //int64_t         rdCostA   = state.rdCost[m_stateId] + pqDataA.deltaDist;
    //int64_t         rdCostB   = state.rdCost[m_stateId] + pqDataB.deltaDist;
    //int64_t         rdCostZ   = state.rdCost[m_stateId];
    __m128i rdCostZ01 = _mm_unpacklo_epi64 (mrd01, mrd23);
    __m128i rdCostZ23 = _mm_unpackhi_epi64 (mrd01, mrd23);
    __m128i deltaDist = _mm_unpacklo_epi64 (_mm_loadu_si64 (&pqData[2].deltaDist), _mm_loadu_si64 (&pqData[1].deltaDist));
    __m128i rdCostB01 = _mm_add_epi64 (rdCostZ23, deltaDist);
    __m128i rdCostB23 = _mm_add_epi64 (rdCostZ01, deltaDist);
    deltaDist = _mm_unpacklo_epi64 (_mm_loadu_si64( &pqData[0].deltaDist ), _mm_loadu_si64 (&pqData[3].deltaDist));
    __m128i rdCostA01 = _mm_add_epi64 (rdCostZ01, deltaDist);
    __m128i rdCostA23 = _mm_add_epi64 (rdCostZ23, deltaDist);

    //const CoeffFracBits &cffBits = m_gtxFracBitsArray[state.ctx.cff[m_stateId]];
    //const BinFracBits    sigBits = m_sigFracBitsArray[state.ctx.sig[m_stateId]];
    //
    //rdCostA += cffBits.bits[ pqDataA.absLevel ];
    //rdCostB += cffBits.bits[ pqDataB.absLevel ];
    __m128i sgbts02   = _mm_unpacklo_epi64 (_mm_loadu_si64 (&state->sigFracBitsArray[0][state->coeffCtx.sig[0]]),
                                            _mm_loadu_si64 (&state->sigFracBitsArray[2][state->coeffCtx.sig[2]]));
    __m128i sgbts13   = _mm_unpacklo_epi64 (_mm_loadu_si64 (&state->sigFracBitsArray[1][state->coeffCtx.sig[1]]),
                                            _mm_loadu_si64 (&state->sigFracBitsArray[3][state->coeffCtx.sig[3]]));

    {
        __m128i sgbts02_0 = _mm_shuffle_epi32 (sgbts02, 0 + ( 2 << 2 ) + ( 0 << 4 ) + ( 2 << 6 ));
        __m128i sgbts02_1 = _mm_shuffle_epi32 (sgbts02, 1 + ( 3 << 2 ) + ( 1 << 4 ) + ( 3 << 6 ));
        __m128i sgbts13_0 = _mm_shuffle_epi32 (sgbts13, 0 + ( 2 << 2 ) + ( 0 << 4 ) + ( 2 << 6 ));
        __m128i sgbts13_1 = _mm_shuffle_epi32 (sgbts13, 1 + ( 3 << 2 ) + ( 1 << 4 ) + ( 3 << 6 ));

        sgbts02 = _mm_unpacklo_epi64 (sgbts02_0, sgbts02_1);
        sgbts13 = _mm_unpacklo_epi64 (sgbts13_0, sgbts13_1);
    }

    {
        // coeff context is indepndent of state

        int32_t cffBitsArr[4] =
        {
            state->gtxFracBitsArray[state->coeffCtx.coeff[1]].intBits[pqData[2].absLevel],
            state->gtxFracBitsArray[state->coeffCtx.coeff[3]].intBits[pqData[1].absLevel],
            state->gtxFracBitsArray[state->coeffCtx.coeff[0]].intBits[pqData[2].absLevel],
            state->gtxFracBitsArray[state->coeffCtx.coeff[2]].intBits[pqData[1].absLevel],
        };

        __m128i cffBits = _mm_loadu_si128 ((__m128i* )cffBitsArr);
        __m128i add     = _mm_cvtepi32_epi64 (cffBits);
        rdCostB01 = _mm_add_epi64 (rdCostB01, add);
        add       = _mm_cvtepi32_epi64 (_mm_unpackhi_epi64 (cffBits, cffBits));
        rdCostB23 = _mm_add_epi64 (rdCostB23, add);
    }

    {

        int32_t cffBitsArr[4] =
        {
            state->gtxFracBitsArray[state->coeffCtx.coeff[0]].intBits[pqData[0].absLevel],
            state->gtxFracBitsArray[state->coeffCtx.coeff[2]].intBits[pqData[3].absLevel],
            state->gtxFracBitsArray[state->coeffCtx.coeff[1]].intBits[pqData[0].absLevel],
            state->gtxFracBitsArray[state->coeffCtx.coeff[3]].intBits[pqData[3].absLevel],
        };

        __m128i cffBits = _mm_loadu_si128 ((__m128i *)cffBitsArr);
        __m128i add     = _mm_cvtepi32_epi64 (cffBits);
        rdCostA01 = _mm_add_epi64 (rdCostA01, add);
        add       = _mm_cvtepi32_epi64 (_mm_unpackhi_epi64 (cffBits, cffBits));
        rdCostA23 = _mm_add_epi64 (rdCostA23, add);
    }

    if (spt == XIN_SCAN_ISCSBB)
    {
        //  rdCostZ += sigBits.intBits[ 0 ];
        rdCostZ01 = _mm_add_epi64 (rdCostZ01, _mm_cvtepi32_epi64 (sgbts02));
        rdCostZ23 = _mm_add_epi64 (rdCostZ23, _mm_cvtepi32_epi64 (sgbts13));

        sgbts02   = _mm_unpackhi_epi64 (sgbts02, sgbts02);
        sgbts13   = _mm_unpackhi_epi64 (sgbts13, sgbts13);

        //  rdCostB += sigBits.intBits[ 1 ];
        rdCostB01 = _mm_add_epi64 (rdCostB01, _mm_cvtepi32_epi64 (sgbts13));
        rdCostB23 = _mm_add_epi64 (rdCostB23, _mm_cvtepi32_epi64 (sgbts02));

        //  rdCostA += sigBits.intBits[ 1 ];
        rdCostA01 = _mm_add_epi64 (rdCostA01, _mm_cvtepi32_epi64 (sgbts02));
        rdCostA23 = _mm_add_epi64 (rdCostA23, _mm_cvtepi32_epi64 (sgbts13));
    }
    else if (spt == XIN_SCAN_SOCSBB)
    {
        //  rdCostA += m_sbbFracBits.intBits[ 1 ] + sigBits.intBits[ 1 ];
        //  rdCostB += m_sbbFracBits.intBits[ 1 ] + sigBits.intBits[ 1 ];
        //  rdCostZ += m_sbbFracBits.intBits[ 1 ] + sigBits.intBits[ 0 ];
        __m128i sbbBits = _mm_loadu_si128 ((__m128i *)state->sbbBits1);
        sbbBits = _mm_shuffle_epi32 (sbbBits, (0 << 0) + (2 << 2) + (1 << 4) + (3 << 6));

        rdCostZ01 = _mm_add_epi64( rdCostZ01, _mm_cvtepi32_epi64( sgbts02 ) );
        rdCostZ23 = _mm_add_epi64( rdCostZ23, _mm_cvtepi32_epi64( sgbts13 ) );

        __m128i add = _mm_cvtepi32_epi64( sbbBits );
        rdCostB23 = _mm_add_epi64( rdCostB23, add );
        rdCostA01 = _mm_add_epi64( rdCostA01, add );
        rdCostZ01 = _mm_add_epi64( rdCostZ01, add );
        add = _mm_cvtepi32_epi64( _mm_unpackhi_epi64( sbbBits, sbbBits ) );
        rdCostB01 = _mm_add_epi64( rdCostB01, add );
        rdCostA23 = _mm_add_epi64( rdCostA23, add );
        rdCostZ23 = _mm_add_epi64( rdCostZ23, add );

        sgbts02   = _mm_unpackhi_epi64( sgbts02, sgbts02 );
        sgbts13   = _mm_unpackhi_epi64( sgbts13, sgbts13 );
        rdCostB01 = _mm_add_epi64( rdCostB01, _mm_cvtepi32_epi64( sgbts13 ) );
        rdCostB23 = _mm_add_epi64( rdCostB23, _mm_cvtepi32_epi64( sgbts02 ) );

        rdCostA01 = _mm_add_epi64( rdCostA01, _mm_cvtepi32_epi64( sgbts02 ) );
        rdCostA23 = _mm_add_epi64( rdCostA23, _mm_cvtepi32_epi64( sgbts13 ) );
    }
    else
    {
        //else if( state.numSig[m_stateId] )
        //{
        //  rdCostA += sigBits.intBits[ 1 ];
        //  rdCostB += sigBits.intBits[ 1 ];
        //  rdCostZ += sigBits.intBits[ 0 ];
        //}
        //else
        //{
        //  rdCostZ = decisionA.rdCost;
        //}
        __m128i numSig = _mm_loadu_si32( state->numSig );

        rdCostZ01 = _mm_add_epi64(  rdCostZ01, _mm_cvtepi32_epi64( sgbts02 ) );
        rdCostZ23 = _mm_add_epi64( rdCostZ23, _mm_cvtepi32_epi64( sgbts13 ) );

        __m128i mask13 = _mm_shuffle_epi8( numSig, _mm_setr_epi8( 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3 ) );
        mask13    = _mm_cmpgt_epi8( mask13, _mm_setzero_si128() );
        __m128i mask02 = _mm_shuffle_epi8( numSig, _mm_setr_epi8( 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2 ) );
        mask02    = _mm_cmpgt_epi8( mask02, _mm_setzero_si128() );

        sgbts02   = _mm_unpackhi_epi64( sgbts02, sgbts02 );
        sgbts13   = _mm_unpackhi_epi64( sgbts13, sgbts13 );

        rdCostB01 = _mm_add_epi64( rdCostB01, _mm_and_si128( mask13, _mm_cvtepi32_epi64( sgbts13 ) ) );
        rdCostB23 = _mm_add_epi64( rdCostB23, _mm_and_si128( mask02, _mm_cvtepi32_epi64( sgbts02 ) ) );

        rdCostA01 = _mm_add_epi64( rdCostA01, _mm_and_si128( mask02, _mm_cvtepi32_epi64( sgbts02 ) ) );
        rdCostA23 = _mm_add_epi64( rdCostA23, _mm_and_si128( mask13, _mm_cvtepi32_epi64( sgbts13 ) ) );

        __m128i rdMax = _mm_set1_epi64x (XIN_MAX_DEP_COST);

        rdCostZ01 = _mm_blendv_epi8( rdMax, rdCostZ01, mask02 );
        rdCostZ23 = _mm_blendv_epi8( rdMax, rdCostZ23, mask13 );
    }
    // decision 0: either A from 0 (pq0), or B from 1 (pq2), or 0 from 0
    // decision 1: either A from 2 (pq3), or B from 3 (pq1), or 0 from 2
    // decision 2: either A from 1 (pq0), or B from 0 (pq2), or 0 from 1
    // decision 3: either A from 3 (pq3), or B from 2 (pq1), or 0 from 3
    // Z0, or A0, or B0
    // Z1, or A1, or B1
    // B2, or Z2, or A2
    // B3, or Z3, or A3

    __m128i rdBest01 = rdCostZ01;
    __m128i rdBest23 = rdCostB23;

    __m128i valBest = _mm_setr_epi32(                  0,                  0, pqData[2].absLevel, pqData[1].absLevel );

#if ENABLE_VALGRIND_CODE
    // just to avoid strange "unknown instruction"  error
    __m128i valCand = _mm_setr_epi32( 0, pqData[3].absLevel,                  0,                  0 );
    valCand =  _mm_insert_epi32 (valCand, pqData[0].absLevel,0);
#else
    __m128i valCand = _mm_setr_epi32( pqData[0].absLevel, pqData[3].absLevel,                  0,                  0 );
#endif
    __m128i idxBest = _mm_setr_epi32( 0, 2, 0, 2 );
    __m128i idxCand = _mm_setr_epi32( 0, 2, 1, 3 );

    __m128i chng01 = _mm_cmpgt_epi64( rdBest01, rdCostA01 );
    __m128i chng23 = _mm_cmpgt_epi64( rdBest23, rdCostZ23 );
    __m128i chng   = _mm_blend_epi16( chng01, chng23, ( 3 << 2 ) + ( 3 << 6 ) ); // 00110011
    chng           = _mm_shuffle_epi32( chng, ( 0 << 0 ) + ( 2 << 2 ) + ( 1 << 4 ) + ( 3 << 6 ) );

    rdBest01 = _mm_blendv_epi8( rdBest01, rdCostA01, chng01 );
    rdBest23 = _mm_blendv_epi8( rdBest23, rdCostZ23, chng23 );

    valBest = _mm_blendv_epi8( valBest, valCand, chng );
    idxBest = _mm_blendv_epi8( idxBest, idxCand, chng );


    valCand = _mm_setr_epi32( pqData[2].absLevel, pqData[1].absLevel, pqData[0].absLevel, pqData[3].absLevel );
    idxCand = _mm_setr_epi32( 1, 3, 1, 3 );

    chng01 = _mm_cmpgt_epi64( rdBest01, rdCostB01 );
    chng23 = _mm_cmpgt_epi64( rdBest23, rdCostA23 );
    chng   = _mm_blend_epi16( chng01, chng23, ( 3 << 2 ) + ( 3 << 6 ) ); // 00110011
    chng   = _mm_shuffle_epi32( chng, ( 0 << 0 ) + ( 2 << 2 ) + ( 1 << 4 ) + ( 3 << 6 ) );

    rdBest01 = _mm_blendv_epi8( rdBest01, rdCostB01, chng01 );
    rdBest23 = _mm_blendv_epi8( rdBest23, rdCostA23, chng23 );

    valBest = _mm_blendv_epi8( valBest, valCand, chng );
    idxBest = _mm_blendv_epi8( idxBest, idxCand, chng );


    valBest = _mm_packs_epi32( valBest, _mm_setzero_si128() );
    idxBest = _mm_packs_epi32( idxBest, _mm_setzero_si128() );
    idxBest = _mm_packs_epi16( idxBest, _mm_setzero_si128() );


    _mm_storeu_si128( ( __m128i* ) &decisions->rdCost[0], rdBest01 );
    _mm_storeu_si128( ( __m128i* ) &decisions->rdCost[2], rdBest23 );

    _mm_storeu_si64( decisions->absLevel, valBest );
    _mm_storeu_si32( decisions->prevId,   idxBest );

}

static void Xin266CheckRdCostStart (
    xin_dp_simd_state *state,
    SINT32            stateId,
    SINT32            lastOffset,
    xin_pq_data       *pqData,
    xin_decision      *decision)
{
    SINT64 rdCost;
    UINT32 value;

    rdCost = pqData->deltaDist + lastOffset;

    if (pqData->absLevel < 4)
    {
        rdCost += state->gtxFracBitsArray[0].intBits[pqData->absLevel];
    }
    else
    {
        value   = (pqData->absLevel - 4) >> 1;
        rdCost += state->gtxFracBitsArray[0].intBits[pqData->absLevel - (value << 1)] + goRiceBits[0][value < 32 ? value : 32-1];
    }

    if (rdCost < decision->rdCost[stateId])
    {
        decision->rdCost  [stateId] = rdCost;
        decision->absLevel[stateId] = (SINT16)pqData->absLevel;
        decision->prevId  [stateId] = (SINT8)-1;
    }
}

static void Xin266CheckRdCostSkipSbb (
    xin_dp_simd_state *state,
    SINT32            stateId,
    xin_decision      *decisions,
    SINT32            idx)
{
    SINT64 rdCost;

    rdCost = state->rdCost[stateId] + state->sbbBits0[stateId];

    if( rdCost < decisions->rdCost[idx] )
    {
        decisions->rdCost[idx]   = rdCost;
        decisions->absLevel[idx] = 0;
        decisions->prevId[idx]   = (SINT8)(4 | stateId);
    }
}

static void Xin266Decide (
    xin_dep_quant *depQuant,
    xin_dp_param  *dpParam,
    xin_scan_info *scanInfo,
    SINT32        absCoeff,
    SINT32        lastOffset,
    xin_decision  *decisions,
    BOOL          zeroOut)
{
    xin_pq_data       pqData[4];
    xin_dp_simd_state *skipState;
    xin_dp_simd_state *prevState;
    BOOL              coeff02ge4, coeff13ge4;

    skipState = &depQuant->simdStateSkip;

    memcpy (
        decisions,
        startDec,
        sizeof(xin_decision));

    if (zeroOut)
    {
        if (scanInfo->spt == XIN_SCAN_EOCSBB)
        {
            Xin266CheckRdCostSkipSbbZeroOut (
                skipState,
                decisions,
                0);

            Xin266CheckRdCostSkipSbbZeroOut (
                skipState,
                decisions,
                1);

            Xin266CheckRdCostSkipSbbZeroOut (
                skipState,
                decisions,
                2);

            Xin266CheckRdCostSkipSbbZeroOut (
                skipState,
                decisions,
                3);

        }

        return;

    }

    prevState = &depQuant->simdStateCurr;

    Xin266PreQuantCoeff (
        absCoeff,
        pqData,
        dpParam);

    coeff02ge4 = pqData[0].absLevel >= 4/* || pqData[2].absLevel >= 4 */;
    coeff13ge4 = /* pqData[1].absLevel >= 4 || */ pqData[3].absLevel >= 4;

    if (coeff02ge4 || coeff13ge4 || prevState->anyRemRegBinsLt4)
    {
        if (prevState->anyRemRegBinsLt4 || coeff02ge4)
        {
            Xin266SetRiceParam (
                prevState,
                scanInfo,
                0,
                coeff02ge4);

            Xin266SetRiceParam (
                prevState,
                scanInfo,
                1,
                coeff02ge4);
        }

        if (prevState->anyRemRegBinsLt4 || coeff13ge4)
        {
            Xin266SetRiceParam (
                prevState,
                scanInfo,
                2,
                coeff13ge4);

            Xin266SetRiceParam (
                prevState,
                scanInfo,
                3,
                coeff13ge4);
        }

        Xin266CheckRdCosts (
            prevState,
            scanInfo->spt,
            0,
            pqData + 0,
            pqData + 2,
            decisions,
            0,
            2);

        Xin266CheckRdCosts (
            prevState,
            scanInfo->spt,
            1,
            pqData + 0,
            pqData + 2,
            decisions,
            2,
            0);

        Xin266CheckRdCosts (
            prevState,
            scanInfo->spt,
            2,
            pqData + 3,
            pqData + 1,
            decisions,
            1,
            3);

        Xin266CheckRdCosts (
            prevState,
            scanInfo->spt,
            3,
            pqData + 3,
            pqData + 1,
            decisions,
            3,
            1);

    }
    else
    {
        Xin266CheckAllRdCosts (
            prevState,
            scanInfo->spt,
            pqData,
            decisions);
    }

    Xin266CheckRdCostStart (
        prevState,
        0,
        lastOffset,
        &pqData[0],
        decisions);

    Xin266CheckRdCostStart (
        prevState,
        2,
        lastOffset,
        &pqData[2],
        decisions);

    if (scanInfo->spt == XIN_SCAN_EOCSBB)
    {
        Xin266CheckRdCostSkipSbb (
            skipState,
            0,
            decisions,
            0);

        Xin266CheckRdCostSkipSbb (
            skipState,
            1,
            decisions,
            1);

        Xin266CheckRdCostSkipSbb (
            skipState,
            2,
            decisions,
            2);

        Xin266CheckRdCostSkipSbb (
            skipState,
            3,
            decisions,
            3);
        
    }

}


static void Xin266UpdateAllLvls (
    xin_dep_quant     *depQuant,
    xin_scan_info     *scanInfo,
    xin_dp_simd_state *currState)
{
    uint8_t *levels0 = depQuant->currSbbCtx[0].levels + scanInfo->scanIdx;
    uint8_t *levels1 = depQuant->currSbbCtx[1].levels + scanInfo->scanIdx;
    uint8_t *levels2 = depQuant->currSbbCtx[2].levels + scanInfo->scanIdx;
    uint8_t *levels3 = depQuant->currSbbCtx[3].levels + scanInfo->scanIdx;

    const int regSize = 16;
    const int ctxSize = scanInfo->sbbSize << 2;

    const __m128i vshuf0 = _mm_setr_epi8(  0,  4,  8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
    const __m128i vshuf1 = _mm_setr_epi8(  1,  5,  9, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
    const __m128i vshuf2 = _mm_setr_epi8(  2,  6, 10, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
    const __m128i vshuf3 = _mm_setr_epi8(  3,  7, 11, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );

    for( int i = 0, j = 0; i < ctxSize; i += regSize, j += 4 )
    {
        __m128i in  = _mm_loadu_si128((__m128i *) &currState->absVal[i] );

        _mm_storeu_si32( &levels0[j], _mm_shuffle_epi8( in, vshuf0 ) );
        _mm_storeu_si32( &levels1[j], _mm_shuffle_epi8( in, vshuf1 ) );
        _mm_storeu_si32( &levels2[j], _mm_shuffle_epi8( in, vshuf2 ) );
        _mm_storeu_si32( &levels3[j], _mm_shuffle_epi8( in, vshuf3 ) );
    }
}

static void Xin266Update (
    xin_dep_quant     *depQuant,
    xin_scan_info     *scanInfo,
    xin_dp_simd_state *currState,
    SINT32            prevId,
    SINT32            stateId)
{
    UINT8           *sbbFlags;
    UINT8           *levels;
    UINT32          setCpSize;
    SINT32          sigNSbb;
    SINT32          scanBegin;
    xin_nb_info_out *nbOut;
    UINT8           *absLevels;
    SINT32          idx;
    COEFF           sumAbs, sumAbs1, sumNum;
    SINT32          maxDist;
    SINT32          sbbSize;

    sbbFlags  = depQuant->currSbbCtx[stateId].sbbFlags;
    levels    = depQuant->currSbbCtx[stateId].levels;
    maxDist   = depQuant->nbInfo[scanInfo->scanIdx - 1].maxDist;
    sbbSize   = scanInfo->sbbSize;
    setCpSize = (maxDist > sbbSize ? maxDist - sbbSize : 0)*sizeof(UINT8);

    if (prevId >= 0)
    {
        memcpy (
            sbbFlags,
            depQuant->prevSbbCtx[prevId].sbbFlags,
            scanInfo->numSbb*sizeof(UINT8));

        memcpy (
            levels + scanInfo->scanIdx + sbbSize,
            depQuant->prevSbbCtx[prevId].levels + scanInfo->scanIdx + sbbSize,
            setCpSize);
    }
    else
    {
        memset (
            sbbFlags,
            0,
            scanInfo->numSbb*sizeof(UINT8));

        memset (
            levels + scanInfo->scanIdx + sbbSize,
            0,
            setCpSize);
    }

    sbbFlags[scanInfo->sbbPos] = !!currState->numSig[stateId];

    sigNSbb = ((scanInfo->nextSbbRight ? sbbFlags[scanInfo->nextSbbRight] : FALSE) || (scanInfo->nextSbbBelow ? sbbFlags[scanInfo->nextSbbBelow] : FALSE) ? 1 : 0);

    currState->refSbbCtxId[stateId] = (SINT8)stateId;
    currState->sbbBits0[stateId]    = depQuant->sbbFlagBits[sigNSbb].intBits[0];
    currState->sbbBits1[stateId]    = depQuant->sbbFlagBits[sigNSbb].intBits[1];

    if (sigNSbb || ((scanInfo->nextSbbRight && scanInfo->nextSbbBelow) ? sbbFlags[scanInfo->nextSbbBelow + 1] : FALSE))
    {
        scanBegin = scanInfo->scanIdx - scanInfo->sbbSize;
        nbOut     = depQuant->nbInfo + scanBegin;
        absLevels = levels + scanBegin;

        for (idx = 0; idx < scanInfo->sbbSize; idx++, nbOut++)
        {
            if (nbOut->num)
            {
                sumAbs  = 0;
                sumAbs1 = 0;
                sumNum  = 0;

                switch (nbOut->num)
                {
                default:
                case 5:
                    XIN_OUT_UPDATE (4);
                case 4:
                    XIN_OUT_UPDATE (3);
                case 3:
                    XIN_OUT_UPDATE (2);
                case 2:
                    XIN_OUT_UPDATE (1);
                case 1:
                    XIN_OUT_UPDATE (0);
                }

                currState->tplAcc[(idx << 2) + stateId] = (UINT8)((sumNum << 5) | sumAbs1);
                currState->sum1st[(idx << 2) + stateId] = (UINT8)XIN_MIN (255, sumAbs);

            }

        }

    }

}

static void Xin266UpdateStateEOS (
    xin_dep_quant     *depQuant,
    xin_scan_info     *scanInfo,
    xin_dp_simd_state *currStates,
    xin_dp_simd_state *skipStates,
    xin_decision      *decisions)
{
    int8_t s[4] = { 0 }, l[4] = { 0 }, z[4] = { 0 };

    for( int i = 0; i < 4; ++i )
    {
        s[i]                  = decisions->prevId[i] >= 4 ? -2 : (SINT8)decisions->prevId[i];
        l[i]                  = s[i] > -2 ? (SINT8)XIN_MIN (decisions->absLevel[i], 254 + ( decisions->absLevel[i] & 1)) : 0;
        z[i]                  = (SINT8)(3 - decisions->prevId[i]);
        currStates->rdCost[i] = decisions->rdCost[i];
    }

    {
        const int ctxSize = 16 * 4;
        const int regSize = 16;

        __m128i vshuf     = _mm_loadu_si32( s );
        vshuf             = _mm_shuffle_epi32( vshuf, 0 );
        __m128i vshufmask = _mm_cmplt_epi8 ( vshuf, _mm_setzero_si128() );
        vshuf             = _mm_add_epi8   ( vshuf, _mm_setr_epi8( 0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12 ) );
        vshuf             = _mm_blendv_epi8( vshuf, _mm_set1_epi8( -1 ), vshufmask );

        for( int i = 0; i < ctxSize; i += regSize )
        {
            __m128i vval = _mm_loadu_si128( ( const __m128i* ) &currStates->absVal[i] );
            vval = _mm_shuffle_epi8( vval, vshuf );
            _mm_storeu_si128( ( __m128i* ) &currStates->absVal[i], vval );
        }

        __m128i numSig = _mm_loadu_si32( currStates->numSig );
        numSig = _mm_shuffle_epi8( numSig, vshuf );
        __m128i lvls   = _mm_loadu_si32( l );
        int addr = ( scanInfo->insidePos << 2 );
        _mm_storeu_si32( &currStates->absVal[addr], lvls );
        lvls   = _mm_cmpgt_epi8( lvls, _mm_setzero_si128() );
        numSig = _mm_subs_epi8( numSig, lvls );
        _mm_storeu_si32( currStates->numSig, numSig );

        __m128i rsc = _mm_loadu_si32( currStates->refSbbCtxId );
        rsc         = _mm_shuffle_epi8( rsc, vshuf );
        rsc         = _mm_blendv_epi8( rsc, vshuf, vshuf );
        _mm_storeu_si32( currStates->refSbbCtxId, rsc );

        vshuf = _mm_shuffle_epi8( vshuf, _mm_setr_epi8( 0, 0, 1, 1, 2, 2, 3, 3, -1, -1, -1, -1, -1, -1, -1, -1 ) );
        vshuf = _mm_slli_epi16( vshuf, 1 );
        vshuf = _mm_add_epi8( vshuf,
                              _mm_blendv_epi8( _mm_setr_epi8( 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 ),
                                               _mm_setzero_si128(),
                                               vshuf ) );

        __m128i rrb = _mm_loadu_si64( (__m128i *) currStates->remRegBins );
        rrb = _mm_shuffle_epi8( rrb, vshuf );
        rrb = _mm_sub_epi16( rrb, _mm_set1_epi16( 1 ) );
        rrb = _mm_blendv_epi8( rrb, _mm_set1_epi16( (SINT16)currStates->initRemRegBins ), vshuf );

        __m128i vskip = _mm_cvtepi8_epi16( _mm_loadu_si32( z ) );
        rrb = _mm_blendv_epi8( rrb, _mm_loadu_si64( (__m128i *) skipStates->remRegBins ), vskip );

        __m128i mlvl  = _mm_loadu_si32( l );
        __m128i mbins = _mm_min_epi8 ( mlvl, _mm_set1_epi8( 2 ) );
        __m128i mlutb = _mm_setr_epi8( 0, 1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
        rrb = _mm_sub_epi16( rrb, _mm_cvtepi8_epi16( _mm_shuffle_epi8( mlutb, mbins ) ) );
        _mm_storeu_si64( ( __m128i *) currStates->remRegBins, rrb );
        rrb = _mm_cmplt_epi16( rrb, _mm_set1_epi16( 4 ) );

        currStates->anyRemRegBinsLt4 = !!_mm_cvtsi128_si64( rrb );
    }

    Xin266UpdateAllLvls (
        depQuant,
        scanInfo,
        currStates);

    memset( currStates->absVal, 0, sizeof( currStates->absVal ) );
    memset( currStates->tplAcc, 0, sizeof( currStates->tplAcc ) );
    memset( currStates->sum1st, 0, sizeof( currStates->sum1st ) );

    for( int i = 0; i < 4; i++ )
    {
        int prevId = decisions->prevId[i];

        if( prevId > -2 )
        {
            const int refId = prevId < 0 ? -1 : ( prevId < 4 ? currStates->refSbbCtxId[i] : prevId - 4 );

            Xin266Update (
                depQuant,
                scanInfo,
                currStates,
                refId,
                i);
        }
    }

    memset( currStates->numSig, 0, sizeof( currStates->numSig ) );

    {
        __m128i tplAcc  = _mm_loadu_si32( &currStates->tplAcc[ ( scanInfo->nextInsidePos << 2 ) ] );

        __m128i sumAbs1 = _mm_and_si128( tplAcc, _mm_set1_epi8( 31 ) );
        __m128i sumNum  = _mm_and_si128( _mm_srli_epi32( tplAcc, 5 ), _mm_set1_epi8( 7 ) );
        __m128i sumGt1  = _mm_sub_epi8 ( sumAbs1, sumNum );
        sumGt1  = _mm_min_epi8( sumGt1, _mm_set1_epi8( 4 ) );
        sumGt1  = _mm_add_epi8( _mm_set1_epi8( (SINT8)scanInfo->gtxCtxOffsetNext ), sumGt1 );
        _mm_storeu_si32( currStates->coeffCtx.coeff, sumGt1 );

        sumAbs1 = _mm_add_epi8  ( sumAbs1, _mm_set1_epi8( 1 ) );
        sumAbs1 = _mm_srli_epi32( sumAbs1, 1 );
        sumAbs1 = _mm_and_si128 ( sumAbs1, _mm_set1_epi8( 127 ) );
        sumAbs1 = _mm_min_epi8  ( sumAbs1, _mm_set1_epi8( 3 ) );
        sumAbs1 = _mm_add_epi8  ( _mm_set1_epi8( (SINT8)scanInfo->sigCtxOffsetNext ), sumAbs1 );
        _mm_storeu_si32( currStates->coeffCtx.sig, sumAbs1 );

        currStates->coeffBitsCtxOffset = scanInfo->gtxCtxOffsetNext;
    }
}

#define update_deps_vec(k) { int addr = scanInfo->currNbInfoSbb.invInPos[k] << 2; __m128i msum = _mm_loadu_si32(&currState->sum1st[addr]);  msum = _mm_adds_epu8( msum, mlvl ); _mm_storeu_si32( &currState->sum1st[addr], msum ); __m128i tpl = _mm_loadu_si32( &currState->tplAcc[addr] ); tpl = _mm_add_epi8( tpl, tpl1 );  _mm_storeu_si32( &currState->tplAcc[addr], tpl );}

static void Xin266UpdateState (
    xin_dp_simd_state  *currState,
    xin_scan_info      *scanInfo,
    xin_decision       *decisions)
{
    int8_t s[4] = { 0 }, t[4] = { 0 }, l[4] = { 0 };

    __m128i v254_4 = _mm_setr_epi16( 254, 254, 254, 254,  4,  4,  4,  4 );
    __m128i v01    = _mm_setr_epi16(   1,   1,   1,   1,  1,  1,  1,  1 );
    __m128i v032   = _mm_setr_epi8 (   0,   0,   0,   0, 32, 32, 32, 32, 0, 0, 0, 0, 0, 0, 0, 0 );
    __m128i vn1    = _mm_set1_epi8 (  -1 );

    assert(sizeof( currState->rdCost ) == sizeof( decisions->rdCost ));

    memcpy (currState->rdCost, decisions->rdCost, sizeof( decisions->rdCost ) );

    // in signalling, the coeffs are always max 16 bit!
    __m128i v = _mm_loadu_si64( decisions->absLevel );
    v = _mm_unpacklo_epi64( v, v );
    __m128i p = _mm_loadu_si32( decisions->prevId );
    _mm_storeu_si32( s, p ); // store previous state indexes
    p = _mm_shuffle_epi32( p, 0 );
    __m128i n2  = _mm_cmplt_epi8( p, vn1 );
    __m128i a_1 = _mm_and_si128( v, v01 );
    __m128i a_m = _mm_min_epi16( v, _mm_add_epi16( v254_4, a_1 ) );
    a_m = _mm_packs_epi16( a_m, vn1 );
    a_m = _mm_or_si128   ( a_m, _mm_sign_epi8( v032, a_m ) );
    a_m = _mm_andnot_si128( n2, a_m );
    _mm_storeu_si32( l, a_m ); // store abs value
    a_m = _mm_shuffle_epi32( a_m, 1 );
    _mm_storeu_si32( t, a_m ); // store store capped abs value

    {
        const int ctxSize = 16 * 4;
        const int regSize = 16;

        __m128i vshuf     = _mm_loadu_si32 ( s );
        vshuf     = _mm_shuffle_epi32( vshuf, 0 );
        __m128i vshufmask = _mm_cmplt_epi8 ( vshuf, _mm_setzero_si128() );
        vshuf             = _mm_add_epi8   ( vshuf, _mm_setr_epi8( 0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12 ) );
        vshuf             = _mm_blendv_epi8( vshuf, _mm_set1_epi8( -1 ), vshufmask );

        for( int i = 0; i < ctxSize; i += regSize )
        {
            __m128i vtpl = _mm_loadu_si128( ( const __m128i* ) &currState->tplAcc[i] );
            vtpl = _mm_shuffle_epi8( vtpl, vshuf );
            _mm_storeu_si128( ( __m128i* ) &currState->tplAcc[i], vtpl );

            __m128i vval = _mm_loadu_si128( ( const __m128i* ) &currState->absVal[i] );
            vval = _mm_shuffle_epi8( vval, vshuf );
            _mm_storeu_si128( ( __m128i* ) &currState->absVal[i], vval );

            __m128i vsum = _mm_loadu_si128( ( const __m128i* ) &currState->sum1st[i] );
            vsum = _mm_shuffle_epi8( vsum, vshuf );
            _mm_storeu_si128( ( __m128i* ) &currState->sum1st[i], vsum );
        }

        __m128i numSig = _mm_loadu_si32( currState->numSig );
        numSig = _mm_shuffle_epi8( numSig, vshuf );
        __m128i lvls   = _mm_loadu_si32( l );
        lvls   = _mm_cmpgt_epi8( lvls, _mm_setzero_si128() );
        numSig = _mm_subs_epi8( numSig, lvls );
        _mm_storeu_si32( currState->numSig, numSig );

        __m128i rsc = _mm_loadu_si32( currState->refSbbCtxId );
        rsc         = _mm_shuffle_epi8( rsc, vshuf );
        rsc         = _mm_blendv_epi8( rsc, vshuf, vshuf );
        _mm_storeu_si32( currState->refSbbCtxId, rsc );

        vshuf = _mm_shuffle_epi8( vshuf, _mm_setr_epi8( 0, 0, 1, 1, 2, 2, 3, 3, -1, -1, -1, -1, -1, -1, -1, -1 ) );
        vshuf = _mm_slli_epi16( vshuf, 1 );
        vshuf = _mm_add_epi8( vshuf,
                              _mm_blendv_epi8( _mm_setr_epi8( 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 ),
                                               _mm_setzero_si128(),
                                               vshuf ) );

        __m128i rrb = _mm_loadu_si64( ( const __m128i* ) currState->remRegBins );
        rrb = _mm_shuffle_epi8( rrb, vshuf );
        rrb = _mm_sub_epi16( rrb, v01 );
        rrb = _mm_blendv_epi8( rrb, _mm_set1_epi16( (SINT16)currState->initRemRegBins ), vshuf );
        __m128i mlvl  = _mm_loadu_si32( l );
        __m128i mbins = _mm_min_epi8 ( mlvl, _mm_set1_epi8( 2 ) );
        __m128i mlutb = _mm_setr_epi8( 0, 1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
        rrb = _mm_sub_epi16( rrb, _mm_cvtepi8_epi16( _mm_shuffle_epi8( mlutb, mbins ) ) );
        _mm_storeu_si64( ( __m128i* ) currState->remRegBins, rrb );
        rrb = _mm_cmplt_epi16( rrb, _mm_set1_epi16( 4 ) );

        currState->anyRemRegBinsLt4 = !!_mm_cvtsi128_si64( rrb );

        __m128i lvl1 = _mm_loadu_si32( l );
        __m128i tpl1 = _mm_loadu_si32( t );

        switch( scanInfo->currNbInfoSbb.numInv )
        {
        default:
        case 5:
            update_deps_vec( 4 );
        case 4:
            update_deps_vec( 3 );
        case 3:
            update_deps_vec( 2 );
        case 2:
            update_deps_vec( 1 );
        case 1:
            update_deps_vec( 0 );
        case 0:
            ;
        }

        int addr = ( scanInfo->insidePos << 2 );
        _mm_storeu_si32( &currState->absVal[addr], lvl1 );
    }

    {
        __m128i tplAcc  = _mm_loadu_si32( &currState->tplAcc[ ( scanInfo->nextInsidePos << 2 ) ] );

        __m128i sumAbs1 = _mm_and_si128( tplAcc, _mm_set1_epi8( 31 ) );
        __m128i sumNum  = _mm_and_si128( _mm_srli_epi32( tplAcc, 5 ), _mm_set1_epi8( 7 ) );
        __m128i sumGt1  = _mm_sub_epi8 ( sumAbs1, sumNum );
        sumGt1  = _mm_min_epi8( sumGt1, _mm_set1_epi8( 4 ) );
        sumGt1  = _mm_add_epi8( _mm_set1_epi8( (SINT8)scanInfo->gtxCtxOffsetNext ), sumGt1 );
        _mm_storeu_si32( currState->coeffCtx.coeff, sumGt1 );

        sumAbs1 = _mm_add_epi8  ( sumAbs1, _mm_set1_epi8( 1 ) );
        sumAbs1 = _mm_srli_epi32( sumAbs1, 1 );
        sumAbs1 = _mm_and_si128 ( sumAbs1, _mm_set1_epi8( 127 ) );
        sumAbs1 = _mm_min_epi8  ( sumAbs1, _mm_set1_epi8( 3 ) );
        sumAbs1 = _mm_add_epi8  ( _mm_set1_epi8( (SINT8)scanInfo->sigCtxOffsetNext ), sumAbs1 );
        _mm_storeu_si32( currState->coeffCtx.sig, sumAbs1 );

        currState->coeffBitsCtxOffset = scanInfo->gtxCtxOffsetNext;
    }
}

static void Xin266DecideUpdate (
    xin_dep_quant *depQuant,
    xin_dp_param  *dpParam,
    SINT32        absCoeff,
    xin_scan_info *scanInfo,
    BOOL          zeroOut)
{
    xin_decision *decisions;
    SINT32       lastOffset;

    decisions  = &depQuant->trellis[scanInfo->scanIdx][0];
    lastOffset = XIN_LASTOFFSET(scanInfo);

    Xin266Decide (
        depQuant,
        dpParam,
        scanInfo,
        absCoeff,
        lastOffset,
        decisions,
        zeroOut);

    if (scanInfo->scanIdx)
    {
        if (scanInfo->spt == XIN_SCAN_SOCSBB)
        {
            memcpy (&depQuant->simdStateSkip, &depQuant->simdStateCurr, offsetof(xin_dp_simd_state, sbbBits1));
        }

        if (scanInfo->insidePos == 0)
        {
            XIN_SWAP (xin_sbb_ctx *, depQuant->currSbbCtx, depQuant->prevSbbCtx);

            Xin266UpdateStateEOS (
                depQuant,
                scanInfo,
                &depQuant->simdStateCurr,
                &depQuant->simdStateSkip,
                decisions);

            memcpy (
                decisions + 1,
                decisions,
                sizeof(xin_decision));

        }
        else if (!zeroOut)
        {
            Xin266UpdateState (
                &depQuant->simdStateCurr,
                scanInfo,
                decisions);

        }

    }

}

void Xin266DepQuant_AVX2 (
    xin_dep_quant   *depQuant,
    xin_fast_md_buf *fastBuf,
    xin_tu_struct   *tu,
    COEFF           *tCoeff,
    COEFF           *rCoeff,
    COEFF           *qCoeff,
    intptr_t        coeffStride,
    UINT32          qp,
    SINT32          width,
    SINT32          height,
    UINT64          *nzCGBitMapRs,
    UINT32          compId)
{
    xin_scan_pos    *scanOrderCG;
    xin_func_struct *funcSet;
    SINT32          defaultTh;
    SINT32          blockIdx;
    SINT32          blockPos;
    SINT32          coeffIdx;
    xin_scan_pos    *scanOrder;
    SINT32          posX;
    SINT32          posY;
    SINT32          blockX;
    SINT32          blockY;
    SINT32          coeffVal;
    SINT32          level;
    SINT64          minPathCost;
    SINT64          pathCost;
    SINT32          prevId;
    SINT32          absLevel;
    SINT32          stateId;
    SINT32          qIdx;
    UINT32          compType;
    UINT32          lgWidth;
    UINT32          lgHeight;
    xin_tu_param    *tuParam;
    xin_scan_info   *scanInfo;
    SINT32          idx;
    SINT32          numCtx;
    SINT32          effWidth;
    SINT32          effHeight;
    UINT64          nzCGBitMap;
    xin_dp_param    *dpParam;
    SINT32          iqScale;
    SINT32          iqShift;
    SINT32          iqAdd;
    BOOL            isNZBlock;

    compType    = compId != PLANE_LUMA;
    lgWidth     = calcLog2[width];
    lgHeight    = calcLog2[height];
    dpParam     = &depQuant->dpParam[lgWidth + lgHeight][qp];
    scanOrderCG = tu->scanOrderCG[compType];
    scanOrder   = tu->scanOrder[compType];
    scanOrder   = tu->scanOrder[compType];
    tuParam     = depQuant->tuParam[lgWidth][lgHeight][compType];
    scanInfo    = tuParam->scanInfo;
    effWidth    = XIN_MIN (width, 32);
    effHeight   = XIN_MIN (height, 32);
    nzCGBitMap  = 0;
    defaultTh   = dpParam->thresLast / (dpParam->qScale << 2);
    funcSet     = depQuant->funcSet;
    numCtx      = compType == PLANE_LUMA ? 21 : 11;

    Xin266InitContext (
        depQuant,
        fastBuf,
        tuParam,
        depQuant->context,
        compId);

    funcSet->pfXinPreDepQuant (
        tCoeff,
        rCoeff,
        qCoeff,
        coeffStride,
        effWidth,
        effHeight,
        4,
        4,
        defaultTh,
        scanOrderCG,
        scanOrder,
        &coeffIdx);

    if (coeffIdx < 0)
    {
        *nzCGBitMapRs = 0;

        return;
    }

    Xin266DepQuantReset (
        depQuant,
        tuParam);

    for (stateId = 0; stateId < 4; stateId++)
    {
        Xin266StateInit (
            &depQuant->simdStateCurr,
            stateId);

        Xin266StateInit (
            &depQuant->simdStateSkip,
            stateId);

        depQuant->simdStateCurr.sigFracBitsArray[stateId] = depQuant->sigFracBits[XIN_MAX(stateId - 1, 0)];
    }

    depQuant->simdStateCurr.gtxFracBitsArray = depQuant->gtxFracBits;

    memset (depQuant->simdStateCurr.sum1st, 0, sizeof(UINT8)*64);

    for (idx = 0; idx < numCtx; idx++)
    {
        depQuant->simdStateCurr.coeffBits1[idx] = depQuant->gtxFracBits[idx].intBits[1];
    }

    depQuant->simdStateCurr.initRemRegBins   = (effWidth * effHeight * MAX_TU_LEVEL_CTX_CODED_BIN) / 16;
    depQuant->simdStateCurr.anyRemRegBinsLt4 = TRUE;

    blockIdx = coeffIdx >> 4;
    coeffIdx = coeffIdx & 15;

    for (; blockIdx >= 0; blockIdx--)
    {
        blockX = scanOrderCG[blockIdx].posX * 4;
        blockY = scanOrderCG[blockIdx].posY * 4;

        for (; coeffIdx >= 0; coeffIdx--)
        {
            posX     = scanOrder[coeffIdx].posX + blockX;
            posY     = scanOrder[coeffIdx].posY + blockY;
            coeffVal = tCoeff[posX + posY*coeffStride];

            Xin266DecideUpdate (
                depQuant,
                dpParam,
                XIN_ABS (coeffVal),
                scanInfo + blockIdx*16+coeffIdx,
                FALSE);

        }

        coeffIdx = 15;

    }

    //===== find best path =====
    minPathCost =  0;
    prevId      = -1;

    for (stateId = 0; stateId < 4; stateId++)
    {
        pathCost = depQuant->trellis[0][0].rdCost[stateId];

        if (pathCost < minPathCost)
        {
            prevId      = stateId;
            minPathCost = pathCost;
        }
    }

    //===== backward scanning =====
    for (coeffIdx = 0; prevId >= 0; coeffIdx++)
    {
        absLevel = depQuant->trellis[coeffIdx][prevId >> 2].absLevel[prevId & 3];
        posX     = scanInfo[coeffIdx].posX;
        posY     = scanInfo[coeffIdx].posY;
        coeffVal = tCoeff[posX + posY*coeffStride];
        prevId   = depQuant->trellis[coeffIdx][prevId >> 2].prevId[prevId & 3];

        qCoeff[posX + posY*coeffStride] = coeffVal < 0 ? (COEFF)(-absLevel) : (COEFF)(absLevel);
    }

    // inverse quantization
    coeffIdx = coeffIdx - 1;
    blockIdx = coeffIdx >> 4;
    coeffIdx = coeffIdx & 15;
    stateId  = 0;
    iqScale  = dpParam->iqScale;
    iqShift  = dpParam->iqShift;
    iqAdd    = 1 << (iqShift - 1);

    for (; blockIdx >= 0; blockIdx--)
    {
        blockX    = scanOrderCG[blockIdx].posX * 4;
        blockY    = scanOrderCG[blockIdx].posY * 4;
        blockPos  = scanOrderCG[blockIdx].posIdx;
        isNZBlock = FALSE;

        for (; coeffIdx >= 0; coeffIdx--)
        {
            posX  = scanOrder[coeffIdx].posX + blockX;
            posY  = scanOrder[coeffIdx].posY + blockY;
            level = qCoeff[posX + posY*coeffStride];

            if (level)
            {
                qIdx      = 2 * level + (level > 0 ? -(stateId >> 1) : (stateId >> 1));
                coeffVal  = (qIdx * iqScale + iqAdd) >> iqShift;
                isNZBlock = TRUE;

                rCoeff[posX + posY*coeffStride] = (COEFF)XIN_CLIP (coeffVal, XIN_MIN_S16, XIN_MAX_S16);
            }

            stateId = (32040 >> ((stateId<<2)+((level&1)<<1))) & 3;
        }

        nzCGBitMap |= (UINT64)isNZBlock << blockPos;
        coeffIdx    = 15;

    }

    *nzCGBitMapRs = nzCGBitMap;

}



