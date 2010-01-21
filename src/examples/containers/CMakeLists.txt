#****************************************************************************#
#* Worfklow examples
#****************************************************************************#


INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include
                     ${DIET_SOURCE_DIR}/src/utils/DAGDA )


SET( SEDs server )

SET (CLIENTS  client )

FOREACH (sed ${SEDs})
  ADD_EXECUTABLE( ${sed} ${sed}.c )
  TARGET_LINK_LIBRARIES( ${sed} ${DIET_SERVER_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/containers ${sed} )
ENDFOREACH (sed)

FOREACH (client ${CLIENTS})
  ADD_EXECUTABLE( ${client} ${client}.c )
  TARGET_LINK_LIBRARIES( ${client} ${DIET_CLIENT_LIBRARIES} )
  INSTALL_TARGETS( /bin/examples/containers ${client} )
ENDFOREACH (client)

