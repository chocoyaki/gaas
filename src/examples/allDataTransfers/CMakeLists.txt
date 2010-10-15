#****************************************************************************#
#* DIET cmake local file                                                    *#
#* Example devoted to the management of scalars (and their encoding length).*#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( transfers_client client.cc )
TARGET_LINK_LIBRARIES( transfers_client ${DIET_CLIENT_LIBRARIES} )

ADD_EXECUTABLE( transfers_server server.cc )
TARGET_LINK_LIBRARIES( transfers_server ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS transfers_client transfers_server DESTINATION bin/examples/allDataTransfers )
