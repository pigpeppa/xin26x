#
# Add include directories here
#
include_directories (
    "${PROJECT_SOURCE_DIR}/video_api/video_common/include"
    "${PROJECT_SOURCE_DIR}/common/include"
    "${PROJECT_SOURCE_DIR}/h26x_core/include"
    "${PROJECT_SOURCE_DIR}/h265p_core/include"
    "${PROJECT_SOURCE_DIR}/video_api/video_encoder/include"
if(_ARM_OPT_)
    "${PROJECT_SOURCE_DIR}/neon_emul/include"
endif()
    "${PROJECT_SOURCE_DIR}/h265p_encoder/include")

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


add_library (${AV1ENC_LIB} OBJECT ${cmake_sources} ${cmake_includes})

