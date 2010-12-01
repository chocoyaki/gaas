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
#* Revision 1.6  2010/12/01 06:32:42  bdepardo
#* The server source file is server.c and not gridrpc_server.c
#*
#* Revision 1.5  2010/08/06 14:25:28  glemahec
#* Cmake corrections + uuid lib module + fPIC error control
#*
#* Revision 1.4  2010/04/01 10:03:59  bdepardo
#* Changed	deprecated INSTALL_FILES and INSTALL_TARGETS to	INSTALL.
#*
#* Revision 1.3  2008/04/07 15:33:44  ycaniou
#* This should remove all HAVE_BATCH occurences (still appears in the doc, which
#*   must be updated.. soon :)
#* Add the definition of DIET_BATCH_JOBID wariable in batch scripts
#*
#* Revision 1.2  2006/07/08 23:04:27  aamar
#* Adding the server used for GridRPC test cases.
#*
#* Revision 1.1  2006/06/29 12:23:26  ecaron
#* Add client for GridRPC interoperability checking
#*
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )     # For DIET_client.h

ADD_EXECUTABLE( standard-test standard-test.c )
TARGET_LINK_LIBRARIES( standard-test ${DIET_CLIENT_LIBRARIES} ${DIET_SERVER_LIBRARIES} )

ADD_EXECUTABLE(server server.c )
TARGET_LINK_LIBRARIES( gridrpc_server ${DIET_SERVER_LIBRARIES} )

INSTALL( TARGETS standard-test gridrpc_server DESTINATION bin/examples/GridRPC )
