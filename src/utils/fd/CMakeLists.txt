#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/src/utils/nodes        # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/include
    )
  
  ADD_LIBRARY( fd
    application_watcher.cc
    fdc.cc
    fdd.cc 
    formulae.cc
    heartbeat_sender.cc
    tcp_server.cc
    udp_server.cc
    hashtable.cc
    heap.cc
    list.cc
  )
  TARGET_LINK_LIBRARIES( fd m UtilsNodes)
  INSTALL_TARGETS( /lib/ fd )
ENDIF( DIET_BUILD_LIBRARIES )
