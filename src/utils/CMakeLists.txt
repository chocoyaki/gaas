# DIET cmake local file

IF( DIET_USE_FD )
  SUBDIRS( fd )
ENDIF (DIET_USE_FD)  

IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
    ${DIET_SOURCE_DIR}/include
  )
  
  ########################################################################
  ### UtilsSeDClt library:
  ADD_LIBRARY( UtilsSeDClt
    AccessController.cc
    DIET_mutex.cc 
    est_internal.cc
  )
  TARGET_LINK_LIBRARIES( UtilsSeDClt ${OMNIORB4_LIBRARIES} )
  ### FIXME: the following is really of poor taste
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
  
  ########################################################################
  ### UtilsCommon library:
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
  
  ########################################################################
  ### UtilsVector library:
  ADD_LIBRARY( UtilsVector
    Vector.c
  )
  INSTALL_TARGETS( /lib/ UtilsVector )
  
  ########################################################################
  ### UtilsSedBatch library:
  IF( DIET_USE_BATCH )
    CONFIGURE_FILE(
      batch_configure.h.in
      ${CMAKE_CURRENT_BINARY_DIR}/batch_configure.h
      IMMEDIATE
    )

    INCLUDE_DIRECTORIES( ${APPLESEEDS_INCLUDE_DIR} 
			 ${CMAKE_CURRENT_BINARY_DIR} )
    ADD_LIBRARY( UtilsSeDBatch batch.c )
    TARGET_LINK_LIBRARIES( UtilsSeDBatch ${APPLESEEDS_LIBRARIES} -lpthread )
    INSTALL_TARGETS( /lib/ UtilsSeDBatch )
  ENDIF( DIET_USE_BATCH )

  IF( DIET_USE_ALT_BATCH )
    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
    ADD_LIBRARY( UtilsSeDBatch 
    BatchCreator.cc
    BatchSystem.cc
    OAR1_6BatchSystem.cc )
    TARGET_LINK_LIBRARIES( UtilsSeDBatch -lpthread )
    INSTALL_TARGETS( /lib/ UtilsSeDBatch )
  ENDIF( DIET_USE_ALT_BATCH )
  
  ########################################################################
  ### UtilsNodes library:
  IF( DIET_USE_CORI )
      IF( DIET_USE_ALT_BATCH )
          # CORI extensions are only compiled when CORI is required:
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
		       Cori_batch.cc
		     )
      ELSE( DIET_USE_ALT_BATCH )
          # CORI extensions are only compiled when CORI is required:
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
      ENDIF( DIET_USE_ALT_BATCH )
  ELSE( DIET_USE_CORI )
    ADD_LIBRARY( UtilsNodes
      Counter.cc
      DietLogComponent.cc
      FASTMgr.cc
      MonitoringThread.cc
      ServiceTable.cc
    )
  ENDIF( DIET_USE_CORI )
  SET_SOURCE_FILES_PROPERTIES( FASTMgr.cc PROPERTIES
      COMPILE_FLAGS "${FAST_INCLUDE_DIR}")
  TARGET_LINK_LIBRARIES( UtilsNodes
    UtilsSeDClt
    UtilsCommon
    ${OMNIORB4_LIBRARIES}
  )
  INSTALL_TARGETS( /lib/ UtilsNodes )
  ### FIXME: the following is kludgy! See note "MacOS X and unresolved symbol"
  # in src/CORBA/CMakeLists.txt
  IF( APPLE )
    SET_TARGET_PROPERTIES( UtilsNodes
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )
  TARGET_LINK_LIBRARIES( UtilsNodes ${FAST_LIBRARIES} )
ENDIF( DIET_BUILD_LIBRARIES )

