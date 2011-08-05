#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# Generates the CorbaCommon static lib and dietForwarder executable

# Include directories for the library compilation:
#    - OmniORB4 include: try to compile without it! ;)
#    - Binary idl directory: for the generated headers files
#    - Utils dir: for DIET_data_internal.hh
#    - DIET main include dir: for DIET_data.h & DIET_server.h
#    - CORBA dir: DIETLogComponent.hh includes ORBMgr.hh
#    - Juxmem dir: JuxMem is broken. So, don't know why it is necessary.

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl    
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/src/utils/config
)

if (DIET_USE_DAGDA)
  include_directories(${DIET_SOURCE_DIR}/src/utils/DAGDA)
endif (DIET_USE_DAGDA)

if (DIET_USE_JUXMEM)
  include_directories(${JUXMEM_INCLUDE_DIR})
endif (DIET_USE_JUXMEM)

# Compile CorbaCommon static library
add_library(CorbaCommon STATIC
  ORBMgr.cc
  marshalling.cc
)

# CorbaCommon library depends on CORBA files
add_dependencies(CorbaCommon CORBAFiles)

# Dependency when compiling with Batch support
if (DIET_USE_ALT_BATCH)
  include_directories( ${DIET_SOURCE_DIR}/src/utils/batch )
  add_dependencies( CorbaCommon BatchSystem.hh )
endif (DIET_USE_ALT_BATCH)

# Link CorbaCommon to DIET_CORBA, DIET_Utils and OmniORB
target_link_libraries(CorbaCommon
  DIET_CORBA
)

# DTM requires to be linked against DIET_Utils
if (NOT DIET_USE_DAGDA)
  target_link_libraries(CorbaCommon
    DIET_Utils
    )
endif (NOT DIET_USE_DAGDA)


if (DIET_USE_ALT_BATCH)
  target_link_libraries(CorbaCommon UtilsSeDBatch)
endif (DIET_USE_ALT_BATCH)

# Forwarder include dirs:
# Need all the DIET objects headers.
include_directories(
  ${DIET_SOURCE_DIR}/src/agent
  ${DIET_SOURCE_DIR}/src/client
  ${DIET_SOURCE_DIR}/src/utils/nodes
  ${DIET_SOURCE_DIR}/src/client/workflow
  ${DIET_SOURCE_DIR}/src/agent/workflow
  ${DIET_SOURCE_DIR}/src/utils/workflow
  ${DIET_SOURCE_DIR}/src/SeD
)

set(Forwarder_SOURCES
  SSHTunnel.cc
  Options.cc
#  NetConfig.cc
  forwarders/Agent.cc
  forwarders/Callback.cc
  forwarders/MasterAgent.cc
  forwarders/SeD.cc
#  forwarders/LogComponent.cc
)

if (DIET_USE_DAGDA)
  set(Forwarder_SOURCES
    ${Forwarder_SOURCES}
    forwarders/Dagda.cc
  )
else (DIET_USE_DAGDA)
  set(Forwarder_SOURCES
    ${Forwarder_SOURCES}
    forwarders/DataMgr.cc
    forwarders/LocMgr.cc
  )
endif (DIET_USE_DAGDA)
if (DIET_USE_WORKFLOW)
  set(Forwarder_SOURCES
    ${Forwarder_SOURCES}
    forwarders/CltMan.cc
    forwarders/MaDag.cc
    forwarders/WfLogService.cc
  )
  include_directories(
    ${XERCES_INCLUDE_DIR}
    ${XQILLA_INCLUDE_DIR}
  )
endif (DIET_USE_WORKFLOW)

add_library(LibForwarder STATIC
  ${Forwarder_SOURCES}
)
add_executable(dietForwarder
  ORBMgr.cc
  marshalling.cc
  dietFwdr.cc
  DIETForwarder.cc
  ${Forwarder_SOURCES}
)

# dietForwarder & LibForwarder depend on CORBA files.
add_dependencies(dietForwarder CORBAFiles)
add_dependencies(LibForwarder CORBAFiles)

if (DIET_USE_DAGDA)
  set(Forwarder_Libs
    ${Forwarder_Libs}
    ${UUID_LIB}
    DIET_Dagda
  )
endif (DIET_USE_DAGDA)

set(Forwarder_Libs
  DIET_Utils
  ${Forwarder_Libs}
  DIET_CORBA
  CorbaCommon
  UtilsNodes
)

if (DIET_USE_WORKFLOW)
  set(Forwarder_Libs
    ${Forwarder_Libs}
    CltWf
    MaDag
    UtilsWf
    UtilsEvents
    ${XQILLA_LIBRARY}
    ${XERCES_LIBRARY}
  )
endif (DIET_USE_WORKFLOW)
set(Forwarder_Libs
  ${Forwarder_Libs}
  AgentCommon
  DIET_SeD
  DIET_client
  ${OMNIORB4_LIBRARIES}
)

message(STATUS "DIET forwarder libs: ${Forwarder_Libs}")

target_link_libraries(dietForwarder
  ${Forwarder_Libs}
)
if (DIET_USE_USERSCHED)
  target_link_libraries(dietForwarder ${CMAKE_DL_LIBS})
endif (DIET_USE_USERSCHED)

## ------------- New : For scheduler load support. ---------------
# Install the files needed for a scheduler creation.

if (DIET_USE_USERSCHED)
  install( FILES marshalling.hh DESTINATION ${INCLUDE_INSTALL_DIR}/scheduler )
endif (DIET_USE_USERSCHED)
  
install(TARGETS dietForwarder DESTINATION ${BIN_INSTALL_DIR})


if (DIET_USE_LOG)
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${LOG_INCLUDE_DIR}/monitor
    ${LOG_INCLUDE_DIR}/idl
    ${DIET_SOURCE_DIR}/src/utils/log)

  set(Forwarder_Libs
    ${Forwarder_Libs}
    DietLogLibrary)

endif (DIET_USE_LOG)
