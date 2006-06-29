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
#* Revision 1.1  2006/06/29 12:23:26  ecaron
#* Add client for GridRPC interoperability checking
#*
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( standard-test standard-test.c )
TARGET_LINK_LIBRARIES( standard-test ${DIET_CLIENT_LIBRARIES} )
INSTALL_TARGETS( /bin/examples/GridRPC standard-test )
