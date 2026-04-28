#=================================================================================
# The required version of cmake
#=================================================================================
cmake_minimum_required(VERSION 3.5)


#=================================================================================
# Project Name
#=================================================================================
project(xin26x) 

#=================================================================================
# Project version number
#=================================================================================
set(XIN26X_VERSION_MAJOR 1)
set(XIN26X_VERSION_MINOR 0)

#=================================================================================
# Options 
#=================================================================================
OPTION(64BIT               "ON for 64 bit executable, OFF for 32 bit executable"           ON)
OPTION(BUILD_SHARED_LIBS   "ON builds dynamic libraries, OFF builds static libraries"     OFF)
OPTION(ARM_PLATFORM        "ON build for ARM platform"                                    OFF)
OPTION(AV1                 "ON for enable AV1 encoder"                                    OFF)
OPTION(H265                "ON for enable H265 encoder"                                    ON)
OPTION(H266                "ON for enable H266 encoder"                                   OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/testbin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/testbin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBUGINFO ${CMAKE_SOURCE_DIR}/testbin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_SOURCE_DIR}/testbin)
if(H265)
    add_definitions(-DENABLE_H265_ENCODER)
endif()

if(AV1)
    add_definitions(-DENABLE_AV1_ENCODER)
endif()

if(H266)
    add_definitions(-DENABLE_H266_ENCODER)
endif()

if(ARM_PLATFORM)
  set( MY_CPU "arm" )
  add_definitions(-DMATCH_NEON_CODE)
  add_definitions(-DCPU_TYPE_ARM)
else()
  set( MY_CPU "x86" )
  add_definitions(-DMATCH_SSE_CODE)
endif()

#=================================================================================
# Windows - Visual Studio
#=================================================================================
if(WIN32)

    set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -W4 /arch:AVX")        

  if(64BIT)
    add_definitions(-D__x86_64__)
  endif(64BIT)

    add_definitions( -DVC -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_MBCS)  

endif()


#=================================================================================
# Mac - natively build on Mac
#=================================================================================
if(CMAKE_SYSTEM_NAME MATCHES "Darwin" AND (NOT IOS))

  set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -Wall -fno-strict-aliasing -pthread -march=corei7")            

  if(64BIT)
    set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -m64")
    add_definitions(-D__x86_64__)
  else()
    set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -m32")
  endif()

    add_definitions(-D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=0)

endif()


#=================================================================================
# Linux x86 - natively build on Linux 
#=================================================================================
if(UNIX AND (NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin"))

  set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -Wall -fno-strict-aliasing -pthread")

  if(64BIT)
    set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -m64")
    add_definitions(-D__x86_64__)
  else()
    set(CUSTOM_BUILD_FLAGS "${CUSTOM_BUILD_FLAGS} -m32")
  endif(64BIT)

  add_definitions(-D_FILE_OFFSET_BITS=64)

endif()

#=================================================================================
# H26X Library Name
#=================================================================================
set (H265ENC_LIB  "h265_enc")
set (H265CORE_LIB "h265_core")
set (H266CORE_LIB "h266_core")
set (H266ENC_LIB  "h266_enc")
set (AV1CORE_LIB  "h265p_core")
set (AV1ENC_LIB   "h265p_enc")
set (H26XCORE_LIB "h26x_core")
set (H26XAPI_LIB  "h26x_api")
set (XIN26X_LIB   "xin26x")

add_library(${H26XCORE_LIB} STATIC IMPORTED)
set_target_properties(${H26XCORE_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h26x_core/${H26XCORE_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(${H265CORE_LIB} STATIC IMPORTED)
set_target_properties(${H265CORE_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h265_core/${H265CORE_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(${AV1CORE_LIB} STATIC IMPORTED)
set_target_properties(${AV1CORE_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h265p_core/${AV1CORE_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
	
add_library(${H266CORE_LIB} STATIC IMPORTED)
set_target_properties(${H266CORE_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h266_core/${H266CORE_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(${H265ENC_LIB} STATIC IMPORTED)
set_target_properties(${H265ENC_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h265_encoder/${H265ENC_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(${AV1ENC_LIB} STATIC IMPORTED)
set_target_properties(${AV1ENC_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h265p_encoder/${AV1ENC_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
	
add_library(${H266ENC_LIB} STATIC IMPORTED)
set_target_properties(${H266ENC_LIB} PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/h266_encoder/${H266ENC_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
	
add_subdirectory (h26x_api)

set(XIN26X_LIBS)

list(APPEND XIN26X_LIBS ${H26XCORE_LIB})

if(H265)
    list(APPEND XIN26X_LIBS ${H265CORE_LIB})
    list(APPEND XIN26X_LIBS ${H265ENC_LIB})
endif()

if(AV1)
    list(APPEND XIN26X_LIBS ${AV1CORE_LIB})
    list(APPEND XIN26X_LIBS ${AV1ENC_LIB})
endif()

if(H266)
    list(APPEND XIN26X_LIBS ${H266CORE_LIB})
    list(APPEND XIN26X_LIBS ${H266ENC_LIB})
endif()

list(APPEND XIN26X_LIBS ${H26XAPI_LIB})

#=================================================================================
# Build Type
#=================================================================================
add_definitions(${MY_DEFINITIONS})
set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}          ${CUSTOM_BUILD_FLAGS} ")
if (WIN32)
set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}        ${CUSTOM_BUILD_FLAGS} -Oi -GL ")
else (WIN32)
set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}        ${CUSTOM_BUILD_FLAGS} ")
endif(WIN32)
set(CMAKE_C_FLAGS_PROFILE        "${CMAKE_C_FLAGS_PROFILE}        ${CUSTOM_BUILD_FLAGS} ")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${CUSTOM_BUILD_FLAGS} ")
set(CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_C_FLAGS_MINSIZEREL}     ${CUSTOM_BUILD_FLAGS} ")


#=================================================================================
# All the subdirectories need to be compiled
#=================================================================================
add_subdirectory(app)

#=================================================================================
# Message
#=================================================================================

MESSAGE(STATUS "CMAKE_HOST_SYSTEM_NAME: " ${CMAKE_HOST_SYSTEM_NAME})
MESSAGE(STATUS "CMAKE_HOST_SYSTEM_VERSION: " ${CMAKE_HOST_SYSTEM_VERSION})
MESSAGE(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
MESSAGE(STATUS "CMAKE_HOST_SYSTEM: " ${CMAKE_HOST_SYSTEM})
