cmake_minimum_required(VERSION 3.17)
project(pp_tasks)

set(CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
set(CMAKE_C_FLAGS "${CMAKE_FLAGS} ${MPI_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_EXE_LINKER_FLAGS}")

add_executable(determinant determinant.cpp)
target_link_libraries(determinant ${MPI_CXX_LIBRARIES})

add_executable(merge_sort merge_sort.cpp)
target_link_libraries(merge_sort ${MPI_CXX_LIBRARIES})

add_executable(floyd_algorithm floyd_algorithm.cpp)
target_link_libraries(floyd_algorithm ${MPI_CXX_LIBRARIES})