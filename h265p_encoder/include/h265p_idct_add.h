/***************************************************************************//**
*
* @file          h265p_idct_add.h
* @brief         This file declare inverse trans and add prediction subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_idct_add_h_
#define _h265p_idct_add_h_

void Xin265pIDctAdd (
    xin_sec_struct *secSet,
    SINT32         *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    PIXEL          *output,
    intptr_t       outputStride,
    UINT32         tranType,
    UINT32         tranSize);

#endif

