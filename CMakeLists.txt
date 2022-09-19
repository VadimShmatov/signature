# CMakeList.txt : CMake project for Signature, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Signature" VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

string(REPLACE "-O2" "-O0" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-O2" "-O0" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

add_subdirectory (src)

enable_testing ()
add_subdirectory (test)