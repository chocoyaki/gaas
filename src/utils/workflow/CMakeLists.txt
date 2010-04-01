IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/src/utils/DAGDA
    ${DIET_SOURCE_DIR}/src/utils/nodes
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/include
    ${DIET_SOURCE_DIR}/src/SeD
    ${DIET_SOURCE_DIR}/src/agent
    ${XERCES_INCLUDE_DIR}
    ${XQILLA_INCLUDE_DIR}
    )
  ADD_LIBRARY ( UtilsWf
    WfNode.cc Dag.cc MetaDag.cc FWorkflow.cc FNode.cc
    WfExpression.cc WfDataWriter.cc
    FActivityNode.cc FIfNode.cc FLoopNode.cc
    WfPort.cc WfPortAdapter.cc
    FNodePort.cc
    DagNode.cc DagNodePort.cc DagNodeLauncher.cc
    DagWfParser.cc
    NodeQueue.cc
    FDataHandle.cc InputIterator.cc
    Thread.cc WfUtils.cc
    )

  TARGET_LINK_LIBRARIES( UtilsWf AgentCommon DIET_Dagda
    ${DIET_DAGDA_LIBRARY}
    ${XERCES_LIBRARY}  ${OMNIORB4_LIBRARIES} ${XQILLA_LIBRARY}
    )

  INSTALL( TARGETS UtilsWf DESTINATION lib )

  INSTALL( FILES FWorkflow.dtd DESTINATION etc )

ENDIF( DIET_BUILD_LIBRARIES )
