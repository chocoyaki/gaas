#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  SET(DAGDA_SOURCES
    DagdaImpl.cc
    DagdaFactory.cc
  )

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/src/CORBA
    ${DIET_SOURCE_DIR}/include
    ${DIET_BINARY_DIR}/src/CORBA/idl
  )
  
  ADD_LIBRARY( Dagda ${DAGDA_SOURCES} )
  
  TARGET_LINK_LIBRARIES( Dagda
    CorbaCommon
    IDLCommon
    IDLDagda
    UtilsCommon
    ${OMNIORB4_LIBRARIES}
  )
  
  INSTALL_TARGETS( /lib/ Dagda )

ENDIF( DIET_BUILD_LIBRARIES )
