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
if (NOT DIET_USE_DAGDA)
  set(AgentCommon_SOURCES
      ${AgentCommon_SOURCES}
      LocMgrImpl.cc
  )
endif (NOT DIET_USE_DAGDA)

# To use Dagda instead of DTM.
if (DIET_USE_DAGDA)
  include_directories(${DIET_SOURCE_DIR}/src/utils/DAGDA ${UUID_PATH})
endif (DIET_USE_DAGDA)

if (DIET_USE_USERSCHED)
  set (AgentCommon_SOURCES ${AgentCommon_SOURCES} UserScheduler.cc)
endif (DIET_USE_USERSCHED)

add_library(AgentCommon STATIC ${AgentCommon_SOURCES})
target_link_libraries(AgentCommon
  DIET_Utils
)

if (DIET_USE_CORI)
  set(CORI_LIBRARIES UtilsCori)
endif (DIET_USE_CORI)
  
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

	
if (DIET_USE_DAGDA)
  set(dietAgent_Libs
    ${dietAgent_Libs}
    ${UUID_LIB}
    DIET_Dagda
  )
endif(DIET_USE_DAGDA)
	
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

target_link_libraries(dietAgent ${dietAgent_Libs})

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
  
if (DIET_USE_JXTA)
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/LA.class
    COMMAND   ${JAVA_COMPILE}
    ARGS      -d ${CMAKE_CURRENT_BINARY_DIR}
              ${CMAKE_CURRENT_SOURCE_DIR}/LA.java
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/LA.java
    COMMENT   "java compile local agent"
  )
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/LA.h
    COMMAND   ${JAVA_HEADER_COMPILE}
    ARGS      -classpath ${CMAKE_CURRENT_BINARY_DIR}
              -d ${CMAKE_CURRENT_BINARY_DIR}
              LA
    DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/LA.class
    COMMENT   "java header compile local agent"
  )
  set_source_files_properties( 
    ${CMAKE_CURRENT_SOURCE_DIR}/localAgentJNI.cc
    PROPERTIES
      OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/LA.h
      COMPILE_FLAGS "-I${JAVA_INCLUDE_PATH}"
  )

  include_directories(${CMAKE_CURRENT_BINARY_DIR})  # For LA.h ...
  add_library(dietLocalAgent ${CMAKE_CURRENT_SOURCE_DIR}/localAgentJNI.cc)
  
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.class
              ${CMAKE_CURRENT_BINARY_DIR}/ReqMgr.class
              ${CMAKE_CURRENT_BINARY_DIR}/UuidsMgr.class
    COMMAND   ${JAVA_COMPILE}
    ARGS      -d ${CMAKE_CURRENT_BINARY_DIR}
    -classpath ${CMAKE_CURRENT_SOURCE_DIR}:${DIET_SOURCE_DIR}/src/lib/jxta.jar
              ${CMAKE_CURRENT_SOURCE_DIR}/JXTAMultiMA.java
              ${CMAKE_CURRENT_SOURCE_DIR}/ReqMgr.java
              ${CMAKE_CURRENT_SOURCE_DIR}/UuidsMgr.java
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/JXTAMultiMA.java
              ${CMAKE_CURRENT_SOURCE_DIR}/ReqMgr.java
              ${CMAKE_CURRENT_SOURCE_DIR}/UuidsMgr.java
    COMMENT   "java compile multi master agent"
  )

  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.h
    COMMAND   ${JAVA_HEADER_COMPILE}
    ARGS      -classpath ${CMAKE_CURRENT_BINARY_DIR}
              -d ${CMAKE_CURRENT_BINARY_DIR}
              JXTAMultiMA
    DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.class
    COMMENT   "java header compile master agent"
  )

  set_source_files_properties( 
    ${CMAKE_CURRENT_SOURCE_DIR}/masterAgentJNI.cc
    PROPERTIES
      OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.h
      COMPILE_FLAGS "-I${JAVA_INCLUDE_PATH}"
  )
  
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_library(dietMasterAgent ${CMAKE_CURRENT_SOURCE_DIR}/masterAgentJNI.cc)  
endif (DIET_USE_JXTA)


if (DIET_USE_LOG)
  include_directories(
    ${LOG_SRC_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)
