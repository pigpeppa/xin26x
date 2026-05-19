/***************************************************************************//**
*
* @file          h265p_super_block_struct.h
* @brief         This files defines av1 super block related structure.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_super_block_struct_h_
#define _h265p_super_block_struct_h_

typedef struct xin_sb_struct
{
    UINT32      sbX;
    UINT32      sbY;
    
    UINT32      sbPelX;
    UINT32      sbPelY;

    UINT32      lgWidth;
    UINT32      width;
    UINT32      height;
    UINT32      pixelNum;

    UINT32      availField;

    UINT32      sbIndex;
    UINT32      sbAddr;

    UINT32      vaildCuCount;   // Count non-skip CU number in CTU
    UINT32      cuNumInCtu;

    UINT32      qp;

    UINT32      minDepth;
    UINT32      maxDepth;

    UINT32      bitPlan;
    SINT32      bitUsed;
    
}xin_sb_struct;

#endif

