SET( DIET_CMAKE_OMNINAMES_LOGDIR "${DIET_BINARY_DIR}/src/Testing" )

# General customisation of ctest (due to some oddity of ctest, this file
# has to be at the root of the binary tree): 
CONFIGURE_FILE(
  CTestCustom.ctest.in
  ${DIET_BINARY_DIR}/CTestCustom.ctest
  IMMEDIATE
)

# Utility for starting/stoping a minimalitic omniNames server:
CONFIGURE_FILE(
  OmniDaemon.bash.in
  ${DIET_BINARY_DIR}/src/Testing/OmniDaemon.bash
  IMMEDIATE
)

