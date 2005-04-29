#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.2  2005/04/29 10:36:38  eboix
#*     idl generated files are now taken into account.  --- Injay 2461
#*
#* Revision 1.1  2005/04/28 13:25:11  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

SET(DIET_client_SOURCES
    CallAsyncMgr.cc
    CallbackImpl.cc
    DIET_client.cc
    DIET_grpc.cc )

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/src/client         # local dir .
  ${DIET_SOURCE_DIR}/src/CORBA          # for e.g. marshalling.hh
  ${DIET_BINARY_DIR}/include            # FIXME: must preceed source/include
                                        # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include            # FIXME: for DIET_data.h 
  ${DIET_BINARY_DIR}/src/CORBA/idl      # FIXME: is this a lib ?
)

ADD_LIBRARY( DIET_client ${DIET_client_SOURCES} )

TARGET_LINK_LIBRARIES( DIET_client
  CorbaCommon
  IDLAgent
  IDLCommon
  IDLMA
  UtilsCommon
  UtilsVector
  UtilsSeDClt
  ${OMNIORB4_LIBRARIES} )

