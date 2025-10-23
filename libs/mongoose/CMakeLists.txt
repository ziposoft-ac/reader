cmake_minimum_required(VERSION 3.20)
project(mongoose C)

set(CMAKE_C_STANDARD 99)
set(SRC "mongoose.c")

add_library(mongoose  ${SRC})


