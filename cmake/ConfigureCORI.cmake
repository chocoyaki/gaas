### CORI (COllector of Ressource Infomation) system call probing:
# Check for existence of various include files and system calls.
# The following BOOL variables (all prefixed with "CORI_") are set (and
# marked as advanced):
#    CORI_HAVE_PROCCPU
#    CORI_HAVE_PROCMEM
#    CORI_HAVE_SYS_TYPES
#    CORI_HAVE_SYS_SYSCTL
#    CORI_HAVE_SYS_SYSINFO
#    CORI_HAVE_get_avphys_pages
#    CORI_HAVE_get_nprocs
#    CORI_HAVE_get_phys_pages
#    CORI_HAVE_getloadavg
#    CORI_HAVE_getpagesize
#    CORI_HAVE_sysconf
#    CORI_HAVE_sysctl
#    CORI_HAVE_sysctlbyname

FIND_FILE( CORI_HAVE_PROCCPU cpuinfo /proc )
IF( CORI_HAVE_PROCCPU )
  SET( CORI_HAVE_PROCCPU "ON" CACHE BOOL
    "CORI related: does system offers /proc/cpuinfo feature ?" FORCE )
  add_definitions(-DCORI_HAVE_PROCCPU)
ENDIF( CORI_HAVE_PROCCPU )

FIND_FILE( CORI_HAVE_PROCMEM meminfo /proc )
IF( CORI_HAVE_PROCMEM )
  SET( CORI_HAVE_PROCMEM "ON" CACHE BOOL
    "CORI related: does system offers /proc/meminfo feature ?" FORCE )
  add_definitions(-DCORI_HAVE_SYS_TYPES)
ENDIF( CORI_HAVE_PROCMEM )

# FIXME: Redundant with check in DIET_INCLUDES_TO_CHECK:
CHECK_INCLUDE_FILE_CXX( "sys/types.h" DUMMY_TYPES )
SET( CORI_HAVE_SYS_TYPES ${DUMMY_TYPES} CACHE BOOL
       "CORI related: is include file sys/types.h available ?" )
if (CORI_HAVE_SYS_TYPES)
add_definitions (-DCORI_HAVE_SYS_TYPES)
endif (CORI_HAVE_SYS_TYPES)

CHECK_INCLUDE_FILE_CXX( "sys/sysctl.h" DUMMY_SYSCTL )
SET( CORI_HAVE_SYS_SYSCTL ${DUMMY_SYSCTL} CACHE BOOL
       "CORI related: is include file sys/sysctl.h available ?" )
if (CORI_HAVE_SYS_SYSCTL)
  add_definitions(-DCORI_HAVE_SYS_SYSCTL)
endif(CORI_HAVE_SYS_SYSCTL)

CHECK_INCLUDE_FILE_CXX( "sys/sysinfo.h" DUMMY_SYSINFO )
SET( CORI_HAVE_SYS_SYSINFO ${DUMMY_SYSINFO} CACHE BOOL
       "CORI related: is include file sys/sysinfo.h available ?" )

if (CORI_HAVE_SYS_SYSINFO)
  add_definitions (-DCORI_HAVE_SYS_SYSINFO)
endif (CORI_HAVE_SYS_SYSINFO)

MARK_AS_ADVANCED(
  CORI_HAVE_PROCCPU CORI_HAVE_PROCMEM 
  CORI_HAVE_SYS_TYPES CORI_HAVE_SYS_SYSCTL CORI_HAVE_SYS_SYSINFO )

SET( CORI_FUNCTIONS_TO_CHECK
  get_avphys_pages
  get_nprocs
  get_phys_pages
  getloadavg
  getpagesize
  sysconf
  sysctl
  sysctlbyname )

set (toto ${CMAKE_FLAGS})
set (titi ${CMAKE_C_FLAGS})
set (CMAKE_FLAGS "")
set(CMAKE_C_FLAGS "")
FOREACH( function_to_check ${CORI_FUNCTIONS_TO_CHECK} )
  CHECK_FUNCTION_EXISTS( ${function_to_check} dummyfound${function_to_check} )
  SET( CORI_HAVE_${function_to_check} ${dummyfound${function_to_check}}
       CACHE BOOL
       "CORI related: is ${function_to_check} function available ?" )
  MARK_AS_ADVANCED( CORI_HAVE_${function_to_check} )
  if(CORI_HAVE_${function_to_check})
    add_definitions(-DCORI_HAVE_${function_to_check})
  endif(CORI_HAVE_${function_to_check})
ENDFOREACH( function_to_check )
set (CMAKE_FLAGS ${toto})
set (CMAKE_C_FLAGS ${titi})