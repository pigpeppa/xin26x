/***************************************************************************//**
*
* @file          h26x_calc_log.h
* @brief         This file block declare log calculation subroutine.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h26x_calc_log_h_
#define _h26x_calc_log_h_

#define XIN_LOG_FRAC_BITS       8
#define XIN_LOG_ROUNDING        (1 << (XIN_LOG_FRAC_BITS - 1))
#define XIN_LOG_FRACTION        (1<<XIN_LOG_FRAC_BITS)
#define XIN_LOG_INT2FLOAT(X)    ((X)/((double)XIN_LOG_FRACTION))
UINT32 XinCalcLog (
    UINT32 value);

#endif
