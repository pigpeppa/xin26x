#
# Add include directories here
#
include_directories (
    "${PROJECT_SOURCE_DIR}/video_api/video_common/include"
    "${PROJECT_SOURCE_DIR}/third_party/getopt"
    "${PROJECT_SOURCE_DIR}/video_api/h265_encoder/include"
    "${PROJECT_SOURCE_DIR}/app/common/include")

FILE(GLOB cmake_includes include/*.h)
FILE(GLOB cmake_sources source/*.c)

add_library(${APP_COMMON_LIB} ${cmake_sources} ${cmake_includes})