########################################################################
### CORI related properties:
IF( DIET_USE_CORI )
  # Refer to ConfigureCORI.cmake for definition of CORI_HAVE_* variables:
  SET( CORI_EASY_CPU_COMPILE_FLAGS  "")
  SET( CORI_EASY_MEMORY_COMPILE_FLAGS  "")
  IF( CORI_HAVE_PROCCPU )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_PROCCPU" )
  ENDIF( CORI_HAVE_PROCCPU )

  IF( CORI_HAVE_PROCMEM )
    SET( CORI_EASY_MEMORY_COMPILE_FLAGS
      "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_PROCMEM" )
  ENDIF( CORI_HAVE_PROCMEM )

  IF( CORI_HAVE_SYS_TYPES )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_TYPES" )
  ENDIF( CORI_HAVE_SYS_TYPES )

  IF( CORI_HAVE_SYS_SYSCTL )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSCTL" )
  ENDIF( CORI_HAVE_SYS_SYSCTL )

  IF( CORI_HAVE_SYS_SYSINFO )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSINFO" )
    SET( CORI_EASY_MEMORY_COMPILE_FLAGS
      "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSINFO" )
  ENDIF( CORI_HAVE_SYS_SYSINFO )
  
  IF( CORI_HAVE_get_avphys_pages )
    SET( CORI_EASY_MEMORY_COMPILE_FLAGS
      "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_get_avphys_pages" )
  ENDIF( CORI_HAVE_get_avphys_pages )

  IF( CORI_HAVE_get_nprocs )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_get_nprocs" )
  ENDIF( CORI_HAVE_get_nprocs )

  # FIXME: looks like never used !
  IF( CORI_HAVE_get_phys_pages )
    SET( CORI_EASY_MEMORY_COMPILE_FLAGS
      "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_get_phys_pages" )
  ENDIF( CORI_HAVE_get_phys_pages )

  IF( CORI_HAVE_getloadavg )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_getloadavg" )
  ENDIF( CORI_HAVE_getloadavg )

  IF( CORI_HAVE_getpagesize )
    SET( CORI_EASY_MEMORY_COMPILE_FLAGS
      "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_getpagesize" )
  ENDIF( CORI_HAVE_getpagesize )

  IF( CORI_HAVE_sysconf )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysconf" )
  ENDIF( CORI_HAVE_sysconf )

  IF( CORI_HAVE_sysctl )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysctl" )
  ENDIF( CORI_HAVE_sysctl )

  IF( CORI_HAVE_sysctlbyname )
    SET( CORI_EASY_CPU_COMPILE_FLAGS
      "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysctlbyname" )
  ENDIF( CORI_HAVE_sysctlbyname )

  SET_SOURCE_FILES_PROPERTIES( Cori_Easy_CPU.cc
     PROPERTIES COMPILE_FLAGS "${CORI_EASY_CPU_COMPILE_FLAGS}" )
  SET_SOURCE_FILES_PROPERTIES( Cori_Easy_Memory.cc
     PROPERTIES COMPILE_FLAGS "${CORI_EASY_MEMORY_COMPILE_FLAGS}" )
ENDIF( DIET_USE_CORI )

########################################################################
IF ( DIET_USE_WORKFLOW )
   INCLUDE_DIRECTORIES ( ${XERCES_INCLUDE_DIR} )
   ADD_LIBRARY ( DietWorkflow
               WfPort.cc Node.cc Dag.cc
               WfExtReader.cc 
	       Thread.cc WfUtils.cc WfConfig.cc
               AbstractWfSched.cc SimpleWfSched.cc HEFT_Sched.cc
               CltReoMan_impl.cc
               )
   
   TARGET_LINK_LIBRARIES( DietWorkflow 
      ${XERCES_LIBRARY}  ${OMNIORB4_LIBRARIES} 
   )

  IF( APPLE )
    SET_TARGET_PROPERTIES( DietWorkflow
      PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress" )
  ENDIF( APPLE )

  INSTALL_TARGETS( /lib/ DietWorkflow )
ENDIF (DIET_USE_WORKFLOW )
########################################################################
## New : For scheduler load support.
## Install the files needed for scheduler development.
IF( DIET_USE_USERSCHED )
    INSTALL_FILES(
      /include/scheduler
      FILES
        est_internal.hh
	debug.hh
	LinkedList.hh
	DIET_data_internal.hh
	Vector.h
	Parsers.hh
	ms_function.hh
    )
    INSTALL_FILES(
      /include/scheduler/ts_container FILES ts_container/ts_set.hh )
ENDIF( DIET_USE_USERSCHED )

