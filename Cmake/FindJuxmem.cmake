#
# Find the JUXtaposed MEMemory (JuxMem) libraries and include dir
#
# For JuxMem reference: 
#     http://juxmem.gforge.inria.fr/
#
 
# JUXMEM_INCLUDE_DIR - Directories to include to use JuxMem
# JUXMEM_LIBRARIES   - Files to link against to use JuxMem
# JUXMEM_FOUND       - When false, don't try to use JuxMem
# JUXMEM_DIR         - (optional) Suggested installation directory to search
#
# JUXMEM_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when JuxMem was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set JUXMEM_DIR it to your specific installation directory
#
# Note and warning:
#       it looks like JuxMem is doing a poor job at hidding it's internal
#       choices. In particular it forces it's users to know and to set the path
#       to the underlying includes (e.g. apr.h). And at link time, the user
#       is again forced to link against, libjpr, lipapr, libjxta...not to
#       mention libxml2 !
#       This apparent lack of taste from JuxMem's developpers complicates
#       the present cmake configuation file. :-( In the sake of simplification
#       we make the strong assumption that the installation of those JuxMem
#       sub-dependencies was nested within JuxMem's installation directory
#       (except for libxml2 which is expected in the canonical library place).

SET( JUXMEM_FAILED FALSE )

######## We need to link against xml2:
FIND_PROGRAM( JUXMEM_XML2_CONFIG_COMMAND
  NAMES xml2-config
  PATHS
  ${CYGWIN_INSTALL_PATH}/bin /bin /usr/bin /usr/local/bin /sbin
)
  
IF( NOT JUXMEM_XML2_CONFIG_COMMAND )
  MESSAGE( "JuxMem: xml2-config command not found (is libxml2 installed?)." )
  SET( JUXMEM_FAILED TRUE )
ELSE( NOT JUXMEM_XML2_CONFIG_COMMAND )
  EXECUTE_PROCESS(COMMAND ${JUXMEM_XML2_CONFIG_COMMAND}
    "--libs"
    OUTPUT_VARIABLE XML2_LDFLAGS
  )
  SEPARATE_ARGUMENTS( XML2_LDFLAGS )
  MARK_AS_ADVANCED( JUXMEM_XML2_CONFIG_COMMAND )
ENDIF( NOT JUXMEM_XML2_CONFIG_COMMAND )
  
######## We need to link against ssl:
FIND_PROGRAM( JUXMEM_PKG-CONFIG_COMMAND
  NAMES pkg-config
  PATHS
  ${CYGWIN_INSTALL_PATH}/bin /bin /usr/bin /usr/local/bin /sbin
)
  
IF( NOT JUXMEM_PKG-CONFIG_COMMAND )
  MESSAGE( "JuxMem: pkg-config (for ssl) command not found." )
  SET( JUXMEM_FAILED TRUE )
ELSE( NOT JUXMEM_PKG-CONFIG_COMMAND )
  EXECUTE_PROCESS(COMMAND ${JUXMEM_PKG-CONFIG_COMMAND}
    "openssl --libs"
    OUTPUT_VARIABLE SSL_LDFLAGS
  )
  SEPARATE_ARGUMENTS( SSL_LDFLAGS )
  MARK_AS_ADVANCED( JUXMEM_PKG-CONFIG_COMMAND )
ENDIF( NOT JUXMEM_PKG-CONFIG_COMMAND )

######## We need to link against apr:
FIND_PROGRAM( JUXMEM_APR-CONFIG_COMMAND
  NAMES apr-1-config
  PATHS
  ${JUXMEM_DIR}/bin /bin /usr/bin /usr/local/bin /sbin
)
  
IF( NOT JUXMEM_APR-CONFIG_COMMAND )
  MESSAGE( "JuxMem: apr-1-config command not found." )
  SET( JUXMEM_FAILED TRUE )
ELSE( NOT JUXMEM_APR-CONFIG_COMMAND )
  EXECUTE_PROCESS(COMMAND ${JUXMEM_APR-CONFIG_COMMAND}
    "--libs"
    OUTPUT_VARIABLE APR_LDFLAGS
  )
  SEPARATE_ARGUMENTS( APR_LDFLAGS )
  MARK_AS_ADVANCED( JUXMEM_APR-CONFIG_COMMAND )
ENDIF( NOT JUXMEM_APR-CONFIG_COMMAND )

######## We need to link against some other apu thingy:
FIND_PROGRAM( JUXMEM_APU-CONFIG_COMMAND
  NAMES apu-1-config
  PATHS
  ${JUXMEM_DIR}/bin /bin /usr/bin /usr/local/bin /sbin
)
  
IF( NOT JUXMEM_APU-CONFIG_COMMAND )
  MESSAGE( "JuxMem: apu-1-config command not found." )
  SET( JUXMEM_FAILED TRUE )
ELSE( NOT JUXMEM_APU-CONFIG_COMMAND )
  EXECUTE_PROCESS(COMMAND ${JUXMEM_APU-CONFIG_COMMAND}
    "--libs"
    OUTPUT_VARIABLE APU_LDFLAGS
  )
  SEPARATE_ARGUMENTS( APU_LDFLAGS )
  MARK_AS_ADVANCED( JUXMEM_APU-CONFIG_COMMAND )
