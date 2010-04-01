#****************************************************************************#
#* DIET cmake local file                                                    *#
#* Example for dynamically adding and removing some services.               *#
#* ONLY WORKS WITH DAGDA                                                    *#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )         # For DIET_client.h
INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA ) # For DIET_Dagda.h

ADD_EXECUTABLE( client_dyn_add_rem client.cc )
TARGET_LINK_LIBRARIES( client_dyn_add_rem ${DIET_CLIENT_LIBRARIES} )

ADD_EXECUTABLE( server_dyn_add_rem server.cc )
TARGET_LINK_LIBRARIES( server_dyn_add_rem ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS client_dyn_add_rem server_dyn_add_rem DESTINATION bin/examples/dyn_add_rem )
INSTALL( FILES README DESTINATION bin/examples/dyn_add_rem )
