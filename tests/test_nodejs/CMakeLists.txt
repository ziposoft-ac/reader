cmake_minimum_required(VERSION 3.15)
project(node_cmake_addon)

# Include Node-API headers provided by the npm module
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-addon-api)

# Declare your addon as a shared library
add_library(${PROJECT_NAME} SHARED addon.cpp)

# Required configuration for Node.js addons
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Ensure proper compiler flags are loaded by cmake-js
if(CMAKE_JS_INC)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
    target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
endif()






cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 14)
include_directories(
        ../../zipolib/include/
        ../../libs/

)



set(TARGET test_nodejs)



file(GLOB SRC "*.c")
file(GLOB HEADERS "*.h")

add_executable(${TARGET}  ${SRC}  ${HEADERS} )
add_definitions( -DUNIX -DLINUX)
target_link_libraries(${TARGET}  zipolib  gpiod )


