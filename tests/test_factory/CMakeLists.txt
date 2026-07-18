cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 14)
include_directories(
        ../../zipolib/include/
        ../../libs/

)



set(TARGET test_fact)


#target_compile_options(${TARGET} PRIVATE -DUNIX   -fPIC)

file(GLOB SRC "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(${TARGET}  ${SRC}  ${HEADERS} )
add_definitions( -DUNIX -DLINUX)
target_link_libraries(${TARGET}  zipolib   )


