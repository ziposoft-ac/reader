cmake_minimum_required(VERSION 3.16)
project(reader3)

set(CMAKE_CXX_STANDARD 17)
include_directories(
        ../zipolib/include/
        ../libs
)



set(TARGET reader3)





file(GLOB SRC "src/*.cpp")
file(GLOB HEADERS "src/*.h")
add_link_options(  -pthread    )

add_executable(${TARGET}  ${SRC}  ${HEADERS}  )
add_dependencies(${TARGET} timestamp)
add_custom_target(timestamp
        COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/src/timestamp.cpp
        OUTPUT src/timestamp.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "setting version...")
add_definitions( -DUNIX -DLINUX)
#link_libraries(pthread ixwebsocket z   websockets ev uv ssl crypto cap dl)
#link_libraries(pthread ixwebsocket z   websockets ev uv ssl crypto cap dl)
#add_link_options(   -static  -static-libstdc++ -static-libgcc     -lc  -Wl,--whole-archive -lpthread -Wl,--no-whole-archive   )
target_link_libraries(${TARGET}   zipolib libz.a   dl gpiod pthread parson )

if (ZS_BUILD_STATIC)
    add_compile_options(-fPIC -static)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    add_link_options(  -pthread -static   )
    message( "ZS_BUILD_STATIC For PI")

endif()

if (ZS_BUILD_ARM)
    message( "Building For PI")

    target_compile_definitions(${TARGET} PUBLIC ARM=1)

    target_link_libraries(${TARGET} mongoose libcap.a)

    #add_custom_command(TARGET ${TARGET} POST_BUILD   COMMAND  scp ${TARGET} pi@timer2.local:/home/pi/rfid/        )
    #add_custom_command(TARGET ${TARGET} POST_BUILD   COMMAND  scp ${TARGET} pi@10.1.1.1:/home/pi/rfid/        )
else()
    target_link_libraries(${TARGET} mongoose cap)
    #target_compile_definitions(${TARGET} PUBLIC NOGPIO=1)

endif()


add_custom_command(
        COMMAND touch src/timestamp.cpp
        OUTPUT src/timestamp.cpp
        DEPENDS  ${SRC}  ${HEADERS}
        )
target_precompile_headers(${TARGET} PRIVATE src/pch.h  )
#target_compile_options(${TARGET} PRIVATE -DUNIX   -static -fPIC)
target_compile_options(${TARGET} PRIVATE -DUNIX   -fPIC)

#set_target_properties(${TARGET} PROPERTIES UNITY_BUILD ON)
add_compile_options(
        $<$<CONFIG:DEBUG>-DDEBUG>
)
