/***************************************************************************//**
*
* @file          h265p_sub_dct.c
* @brief         This file declare av1 substract and forward transform subroutine.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_sub_dct_h_
#define _h265p_sub_dct_h_

void Xin265pSubFDct (
    xin_sec_struct *secSet,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         *output,
    intptr_t       outputStride,
    UINT32         tranType,
    UINT32         tranSize);

#endif

