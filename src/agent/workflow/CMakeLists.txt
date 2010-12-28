# DIET cmake local file

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/utils/workflow
    ${DIET_SOURCE_DIR}/src/utils/events
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
    )

if (DIET_USE_ALT_BATCH) 
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
endif (DIET_USE_ALT_BATCH)  


  INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )

  set(MaDag_SOURCES
    MaDagNodeLauncher.cc
    MaDag_impl.cc
    MaDagLogCentralDispatcher.cc
    WfScheduler.cc HEFTScheduler.cc
    MultiWfScheduler.cc
    MultiWfBasicScheduler.cc
    MultiWfFOFT.cc
    MultiWfHEFT.cc
    MultiWfSRPT.cc
    MultiWfFCFS.cc 
  )

if (DIET_USE_LOG)
  include_directories(
    ${LOG_SRC_DIR}
    ${LOG_SRC_DIR}/monitor
    ${LOG_SRC_DIR}/idl
    ${DIET_SOURCE_DIR}/src/utils/log)

  set(Forwarder_Libs
    ${Forwarder_Libs}
    DietLogLibrary)

endif (DIET_USE_LOG)

  ADD_LIBRARY( MaDag STATIC
    ${MaDag_SOURCES}
  )

  ADD_EXECUTABLE ( maDagAgent maDagAgent.cc ${MaDag_SOURCES} )

# maDagAgent & MaDag depends on CORBA files.
add_dependencies(maDagAgent CORBAFiles)
add_dependencies(MaDag CORBAFiles)

TARGET_LINK_LIBRARIES (maDagAgent
  DIET_Dagda
  UtilsWf
  CorbaCommon
  DIET_Utils
  ${UUID_LIB}
  ${XQILLA_LIBRARY}
  ${XERCES_LIBRARY}
  ${OMNIORB4_LIBRARIES}
  pthread
  )
INSTALL( TARGETS maDagAgent DESTINATION bin )
