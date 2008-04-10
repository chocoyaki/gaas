# DIET cmake local file

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

  ADD_LIBRARY( MaDag
    MaDag_impl.cc
    WfScheduler.cc RRScheduler.cc HEFTScheduler.cc
    MultiDag.cc 
    MultiWfScheduler.cc
    MultiWfBasicScheduler.cc MultiWfFOFT.cc )
  
  TARGET_LINK_LIBRARIES ( MaDag 
    IDLWf
    AgentCommon 
    UtilsWf 
    DIET_client
    ${XERCES_LIBRARY}
    ${OMNIORB4_LIBRARIES}
    )

  ADD_EXECUTABLE ( maDagAgent maDagAgent.cc )
  TARGET_LINK_LIBRARIES (maDagAgent MaDag)
  INSTALL_TARGETS(/bin/ maDagAgent )
  INSTALL_TARGETS(/lib/ MaDag )
  
ENDIF( DIET_BUILD_LIBRARIES )
