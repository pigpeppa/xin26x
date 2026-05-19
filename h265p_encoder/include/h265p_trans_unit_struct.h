/***************************************************************************//**
*
* @file          h265p_trans_unit_struct.h
* @brief         Transform unit struct definition.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_trans_unit_struct_h_
#define _h265p_trans_unit_struct_h_

typedef struct xin_tu_struct
{
    UINT8       tuIdx;
    UINT8       txType;
    UINT8       txSize;
    UINT8       partIdx;
    UINT8       offsetX;    // x offet within current coding unit
    UINT8       offsetY;    // y offset within current coding unit

    UINT8       width;
    UINT8       height;
    UINT8       lgWidth;
    UINT8       lgHeight;
    UINT8       widthInMi;
    UINT8       heightInMi;

    BOOL        splitFlag;

    SINT32      eob;

    UINT8       culLevel;
    
    SINT32      dcSignCtx;
    SINT32      txSkipCtx;

    COEFF       *qCoeff;
    intptr_t    coeffStride;
    
}xin_tu_struct;

#endif

