cmake_minimum_required (VERSION 3.1)
project (Sheep)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default include and library dirs
# TFHE
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/lib/tfhe/src/include/")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/lib/tfhe/build/libtfhe/")
# HElib
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/lib/HElib/src")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/lib/HElib/src")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

enable_testing()

include_directories(include)

find_package(TFHE)
if (TFHE_FOUND)
    include_directories(SYSTEM ${TFHE_INCLUDE_DIRS})
    add_definitions(-DHAVE_TFHE)
endif (TFHE_FOUND)

find_package(HElib)
if (HElib_FOUND)
   include_directories(SYSTEM ${HElib_INCLUDE_DIRS})
   find_library(NTL_LIBRARIES NAMES ntl)
   find_library(GMP_LIBRARIES NAMES gmp)
   add_definitions(-DHAVE_HElib)
endif (HElib_FOUND)

find_package(TBB)
if (TBB_FOUND)
  link_libraries(tbb)
  include_directories(SYSTEM ${TBB_INCLUDE_DIRS})
  add_definitions(-DHAVE_TBB)
endif()

find_package(SEAL)
if (SEAL_FOUND)
  include_directories(SYSTEM ${SEAL_INCLUDE_DIRS})
  add_definitions(-DHAVE_SEAL)
endif()

add_subdirectory(example bin)
add_subdirectory(tests testbin)
