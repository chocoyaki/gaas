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

ADD_EXECUTABLE( client1 client1.c )
TARGET_LINK_LIBRARIES( client1 ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/workflow/bin client1 )
