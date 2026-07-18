cmake_minimum_required(VERSION 3.16)
project(reader3)

set(CMAKE_CXX_STANDARD 17)
include_directories(
        ../zipolib/include/
        ../libs
)





set(SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SRC "src/*.cpp")
file(GLOB SRC_TEST "src/tests/*.cpp")
file(GLOB SRC_RFID "src/rfid/*.cpp")
file(GLOB SRC_IO "src/io/*.cpp")
file(GLOB SRC_COMMS "src/api/*.cpp")
file(GLOB SRC_WEB "src/web/*.cpp")
file(GLOB SRC_UTIL "src/util/*.cpp")
file(GLOB HEADERS "src/*.h")
add_link_options(  -pthread    )



add_definitions( -DUNIX -DLINUX)
#link_libraries(pthread ixwebsocket z   websockets ev uv ssl crypto cap dl)
#link_libraries(pthread ixwebsocket z   websockets ev uv ssl crypto cap dl)
#add_link_options(   -static  -static-libstdc++ -static-libgcc     -lc  -Wl,--whole-archive -lpthread -Wl,--no-whole-archive   )



#set_target_properties(${TARGET} PROPERTIES UNITY_BUILD ON)


add_custom_target(timestamp
        COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/src/timestamp.cpp
        src/timestamp.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "setting version...")


function(target_func TARGET )
    add_executable(${TARGET} src/timestamp.cpp src/main.cpp ${ARGN}  ${SRC_IO}  ${SRC_UTIL} ${SRC_COMMS}  ${HEADERS}  )
    add_dependencies(${TARGET} timestamp)
    target_link_libraries(${TARGET}   zipolib libz.a   dl gpiod pthread parson )
    target_precompile_headers(${TARGET} PRIVATE src/pch.h  )
    #target_compile_options(${TARGET} PRIVATE -DUNIX   -static -fPIC)
    target_compile_options(${TARGET} PRIVATE -DUNIX   -fPIC)
    target_include_directories(${TARGET} PRIVATE ${SRC_DIR} ${SRC_DIR}/comms/ ${SRC_DIR}/util/)
    if (ZS_BUILD_ARM)
        message( "Building For PI")

        target_compile_definitions(${TARGET} PUBLIC ARM=1)

        target_link_libraries(${TARGET} mongoose libcap.a)

        #add_custom_command(TARGET ${TARGET} POST_BUILD   COMMAND  scp ${TARGET} pi@timer2.local:/home/pi/rfid/        )
        #add_custom_command(TARGET ${TARGET} POST_BUILD   COMMAND  scp ${TARGET} pi@10.1.1.1:/home/pi/rfid/        )
    else()
        target_link_libraries(${TARGET} mongoose cap)
        target_compile_definitions(${TARGET} PUBLIC NOGPIO=1)
        message( "Default Build")

    endif()

endfunction()


target_func(omnibus src/rfid/ReaderService.cpp src/root.cpp ${SRC_RFID} src/leds/LedService.cpp)
target_func(ledService src/leds/LedService.cpp src/leds/LedApi.cpp)
target_func(beepService src/beep/BeepService.cpp)

add_compile_options(
        $<$<CONFIG:DEBUG>-DDEBUG>
)

if (ZS_BUILD_STATIC)
    add_compile_options(-fPIC -static)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    add_link_options(  -pthread -static   )
    message( "ZS_BUILD_STATIC For PI")

endif()



