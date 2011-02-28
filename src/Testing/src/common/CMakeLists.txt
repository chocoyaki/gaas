# setup config_tests.h.in
file( TO_NATIVE_PATH ${BIN_DIR} BIN_DIR_NATIVE_PATH )
file( TO_NATIVE_PATH ${CONFIG_DIR} CONFIG_DIR_NATIVE_PATH )
file( TO_NATIVE_PATH ${OMNINAMES_LOGDIR} OMNINAMES_LOGDIR_NATIVE_PATH )
file( TO_NATIVE_PATH "${CONFIG_DIR}/omniORB4.cfg" OMNIORB_CONFIG_NATIVE_PATH )
file( TO_NATIVE_PATH "${CONFIG_DIR}/agent.cfg" AGENT_CONFIG_NATIVE_PATH )
file( TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/src/agent" DIETAGENT_DIR_NATIVE_PATH )

# System dependent environment variables
SET(TESTENV_LIBRARY_PATH "${PROJECT_BINARY_DIR}/src/client/:${PROJECT_BINARY_DIR}/src/SeD/:")
if (DIET_USE_DAGDA)
  SET(TESTENV_LIBRARY_PATH "${TESTENV_LIBRARY_PATH}${PROJECT_BINARY_DIR}/src/utils/DAGDA:")
endif (DIET_USE_DAGDA)

if (APPLE)
  SET(TESTENV_LIBRARY_PATH_NAME "DYLD_LIBRARY_PATH")
else (APPLE)
  SET(TESTENV_LIBRARY_PATH_NAME "LD_LIBRARY_PATH")
endif (APPLE) 




configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config_tests.h.in 
  ${PROJECT_BINARY_DIR}/include/config_tests.h )

add_library( utils STATIC utils.cpp )


###############################################################################
# SeD c'est bien
###############################################################################
add_executable( SimpleAddSeD SimpleAddSeD.cpp )
target_link_libraries( SimpleAddSeD
  ${DIET_SERVER_LIBRARIES} 
  ${Boost_LIBRARIES} 
  pthread )

