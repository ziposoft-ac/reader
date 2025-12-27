cmake_minimum_required(VERSION 3.16)
project(zipolib)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(include/zipolib)
include_directories(src)



file(GLOB SRC "./src/*.cpp")
file(GLOB HEADERS  "./include/zipolib/*.h")
add_library(zipolib STATIC ${SRC}  ${HEADERS} src/pch.h )
target_precompile_headers(zipolib PRIVATE src/pch.h  )
target_compile_options(zipolib PRIVATE -DUNIX   -static -fPIC)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    add_compile_options(-DARM=1)
endif ()
