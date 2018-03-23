include_directories(
  ${XROOTD_INCLUDE_DIR}
)

#-------------------------------------------------------------------------------
# Source files
#-------------------------------------------------------------------------------
add_library(eostester STATIC
  testcases/TreeBuilder.cc                               testcases/TreeBuilder.hh
  testcases/TreeValidator.cc                             testcases/TreeValidator.hh
                                                         utils/AssistedThread.hh
  utils/ProgressTicker.cc                                utils/ProgressTicker.hh
  utils/ProgressTracker.cc                               utils/ProgressTracker.hh
                                                         utils/Sealing.hh
  utils/TestcaseStatus.cc                                utils/TestcaseStatus.hh
  HashCalculator.cc                                      HashCalculator.hh
  HierarchyBuilder.cc                                    HierarchyBuilder.hh
  Manifest.cc                                            Manifest.hh
  SelfCheckedFile.cc                                     SelfCheckedFile.hh
  Styling.cc                                             Styling.hh
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
