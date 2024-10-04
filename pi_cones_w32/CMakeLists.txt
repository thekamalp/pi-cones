# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.13)

project(pi_cones C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(${CMAKE_PROJECT_NAME})

# pull in common dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} winmm)

# Add source files
add_subdirectory(../src ./src)
