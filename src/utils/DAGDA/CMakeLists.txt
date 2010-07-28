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
    DIET_Dagda.cc
  )

#  SET(DIET_DAGDA_SOURCES
#    DIET_Dagda.cc
#  )

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

  IF( DIET_TRANSFER_PROGRESSION )
    SET( DAGDA_SOURCES ${DAGDA_SOURCES} Transfers.cc )
  ENDIF( DIET_TRANSFER_PROGRESSION )

  ADD_LIBRARY( Dagda STATIC ${DAGDA_SOURCES} )
#  ADD_LIBRARY( DIET_Dagda SHARED ${DIET_DAGDA_SOURCES} )
  ADD_LIBRARY( DIET_Dagda SHARED ${DAGDA_SOURCES} )
#  ADD_LIBRARY( DIET_Dagda_static STATIC ${DIET_DAGDA_SOURCES} )
  
  SET(DAGDA_Libs
    CorbaCommon
    IDLCommon
    IDLDagda
    UtilsCommon
    UtilsNodes
    UtilsDagda
#    Dagda
#    IDLForwarder
#    ${OMNIORB4_LIBRARIES}
    ${UUIDLIB}
  )
  IF( CYGWIN )
    TARGET_LINK_LIBRARIES( Dagda
      ${DAGDA_Libs}
      /lib/e2fsprogs/libuuid.a
    )
  ELSE ( CYGWIN )
    TARGET_LINK_LIBRARIES( Dagda
      ${DAGDA_Libs}
#      IDLMA
#      IDLLA
    )
  ENDIF( CYGWIN )  

  TARGET_LINK_LIBRARIES( DIET_Dagda
    ${DAGDA_Libs}
 #   CorbaCommon
 #   IDLCommon
 #   IDLDagda
 #   AgentCommon
 #   IDLForwarder
 #   Dagda
 #   IDLLA
 #   ${OMNIORB4_LIBRARIES}
  )
#  TARGET_LINK_LIBRARIES( DIET_Dagda_static
#    CorbaCommon
#    IDLCommon
#    IDLDagda
#    AgentCommon
#    IDLForwarder
#    Dagda
#    ${OMNIORB4_LIBRARIES}
#  )

#  INSTALL( TARGETS Dagda DIET_Dagda DESTINATION lib )
  INSTALL(TARGETS DIET_Dagda DESTINATION lib)
  INSTALL( FILES DIET_Dagda.h DESTINATION include )

ENDIF( DIET_BUILD_LIBRARIES )
