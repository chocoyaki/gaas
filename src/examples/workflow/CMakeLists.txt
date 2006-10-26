#****************************************************************************#
#* Worfklow examples
#****************************************************************************#

SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_SeD )

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

ADD_EXECUTABLE( succ succ.c )
TARGET_LINK_LIBRARIES( succ ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin succ )

ADD_EXECUTABLE( double double.c )
TARGET_LINK_LIBRARIES( double ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin double )

ADD_EXECUTABLE( sum sum.c )
TARGET_LINK_LIBRARIES( sum ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin sum )

ADD_EXECUTABLE( mul_succ mul_succ.c )
TARGET_LINK_LIBRARIES( mul_succ ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin mul_succ )

ADD_EXECUTABLE( mul_sum mul_sum.c )
TARGET_LINK_LIBRARIES( mul_sum ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin mul_sum )

ADD_EXECUTABLE( square square.c )
TARGET_LINK_LIBRARIES( square ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin square )

ADD_EXECUTABLE( scalar_server scalar_server.c )
TARGET_LINK_LIBRARIES( scalar_server ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin scalar_server )

ADD_EXECUTABLE( strlen strlen.c )
TARGET_LINK_LIBRARIES( strlen ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin strlen )

ADD_EXECUTABLE( int2str int2str.c )
TARGET_LINK_LIBRARIES( int2str ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin int2str )

ADD_EXECUTABLE( file_manip file_manip.c )
TARGET_LINK_LIBRARIES( file_manip ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin file_manip )

ADD_EXECUTABLE( generic_client generic_client.c )
TARGET_LINK_LIBRARIES( generic_client ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin generic_client )
