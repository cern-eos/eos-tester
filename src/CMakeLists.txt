include_directories(
  ${XROOTD_INCLUDE_DIR}
)

#-------------------------------------------------------------------------------
# Source files
#-------------------------------------------------------------------------------
add_library(eostester STATIC
  testcases/TreeBuilder.cc                               testcases/TreeBuilder.hh
                                                         utils/AssistedThread.hh
  utils/ErrorAccumulator.cc                              utils/ErrorAccumulator.hh
  HashCalculator.cc                                      HashCalculator.hh
  HierarchyBuilder.cc                                    HierarchyBuilder.hh
  Manifest.cc                                            Manifest.hh
  SelfCheckedFile.cc                                     SelfCheckedFile.hh
  Utils.cc                                               Utils.hh
  XrdClExecutor.cc                                       XrdClExecutor.hh
)

#-------------------------------------------------------------------------------
# eos-tester executable
#-------------------------------------------------------------------------------
add_executable(eos-tester
  main.cc
)

#-------------------------------------------------------------------------------
# Link.
#-------------------------------------------------------------------------------
target_link_libraries(eos-tester
  eostester
  folly
  ${OPENSSL_LIBRARIES}
  ${XROOTD_CL}
)
