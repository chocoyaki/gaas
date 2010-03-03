#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )

# INCLUDE HEADERS

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/agent            # for MasterAgent.hh
    ${DIET_SOURCE_DIR}/src/utils
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
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA  ${DIET_SOURCE_DIR}/src/utils/nodes ${UUID_PATH} )
  ENDIF( DIET_USE_DAGDA )

# COMPILE LIBRARY DIET_client

  ADD_LIBRARY( DIET_client
    DIET_client.cc
    DIET_grpc.cc
  )

# COMPILE LIBRARY DIETCall

  SET(DIETCall_SOURCES
    DIETCall.cc
    CallAsyncMgr.cc
    CallbackImpl.cc
  )

  IF (DIET_USE_MULTICALL)
    SET(DIETCall_SOURCES
      MultiCall.cc
      ${DIETCall_SOURCES}
    )
  ENDIF (DIET_USE_MULTICALL)

  IF (DIET_USE_CCS)
    SET(DIETCall_SOURCES
      SpecificClientScheduler.cc
      ${DIETCall_SOURCES}
      )
  ENDIF (DIET_USE_CCS)

  ADD_LIBRARY( DIETCall ${DIETCall_SOURCES} )

# COMPILE LIBRARIES FROM SUBDIRS

  IF (DIET_USE_WORKFLOW)
      SUBDIRS( workflow )
  ENDIF ( DIET_USE_WORKFLOW)

# LINK LIBRARIES

  IF( DIET_USE_DAGDA )
    SET( DAGDA_LIBRARIES
         IDLDagda
	 Dagda
         DIET_Dagda
    )
  ENDIF( DIET_USE_DAGDA )

  TARGET_LINK_LIBRARIES( DIET_client
    CorbaCommon
    IDLAgent
    IDLCommon
    IDLMA
    UtilsCommon
    UtilsVector
    DIETCall
    ${OMNIORB4_LIBRARIES}
    ${DAGDA_LIBRARIES}
  )

  TARGET_LINK_LIBRARIES( DIETCall
    CorbaCommon
    IDLAgent
    IDLCommon
    IDLMA
    UtilsCommon
    UtilsVector
    UtilsNodes
    ${OMNIORB4_LIBRARIES}
    ${DAGDA_LIBRARIES}
  )

  IF( DIET_USE_BATCH )
    TARGET_LINK_LIBRARIES( DIETCall UtilsSeDBatch )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_BATCH )
  IF( DIET_USE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( DIETCall UtilsSeDBatch )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_ALT_BATCH )

  IF (DIET_USE_WORKFLOW)
    TARGET_LINK_LIBRARIES( DIET_client
      IDLWf
      CltWf
      ${XERCES_LIBRARY}
    )
  ENDIF ( DIET_USE_WORKFLOW)

  IF( DIET_USE_JUXMEM )
    TARGET_LINK_LIBRARIES( DIETCall ${JUXMEM_LIBRARIES} )
  ENDIF( DIET_USE_JUXMEM )

  IF (DIET_USE_FD)
    TARGET_LINK_LIBRARIES( DIETCall fd )
  ENDIF (DIET_USE_FD)

  IF( NOT BUILD_SHARED_LIBS )
    TARGET_LINK_LIBRARIES( DIET_client stdc++ )
    TARGET_LINK_LIBRARIES( DIETCall stdc++ )
  ENDIF( NOT BUILD_SHARED_LIBS )

# INSTALL LIBRARIES

  INSTALL_TARGETS( /lib/ DIETCall )
  INSTALL_TARGETS( /lib/ DIET_client )

ENDIF( DIET_BUILD_LIBRARIES )
