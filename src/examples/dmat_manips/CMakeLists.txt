#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

INCLUDE_DIRECTORIES(
  ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                      # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include
  )

####################################### C written examples
### A single server...
ADD_EXECUTABLE( server server.c )
TARGET_LINK_LIBRARIES( server ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/dmat_manips server )

### ... and many clients
SET( C_CLIENT_EXAMPLES
  client
  clientWithID
  clientJuxMem )

FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/dmat_manips ${loop_var} )
ENDFOREACH( loop_var )

####################################### C++ written examples
SET( CXX_EXAMPLES
  serialClient
  parallelClient
  parallelClient2
  parallelClient3
  parallelClient3
  parallelClient4 )

# FIXME: only used by the parallelClient* examples. Maybe this is a poor
# design of such clients that should not bypass the DIET API ? CheckItOutDude.
INCLUDE_DIRECTORIES( ${OMNIORB4_INCLUDE_DIR} )

FOREACH( loop_var ${CXX_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/dmat_manips ${loop_var} )
ENDFOREACH( loop_var )

