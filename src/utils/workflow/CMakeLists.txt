IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/include
    ${XERCES_INCLUDE_DIR}
    )
  ADD_LIBRARY ( UtilsWf
    Node.cc Dag.cc
    WfPort.cc WfPortAdapter.cc
    DagNode.cc DagNodePort.cc
    DagWfParser.cc
    NodeQueue.cc
    Thread.cc WfUtils.cc
    )

  TARGET_LINK_LIBRARIES( UtilsWf
    ${DIET_DAGDA_LIBRARY}
    ${XERCES_LIBRARY}  ${OMNIORB4_LIBRARIES}
    )

  IF( APPLE )
    SET_TARGET_PROPERTIES( UtilsWf
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )

  INSTALL_TARGETS( /lib/ UtilsWf )
ENDIF( DIET_BUILD_LIBRARIES )
