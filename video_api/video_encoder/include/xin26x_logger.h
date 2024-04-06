/***************************************************************************//**
*
* @file          xin26x_logger.h
* @brief         This files contains h26x logger definition.
* @authors       Pig Peppa
* @copyright     (c) 2020, Pig Peppa <pig.peppa@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin26x_logger_h_
#define _xin26x_logger_h_
#include "xin_typedef.h"

typedef void (*XinLogEntry) (
    UINT32   level,
    char*    pMsg,
    ...);

void Xin26xLogEntry (
    UINT32   level,
    char*    pMsg,
    ...);

extern void (*pfXin26xLogEntry) (
    UINT32   level,
    char*    pMsg,
    ...);

#define XIN_LOGGER_NONE          (-1)
#define XIN_LOGGER_ERROR          0
#define XIN_LOGGER_WARNING        1
#define XIN_LOGGER_STATUS         2
#define XIN_LOGGER_DEBUG          3

#endif