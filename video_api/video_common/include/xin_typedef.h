/***************************************************************************//**
*
* @file          xin_typedef.h
* @brief         data type definition
* @authors       Pig Peppa
* @copyright     (c) 2020, Pig Peppa <pig.peppa@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _xin_typedef_h_
#define _xin_typedef_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if defined(_MSC_VER) 
#define inline __inline
#elif defined(__linux__) || defined(__APPLE__)
#define inline __inline__
#endif

#ifndef NULL
#define NULL    ((void*)0)
#endif

#ifndef FALSE
#define FALSE   (1 == 0)
#endif

#ifndef TRUE
#define TRUE    (1 == 1)
#endif

#define XIN_SUCCESS     0
#define XIN_FAIL        -1

typedef void * XIN_HANDLE;

#ifdef _MSC_VER

typedef unsigned __int8     UINT8;
typedef unsigned __int16    UINT16;
typedef unsigned __int32    UINT32;
typedef unsigned __int64    UINT64;
typedef signed __int8       SINT8;
typedef signed __int16      SINT16;
typedef signed __int32      SINT32;
typedef signed __int64      SINT64;
typedef signed __int32      BOOL;

#else

typedef uint8_t             UINT8;
typedef uint16_t            UINT16;
typedef uint32_t            UINT32;
typedef uint64_t            UINT64;
typedef int8_t              SINT8;
typedef int16_t             SINT16;
typedef int32_t             SINT32;
typedef int64_t             SINT64;
typedef int32_t             BOOL;

#endif

typedef float               FLOAT32;
typedef double              FLOAT64;


// Define pixel and coefficient data type
#define PIXEL               UINT8
#define PIXEL4              UINT32
#define PIXEL2              UINT16
#define COEFF               SINT16
#define MIN_PIXEL_VALUE     (0)
#define MAX_PIXEL_VALUE     (255)
#define MAX_COEFF_VALUE     (32767)
#define MIN_COEFF_VALUE     (-32768)

#ifdef __cplusplus
}
#endif
#endif