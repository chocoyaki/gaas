#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/agent            # for MasterAgent.hh
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/src/utils/nodes
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
  ${DIET_SOURCE_DIR}/include          # for DIET_data.h
  ${DIET_SOURCE_DIR}/src/agent
  ${DIET_SOURCE_DIR}/src/utils/config
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
)


if (DIET_USE_LOG)
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)

if (DIET_USE_WORKFLOW)
  include_directories( ${XERCES_INCLUDE_DIR} )
endif ( DIET_USE_WORKFLOW)

set(DIET_client_SOURCES
  DIET_client.cc
  DIET_grpc.cc
  DIETCall.cc
  CallAsyncMgr.cc
  CallbackImpl.cc
  ${DIET_IDL_OUTPUT})

if (DIET_USE_MULTICALL)
  set(DIET_client_SOURCES ${DIET_client_SOURCES} MultiCall.cc)
endif (DIET_USE_MULTICALL)

if (DIET_USE_CCS)
  set(DIET_client_SOURCES ${DIET_client_SOURCES} SpecificClientScheduler.cc)
endif (DIET_USE_CCS)

add_library(DIET_client ${DIET_client_SOURCES})

SET_TARGET_PROPERTIES (DIET_client  PROPERTIES VERSION ${DIET_VERSION})

# /!\ DIET_Dagda has to be included first (before any other DIET
# library), otherwise it won't work on APPLE.
set(DIET_client_Libs
  DIET_Dagda
  DIET_Utils
  LibForwarder
  CorbaCommon
  UtilsNodes
  ${OMNIORB4_LIBRARIES})

if (DIET_USE_WORKFLOW)
  add_subdirectory(workflow)
  set(DIET_client_Libs ${DIET_client_Libs}
    CltWf
    MaDag
    UtilsWf
    UtilsEvents
    LibForwarder
    ${XQILLA_LIBRARY}
    ${XERCES_LIBRARY})
endif ( DIET_USE_WORKFLOW )

target_link_libraries(DIET_client ${DIET_client_Libs})

if (DIET_USE_ALT_BATCH)
  target_link_libraries( DIET_client UtilsSeDBatch )
endif (DIET_USE_ALT_BATCH)

if (DIET_USE_WORKFLOW)
  target_link_libraries(DIET_client 
    CltWf
    ${XERCES_LIBRARY}
  )
endif (DIET_USE_WORKFLOW)

# Link with log only if log enabled
if (DIET_USE_LOG)
  Target_link_libraries(DIET_client DietLogLibrary)
endif (DIET_USE_LOG)


install(TARGETS DIET_client DESTINATION ${LIB_INSTALL_DIR})
