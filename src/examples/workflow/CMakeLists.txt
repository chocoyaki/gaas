#****************************************************************************#
#* Worfklow examples
#****************************************************************************#


INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include
                     ${DIET_SOURCE_DIR}/src/utils/DAGDA )


SET( SEDs scalar_server
          strlen int2str
          file_manip
          string_server )

SET (CLIENTS  generic_client
              client_scalar
              client_string
              client_file
              client_matrix
              client_container
              utils_client )

FOREACH (sed ${SEDs})
  ADD_EXECUTABLE( ${sed} ${sed}.c )
  TARGET_LINK_LIBRARIES( ${sed} ${DIET_SERVER_LIBRARIES} )
  INSTALL( TARGETS ${sed} DESTINATION bin/examples/workflow )
ENDFOREACH (sed)

FOREACH (client ${CLIENTS})
  ADD_EXECUTABLE( ${client} ${client}.c )
  TARGET_LINK_LIBRARIES( ${client} ${DIET_CLIENT_LIBRARIES} )
  INSTALL( TARGETS ${client} DESTINATION bin/examples/workflow )
ENDFOREACH (client)

ADD_SUBDIRECTORY( xml )
