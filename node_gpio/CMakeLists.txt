cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 14)
include_directories(
        ../zipolib/include/
        /usr/local/lib/node_modules/node-addon-api/
        /usr/local/include/node/
)



project(node_gpio)



file(GLOB SRC "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(node_gpio  ${SRC}  ${HEADERS} )
add_definitions( -DUNIX -DLINUX)
target_link_libraries(node_gpio  zipolib  )



if (ZS_BUILD_ARM)
    target_compile_definitions(node_gpio PUBLIC ARM=1)


else()


endif()

if (NOT_USED)
    add_custom_command(TARGET DebugView POST_BUILD
            COMMAND  scp DebugView pi@10.1.1.1:/home/pi/test/
    )
endif()


target_precompile_headers(node_gpio PRIVATE pch.h  )
target_compile_options(node_gpio PRIVATE -DUNIX   -fPIC)

