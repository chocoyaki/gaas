#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
  
IF( DIET_BUILD_LIBRARIES )
  # Generate the code out of the IDL source files and build the ad-hoc
  # libraries (idl is part of omniORB which is mandatory).
  
  # Define the flags for the IDL compiler
  SET( DIET_IDL_FLAGS "-bcxx -Wba -Wbtp")

  IF( DIET_USE_BATCH )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_BATCH" )
  ENDIF( DIET_USE_BATCH )

  IF( DIET_USE_ALT_BATCH )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_ALT_BATCH" )
  ENDIF( DIET_USE_ALT_BATCH )

  IF( DIET_USE_JXTA )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_JXTA" )
  ENDIF( DIET_USE_JXTA )
  
  IF( DIET_USE_WORKFLOW )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_WORKFLOW" )
  ENDIF( DIET_USE_WORKFLOW )

  IF( DIET_WITH_MULTI_MA )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_MULTI_MA" )
  ENDIF( DIET_WITH_MULTI_MA )

  ## Dagda compilation
  IF( DIET_USE_DAGDA )
    SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_DAGDA" )
  ENDIF( DIET_USE_DAGDA )

## Install the files needed for scheduler development.
  IF( DIET_USE_USERSCHED )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DUSERSCHED" )
     INSTALL_FILES(
       /include/scheduler
       FILES
         response.hh
	 common_types.hh
	 SeD.hh
     )
  ENDIF( DIET_USE_USERSCHED )
# ------------------------------------------------------------
  
  SEPARATE_ARGUMENTS( DIET_IDL_FLAGS )
  
  SET( IDL_SOURCES
    Agent
    Callback
    common_types
    DataMgr
    LogComponent
    LocalAgent
    LocMgr
    LogTypes
    MasterAgent
    response
    SeD )
  
  IF ( DIET_USE_WORKFLOW )
  SET ( IDL_SOURCES
      MaDag
      CltReoMan
      WfLogService
      ${IDL_SOURCES} )
  ENDIF ( DIET_USE_WORKFLOW )

  ## Use Dagda as data manager.
  IF( DIET_USE_DAGDA )
  SET( IDL_SOURCES
     Dagda
     ${IDL_SOURCES} )
  ENDIF( DIET_USE_DAGDA )

  # Some libraries of the utils directory (e.g. UtilsCommon) need the 
  # idl generated header files. This is a technical variable used to
  # define the target enabling to express this dependency (look below
  # for the DIET_TARGET_IDL_GENERATED_HEADERS definition):
  SET( DIET_TARGET_IDL_GENERATED_HEADER_FILES )
  
  FOREACH( loop_var ${IDL_SOURCES} )
  
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
      COMMAND   ${OMNIORB4_IDL_COMPILER}
      ARGS      ${DIET_IDL_FLAGS}
                ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
      DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
      COMMENT   "idl generated code"
    )
  
    # Dirty trick to avoid executing twice the IDL compiler which produces
    # multiple outputs ${loop_var}SK.cc, ${loop_var}DynSK.cc and
    # ${loop_var}.hh [refer to
    # http://public.kitware.com/pipermail/cmake/2004-August/005392.html ]
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}DynSK.cc
      COMMAND   echo
      ARGS      -n
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
      COMMENT   "idl generated code"
    )
    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}.hh
      COMMAND   echo
      ARGS      -n
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
      COMMENT   "idl generated code"
    )
  
    SET( DIET_TARGET_IDL_GENERATED_HEADER_FILES
      ${DIET_TARGET_IDL_GENERATED_HEADER_FILES}
         ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}.hh )
  ENDFOREACH(loop_var)

  # idl generated header files:
  ADD_CUSTOM_TARGET( DIET_TARGET_IDL_GENERATED_HEADERS
    DEPENDS  ${DIET_TARGET_IDL_GENERATED_HEADER_FILES}
  )
  
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  IF (DIET_USE_WORKFLOW)
    ADD_LIBRARY( IDLCommon
      CallbackDynSK.cc
      CallbackSK.cc
      common_typesSK.cc
      common_typesDynSK.cc
      responseDynSK.cc
      responseSK.cc
      SeDSK.cc
      SeDDynSK.cc
      CltReoManSK.cc
      CltReoManDynSK.cc
    )
  ELSE ( DIET_USE_WORKFLOW )
    ADD_LIBRARY( IDLCommon
      CallbackDynSK.cc
      CallbackSK.cc
      common_typesSK.cc
      common_typesDynSK.cc
      responseDynSK.cc
      responseSK.cc
      SeDSK.cc
      SeDDynSK.cc
    )
  ENDIF (DIET_USE_WORKFLOW)

  TARGET_LINK_LIBRARIES( IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
  INSTALL_TARGETS(/lib/ IDLCommon )
  
  ADD_LIBRARY( IDLAgent
    AgentSK.cc
    AgentDynSK.cc
    DataMgrSK.cc
    DataMgrDynSK.cc
    LogComponentDynSK.cc
    LogComponentSK.cc
    LocMgrSK.cc
    LocMgrDynSK.cc
    LogTypesSK.cc
    LogTypesDynSK.cc 
  )
  TARGET_LINK_LIBRARIES( IDLAgent
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
  INSTALL_TARGETS(/lib/ IDLAgent )
  
  ADD_LIBRARY( IDLLA
    LocalAgentSK.cc
    LocalAgentDynSK.cc
   )
  TARGET_LINK_LIBRARIES( IDLLA
    IDLAgent
    ${OMNIORB4_LIBRARIES}
  )
  INSTALL_TARGETS(/lib/ IDLLA )
  
  ADD_LIBRARY( IDLMA
    MasterAgentSK.cc
    MasterAgentDynSK.cc
  )
  TARGET_LINK_LIBRARIES( IDLMA
    IDLAgent
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
  INSTALL_TARGETS(/lib/ IDLMA )

  ## Use Dagda as data manager.
  IF( DIET_USE_DAGDA )
    ADD_LIBRARY( IDLDagda
      DagdaSK.cc )
    TARGET_LINK_LIBRARIES( IDLDagda
      IDLCommon
      ${OMNIORB4_LIBRARIES} )
    INSTALL_TARGETS(/lib/ IDLDagda )
  ENDIF( DIET_USE_DAGDA )

  ##########################################################################
  IF ( DIET_USE_WORKFLOW ) 
  ADD_LIBRARY ( IDLMaDag
    MaDagSK.cc
    MaDagDynSK.cc )
  
  ADD_LIBRARY ( IDLWfLog
            WfLogServiceSK.cc
            WfLogServiceDynSK.cc )

  TARGET_LINK_LIBRARIES( IDLMaDag
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )

  IF( APPLE )
    SET_TARGET_PROPERTIES( IDLWfLog
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )

  INSTALL_TARGETS(/lib/ IDLMaDag IDLWfLog )
  ENDIF ( DIET_USE_WORKFLOW ) 
  ##########################################################################
ENDIF( DIET_BUILD_LIBRARIES )
