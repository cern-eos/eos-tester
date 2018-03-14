cmake_minimum_required(VERSION 2.6)
project(grid-hammer)
include(GNUInstallDirs)


#-------------------------------------------------------------------------------
# Search for dependencies
#-------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(XRootD REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(deps)

#-------------------------------------------------------------------------------
# Set-up folly
#-------------------------------------------------------------------------------
add_subdirectory(deps/folly)
include_directories(
  deps/folly/
  "${CMAKE_BINARY_DIR}/deps/folly/"
)

#-------------------------------------------------------------------------------
# Compiler options
#-------------------------------------------------------------------------------
add_definitions(-Wall -Wextra -Werror -Wno-unused-parameter -g3 -std=c++14)

#-------------------------------------------------------------------------------
# Build.
#-------------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(test)
