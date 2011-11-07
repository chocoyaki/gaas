#****************************************************************************#
#* DIET cmake local file                                                    *#
#*
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( client client.c )
TARGET_LINK_LIBRARIES( client ${DIET_CLIENT_LIBRARIES} )

ADD_EXECUTABLE( server server.c )
TARGET_LINK_LIBRARIES( server ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS client server DESTINATION ${EXAMPLE_INSTALL_DIR}/multicall )

