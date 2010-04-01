#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  SET(DAGDA_SOURCES
    DagdaImpl.cc
    DagdaFactory.cc
    DagdaCatalog.cc
    CacheAlgorithms.cc
    NetworkStats.cc
    AdvancedDagdaComponent.cc
    Container.cc
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
    ${DIET_SOURCE_DIR}/src/utils/nodes
  )

  ADD_LIBRARY( Dagda ${DAGDA_SOURCES} )
  ADD_LIBRARY( DIET_Dagda ${DIET_DAGDA_SOURCES} )
  
  IF( CYGWIN )
    TARGET_LINK_LIBRARIES( Dagda
      CorbaCommon
      IDLCommon
      IDLDagda
      UtilsCommon
      UtilsNodes
      UtilsDagda
      ${OMNIORB4_LIBRARIES}
      ${UUIDLIB}
      /lib/e2fsprogs/libuuid.a
    )
  ELSE ( CYGWIN )
    TARGET_LINK_LIBRARIES( Dagda
      CorbaCommon
      IDLCommon
      IDLDagda
      UtilsCommon
      UtilsNodes
      UtilsDagda
      ${OMNIORB4_LIBRARIES}
      ${UUIDLIB}
    )
  ENDIF( CYGWIN )  

  TARGET_LINK_LIBRARIES( DIET_Dagda
    CorbaCommon
    IDLCommon
    IDLDagda
    AgentCommon
    Dagda
    ${OMNIORB4_LIBRARIES}
  )


  INSTALL( TARGETS Dagda DIET_Dagda DESTINATION lib )
  INSTALL( FILES DIET_Dagda.h DESTINATION include )

ENDIF( DIET_BUILD_LIBRARIES )
