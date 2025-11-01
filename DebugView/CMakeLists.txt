cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
include_directories(
        ../zipolib/include/

)



project(DebugView)



file(GLOB SRC "src/*.cpp")
file(GLOB HEADERS "src/*.h")

add_executable(DebugView  ${SRC}  ${HEADERS} )
add_definitions( -DUNIX -DLINUX)
target_link_libraries(DebugView  zipolib  )



if (ZS_BUILD_ARM)
    target_compile_definitions(DebugView PUBLIC ARM=1)


else()


endif()

if (NOT_USED)
    add_custom_command(TARGET DebugView POST_BUILD
            COMMAND  scp DebugView pi@10.1.1.1:/home/pi/test/
    )
endif()

add_custom_command(
        COMMAND touch src/timestamp.cpp
        OUTPUT src/timestamp.cpp
        DEPENDS  ${SRC}  ${HEADERS}
        )
target_precompile_headers(DebugView PRIVATE src/pch.h  )
target_compile_options(DebugView PRIVATE -DUNIX   -fPIC)

