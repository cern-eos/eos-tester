#-------------------------------------------------------------------------------
# Set-up gtest
#-------------------------------------------------------------------------------
set(GTEST "${CMAKE_SOURCE_DIR}/deps/googletest/googletest")
set(GTEST_BINARY_DIR "${CMAKE_BINARY_DIR}/deps/googletest/googletest")
add_subdirectory("${GTEST}" "${GTEST_BINARY_DIR}")

#-------------------------------------------------------------------------------
# Add inclues
#-------------------------------------------------------------------------------
include_directories(
  ../src
  ${XROOTD_INCLUDE_DIR}
)

#-------------------------------------------------------------------------------
# Testception
#-------------------------------------------------------------------------------
add_executable(eos-tester-tests
  base.cc
  hierarchy-builder.cc
  manifest.cc
  self-checked-file.cc
)

add_executable(eos-tester-functional-tests
  functional/xrdcl-executor.cc
)

#-------------------------------------------------------------------------------
# Link
#-------------------------------------------------------------------------------
target_link_libraries(eos-tester-tests
  eostester
  folly
  gtest_main
  ${OPENSSL_LIBRARIES}
  ${XROOTD_CL}
  ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(eos-tester-functional-tests
  eostester
  folly
  gtest_main
  ${OPENSSL_LIBRARIES}
  ${XROOTD_CL}
  ${CMAKE_THREAD_LIBS_INIT}
)
