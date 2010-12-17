#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

####################################### C written examples
### A single server...
ADD_EXECUTABLE( dmat_manips_server dmat_manips_server.c )
TARGET_LINK_LIBRARIES( dmat_manips_server ${DIET_SERVER_LIBRARIES} )
INSTALL( TARGETS dmat_manips_server DESTINATION bin/examples/dmat_manips )

### ... and many clients
SET( C_CLIENT_EXAMPLES
  dmat_manips_client
  clientWithID
  clientJuxMem )

FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
  INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/dmat_manips )
ENDFOREACH( loop_var )


####################################### C++ written examples
SET( CXX_EXAMPLES
  serialClient
  parallelClient
  parallelClient2
  parallelClient3
  parallelClient4 )

# FIXME: only used by the parallelClient* examples. Maybe this is a poor
# design of such clients that should not bypass the DIET API ? CheckItOutDude.
INCLUDE_DIRECTORIES( ${OMNIORB4_INCLUDE_DIR} )

FOREACH( loop_var ${CXX_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
  INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/dmat_manips )
ENDFOREACH( loop_var )

