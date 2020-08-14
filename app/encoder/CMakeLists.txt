include_directories (
if(WIN32)
    "${PROJECT_SOURCE_DIR}/third_party/getopt"
endif()
    "${PROJECT_SOURCE_DIR}/video_api/video_common/include"
    "${PROJECT_SOURCE_DIR}/video_api/video_encoder/include"
    "${PROJECT_SOURCE_DIR}/app/common/include"
    "${PROJECT_SOURCE_DIR}/app/encoder/include")

link_directories(
    "${PROJECT_SOURCE_DIR}/lib")
    

FILE(GLOB cmake_includes *.h)
FILE(GLOB cmake_sources  source/*.c)

FILE(GLOB cmake_common_sources  ${PROJECT_SOURCE_DIR}/app/common/source/*.c)
SET(cmake_sources ${cmake_sources} ${cmake_common_sources})

if(WIN32)

    FILE(GLOB cmake_3party_sources  ${PROJECT_SOURCE_DIR}/third_party/getopt/*.c)
    SET(cmake_sources ${cmake_sources} ${cmake_3party_sources})

    set_source_files_properties(${PROJECT_SOURCE_DIR}/third_party/getopt/getopt.c PROPERTIES COMPILE_FLAGS "-DHAVE_STRING_H=1")

    if(MSVC)
        set_source_files_properties(${PROJECT_SOURCE_DIR}/third_party/getopt/getopt.c PROPERTIES COMPILE_FLAGS "/wd4100 /wd4131 -DHAVE_STRING_H=1")
    endif(MSVC)
    
endif()

if(XCODE)

    add_executable(xin26x_test ${cmake_sources} ${cmake_target_obj})
    
else(XCODE)

    add_executable(xin26x_test ${cmake_sources})
    target_link_libraries(xin26x_test ${XIN26X_LIB})
    
endif()

if (UNIX AND (NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin"))
    target_link_libraries(xin26x_test dl m)
endif()