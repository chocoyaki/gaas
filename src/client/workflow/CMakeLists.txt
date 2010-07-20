#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/utils/events
    ${DIET_SOURCE_DIR}/src/utils/workflow
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
    ${DIET_SOURCE_DIR}/src/SeD
    )

  INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )

  ADD_LIBRARY( CltWf STATIC
    CltDagNodeLauncher.cc
    CltWfLogCentralDispatcher.cc
    CltWfMgr.cc )

  TARGET_LINK_LIBRARIES( CltWf
    UtilsWf IDLWf DIETCall forwarderIDL
    ${XERCES_LIBRARY}  ${OMNIORB4_LIBRARIES}
    )

#  INSTALL( TARGETS CltWf DESTINATION lib )

ENDIF( DIET_BUILD_LIBRARIES )
