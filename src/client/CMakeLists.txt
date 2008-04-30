#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  SET(DIET_client_SOURCES
    CallAsyncMgr.cc
    CallbackImpl.cc
    DIET_client.cc
    DIET_grpc.cc
  )

  IF (DIET_USE_CCS)
    SET(DIET_client_SOURCES
      SpecificClientScheduler.cc
      ${DIET_client_SOURCES}
      )
  ENDIF (DIET_USE_CCS)

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${DIET_SOURCE_DIR}/include          # for DIET_data.h
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/agent
  )

  ADD_LIBRARY( DIET_client ${DIET_client_SOURCES} )

  IF (DIET_USE_WORKFLOW)
    INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )
  ENDIF ( DIET_USE_WORKFLOW)

  IF( DIET_USE_JUXMEM )
    INCLUDE_DIRECTORIES( ${JUXMEM_INCLUDE_DIR} )
  ENDIF( DIET_USE_JUXMEM )

  IF( DIET_USE_DAGDA )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA ${UUID_PATH} )
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
    UtilsSeDClt
    ${OMNIORB4_LIBRARIES}
    ${DAGDA_LIBRARIES}
  )

  IF (DIET_USE_WORKFLOW)
      SUBDIRS( workflow )
  ENDIF ( DIET_USE_WORKFLOW)

  IF( DIET_USE_BATCH )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_BATCH )
  IF( DIET_USE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_ALT_BATCH )

  IF (DIET_USE_WORKFLOW)
    TARGET_LINK_LIBRARIES( DIET_client
      UtilsWf
      IDLWf
      CltWf
      ${XERCES_LIBRARY}
    )
  ENDIF ( DIET_USE_WORKFLOW)

  IF( DIET_USE_JUXMEM )
    TARGET_LINK_LIBRARIES( DIET_client
      ${JUXMEM_LIBRARIES}
    )
  ENDIF( DIET_USE_JUXMEM )

  IF (DIET_USE_FD)
    TARGET_LINK_LIBRARIES( DIET_client fd )
  ENDIF (DIET_USE_FD)

  IF( NOT BUILD_SHARED_LIBS )
    TARGET_LINK_LIBRARIES( DIET_client stdc++ )
  ENDIF( NOT BUILD_SHARED_LIBS )


  INSTALL_TARGETS( /lib/ DIET_client )

ENDIF( DIET_BUILD_LIBRARIES )
