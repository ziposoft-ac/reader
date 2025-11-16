cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 23)

include(CMakePrintHelpers)

project(ziposoft)
option(ZS_BUILD_ARM "Compile for ARM" OFF)
option(ZS_BUILD_STATIC "Compile STATIC" OFF)
option(DEBUG "Debug" OFF)
option(BIN_DIR "Binary dir" OFF)
set(BUILDTYPE "x64" CACHE STRING "pi/x64")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    message("debug mode")
    set(DEBUG 1)

endif ()
set( PROJECT_ROOT  ${CMAKE_CURRENT_SOURCE_DIR})


if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    add_compile_options(-DARM=1)
    set(ZS_BUILD_ARM 1)

endif ()
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin/${BUILDTYPE}/ )
set( CMAKE_BINARY_DIR /zs/build/timer/${BUILDTYPE}/ )




if (DEBUG)
    add_definitions( -DDEBUG -DZ_TRACE_ENABLE)
else()


endif ()


cmake_print_variables(DEBUG ZS_BUILD_ARM ZS_BUILD_STATIC CMAKE_BUILD_TYPE)

add_subdirectory(libs)
add_subdirectory(zipolib)
add_subdirectory(reader3)
add_subdirectory(DebugView)
add_subdirectory(tests)
