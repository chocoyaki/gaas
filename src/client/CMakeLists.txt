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
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/agent
)

if (DIET_USE_LOG)
  include_directories(
    ${LOG_SRC_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)

if (DIET_USE_WORKFLOW)
  include_directories( ${XERCES_INCLUDE_DIR} )
endif ( DIET_USE_WORKFLOW)

if (DIET_USE_JUXMEM)
  include_directories(${JUXMEM_INCLUDE_DIR})
endif (DIET_USE_JUXMEM)

if (DIET_USE_DAGDA)
  include_directories(${DIET_SOURCE_DIR}/src/utils/DAGDA  ${UUID_PATH})
endif (DIET_USE_DAGDA)

set(DIET_client_SOURCES
  DIET_client.cc
  DIET_grpc.cc
  DIETCall.cc
  CallAsyncMgr.cc
  CallbackImpl.cc
)
if (DIET_USE_MULTICALL)
  set(DIET_client_SOURCES
    MultiCall.cc
    ${DIET_client_SOURCES}
  )
endif (DIET_USE_MULTICALL)

if (DIET_USE_CCS)
  set(DIET_client_SOURCES
    SpecificClientScheduler.cc
    ${DIET_client_SOURCES}
  )
endif (DIET_USE_CCS)

add_library(DIET_client
  ${DIET_client_SOURCES}
)

SET_TARGET_PROPERTIES (DIET_client  PROPERTIES VERSION ${DIET_VERSION})

# DIET client shared & static libraries depends on CORBA files.
add_dependencies(DIET_client CORBAFiles)


# /!\ DIET_Dagda has to be included first (before any other DIET
# library), otherwise it won't work on APPLE.
if (DIET_USE_DAGDA)
  set(DIET_client_Libs
    ${DIET_client_Libs}
    ${UUID_LIB}
    DIET_Dagda
  )
endif( DIET_USE_DAGDA )

set(DIET_client_Libs
  ${DIET_client_Libs}
  DIET_CORBA
  DIET_Utils
  LibForwarder
  CorbaCommon
  UtilsNodes
  ${OMNIORB4_LIBRARIES}
)

if (DIET_USE_WORKFLOW)
  add_subdirectory(workflow)
  set(DIET_client_Libs
    ${DIET_client_Libs}
    CltWf
    MaDag
    UtilsWf
    UtilsEvents
    ${XQILLA_LIBRARY}
    ${XERCES_LIBRARY}
    pthread
  )
endif ( DIET_USE_WORKFLOW )

if (DIET_USE_DAGDA)
  set(DIET_client_Libs
    ${DIET_client_Libs}
    ${UUID_LIB}
    DIET_Dagda
  )
endif( DIET_USE_DAGDA )


target_link_libraries( DIET_client
  ${DIET_client_Libs}
)

if (DIET_USE_ALT_BATCH)
  target_link_libraries( DIET_client UtilsSeDBatch )
endif (DIET_USE_ALT_BATCH)

if (DIET_USE_WORKFLOW)
  target_link_libraries(DIET_client
    CltWf
    ${XERCES_LIBRARY}
  )
endif (DIET_USE_WORKFLOW)

if (DIET_USE_JUXMEM)
  target_link_libraries( DIET_client ${JUXMEM_LIBRARIES} )
endif (DIET_USE_JUXMEM)

if (DIET_USE_FD)
  target_link_libraries(DIET_client fd)
endif (DIET_USE_FD)

if (NOT BUILD_SHARED_LIBS)
  target_link_libraries(DIET_client stdc++)
endif (NOT BUILD_SHARED_LIBS)


# Link with log only if log enabled
if (DIET_USE_LOG)
  Target_link_libraries(DIET_client
    DietLogLibrary)
endif (DIET_USE_LOG)


install(TARGETS DIET_client DESTINATION ${LIB_INSTALL_DIR})
