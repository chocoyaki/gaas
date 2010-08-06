#****************************************************************************#
#* DIET cmake local file                                                    *#
#* Example devoted to the management of scalars (and their encoding length).*#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( scalars_client scalars_client.c )
TARGET_LINK_LIBRARIES( scalars_client ${DIET_CLIENT_LIBRARIES} )

ADD_EXECUTABLE( scalars_server scalars_server.c )
TARGET_LINK_LIBRARIES( scalars_server ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS scalars_client scalars_server DESTINATION bin/examples/scalars )
