# DIET cmake local file

include_directories(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/utils/workflow
    ${DIET_SOURCE_DIR}/src/utils/events
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
    ${XERCES_INCLUDE_DIR}
    )

if (DIET_USE_ALT_BATCH) 
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
endif (DIET_USE_ALT_BATCH)  

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
  MultiWfFCFS.cc)

if (DIET_USE_LOG)
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${LOG_INCLUDE_DIR}/monitor
    ${LOG_INCLUDE_DIR}/idl
    ${DIET_SOURCE_DIR}/src/utils/log)

  set(Forwarder_Libs ${Forwarder_Libs} DietLogLibrary)
endif (DIET_USE_LOG)

add_library(MaDag STATIC ${MaDag_SOURCES})

add_executable(maDagAgent maDagAgent.cc ${MaDag_SOURCES} )

target_link_libraries(maDagAgent
  DIET_Dagda
  UtilsWf
  CorbaCommon
  DIET_Utils
  ${XQILLA_LIBRARY}
  ${XERCES_LIBRARY}
  ${OMNIORB4_LIBRARIES})
install(TARGETS maDagAgent DESTINATION ${BIN_INSTALL_DIR})
