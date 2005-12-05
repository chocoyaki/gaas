#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
# BLAS (Basic Linear Algebric Subroutines) example
FIND_PACKAGE( BLAS )

INCLUDE_DIRECTORIES(
  ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                      # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include
  )

####################################### C written examples
### A single server...
ADD_EXECUTABLE( server server.c )
TARGET_LINK_LIBRARIES( server ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/BLAS server )

### ... and many clients
SET( C_CLIENT_EXAMPLES
  client
  dgemm_client )

FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
  ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
  TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/dmat_manips ${loop_var} )
ENDFOREACH( loop_var )
