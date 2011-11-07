#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( file_transfer_client file_transfer_client.c )
TARGET_LINK_LIBRARIES( file_transfer_client ${DIET_CLIENT_LIBRARIES} )

ADD_EXECUTABLE( file_transfer_server file_transfer_server.c )
TARGET_LINK_LIBRARIES( file_transfer_server ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS file_transfer_client file_transfer_server DESTINATION ${EXAMPLE_INSTALL_DIR}/file_transfer )
