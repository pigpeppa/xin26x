#
# Add include directories here
#

FILE(GLOB cmake_includes getopt/*.h)
FILE(GLOB cmake_sources getopt/*.c)

set_source_files_properties(getopt/getopt.c PROPERTIES COMPILE_FLAGS "-DHAVE_STRING_H=1")
if(MSVC)
    set_source_files_properties(getopt/getopt.c PROPERTIES COMPILE_FLAGS "/wd4100 /wd4131 -DHAVE_STRING_H=1")
endif(MSVC)

add_library(${THIRD_PARTY_LIB} ${cmake_sources} ${cmake_includes})

