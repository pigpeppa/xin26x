# CMakeLists.txt
#
# Build configuration for the h266 core module.
#
# Xin26x Video Codec Library
#
# Copyright (C) 2020-2026 Chao Zhou <czhou2@qq.com>
#
# This file is part of Xin26x.
#
# Licensed under the GNU General Public License, Version 3 or later
# (GPL-3.0-or-later). See the LICENSE file for details.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

#
# Add include directories here
#
include_directories (
    "${PROJECT_SOURCE_DIR}/video_api/video_common/include"
    "${PROJECT_SOURCE_DIR}/common/include"
    "${PROJECT_SOURCE_DIR}/h26x_core/include"
    "${PROJECT_SOURCE_DIR}/video_api/video_encoder/include"
if(_ARM_OPT_)
    "${PROJECT_SOURCE_DIR}/neon_emul/include"
endif()
    "${PROJECT_SOURCE_DIR}/h266_core/include")

FILE(GLOB cmake_includes *.h)
FILE(GLOB cmake_sources source/*.c)

if(USE_INTRINSICS)
    if(PROC_ARCH_ARM)
        FILE(GLOB cmake_arm_sources source/arm/*.c)
        SET(cmake_sources ${cmake_sources} ${cmake_arm_sources})
    else()
        FILE(GLOB cmake_x86_sources source/x86/*.c)
        SET(cmake_sources ${cmake_sources} ${cmake_x86_sources})
    endif()
endif(USE_INTRINSICS)

add_library (${H266CORE_LIB} OBJECT ${cmake_sources} ${cmake_includes})

