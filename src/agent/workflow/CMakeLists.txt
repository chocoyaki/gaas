# DIET cmake local file

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/utils/workflow
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
    )

  INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )

  ADD_LIBRARY( MaDag
    MaDagNodeLauncher.cc
    MaDag_impl.cc
    WfScheduler.cc HEFTScheduler.cc
    MultiWfScheduler.cc
    MultiWfBasicScheduler.cc
    MultiWfFOFT.cc
    MultiWfHEFT.cc
    MultiWfSRPT.cc
    MultiWfFCFS.cc )

  TARGET_LINK_LIBRARIES ( MaDag
    IDLWf
    AgentCommon
    UtilsWf
    CltWf
    DIETCall
    ${XERCES_LIBRARY}
    ${OMNIORB4_LIBRARIES}
    )

  ADD_EXECUTABLE ( maDagAgent maDagAgent.cc )
  TARGET_LINK_LIBRARIES (maDagAgent
      MaDag
      UtilsNodes
      )
  INSTALL( TARGETS maDagAgent DESTINATION bin )
  INSTALL( TARGETS MaDag DESTINATION lib )

ENDIF( DIET_BUILD_LIBRARIES )
