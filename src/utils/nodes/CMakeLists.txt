include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
  ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/utils	
  ${DIET_SOURCE_DIR}/src/SeD		# for SeDImpl.hh in Cori_batch
  ${DIET_SOURCE_DIR}/src/agent	# for ChildID.hh in SeDImpl
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
  ${DIET_SOURCE_DIR}/src/utils/log
  ${DIET_SOURCE_DIR}/src/utils/config
  )

if (DIET_USE_ALT_BATCH) 
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
endif (DIET_USE_ALT_BATCH)  

if (DIET_USE_LOG)
  #  MESSAGE ("${LOG_LIB_DIR}")
  #  MESSAGE ("${LOG_INCLUDE_DIR}")
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${LOG_INCLUDE_DIR}/idl
    ${LOG_INCLUDE_DIR}/monitor
    ${DIET_SOURCE_DIR}/src/utils/log)
endif(DIET_USE_LOG)

########################################################################
### UtilsNodes library: contains CoRI if asked by the installer!
# Note:
#    lib only built, linked with SeD and agent when HAVE_ALT_BATCH not set,
#    or included in DIET_SeD *and* a lib built and linked to the agent

# Build the lib, either for SeD and agent if !HAVE_ALT_BATCH, either
# for the agent if HAVE_ALT_BATCH

set(UtilsNodesSource
  Counter.cc
  #  DietLogComponent.cc
  ServiceTable.cc
  )

if (DIET_USE_LOG)
  set(UtilsNodesSource
    ${UtilsNodesSource}
    MonitoringThread.cc)
endif (DIET_USE_LOG)

add_library(UtilsNodes STATIC ${UtilsNodesSource})


########################################################################
### Cori library:          ***      for the SeD       ***
# Note: lib only built when HAVE_ALT_BATCH not set,
#       or included in DIET_SeD *and* a lib built for the agent

# Still to verify that compilation goes ok after, ie,
# things must be included in the files when compiling DIET_SeD

set(CORI_BATCH_CLEAN_COMPILE_FLAG "")
if (DIET_USE_ALT_BATCH)      # Then extract from the lib SeD related
  set(CORI_BATCH_CLEAN_COMPILE_FLAG ${CORI_BATCH_CLEAN_COMPILE_FLAG}
    "-D CLEAN_CORILIB_FROM_BATCH_STAFF"
    )
endif (DIET_USE_ALT_BATCH)
set_source_files_properties(Cori_Metric.cc
  PROPERTIES COMPILE_FLAGS "${CORI_BATCH_CLEAN_COMPILE_FLAG}"
  )

# Build the lib, either for SeD and agent if !HAVE_ALT_BATCH, either
# for the agent if HAVE_ALT_BATCH

# CORI extensions are only compiled when CORI is required:
set(UtilsCoriSource
  Cori_Data_Easy.cc
  Cori_Easy_CPU.cc
  Cori_Easy_Disk.cc
  Cori_Easy_Memory.cc
  Cori_Metric.cc
  CORIMgr.cc
  ${DIET_IDL_OUTPUT})

add_library(UtilsCori STATIC ${UtilsCoriSource})

########################################################################
### CORI related properties:

# Refer to ConfigureCORI.cmake for definition of CORI_HAVE_* variables:
set(CORI_EASY_CPU_COMPILE_FLAGS  "")
set( CORI_EASY_MEMORY_COMPILE_FLAGS  "")
if (CORI_HAVE_PROCCPU)
  set( CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_PROCCPU"
    )
endif(CORI_HAVE_PROCCPU)

if (CORI_HAVE_PROCMEM)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_PROCMEM"
    )
endif(CORI_HAVE_PROCMEM)

if (CORI_HAVE_SYS_TYPES)
  set( CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_TYPES"
    )
endif (CORI_HAVE_SYS_TYPES)

if (CORI_HAVE_SYS_SYSCTL)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSCTL"
    )
endif (CORI_HAVE_SYS_SYSCTL)

if (CORI_HAVE_SYS_SYSINFO)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSINFO"
    )
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSINFO"
    )
endif (CORI_HAVE_SYS_SYSINFO)

if (CORI_HAVE_get_avphys_pages)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_get_avphys_pages"
    )
endif (CORI_HAVE_get_avphys_pages)

if (CORI_HAVE_get_nprocs)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_get_nprocs"
    )
endif (CORI_HAVE_get_nprocs)

# FIXME: looks like never used !
if (CORI_HAVE_get_phys_pages)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_get_phys_pages"
    )
endif (CORI_HAVE_get_phys_pages)

if (CORI_HAVE_getloadavg)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_getloadavg"
    )
endif (CORI_HAVE_getloadavg)

if (CORI_HAVE_getpagesize)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_getpagesize"
    )
endif (CORI_HAVE_getpagesize)

if (CORI_HAVE_sysconf)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysconf"
    )
endif (CORI_HAVE_sysconf)

if (CORI_HAVE_sysctl)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysctl"
    )
endif (CORI_HAVE_sysctl)

if (CORI_HAVE_sysctlbyname)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysctlbyname"
    )
endif (CORI_HAVE_sysctlbyname)

set_source_files_properties(Cori_Easy_CPU.cc
  PROPERTIES COMPILE_FLAGS "${CORI_EASY_CPU_COMPILE_FLAGS}"
  )
set_source_files_properties( Cori_Easy_Memory.cc
  PROPERTIES COMPILE_FLAGS "${CORI_EASY_MEMORY_COMPILE_FLAGS}"
  )

