# DIET cmake local file

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
  )
  
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
  ADD_LIBRARY( AgentCommon ${AgentCommon_SOURCES} )
  TARGET_LINK_LIBRARIES( AgentCommon
    UtilsNodes
    UtilsVector
    CorbaCommon
    IDLLA
    IDLMA
    ${OMNIORB4_LIBRARIES}
  )
  INSTALL_TARGETS(/lib/ AgentCommon )
  
  #########################################################################
  ADD_EXECUTABLE( dietAgent dietAgent.cc )
  TARGET_LINK_LIBRARIES( dietAgent AgentCommon)
  
  IF (DIET_USE_WORKFLOW) 
    ADD_LIBRARY( MaDag MaDag_impl.cc
          MaDag_sched.cc RoundRobbin_MaDag_Sched.cc HEFT_MaDag_Sched.cc
          MetaDag.cc 
          AbsWfMetaSched.cc
          WfMetaSched.cc MultiWfFairness_Sched.cc )

    TARGET_LINK_LIBRARIES ( MaDag 
         IDLMaDag
         AgentCommon 
         DietWorkflow 
         DIET_client
         ${XERCES_LIBRARY}
         ${OMNIORB4_LIBRARIES}
    )

    ADD_EXECUTABLE ( maDagAgent maDag.cc )
    TARGET_LINK_LIBRARIES (maDagAgent MaDag)
    INSTALL_TARGETS(/bin/ maDagAgent )
    INSTALL_TARGETS(/lib/ MaDag )
  
  ENDIF ( DIET_USE_WORKFLOW)
  
  INSTALL_TARGETS(/bin/ dietAgent )
  
  #########################################################################
  IF( DIET_USE_JXTA )

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${DIET_BINARY_DIR}/src/agent/LA.class
      COMMAND   ${JAVA_COMPILE}
      ARGS      -d ${DIET_BINARY_DIR}/src/agent
                ${DIET_SOURCE_DIR}/src/agent/LA.java
      DEPENDS   ${DIET_SOURCE_DIR}/src/agent/LA.java
      COMMENT   "java compile local agent"
    )
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${DIET_BINARY_DIR}/src/agent/LA.h
      COMMAND   ${JAVA_HEADER_COMPILE}
      ARGS      -classpath ${DIET_BINARY_DIR}/src/agent
                -d ${DIET_BINARY_DIR}/src/agent
                LA
      DEPENDS   ${DIET_BINARY_DIR}/src/agent/LA.class
      COMMENT   "java header compile local agent"
    )
    SET_SOURCE_FILES_PROPERTIES( 
      ${DIET_SOURCE_DIR}/src/agent/localAgentJNI.cc
      PROPERTIES
      OBJECT_DEPENDS ${DIET_BINARY_DIR}/src/agent/LA.h
    )
    INCLUDE_DIRECTORIES( ${DIET_BINARY_DIR}/src/agent )  # For LA.h ...
    ADD_LIBRARY( dietLocalAgent ${DIET_SOURCE_DIR}/src/agent/localAgentJNI.cc )

    TARGET_LINK_LIBRARIES( dietLocalAgent
	UtilsNodes
	UtilsVector
	CorbaCommon
	IDLLA
	IDLMA
	AgentCommon	
	${OMNIORB4_LIBRARIES}
  )

    INSTALL_TARGETS( /lib/ dietLocalAgent )
  
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${DIET_BINARY_DIR}/src/agent/JXTAMultiMA.class
		${DIET_BINARY_DIR}/src/agent/ReqMgr.class
		${DIET_BINARY_DIR}/src/agent/UuidsMgr.class
      COMMAND   ${JAVA_COMPILE}
      ARGS      -d ${DIET_BINARY_DIR}/src/agent
		-classpath ${DIET_SOURCE_DIR}/src/agent:${DIET_SOURCE_DIR}/src/lib/jxta.jar
                ${DIET_SOURCE_DIR}/src/agent/JXTAMultiMA.java
                ${DIET_SOURCE_DIR}/src/agent/ReqMgr.java
                ${DIET_SOURCE_DIR}/src/agent/UuidsMgr.java
      DEPENDS   ${DIET_SOURCE_DIR}/src/agent/JXTAMultiMA.java
                ${DIET_SOURCE_DIR}/src/agent/ReqMgr.java
                ${DIET_SOURCE_DIR}/src/agent/UuidsMgr.java
      COMMENT   "java compile multi master agent"
    )

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${DIET_BINARY_DIR}/src/agent/JXTAMultiMA.h
      COMMAND   ${JAVA_HEADER_COMPILE}
      ARGS      -classpath ${DIET_BINARY_DIR}/src/agent
                -d ${DIET_BINARY_DIR}/src/agent
                JXTAMultiMA
      DEPENDS   ${DIET_BINARY_DIR}/src/agent/JXTAMultiMA.class
      COMMENT   "java header compile master agent"
    )
    SET_SOURCE_FILES_PROPERTIES( 
      ${DIET_SOURCE_DIR}/src/agent/masterAgentJNI.cc
      PROPERTIES
      OBJECT_DEPENDS ${DIET_BINARY_DIR}/src/agent/JXTAMultiMA.h
    )
  
    INCLUDE_DIRECTORIES( ${DIET_BINARY_DIR}/src/agent )
    ADD_LIBRARY( dietMasterAgent ${DIET_SOURCE_DIR}/src/agent/masterAgentJNI.cc )

    TARGET_LINK_LIBRARIES( dietMasterAgent
	UtilsNodes
	UtilsVector
	CorbaCommon
	IDLLA
	IDLMA
	AgentCommon	
	${OMNIORB4_LIBRARIES}
  )

    INSTALL_TARGETS( /lib/ dietMasterAgent )
  
  ENDIF( DIET_USE_JXTA )
#########################################################################
ENDIF( DIET_BUILD_LIBRARIES )
