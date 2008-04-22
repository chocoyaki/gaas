#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  SET(DAGDA_SOURCES
    DagdaImpl.cc
    DagdaFactory.cc
    CacheAlgorithms.cc
    AdvancedDagdaComponent.cc
  )

  SET(DIET_DAGDA_SOURCES
    DIET_Dagda.cc
  )

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/src/CORBA
    ${DIET_SOURCE_DIR}/include
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/agent
  )
  
  ADD_LIBRARY( Dagda ${DAGDA_SOURCES} )
  ADD_LIBRARY( DIET_Dagda ${DIET_DAGDA_SOURCES} )
  
  TARGET_LINK_LIBRARIES( Dagda
    CorbaCommon
    IDLCommon
    IDLDagda
    UtilsCommon
    ${OMNIORB4_LIBRARIES}
    ${UUIDLIB}
  )

  TARGET_LINK_LIBRARIES( DIET_Dagda
    CorbaCommon
    IDLCommon
    IDLDagda
    AgentCommon
    Dagda
    ${OMNIORB4_LIBRARIES}
  )
  

  INSTALL_TARGETS( /lib/ Dagda DIET_Dagda )

ENDIF( DIET_BUILD_LIBRARIES )
