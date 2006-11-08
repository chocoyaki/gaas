#****************************************************************************#
#* Worfklow examples
#****************************************************************************#


SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_SeD )

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

ADD_EXECUTABLE( succ succ.c )
TARGET_LINK_LIBRARIES( succ ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow succ )

ADD_EXECUTABLE( double double.c )
TARGET_LINK_LIBRARIES( double ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow double )

ADD_EXECUTABLE( sum sum.c )
TARGET_LINK_LIBRARIES( sum ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow sum )

ADD_EXECUTABLE( mul_succ mul_succ.c )
TARGET_LINK_LIBRARIES( mul_succ ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow mul_succ )

ADD_EXECUTABLE( mul_sum mul_sum.c )
TARGET_LINK_LIBRARIES( mul_sum ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow mul_sum )

ADD_EXECUTABLE( square square.c )
TARGET_LINK_LIBRARIES( square ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow square )

ADD_EXECUTABLE( scalar_server scalar_server.c )
TARGET_LINK_LIBRARIES( scalar_server ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow scalar_server )

ADD_EXECUTABLE( strlen strlen.c )
TARGET_LINK_LIBRARIES( strlen ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow strlen )

ADD_EXECUTABLE( int2str int2str.c )
TARGET_LINK_LIBRARIES( int2str ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow int2str )

ADD_EXECUTABLE( file_manip file_manip.c )
TARGET_LINK_LIBRARIES( file_manip ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow file_manip )

ADD_EXECUTABLE( generic_client generic_client.c )
TARGET_LINK_LIBRARIES( generic_client ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow generic_client )

ADD_EXECUTABLE( client_scalar client_scalar.c )
TARGET_LINK_LIBRARIES( client_scalar ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow client_scalar )

ADD_EXECUTABLE( client_string client_string.c )
TARGET_LINK_LIBRARIES( client_string ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow client_string )

ADD_EXECUTABLE( client_file client_file.c )
TARGET_LINK_LIBRARIES( client_file ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow client_file )

ADD_EXECUTABLE( client_matrix client_matrix.c )
TARGET_LINK_LIBRARIES( client_matrix ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow client_matrix )

ADD_SUBDIRECTORY( xml )
