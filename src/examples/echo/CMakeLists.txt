#****************************************************************************#
#* DIET cmake local file                                                    *#
#* Example of an Echo service                                               *#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( echo_client echo_client.c )
TARGET_LINK_LIBRARIES( echo_client ${DIET_CLIENT_LIBRARIES} )

ADD_EXECUTABLE( echo_server echo_server.c )
TARGET_LINK_LIBRARIES( echo_server ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS echo_client echo_server DESTINATION ${EXAMPLE_INSTALL_DIR}/echo )
