#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( NOT DIET_USE_ALT_BATCH )
  MESSAGE( STATUS "Batch Decrypthon related example NOT compiled." )
ELSE( NOT DIET_USE_ALT_BATCH )

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

### One server...
SET( C_SERVER_EXAMPLES
  batch_server )

FOREACH( loop_var ${C_SERVER_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_SERVER_LIBRARIES} )
  SET_TARGET_PROPERTIES( ${loop_var} PROPERTIES LINKER_LANGUAGE CXX )
  INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/Batch/TestAllBatch )
ENDFOREACH( loop_var )
  
### ... and the client
SET( C_CLIENT_EXAMPLES client )
 
FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
  INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/Batch/TestAllBatch )
ENDFOREACH( loop_var )

ENDIF( NOT DIET_USE_ALT_BATCH )
