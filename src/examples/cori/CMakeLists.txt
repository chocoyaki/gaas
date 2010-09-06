#* DIET cmake local file                                                    *#

IF( NOT DIET_USE_CORI )
  MESSAGE( STATUS "Cori related example NOT compiled." )
ELSE( NOT DIET_USE_CORI )
  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### A server...
  ADD_EXECUTABLE( server_cori server.c )
  TARGET_LINK_LIBRARIES( server_cori ${DIET_SERVER_LIBRARIES} )
  
  ### ... a client...
  ADD_EXECUTABLE( client_cori client.c )
  TARGET_LINK_LIBRARIES( client_cori ${DIET_CLIENT_LIBRARIES} )

  ### ... a multithreaded client ... 
  ADD_EXECUTABLE( clientthread_cori client_thread.cc )
  TARGET_LINK_LIBRARIES( clientthread_cori ${DIET_CLIENT_LIBRARIES} )

  ### ... and a test program
  ADD_EXECUTABLE( coriEasyTester coriEasyTester.cc )
  TARGET_LINK_LIBRARIES( coriEasyTester ${DIET_SERVER_LIBRARIES} )

  ### install everything
  INSTALL( TARGETS server_cori client_cori clientthread_cori coriEasyTester DESTINATION bin/examples/cori )
ENDIF( NOT DIET_USE_CORI )
