#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
# AppleSeeds related examples

IF( NOT DIET_USE_BATCH )
  MESSAGE( STATUS "AppleSeeds related example NOT compiled." )
ELSE( NOT DIET_USE_BATCH )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### Two servers...
  SET( C_SERVER_EXAMPLES
    batch_server
    parallel_server 
    sequential_server )

  FOREACH( loop_var ${C_SERVER_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_SERVER_LIBRARIES} )
    SET_TARGET_PROPERTIES( ${loop_var} PROPERTIES LINKER_LANGUAGE CXX )
    INSTALL_TARGETS( /bin/examples/Batch ${loop_var} )
  ENDFOREACH( loop_var )
  
  ### ... and their respective clients
  SET( C_CLIENT_EXAMPLES client )
  
  FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
    INSTALL_TARGETS( /bin/examples/Batch ${loop_var} )
  ENDFOREACH( loop_var )
ENDIF( NOT DIET_USE_BATCH )
