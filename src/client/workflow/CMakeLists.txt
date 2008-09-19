#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
    )

  INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )

  ADD_LIBRARY( CltWf
    CltWfMgr.cc )

  TARGET_LINK_LIBRARIES( CltWf
    UtilsWf
    ${XERCES_LIBRARY}  ${OMNIORB4_LIBRARIES}
    )

  IF( APPLE )
    SET_TARGET_PROPERTIES( CltWf
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )

  INSTALL_TARGETS( /lib/ CltWf )

ENDIF( DIET_BUILD_LIBRARIES )