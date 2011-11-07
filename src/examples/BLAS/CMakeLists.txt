#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
# BLAS (Basic Linear Algebric Subroutines) example

IF( NOT BLAS_FOUND )
  MESSAGE( STATUS "BLAS example NOT compiled: BLAS library unavailable." )
ELSE( NOT BLAS_FOUND )
  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### A single server...
  ADD_EXECUTABLE( server server.c )
  TARGET_LINK_LIBRARIES( server ${DIET_SERVER_LIBRARIES} ${BLAS_LIBRARIES} )
  INSTALL( TARGETS server DESTINATION ${EXAMPLE_INSTALL_DIR}/BLAS )
  
  ### ... and many clients
  SET( C_CLIENT_EXAMPLES
    client
    dgemm_client )
  
  FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var}
                           ${DIET_CLIENT_LIBRARIES} ${BLAS_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/BLAS )
  ENDFOREACH( loop_var )

  ADD_EXECUTABLE( clientthread dgemm_client_thread.cc )
  TARGET_LINK_LIBRARIES( clientthread ${DIET_CLIENT_LIBRARIES} )
  INSTALL( TARGETS clientthread DESTINATION ${EXAMPLE_INSTALL_DIR}/BLAS )

ENDIF( NOT BLAS_FOUND )
