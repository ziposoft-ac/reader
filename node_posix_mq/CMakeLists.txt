cmake_minimum_required(VERSION 3.10)
project(node-zmq)
# Include Node-API headers
include_directories(${CMAKE_JS_INC})
include_directories(/usr/local/include/node/)

# Define the shared library target
add_library(zmq MODULE posixmq.cpp)

# Link Node-API libs and strip prefixes
target_link_libraries(zmq ${CMAKE_JS_LIB})
set_target_properties(zmq PROPERTIES
        PREFIX ""
        SUFFIX ".node"
)