include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
  ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
  ${DIET_SOURCE_DIR}/src/utils/nodes
  ${DIET_SOURCE_DIR}/src/utils/events
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/SeD
  ${DIET_SOURCE_DIR}/src/agent
  ${XQILLA_INCLUDE_DIR}
  ${XERCES_INCLUDE_DIR}
)


if (DIET_USE_LOG)
  include_directories(
    ${LOG_SRC_DIR}
    ${LOG_SRC_DIR}/monitor
    ${LOG_SRC_DIR}/idl
    ${DIET_SOURCE_DIR}/src/utils/log)

  set(Forwarder_Libs
    ${Forwarder_Libs}
    DietLogLibrary)

endif (DIET_USE_LOG)


add_library(UtilsWf STATIC
  WfLogDispatcher.cc
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
  WfLogServiceImpl.cc
)

target_link_libraries(UtilsWf
  DIET_Dagda
)
install(FILES FWorkflow.dtd DESTINATION etc)
