#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )

# INCLUDE HEADERS

  INCLUDE_DIRECTORIES(
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

  IF (DIET_USE_WORKFLOW)
    INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )
  ENDIF ( DIET_USE_WORKFLOW)

  IF( DIET_USE_JUXMEM )
    INCLUDE_DIRECTORIES( ${JUXMEM_INCLUDE_DIR} )
  ENDIF( DIET_USE_JUXMEM )

  IF( DIET_USE_DAGDA )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA  ${UUID_PATH} )
  ENDIF( DIET_USE_DAGDA )

# COMPILE LIBRARY DIET_client

  SET(DIET_client_SOURCES
    DIET_client.cc
    DIET_grpc.cc
    DIETCall.cc
    CallAsyncMgr.cc
    CallbackImpl.cc
  )
  IF (DIET_USE_MULTICALL)
    SET(DIET_client_SOURCES
      MultiCall.cc
      ${DIET_client_SOURCES}
    )
  ENDIF (DIET_USE_MULTICALL)

  IF (DIET_USE_CCS)
    SET(DIET_client_SOURCES
      SpecificClientScheduler.cc
      ${DIET_client_SOURCES}
      )
  ENDIF (DIET_USE_CCS)

  ADD_LIBRARY( DIET_client SHARED
    ${DIET_client_SOURCES}
  )
  ADD_LIBRARY( DIET_client_static STATIC
    ${DIET_client_SOURCES}
  )
# COMPILE LIBRARY DIETCall

#  SET(DIETCall_SOURCES
#    DIETCall.cc
#    CallAsyncMgr.cc
#    CallbackImpl.cc
#  )

#  IF (DIET_USE_MULTICALL)
#    SET(DIETCall_SOURCES
#      MultiCall.cc
#      ${DIETCall_SOURCES}
#    )
#  ENDIF (DIET_USE_MULTICALL)

#  IF (DIET_USE_CCS)
#    SET(DIETCall_SOURCES
#      SpecificClientScheduler.cc
#      ${DIETCall_SOURCES}
#      )
#  ENDIF (DIET_USE_CCS)

#  ADD_LIBRARY( DIETCall STATIC ${DIETCall_SOURCES} )

# COMPILE LIBRARIES FROM SUBDIRS

  set(DIET_client_Libs
      DIET_CORBA
      DIET_Utils
      LibForwarder
      CorbaCommon
      UtilsNodes
      ${OMNIORB4_LIBRARIES}
      pthread
  )

  IF ( DIET_USE_WORKFLOW )
    ADD_SUBDIRECTORY( workflow )
    set(DIET_client_Libs
      ${DIET_client_Libs}
      CltWf
      MaDag
      UtilsWf
      UtilsEvents
      ${XERCES_LIBRARY}
      ${XQILLA_LIBRARY}
    )
  ENDIF ( DIET_USE_WORKFLOW )

  IF( DIET_USE_DAGDA )
    set(DIET_client_Libs
      ${UUIDLIB}
      Dagda
      ${DIET_client_Libs}
    )
  ENDIF( DIET_USE_DAGDA )

  TARGET_LINK_LIBRARIES( DIET_client
    ${DIET_client_Libs}
  )

  IF( DIET_USE_BATCH )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_BATCH )
  IF( DIET_USE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_ALT_BATCH )

  IF (DIET_USE_WORKFLOW)
    TARGET_LINK_LIBRARIES( DIET_client
      CltWf
      ${XERCES_LIBRARY}
    )
  ENDIF ( DIET_USE_WORKFLOW)

  IF( DIET_USE_JUXMEM )
    TARGET_LINK_LIBRARIES( DIET_client ${JUXMEM_LIBRARIES} )
  ENDIF( DIET_USE_JUXMEM )

  IF (DIET_USE_FD)
    TARGET_LINK_LIBRARIES( DIET_client fd )
  ENDIF (DIET_USE_FD)

  IF( NOT BUILD_SHARED_LIBS )
    TARGET_LINK_LIBRARIES( DIET_client stdc++ )
  ENDIF( NOT BUILD_SHARED_LIBS )

  INSTALL(TARGETS DIET_client DESTINATION lib)
ENDIF( DIET_BUILD_LIBRARIES )
