#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.1  2005/04/28 13:25:11  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/CORBA/idl
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
  statistics.cc )

ADD_LIBRARY( UtilsNodes
  Counter.cc
  DietLogComponent.cc
  FASTMgr.cc
  MonitoringThread.cc
  ServiceTable.cc )

ADD_LIBRARY( UtilsSeDClt
  DIET_mutex.cc 
  AccessController.cc )

ADD_LIBRARY( UtilsVector
  estVector.c 
  Vector.c )
