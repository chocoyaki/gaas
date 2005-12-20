#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

SET( DIET_SeD_SOURCES
 DIET_server.cc
 DataMgrImpl.cc
 SeDImpl.cc )

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/src/Sed            # local dir .
  ${DIET_SOURCE_DIR}/src/agent          # for e.g. ChildID.hh
  ${DIET_SOURCE_DIR}/src/CORBA          # for e.g. marshalling.hh
  ${DIET_BINARY_DIR}/include            # FIXME: must preceed source/include
                                        # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include            # FIXME: for DIET_data.h 
  ${DIET_BINARY_DIR}/src/CORBA/idl      # FIXME: is this a lib ?
)

ADD_LIBRARY( DIET_SeD ${DIET_SeD_SOURCES} )

TARGET_LINK_LIBRARIES( DIET_SeD
  CorbaCommon
  IDLAgent
  IDLCommon
  UtilsCommon
  UtilsNodes
  UtilsSeDClt
  UtilsVector
  ${OMNIORB4_LIBRARIES} )

### FIXME: missing
###        - the USE_BATCH/ELAGI stuff
###        - all the JXTA_MODE related stuff...
