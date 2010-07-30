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

  ADD_LIBRARY( CorbaCommon STATIC
    ORBMgr.cc
    marshalling.cc
  )

IF( DIET_USE_ALT_BATCH )
  ADD_DEPENDENCIES( CorbaCommon
		    DIET_TARGET_IDL_GENERATED_HEADERS
		    BatchSystem.hh )
#  TARGET_LINK_LIBRARIES( CorbaCommon
#    UtilsSeDBatch
#  )
ELSE( DIET_USE_ALT_BATCH )
  ADD_DEPENDENCIES( CorbaCommon 
		    DIET_TARGET_IDL_GENERATED_HEADERS )
ENDIF( DIET_USE_ALT_BATCH )

  TARGET_LINK_LIBRARIES( CorbaCommon
    DIET_CORBA
  )

# Forwarder
  INCLUDE_DIRECTORIES(
    ${DIET_SOURCE_DIR}/src/agent
    ${DIET_SOURCE_DIR}/src/client
    ${DIET_SOURCE_DIR}/src/utils/nodes
    ${DIET_SOURCE_DIR}/src/client/workflow
    ${DIET_SOURCE_DIR}/src/agent/workflow
    ${DIET_SOURCE_DIR}/src/utils/workflow
    ${DIET_SOURCE_DIR}/src/SeD
  )

  SET(Forwarder_SOURCES
    SSHTunnel.cc
    Options.cc
    NetConfig.cc
    forwarders/Agent.cc
    forwarders/Callback.cc
    forwarders/MasterAgent.cc
    forwarders/SeD.cc
    forwarders/LogComponent.cc
  )

  IF (DIET_USE_DAGDA)
    SET(Forwarder_SOURCES
      ${Forwarder_SOURCES}
      forwarders/Dagda.cc
    )
  ELSE (DIET_USE_DAGDA)
    SET(Forwarder_SOURCES
	${Forwarder_SOURCES}
	forwarders/DataMgr.cc
	forwarders/LocMgr.cc
    )
  ENDIF (DIET_USE_DAGDA)
  IF (DIET_USE_WORKFLOW)
    SET(Forwarder_SOURCES
      ${Forwarder_SOURCES}
      forwarders/CltMan.cc
      forwarders/MaDag.cc
      forwarders/WfLogService.cc
    )
    INCLUDE_DIRECTORIES(
      ${XERCES_INCLUDE_DIR}
      ${XQILLA_INCLUDE_DIR}
    )
  ENDIF (DIET_USE_WORKFLOW)

  ADD_LIBRARY(LibForwarder STATIC
    ${Forwarder_SOURCES}
  )
  ADD_EXECUTABLE(dietForwarder
    ORBMgr.cc
    marshalling.cc
    dietFwdr.cc
    DIETForwarder.cc
    ${Forwarder_SOURCES}
  )

  IF (DIET_USE_DAGDA)
    SET(Forwarder_Libs
      ${Forwarder_Libs}
      ${UUIDLIB}
      Dagda
    )
  ENDIF (DIET_USE_DAGDA)

#  IF (DIET_USE_ALT_BATCH)
#    SET(Forwarder_Libs
#      ${Forwarder_Libs}
#      UtilsSeDBatch
#    )
#  ENDIF (DIET_USE_ALT_BATCH)

  SET(Forwarder_Libs
#    UtilsNodes
    DIET_Utils
#    ${Forwarder_Libs}
#    DIET_CORBA
#    DIET_SeD_static
#    DIET_client_static
    ${Forwarder_Libs}
    DIET_CORBA
    CorbaCommon
    UtilsNodes
    DIET_Utils
  )

IF (DIET_USE_WORKFLOW)
  SET(Forwarder_Libs
    ${Forwarder_Libs}
      CltWf
      MaDag
      UtilsWf
      UtilsEvents
      ${XERCES_LIBRARY}
      ${XQILLA_LIBRARY}
  )
ENDIF(DIET_USE_WORKFLOW)
  SET(Forwarder_Libs
    ${Forwarder_Libs}

    AgentCommon
    DIET_SeD_static
    DIET_client_static

    ${OMNIORB4_LIBRARIES}
    pthread
  )

  TARGET_LINK_LIBRARIES( dietForwarder
    ${Forwarder_Libs}
  )
  if (DIET_USE_USERSCHED)
    target_link_libraries(dietForwarder ${CMAKE_DL_LIBS})
  endif (DIET_USE_USERSCHED)

  ## ------------- New : For scheduler load support. ---------------
  # Install the files needed for a scheduler creation.

  IF( DIET_USE_USERSCHED )
    INSTALL( FILES marshalling.hh DESTINATION include/scheduler )
  ENDIF( DIET_USE_USERSCHED )
# ------------------------------------------------------------------
  
#  INSTALL( TARGETS CorbaCommon DESTINATION lib )
INSTALL( TARGETS dietForwarder DESTINATION bin )
ENDIF( DIET_BUILD_LIBRARIES )
