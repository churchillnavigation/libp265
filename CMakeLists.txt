cmake_minimum_required(VERSION 3.21)

project(libp265
    LANGUAGES C CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(Threads REQUIRED)

include(GNUInstallDirs)
include(CheckFunctionExists)

CHECK_INCLUDE_FILE(malloc.h HAVE_MALLOC_H)
CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)

if (HAVE_MALLOC_H)
  add_definitions(-DHAVE_MALLOC_H)
endif()
if (HAVE_POSIX_MEMALIGN)
  add_definitions(-DHAVE_POSIX_MEMALIGN)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  add_definitions(-Wall)
endif()

option(BUILD_SHARED_LIBS "Build shared library" ON)
if(NOT BUILD_SHARED_LIBS)
  add_definitions(-DLIBP265_STATIC_BUILD)
endif()

add_subdirectory(src)
add_subdirectory(include)