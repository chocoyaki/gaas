# DIET cmake local file

IF( DIET_BUILD_LIBRARIES )
  IF( HAVE_ALT_BATCH )
    INCLUDE_DIRECTORIES(
      ${OMNIORB4_INCLUDE_DIR}
      ${DIET_SOURCE_DIR}/src/utils
      ${DIET_BINARY_DIR}/src/CORBA/idl
      ${DIET_SOURCE_DIR}/src/CORBA
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${DIET_SOURCE_DIR}/include
    )
  ELSE( HAVE_ALT_BATCH )
    INCLUDE_DIRECTORIES(
      ${OMNIORB4_INCLUDE_DIR}
      ${DIET_SOURCE_DIR}/src/utils
      ${DIET_BINARY_DIR}/src/CORBA/idl
      ${DIET_SOURCE_DIR}/src/CORBA
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${DIET_SOURCE_DIR}/include
      ${DIET_SOURCE_DIR}/src/SeD	# for SeDImpl.hh from Cori_Metric
    )
  ENDIF( HAVE_ALT_BATCH )
  
  #########################################################################
  IF (DIET_USE_WORKFLOW) 
    INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )
  ENDIF ( DIET_USE_WORKFLOW)
  #########################################################################
  ### AgentCommon library:
  SET(AgentCommon_SOURCES
      AgentImpl.cc
      BindService.cc
      ExitClass.cc
      FloodRequest.cc
      FloodRequestsList.cc
      GlobalSchedulers.cc
      LocalAgentImpl.cc
      LocMgrImpl.cc
      MasterAgentImpl.cc
      ReferenceUpdateThread.cc
      Request.cc
      Schedulers.cc
  )
  #########################################################################
  ### New : When the DIET_USE_USERSCHED option is set, add the source code
  ###       for dynamic library load at agent level.
  ###       Install the needed files for user scheduler development.
  IF( DIET_USE_USERSCHED )
    SET( AgentCommon_SOURCES ${AgentCommon_SOURCES} UserScheduler.cc)
    SET(DynamicLoadingLibrary dl)
    INSTALL_FILES(
      /include/scheduler
      FILES
        GlobalSchedulers.hh
	UserScheduler.hh
	Schedulers.hh
    )
  ENDIF( DIET_USE_USERSCHED )
  #########################################################################
  # To use Dagda instead of DTM.
  IF( DIET_USE_DAGDA )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA ${UUID_PATH} )
    SET( DAGDA_LIBRARIES
      Dagda
      IDLDagda
    )
  ENDIF( DIET_USE_DAGDA )

  ADD_LIBRARY( AgentCommon ${AgentCommon_SOURCES} )

  IF( DIET_USE_CORI )
      SET( CORI_LIBRARIES
	   UtilsCori )
  ENDIF( DIET_USE_CORI )

  TARGET_LINK_LIBRARIES( AgentCommon
    UtilsNodes
    UtilsVector
    CorbaCommon
    IDLLA
    IDLMA
    ${OMNIORB4_LIBRARIES}
  ## New : This variable is only set when DIET_USE_USERSCHED is on. Seems
  ##       to be useless on MacOS X...
    ${DynamicLoadingLibrary}
  ## Only set when DIET_USE_DAGDA is on.
    ${DAGDA_LIBRARIES}
    ${CORI_LIBRARIES}                          # defined a little above
  )
  INSTALL_TARGETS(/lib/ AgentCommon )
  
  #########################################################################
  ADD_EXECUTABLE( dietAgent dietAgent.cc )
  TARGET_LINK_LIBRARIES( dietAgent
      AgentCommon
      UtilsNodes
     )

  IF (DIET_USE_WORKFLOW)
      SUBDIRS( workflow )
  ENDIF ( DIET_USE_WORKFLOW)
  
  INSTALL_TARGETS(/bin/ dietAgent )
  
  #########################################################################
  IF( DIET_USE_JXTA )

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/LA.class
      COMMAND   ${JAVA_COMPILE}
      ARGS      -d ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/LA.java
      DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/LA.java
      COMMENT   "java compile local agent"
    )
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/LA.h
      COMMAND   ${JAVA_HEADER_COMPILE}
      ARGS      -classpath ${CMAKE_CURRENT_BINARY_DIR}
                -d ${CMAKE_CURRENT_BINARY_DIR}
                LA
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/LA.class
      COMMENT   "java header compile local agent"
    )
    SET_SOURCE_FILES_PROPERTIES( 
      ${CMAKE_CURRENT_SOURCE_DIR}/localAgentJNI.cc
      PROPERTIES
        OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/LA.h
        COMPILE_FLAGS "-I${JAVA_INCLUDE_PATH}"
    )

    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )  # For LA.h ...
    ADD_LIBRARY( dietLocalAgent ${CMAKE_CURRENT_SOURCE_DIR}/localAgentJNI.cc )

    TARGET_LINK_LIBRARIES( dietLocalAgent
      AgentCommon
      UtilsNodes
      UtilsVector
      CorbaCommon
      IDLLA
      IDLMA
      ${OMNIORB4_LIBRARIES}
    )

    INSTALL_TARGETS( /lib/ dietLocalAgent )
  
    ADD_CUSTOM_COMMAND(
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

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.h
      COMMAND   ${JAVA_HEADER_COMPILE}
      ARGS      -classpath ${CMAKE_CURRENT_BINARY_DIR}
                -d ${CMAKE_CURRENT_BINARY_DIR}
                JXTAMultiMA
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.class
      COMMENT   "java header compile master agent"
    )

    SET_SOURCE_FILES_PROPERTIES( 
      ${CMAKE_CURRENT_SOURCE_DIR}/masterAgentJNI.cc
      PROPERTIES
        OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/JXTAMultiMA.h
        COMPILE_FLAGS "-I${JAVA_INCLUDE_PATH}"
    )
  
    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
    ADD_LIBRARY( dietMasterAgent ${CMAKE_CURRENT_SOURCE_DIR}/masterAgentJNI.cc )

    TARGET_LINK_LIBRARIES( dietMasterAgent
      AgentCommon
      UtilsNodes
      UtilsVector
      CorbaCommon
      IDLLA
      IDLMA
      ${OMNIORB4_LIBRARIES}
    )

    INSTALL_TARGETS( /lib/ dietMasterAgent )
  
  ENDIF( DIET_USE_JXTA )
#########################################################################
ENDIF( DIET_BUILD_LIBRARIES )
