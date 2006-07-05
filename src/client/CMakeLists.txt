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
  
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/client       # local dir .
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${DIET_BINARY_DIR}/include          # CLEANME: must preceed source/include
                                        # to take cmake-generated DIET_config.h
    ${DIET_SOURCE_DIR}/include          # CLEANME: for DIET_data.h 
    ${DIET_BINARY_DIR}/src/CORBA/idl
  )
  
  ADD_LIBRARY( DIET_client ${DIET_client_SOURCES} )
  
  IF (DIET_USE_WORKFLOW) 
    INCLUDE_DIRECTORIES( ${QT_INC} )
  ENDIF ( DIET_USE_WORKFLOW)
  
  TARGET_LINK_LIBRARIES( DIET_client
    CorbaCommon
    IDLAgent
    IDLCommon
    IDLMA
    UtilsCommon
    UtilsVector
    UtilsSeDClt
    ${OMNIORB4_LIBRARIES}
  )
  
  IF( DIET_USE_BATCH )
    TARGET_LINK_LIBRARIES( DIET_client UtilsSeDBatch )
  ENDIF( DIET_USE_BATCH )
  
  IF (DIET_USE_WORKFLOW) 
    TARGET_LINK_LIBRARIES( DIET_client DietWorkflow IDLMaDag ${QTXML_LIBRARY} )
  ENDIF ( DIET_USE_WORKFLOW)
  
  IF (DIET_USE_FD)
    TARGET_LINK_LIBRARIES( DIET_client fd )
  ENDIF (DIET_USE_FD)
  
  INSTALL_TARGETS( /lib/ DIET_client )
  
ENDIF( DIET_BUILD_LIBRARIES )
