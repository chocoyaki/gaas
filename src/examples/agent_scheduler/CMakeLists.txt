# Scheduler example
IF( NOT DIET_USE_USERSCHED )
  MESSAGE( STATUS "User scheduler related example is NOT compiled." )
ELSE( NOT DIET_USE_USERSCHED )
  SET(HostnameRR_SOURCES
    HostnameRR.cc)

  LINK_DIRECTORIES( ${DIET_DIR}/lib ${OMNIORB4_DIR}/lib)
  ADD_LIBRARY(HostnameRR MODULE ${HostnameRR_SOURCES})

  INCLUDE_DIRECTORIES( ${DIET_DIR}/include ${DIET_DIR}/include/scheduler
                       ${OMNIORB4_DIR}/include)

  IF ( APPLE )
    TARGET_LINK_LIBRARIES(HostnameRR CorbaCommon omniORB4
                          AgentCommon omnithread )
  ENDIF( APPLE )
  INSTALL( TARGETS HostnameRR DESTINATION bin/examples/agent_scheduler )
ENDIF( NOT DIET_USE_USERSCHED )
