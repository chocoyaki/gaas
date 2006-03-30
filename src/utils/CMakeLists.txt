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

ADD_LIBRARY( UtilsSeDClt
  AccessController.cc
  DIET_mutex.cc 
  est_internal.cc
)
TARGET_LINK_LIBRARIES( UtilsSeDClt ${OMNIORB4_LIBRARIES} )
### FIXME: the following is really of poor taster
# * AccessControler.cc makes references to TRACE_LEVEL declared in debug.hh
#   and defined as global in debug.cc
# * The three binaries (associated to each C source) make references to
#   debug_log_mutex declared as omni_mutex in debug.hh and defined in debug.cc
# Hence the following lines:
IF( APPLE )
  SET_TARGET_PROPERTIES( UtilsSeDClt
    PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
ENDIF( APPLE )
INSTALL_TARGETS( /lib/ UtilsSeDClt )

ADD_LIBRARY( UtilsCommon 
  debug.cc
  DIET_data.cc
  KeyString.cc
  ms_function.cc
  Parsers.cc
  statistics.cc
)
# The following dependency expresses that before compiling UtilsCommon one
# first needs to generate the header files [like common_types.h] with idl.
# Track down the definition of the commidity target
# DIET_TARGET_IDL_GENERATED_HEADER_FILES for more details.
ADD_DEPENDENCIES( UtilsCommon ALL DIET_TARGET_IDL_GENERATED_HEADER_FILES )
TARGET_LINK_LIBRARIES( UtilsCommon UtilsSeDClt ${OMNIORB4_LIBRARIES} )
INSTALL_TARGETS( /lib/ UtilsCommon )

ADD_LIBRARY( UtilsVector
  Vector.c
)
INSTALL_TARGETS( /lib/ UtilsVector )

IF( DIET_USE_BATCH )
  INCLUDE_DIRECTORIES( ${APPLESEEDS_INCLUDE_DIR} )
  ADD_LIBRARY( UtilsSeDBatch batch.c )
  TARGET_LINK_LIBRARIES( UtilsSeDBatch ${APPLESEEDS_LIBRARIES} )
  INSTALL_TARGETS( /lib/ UtilsSeDBatch )
ENDIF( DIET_USE_BATCH )

# CORI extensions are only compiled when CORI is required:
IF( DIET_USE_CORI )
  ADD_LIBRARY( UtilsNodes
    Counter.cc
    DietLogComponent.cc
    FASTMgr.cc
    MonitoringThread.cc
    ServiceTable.cc
    Cori_Data_Easy.cc
    Cori_Easy_CPU.cc
    Cori_Easy_Disk.cc
    Cori_Easy_Memory.cc
    Cori_Fast.cc
    Cori_Metric.cc
    CORIMgr.cc
  )
ELSE( DIET_USE_CORI )
  ADD_LIBRARY( UtilsNodes
    Counter.cc
    DietLogComponent.cc
    FASTMgr.cc
    MonitoringThread.cc
    ServiceTable.cc
  )
ENDIF( DIET_USE_CORI )
INSTALL_TARGETS( /lib/ UtilsNodes )

SET_SOURCE_FILES_PROPERTIES( FASTMgr.cc PROPERTIES
    COMPILE_FLAGS "${FAST_INCLUDE_DIR}")
TARGET_LINK_LIBRARIES( UtilsNodes ${FAST_LIBRARIES} )

IF( DIET_USE_CORI )
  # Refer to ConfigureCORI.cmake for definition of CORI_HAVE_* variables:
  IF( CORI_HAVE_PROCCPU )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D CORI_HAVE_PROCCPU" )
  ENDIF( CORI_HAVE_PROCCPU )

  IF( CORI_HAVE_PROCMEM )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_Memory.cc
      PROPERTIES COMPILE_FLAGS "-D CORI_HAVE_PROCMEM" )
  ENDIF( CORI_HAVE_PROCMEM )

  IF( CORI_HAVE_SYS_TYPES )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D CORI_HAVE_SYS_TYPES" )
  ENDIF( CORI_HAVE_SYS_TYPES )

  IF( CORI_HAVE_SYS_SYSCTL )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D CORI_HAVE_SYS_SYSCTL" )
  ENDIF( CORI_HAVE_SYS_SYSCTL )

  IF( CORI_HAVE_SYS_SYSINFO )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc Cori_Easy_Memory.cc
      PROPERTIES COMPILE_FLAGS "-D CORI_HAVE_SYS_SYSINFO" )
  ENDIF( CORI_HAVE_SYS_SYSINFO )
  
  IF( CORI_HAVE_get_avphys_pages )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_Memory.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_GET_AVPHYS_PAGES" )
  ENDIF( CORI_HAVE_get_avphys_pages )

  IF( CORI_HAVE_get_nprocs )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_GET_NPROCS" )
  ENDIF( CORI_HAVE_get_nprocs )

  IF( CORI_HAVE_get_phys_pages )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_Memory.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_GET_PHYS_PAGES" )
  ENDIF( CORI_HAVE_get_phys_pages )

  IF( CORI_HAVE_getloadavg )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_GETLOADAVG" )
  ENDIF( CORI_HAVE_getloadavg )

  IF( CORI_HAVE_getpagesize )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_Memory.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_GETPAGESIZE" )
  ENDIF( CORI_HAVE_getpagesize )

  IF( CORI_HAVE_sysconf )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_SYSCONF" )
  ENDIF( CORI_HAVE_sysconf )

  IF( CORI_HAVE_sysctl )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_SYSCTL" )
  ENDIF( CORI_HAVE_sysctl )

  IF( CORI_HAVE_sysctlbyname )
    SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
      PROPERTIES COMPILE_FLAGS "-D HAVE_SYSCTLBYNAME" )
  ENDIF( CORI_HAVE_sysctlbyname )

ENDIF( DIET_USE_CORI )

