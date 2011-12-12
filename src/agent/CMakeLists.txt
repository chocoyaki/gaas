# DIET cmake local file

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/src/utils/nodes
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/SeD	# for SeDImpl.hh from Cori_Metric
  ${DIET_SOURCE_DIR}/src/utils/config
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
  )

if (DIET_USE_ALT_BATCH)
  include_directories(
      ${DIET_SOURCE_DIR}/src/utils/batch
    )
endif (DIET_USE_ALT_BATCH)
  
if (DIET_USE_WORKFLOW) 
  include_directories(${XERCES_INCLUDE_DIR})
endif (DIET_USE_WORKFLOW)

# AgentCommon library:
set(AgentCommon_SOURCES
    AgentImpl.cc
    BindService.cc
    ExitClass.cc
    FloodRequest.cc
    FloodRequestsList.cc
    GlobalSchedulers.cc
    LocalAgentImpl.cc
    MasterAgentImpl.cc
    ReferenceUpdateThread.cc
    Request.cc
    Schedulers.cc
)

if (DIET_USE_USERSCHED)
  set (AgentCommon_SOURCES ${AgentCommon_SOURCES} UserScheduler.cc)
endif (DIET_USE_USERSCHED)

add_library(AgentCommon STATIC ${AgentCommon_SOURCES})
target_link_libraries(AgentCommon
  DIET_Utils DietConfig
)

set(CORI_LIBRARIES UtilsCori)
  
add_executable(dietAgent ${AgentCommon_SOURCES} dietAgent.cc)

corba_headers(IDL_HDRS
  Agent
  MasterAgent
  LocalAgent
  common_types
  Dagda
)

# AgentCommon library & dietAgent depends on CORBA files.
add_dependencies(dietAgent CORBAFiles)
add_dependencies(AgentCommon CORBAFiles)

set(dietAgent_Libs
  ${dietAgent_Libs}
  DIET_Dagda
  DIET_client
  DIET_SeD
  )
	
set(dietAgent_Libs
  ${dietAgent_Libs}
  DIET_CORBA
  CorbaCommon
  DIET_Utils
  LibForwarder
  ${OMNIORB4_LIBRARIES}
  pthread
  ${CMAKE_DL_LIBS}
)
if (DIET_USE_WORKFLOW)
  set(dietAgent_Libs
    ${dietAgent_Libs}
    CltWf
    MaDag
    ${XQILLA_LIBRARY}
    ${XERCES_LIBRARY}
  )
endif (DIET_USE_WORKFLOW)

if (DIET_USE_LOG)
  set (dietAgent_Libs
    ${dietAgent_Libs}
    DietLogLibrary)
endif (DIET_USE_LOG)

target_link_libraries(dietAgent ${dietAgent_Libs} DietConfig)

if (DIET_USE_WORKFLOW)
  add_subdirectory(workflow)
endif (DIET_USE_WORKFLOW)
  
install(TARGETS dietAgent DESTINATION ${BIN_INSTALL_DIR})

# When the DIET_USE_USERSCHED option is set, add the source code
# for dynamic library load at agent level.
# Install the needed files for user scheduler development.
if (DIET_USE_USERSCHED)
  install(FILES
    GlobalSchedulers.hh
    UserScheduler.hh
    Schedulers.hh
    DESTINATION include/scheduler
  )
  install(TARGETS AgentCommon DESTINATION ${LIB_INSTALL_DIR})
endif (DIET_USE_USERSCHED)

if (DIET_USE_LOG)
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)
