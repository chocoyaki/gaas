#****************************************************************************#
#* Worfklow examples
#****************************************************************************#


INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include
                     ${DIET_SOURCE_DIR}/src/utils/DAGDA )


SET( SEDs containers_server )

SET (CLIENTS  containers_client )

FOREACH (sed ${SEDs})
  ADD_EXECUTABLE( ${sed} ${sed}.c )
  TARGET_LINK_LIBRARIES( ${sed} ${DIET_SERVER_LIBRARIES} )
  INSTALL( TARGETS ${sed} DESTINATION ${EXAMPLE_INSTALL_DIR}/containers )
ENDFOREACH (sed)

FOREACH (client ${CLIENTS})
  ADD_EXECUTABLE( ${client} ${client}.c )
  TARGET_LINK_LIBRARIES( ${client} ${DIET_CLIENT_LIBRARIES} )
  INSTALL( TARGETS ${client} DESTINATION ${EXAMPLE_INSTALL_DIR}/containers )
ENDFOREACH (client)

