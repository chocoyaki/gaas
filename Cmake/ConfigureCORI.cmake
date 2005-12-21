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
ENDIF( CORI_HAVE_PROCCPU )

FIND_FILE( CORI_HAVE_PROCMEM meminfo /proc )
IF( CORI_HAVE_PROCMEM )
  SET( CORI_HAVE_PROCMEM "ON" CACHE BOOL
       "CORI related: does system offers /proc/meminfo feature ?" FORCE )
ENDIF( CORI_HAVE_PROCMEM )

# Redundant with check in DIET_INCLUDES_TO_CHECK:
CHECK_INCLUDE_FILE_CXX( "sys/types.h" DUMMY )
SET( CORI_HAVE_SYS_TYPES ${DUMMY} CACHE BOOL
       "CORI related: is include file sys/types.h available ?" )

CHECK_INCLUDE_FILE_CXX( "sys/sysctl.h" DUMMY )
SET( CORI_HAVE_SYS_SYSCTL ${DUMMY} CACHE BOOL
       "CORI related: is include file sys/sysctl.h available ?" )

CHECK_INCLUDE_FILE_CXX( "sys/sysinfo.h" DUMMY )
SET( CORI_HAVE_SYS_SYSINFO ${DUMMY} CACHE BOOL
       "CORI related: is include file sys/sysinfo.h available ?" )

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

FOREACH( function_to_check ${CORI_FUNCTIONS_TO_CHECK} )
  CHECK_FUNCTION_EXISTS( ${function_to_check} dummyfound${function_to_check} )
  SET( CORI_HAVE_${function_to_check} ${dummyfound${function_to_check}}
       CACHE BOOL
       "CORI related: is ${function_to_check} function available ?" )
  MARK_AS_ADVANCED( CORI_HAVE_${function_to_check} )
ENDFOREACH( function_to_check )
