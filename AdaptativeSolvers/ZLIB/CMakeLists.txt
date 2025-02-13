cmake_minimum_required(VERSION 3.14)
project(ZLIB)

# Add source files
file(GLOB  ALL_SRC_FILES  src/*)

# Declare library
add_library(${PROJECT_NAME} STATIC ${ALL_SRC_FILES})