ENDIF( NOT JUXMEM_APU-CONFIG_COMMAND )

####### Summarise all the libraries dependencies we need to provide:
IF( NOT JUXMEM_FAILED )
  SET( SUB_LIBRARIES ${XML2_LDFLAGS} ${SSL_LDFLAGS} )
  SEPARATE_ARGUMENTS( SUB_LIBRARIES )
  SET( JUXMEM_SUB_LIBRARIES
    ${XML2_LDFLAGS}
    ${SSL_LDFLAGS}
    ${APR_LDFLAGS}
    ${APU_LDFLAGS}
    CACHE STRING "Juxmem libraries dependencies." 
  )
  MARK_AS_ADVANCED( JUXMEM_SUB_LIBRARIES )
ENDIF( NOT JUXMEM_FAILED )
  
FIND_PATH( JUXMEM_INCLUDE_DIR juxmem.h
  PATHS
  ${JUXMEM_DIR}/include
  /usr/include
  /usr/local/include
)

FIND_PATH( JUXMEM_APR_INCLUDE_DIR apr.h
  PATHS
  ${JUXMEM_DIR}/include/apr-1
  /usr/include
  /usr/local/include
)

# FIXME: really force JuxMem folks to be cleaner !
FIND_LIBRARY( JUXMEM_XML2_LIBRARY xml2
  PATHS
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY( JUXMEM_LIBRARY juxmem
  PATHS
  ${JUXMEM_DIR}/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY( JUXMEM_JXTA_LIBRARY jxta
  PATHS
  ${JUXMEM_DIR}/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY( JUXMEM_APR_LIBRARY apr-1
  PATHS
  ${JUXMEM_DIR}/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY( JUXMEM_APRUTIL_LIBRARY aprutil-1
  PATHS
  ${JUXMEM_DIR}/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY( JUXMEM_JPR_LIBRARY jpr
  PATHS
  ${JUXMEM_DIR}/lib
  /usr/lib
  /usr/local/lib
)

IF( NOT JUXMEM_FAILED )
IF( JUXMEM_INCLUDE_DIR )
IF( JUXMEM_APR_INCLUDE_DIR )
IF( JUXMEM_XML2_LIBRARY )
IF( JUXMEM_LIBRARY )
IF( JUXMEM_JXTA_LIBRARY )
IF( JUXMEM_APR_LIBRARY )
IF( JUXMEM_APRUTIL_LIBRARY )
IF( JUXMEM_JPR_LIBRARY )

  SET( JUXMEM_FOUND TRUE
    CACHE BOOL "When false, don't try to use JuxMem." )
  MARK_AS_ADVANCED( JUXMEM_FOUND )
  MARK_AS_ADVANCED( JUXMEM_DIR )
  MARK_AS_ADVANCED( JUXMEM_APR_INCLUDE_DIR )
  SET( JUXMEM_INCLUDE_DIR ${JUXMEM_APR_INCLUDE_DIR} ${JUXMEM_INCLUDE_DIR} )
  MARK_AS_ADVANCED( JUXMEM_INCLUDE_DIR )
  MARK_AS_ADVANCED( JUXMEM_XML2_LIBRARY )
  MARK_AS_ADVANCED( JUXMEM_LIBRARY )
  MARK_AS_ADVANCED( JUXMEM_JXTA_LIBRARY )
  MARK_AS_ADVANCED( JUXMEM_APR_LIBRARY )
  MARK_AS_ADVANCED( JUXMEM_APRUTIL_LIBRARY )
  MARK_AS_ADVANCED( JUXMEM_JPR_LIBRARY )
  SET( JUXMEM_LIBRARIES
    ${JUXMEM_LIBRARY}
    ${JUXMEM_JXTA_LIBRARY}
    ${JUXMEM_APR_LIBRARY}
    ${JUXMEM_APRUTIL_LIBRARY}
    ${JUXMEM_JPR_LIBRARY}
    ${JUXMEM_XML2_LIBRARY}
	 ${JUXMEM_SUB_LIBRARIES}
    CACHE STRING "Libraries to link against for JuxMem usage."
  )
  MARK_AS_ADVANCED( JUXMEM_LIBRARIES )

ENDIF( JUXMEM_JPR_LIBRARY )
ENDIF( JUXMEM_APRUTIL_LIBRARY )
ENDIF( JUXMEM_APR_LIBRARY )
ENDIF( JUXMEM_JXTA_LIBRARY )
ENDIF( JUXMEM_LIBRARY )
ENDIF( JUXMEM_XML2_LIBRARY )
ENDIF( JUXMEM_APR_INCLUDE_DIR )
ENDIF( JUXMEM_INCLUDE_DIR )
ENDIF( NOT JUXMEM_FAILED )

IF( NOT JUXMEM_FOUND )
  MESSAGE("JuxMem library was not found. Please provide JUXMEM_DIR:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DJUXMEM_DIR:PATH=$HOME/local/juxmem-0.3 ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. JUXMEM_INCLUDE_DIR)")
  MESSAGE("    Providing JUXMEM_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    JuxMem installation...")
  SET( JUXMEM_DIR "" CACHE PATH "Root of JuxMem instal tree." )
ENDIF( NOT JUXMEM_FOUND )

