cmake_minimum_required(VERSION 3.10)

project(mandelbrot)

set(CMAKE_VERBOSE_MAKEFILE 1)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_CXX_FLAGS "-O3 -march=native -mtune=native ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using icc
    set(CMAKE_CXX_FLAGS "-O3 -mavx2 -xHost -g -qopenmp-simd -qopt-report=5 -qopt-report-phase=vec")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif()

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIRS})




set(SOURCE_FILES
    calculators/BaseMandelCalculator.cc
    calculators/BatchMandelCalculator.cc
    calculators/LineMandelCalculator.cc
    calculators/RefMandelCalculator.cc
    common/cnpy.cc
    main.cc
)

include_directories(common)
include_directories(calculators)

add_executable(mandelbrot ${SOURCE_FILES})
target_link_libraries(mandelbrot ${ZLIB_LIBRARIES})
