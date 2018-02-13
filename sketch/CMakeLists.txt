cmake_minimum_required (VERSION 3.1)
project (Sheep)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

enable_testing()

include_directories(include)

find_package(TFHE)
if (TFHE_FOUND)
    include_directories(SYSTEM ${TFHE_INCLUDE_DIRS})
endif (TFHE_FOUND)

add_subdirectory(example bin)
