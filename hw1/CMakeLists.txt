cmake_minimum_required(VERSION 2.8)

project(alignment)

add_definitions(-O2 --std=c++0x)
add_definitions(-fopenmp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(ALIGN_SRC ibm_model1.hpp preprocess.hpp align.cpp)

add_executable(align ${ALIGN_SRC})

target_link_libraries(align gomp)