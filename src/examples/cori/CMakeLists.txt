#* DIET cmake local file                                                    *#

IF( NOT DIET_USE_CORI )
  MESSAGE( STATUS "Cori related example NOT compiled." )
ELSE( NOT DIET_USE_CORI )
  INCLUDE_DIRECTORIES(
    ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                        # to take cmake-generated DIET_config.h
    ${DIET_SOURCE_DIR}/include
  )

  ### A server...
  ADD_EXECUTABLE( server server.c )
  TARGET_LINK_LIBRARIES( server ${DIET_SERVER_LIBRARIES} )
  #SET_TARGET_PROPERTIES( server PROPERTIES LINKER_LANGUAGE CXX )
  INSTALL_TARGETS( /bin/examples/cori server )
  
  ### ... a client...
  ADD_EXECUTABLE( client client.c )
  TARGET_LINK_LIBRARIES( client ${DIET_CLIENT_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/cori client )

  ### ... and a test program
  ADD_EXECUTABLE( coriEasyTester coriEasyTester.cc )
  TARGET_LINK_LIBRARIES( coriEasyTester ${DIET_SERVER_LIBRARIES} )
  #SET_TARGET_PROPERTIES( server PROPERTIES LINKER_LANGUAGE CXX )
  INSTALL_TARGETS( /bin/examples/cori coriEasyTester )
ENDIF( NOT DIET_USE_CORI )
