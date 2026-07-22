cmake_minimum_required(VERSION 3.10)
project(addon_module)
# Include Node-API headers
include_directories(${CMAKE_JS_INC})
include_directories(node_modules/node-addon-api)

# Define the shared library target
add_library(addon MODULE main.cpp)

# Link Node-API libs and strip prefixes
target_link_libraries(addon ${CMAKE_JS_LIB})
set_target_properties(addon PROPERTIES
        PREFIX ""
        SUFFIX ".node"
)