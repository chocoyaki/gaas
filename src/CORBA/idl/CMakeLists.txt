#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  IF( DIET_USE_DAGDA )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/nodes )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA )
  ENDIF( DIET_USE_DAGDA )

  IF( DIET_USE_ALT_BATCH )
    INCLUDE_DIRECTORIES(
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${DIET_BINARY_DIR}/src/CORBA        # for ORBMgr (for DietLogComponent)
      ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
      ${DIET_SOURCE_DIR}/src/agent        # for e.g. ChildID.hh (for SeDImpl)
      ${DIET_SOURCE_DIR}/src/SeD          # for SeDImpl and batch
    )
    IF( DIET_USE_JUXMEM )
      INCLUDE_DIRECTORIES( ${JUXMEM_INCLUDE_DIR} )
    ENDIF( DIET_USE_JUXMEM )
  ENDIF( DIET_USE_ALT_BATCH )


  # Generate the code out of the IDL source files and build the ad-hoc
  # libraries (idl is part of omniORB which is mandatory).

  # Define the flags for the IDL compiler
  SET( DIET_IDL_FLAGS "-bcxx -Wba -Wbtp")

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
     INSTALL( FILES
         ${CMAKE_CURRENT_BINARY_DIR}/response.hh
	 ${CMAKE_CURRENT_BINARY_DIR}/common_types.hh
	 ${CMAKE_CURRENT_BINARY_DIR}/SeD.hh
	 DESTINATION include/scheduler
     )
  ENDIF( DIET_USE_USERSCHED )

  IF( DIET_USE_DYNAMICS )
     SET( DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_DYNAMICS" )
  ENDIF( DIET_USE_DYNAMICS )
    

# ------------------------------------------------------------

  SEPARATE_ARGUMENTS( DIET_IDL_FLAGS )

  SET( IDL_SOURCES
    common_types
    Agent
    Callback
    LogComponent
    LocalAgent
    LogTypes
    MasterAgent
    response
    SeD
    AgentFwdr
    CallbackFwdr
    LocalAgentFwdr
    LogComponentFwdr
    MasterAgentFwdr
    SeDFwdr
    Forwarder
  )

  IF ( DIET_USE_WORKFLOW )
  SET ( IDL_SOURCES
      MaDag
      CltMan
      CltManFwdr
      MaDagFwdr
      WfLogService
      WfLogServiceFwdr
      ${IDL_SOURCES} )
  ENDIF ( DIET_USE_WORKFLOW )

  ## Use Dagda as data manager.
  IF( DIET_USE_DAGDA )
    SET( IDL_SOURCES
       Dagda
       DagdaFwdr
       ${IDL_SOURCES}
    )
    SET( DATAMGR_IDL_BUILD_SRC
       DagdaSK.cc
       DagdaDynSK.cc
       DagdaFwdrSK.cc
       DagdaFwdrDynSK.cc
    )
  ELSE(DIET_USE_DAGDA)
    SET( IDL_SOURCES
        DataMgr
        LocMgr
        DataMgrFwdr
        LocMgrFwdr
       ${IDL_SOURCES}
    )
    SET( DATAMGR_IDL_BUILD_SRC
      DataMgrSK.cc
      DataMgrDynSK.cc
      LocMgrSK.cc
      LocMgrDynSK.cc
      DataMgrFwdrSK.cc
      DataMgrFwdrDynSK.cc
      LocMgrFwdrSK.cc
      LocMgrFwdrDynSK.cc
    )
  ENDIF( DIET_USE_DAGDA )

  # A clean way to call omniidl:
  foreach( loop_var ${IDL_SOURCES} )
    add_custom_command(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}SK.cc
                ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}DynSK.cc
                ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}.hh
      COMMAND   ${OMNIORB4_IDL_COMPILER} ARGS ${DIET_IDL_FLAGS}
                ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
      DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}.idl
      IMPLICIT_DEPENDS CXX ${loop_var}.idl
      COMMENT   "idl generated code: ${loop_var}SK.cc, ${loop_var}DynSK.cc and ${loop_var}.hh"
  ) 
  endforeach(loop_var)

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  IF (DIET_USE_WORKFLOW)
    ADD_LIBRARY( IDLCommon STATIC
      CallbackDynSK.cc
      CallbackSK.cc
      common_typesSK.cc
      common_typesDynSK.cc
      responseDynSK.cc
      responseSK.cc
      SeDSK.cc
      SeDDynSK.cc
      CltManSK.cc
      CltManDynSK.cc
    )
  ELSE ( DIET_USE_WORKFLOW )
    ADD_LIBRARY( IDLCommon STATIC
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
# Forwarder libs
  SET(ForwarderLibs
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )

  TARGET_LINK_LIBRARIES( IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
#  INSTALL( TARGETS IDLCommon DESTINATION lib )

  ADD_LIBRARY( IDLAgent STATIC
    common_typesSK.cc
    common_typesDynSK.cc
    SeDSK.cc
    SeDDynSK.cc
    responseSK.cc
    responseDynSK.cc
    AgentSK.cc
    AgentDynSK.cc
    LocalAgentSK.cc
    LocalAgentDynSK.cc
    MasterAgentSK.cc
    MasterAgentDynSK.cc
    LogComponentDynSK.cc
    LogComponentSK.cc
    LogTypesSK.cc
    LogTypesDynSK.cc
    ${DATAMGR_IDL_BUILD_SRC}
  )
  IF( DIET_USE_DAGDA )
    SET( DagdaLibs IDLDagda )
    SET(ForwarderLibs
      ${ForwarderLibs}
      IDLDagda
    )
  ENDIF( DIET_USE_DAGDA )

  TARGET_LINK_LIBRARIES( IDLAgent
    IDLCommon
    ${OMNIORB4_LIBRARIES}
    ${DagdaLibs}
  )
#  INSTALL( TARGETS IDLAgent DESTINATION lib )

  ADD_LIBRARY( IDLLA STATIC
    LocalAgentSK.cc
    LocalAgentDynSK.cc
   )
  TARGET_LINK_LIBRARIES( IDLLA
    IDLAgent
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
#  INSTALL( TARGETS IDLLA DESTINATION lib )

  ADD_LIBRARY( IDLMA STATIC
    MasterAgentSK.cc
    MasterAgentDynSK.cc
  )
  TARGET_LINK_LIBRARIES( IDLMA
    IDLAgent
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
#  INSTALL( TARGETS IDLMA DESTINATION lib )

  ## Use Dagda as data manager.
  IF( DIET_USE_DAGDA )
    ADD_LIBRARY( IDLDagda STATIC
      DagdaSK.cc )
    TARGET_LINK_LIBRARIES( IDLDagda
      IDLCommon
      ${OMNIORB4_LIBRARIES} )
#    INSTALL( TARGETS IDLDagda DESTINATION lib )
  ENDIF( DIET_USE_DAGDA )

  ##########################################################################
  IF ( DIET_USE_WORKFLOW )
  ADD_LIBRARY ( IDLWf STATIC
    MaDagSK.cc
    MaDagDynSK.cc
    WfLogServiceSK.cc
    WfLogServiceDynSK.cc
    CltManSK.cc
    CltManDynSK.cc
    )


  TARGET_LINK_LIBRARIES( IDLWf
    IDLCommon
    ${OMNIORB4_LIBRARIES}
  )
  SET(ForwarderLibs
      ${ForwarderLibs}
      IDLWf
    )
#  INSTALL( TARGETS IDLWf DESTINATION lib )
  ENDIF ( DIET_USE_WORKFLOW )
  ##########################################################################
  SET(FWDRLIB_SOURCES
    ForwarderSK.cc
    ForwarderDynSK.cc
    LogTypesSK.cc
    LogTypesDynSK.cc
    AgentFwdrSK.cc
    CallbackFwdrSK.cc
    LocalAgentFwdrSK.cc
    LogComponentFwdrSK.cc
    MasterAgentFwdrSK.cc
    SeDFwdrSK.cc
    AgentFwdrDynSK.cc
    CallbackFwdrDynSK.cc
    LocalAgentFwdrDynSK.cc
    LogComponentFwdrDynSK.cc
    MasterAgentFwdrDynSK.cc
    SeDFwdrDynSK.cc
  )
  IF (DIET_USE_DAGDA)
    SET(FWDRLIB_SOURCES
      DagdaFwdrSK.cc
      DagdaFwdrDynSK.cc
      ${FWDRLIB_SOURCES}
    )
  ELSE (DIET_USE_DAGDA)
    SET(FWDRLIB_SOURCES
      DataMgrFwdrSK.cc
      LocMgrFwdrSK.cc
      DataMgrFwdrDynSK.cc
      LocMgrFwdrDynSK.cc
      ${FWDRLIB_SOURCES}
    )
  ENDIF(DIET_USE_DAGDA)

  IF (DIET_USE_WORKFLOW)
    SET(FWDRLIB_SOURCES
      CltManFwdrSK.cc
      MaDagFwdrSK.cc
      WfLogServiceFwdrSK.cc
      CltManFwdrDynSK.cc
      MaDagFwdrDynSK.cc
      WfLogServiceFwdrDynSK.cc
      ${FWDRLIB_SOURCES}
    )
  ENDIF(DIET_USE_WORKFLOW)

  ADD_LIBRARY(IDLForwarder STATIC
    ${FWDRLIB_SOURCES}
  )
  SET(FORWARDER_LIBS
    ${FORWARDER_LIBS}
    ${OMNIORB4_LIBRARIES}
  )
  TARGET_LINK_LIBRARIES(IDLForwarder
    ${ForwarderLibs}
  )

ENDIF( DIET_BUILD_LIBRARIES )
