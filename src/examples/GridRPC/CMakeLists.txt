#****************************************************************************#
#* GridRPC interoperability example: CMakeLists file                        *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Eddy CARON (Eddy.Caron@ens-lyon.fr)                                 *#
#*                                                                          *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.2  2006/07/08 23:04:27  aamar
#* Adding the server used for GridRPC test cases.
#*
#* Revision 1.1  2006/06/29 12:23:26  ecaron
#* Add client for GridRPC interoperability checking
#*
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( standard-test standard-test.c )
TARGET_LINK_LIBRARIES( standard-test ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/GridRPC standard-test )

ADD_EXECUTABLE( server server.c )
TARGET_LINK_LIBRARIES( server ${DIET_SERVER_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/GridRPC server )
