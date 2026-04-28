#
# Add include directories here
#
include_directories (
    "${PROJECT_SOURCE_DIR}/video_api/video_common/include"
    "${PROJECT_SOURCE_DIR}/common/include"
    "${PROJECT_SOURCE_DIR}/video_api/video_encoder/include"
    "${PROJECT_SOURCE_DIR}/h26x_api/include")

FILE(GLOB cmake_includes *.h)
FILE(GLOB cmake_sources source/*.c)

add_library (${H26XAPI_LIB} OBJECT ${cmake_sources} ${cmake_includes})

