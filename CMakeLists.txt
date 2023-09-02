# This File is only development -> IDE and not needed for build / compile

cmake_minimum_required(VERSION 3.10)
project(zlib)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(R REQUIRED)
message("R libraries: ${R_LIBRARIES}")
message("R include dirs: ${R_INCLUDE_DIR} ${Rcpp_INCLUDE_DIR}")

# Add inst/include directory for libdeflate headers
include_directories(inst/include)
link_directories(inst/lib64)

add_library(zlib SHARED src/decompressor.cpp src/Makevars
        src/validate.cpp
        src/compressor.cpp
        src/constants.cpp)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(zlib ${R_INCLUDE_DIR} ${Rcpp_INCLUDE_DIR})
target_link_libraries(zlib ${R_LIBRARIES})
