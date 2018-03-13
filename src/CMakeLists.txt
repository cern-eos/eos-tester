#-------------------------------------------------------------------------------
# Source files
#-------------------------------------------------------------------------------
add_library(eostester STATIC
  HashCalculator.cc                                      HashCalculator.hh
  HierarchyBuilder.cc                                    HierarchyBuilder.hh
  Manifest.cc                                            Manifest.hh
  SelfCheckedFile.cc                                     SelfCheckedFile.hh
  Utils.cc                                               Utils.hh
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
  ${OPENSSL_LIBRARIES}
  ${XROOTD_CL}
)
