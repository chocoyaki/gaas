#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
  ${DIET_SOURCE_DIR}/src/utils        # for DIET_data_internal.hh
  ${DIET_BINARY_DIR}/include          # FIXME: must preceed source/include
                                      # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include          # FIXME: generate DIET_config.h
                                      #        see HOME/CMakeLists.txt
  )

ADD_LIBRARY( UtilsCommon 
  debug.cc
  DIET_data.cc
  KeyString.cc
  ms_function.cc
  Parsers.cc
  statistics.cc
)
INSTALL_TARGETS( /lib/ UtilsCommon )

ADD_LIBRARY( UtilsNodes
  Counter.cc
  DietLogComponent.cc
  FASTMgr.cc
  MonitoringThread.cc
  ServiceTable.cc
)
INSTALL_TARGETS( /lib/ UtilsNodes )

ADD_LIBRARY( UtilsSeDClt
  AccessController.cc
  DIET_mutex.cc 
  est_internal.cc
)
INSTALL_TARGETS( /lib/ UtilsSeDClt )

ADD_LIBRARY( UtilsVector
  Vector.c
)
INSTALL_TARGETS( /lib/ UtilsVector )
