cmake_minimum_required(VERSION 3.10)

project(PMC CXX C)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE 1)



if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_CXX_FLAGS "-O3 -march=native -mtune=native ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using icc
    set(CMAKE_CXX_FLAGS "-O3 -xHost -qopt-zmm-usage=high -g -qopenmp-simd ")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif()





find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(SOURCE_FILES
    common/base_mesh_builder.h common/base_mesh_builder.cpp
    common/parametric_scalar_field.h common/parametric_scalar_field.cpp
    common/ref_mesh_builder.h common/ref_mesh_builder.cpp
    common/vector_helpers.h common/cxxopts.hpp
    
    parallel_builder/loop_mesh_builder.h   parallel_builder/loop_mesh_builder.cpp
    parallel_builder/tree_mesh_builder.h   parallel_builder/tree_mesh_builder.cpp
    
    main.cpp
)

include_directories(common)
include_directories(parallel_builder)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
