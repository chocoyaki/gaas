#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( NOT DIET_USE_ALT_BATCH )
  MESSAGE( STATUS "Batch related example NOT compiled." )
ELSE( NOT DIET_USE_ALT_BATCH )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### One server...
  SET( C_SERVER_EXAMPLES
       cloud_server 
       )

  FOREACH( loop_var ${C_SERVER_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    ADD_DEFINITIONS( -DHAVE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_SERVER_LIBRARIES} )
    SET_TARGET_PROPERTIES( ${loop_var} PROPERTIES LINKER_LANGUAGE CXX )
    INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/cloud )
  ENDFOREACH( loop_var )
  
  ### ... and its respective client
  SET( C_CLIENT_EXAMPLES cloud_client )
  
  FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    ADD_DEFINITIONS( -DHAVE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/cloud )
  ENDFOREACH( loop_var )

  INSTALL( FILES A.in DESTINATION bin/examples/cloud )
  INSTALL( FILES B.in DESTINATION bin/examples/cloud )
ENDIF( NOT DIET_USE_ALT_BATCH )
