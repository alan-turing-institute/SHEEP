cmake_minimum_required (VERSION 3.5)
project (Sheep)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/HElib/src/")
set(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/lib/HElib/src/")

# C++ REST API for interaction with web app - also needs openSSL, Boost, Threads
set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR} /usr/local/opt/openssl)
find_package(OpenSSL)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(cpprestsdk REQUIRED)
find_package(Boost COMPONENTS system filesystem thread date_time program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
find_package(Threads REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

enable_testing()

include_directories(include)

# Flags for profiling
# set (CMAKE_C_FLAGS " -pg ")
# set (CMAKE_CXX_FLAGS " -pg ")

find_package(LP)
if (LP_FOUND)
  include_directories(SYSTEM ${LP_INCLUDE_DIRS})
  find_library(GMP_LIBRARIES NAMES gmp)
  add_definitions(-DHAVE_LP)
endif (LP_FOUND)

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
  add_definitions(-DHAVE_SEAL_BFV)
  add_definitions(-DHAVE_SEAL_CKKS)
endif()

link_libraries(pthread)

add_subdirectory(applications)
add_subdirectory(tests)
