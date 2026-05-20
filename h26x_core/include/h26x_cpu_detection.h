/***************************************************************************//**
*
* @file          h26x_cpu_detection.h
* @brief         This file contain definitions related to instruction sets.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/

#ifndef _h26x_cpu_detection_h_
#define _h26x_cpu_detection_h_

#define XIN_CPU_MMX        0x00000001
#define XIN_CPU_MMXEXT     0x00000002
#define XIN_CPU_SSE        0x00000004
#define XIN_CPU_SSE2       0x00000008
#define XIN_CPU_SSE3       0x00000010
#define XIN_CPU_SSE41      0x00000020
#define XIN_CPU_3DNOW      0x00000040
#define XIN_CPU_3DNOWEXT   0x00000080
#define XIN_CPU_ALTIVEC    0x00000100
#define XIN_CPU_SSSE3      0x00000200
#define XIN_CPU_SSE42      0x00000400

#define XIN_CPU_AVX        0x00000800
#define XIN_CPU_FPU        0x00001000
#define XIN_CPU_HTT        0x00002000
#define XIN_CPU_CMOV       0x00004000
#define XIN_CPU_MOVBE      0x00008000
#define XIN_CPU_AES        0x00010000
#define XIN_CPU_FMA        0x00020000
#define XIN_CPU_AVX2       0x00040000

#define XIN_CPU_CACHELINE_16    0x10000000
#define XIN_CPU_CACHELINE_32    0x20000000
#define XIN_CPU_CACHELINE_64    0x40000000
#define XIN_CPU_CACHELINE_128   0x80000000

#define XIN_CPU_ARMv7      0x000001
#define XIN_CPU_VFPv3      0x000002
#define XIN_CPU_NEON       0x000004

void Xin26xCpuDetection (
    UINT32      *cpuFeaturePtr,
    UINT32      *cpuCoreNumPtr);

#endif

