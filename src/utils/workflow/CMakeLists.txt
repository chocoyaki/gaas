IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/include
    ${XERCES_INCLUDE_DIR}
    ${XQILLA_INCLUDE_DIR}
    )
  ADD_LIBRARY ( UtilsWf
    WfNode.cc Dag.cc MetaDag.cc FWorkflow.cc FNode.cc
    WfExpression.cc WfDataWriter.cc
    FActivityNode.cc FIfNode.cc FLoopNode.cc
    WfPort.cc WfPortAdapter.cc
    FNodePort.cc
    DagNode.cc DagNodePort.cc
    DagWfParser.cc
    NodeQueue.cc
    FDataHandle.cc InputIterator.cc
    Thread.cc WfUtils.cc
    )

  TARGET_LINK_LIBRARIES( UtilsWf
    DIETCall
    ${DIET_DAGDA_LIBRARY}
    ${XERCES_LIBRARY}  ${OMNIORB4_LIBRARIES} ${XQILLA_LIBRARY}
    )

  IF( APPLE )
    SET_TARGET_PROPERTIES( UtilsWf
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )

  INSTALL_TARGETS( /lib/ UtilsWf )

  INSTALL_FILES( /etc/ FILES FWorkflow.dtd )

#    ADD_EXECUTABLE( wftest WfUnitTest.cc )
#    TARGET_LINK_LIBRARIES( wftest UtilsWf DIET_client)
#    INSTALL_TARGETS( /bin/ wftest )

ENDIF( DIET_BUILD_LIBRARIES )
