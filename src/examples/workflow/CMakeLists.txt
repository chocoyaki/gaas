#****************************************************************************#
#* Worfklow examples
#****************************************************************************#


SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_SeD )

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include
                     ${DIET_SOURCE_DIR}/src/utils/DAGDA )


SET( SEDs succ double sum mul_succ mul_sum square scalar_server
          strlen int2str
          file_manip
          string_server )

SET (CLIENTS generic_client client_scalar client_string client_file client_matrix)

FOREACH (sed ${SEDs})
  ADD_EXECUTABLE( ${sed} ${sed}.c )
  TARGET_LINK_LIBRARIES( ${sed} ${DIET_SERVER_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/workflow ${sed} )
ENDFOREACH (sed)

FOREACH (client ${CLIENTS})
  ADD_EXECUTABLE( ${client} ${client}.c )
  TARGET_LINK_LIBRARIES( ${client} ${DIET_CLIENT_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/workflow ${client} )
ENDFOREACH (client)

ADD_SUBDIRECTORY( xml )
