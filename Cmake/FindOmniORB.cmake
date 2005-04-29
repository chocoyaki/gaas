#
# Find the omniORB libraries and include dir
#
 
# OMNIORB4_INCLUDE_DIR  - Directories to include to use omniORB
# OMNIORB4_LIBRARIES    - Files to link against to use omniORB
# OMNIORB4_IDL_COMPILER
# OMNIORB4_FOUND        - When false, don't try to use omniORB
# OMNIORB4_DIR          - (optional) Suggested installation directory to search
#
# OMNIORB4_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when omniORB was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set OMNIORB4_DIR it to your specific installation directory
#
 
FIND_PATH( OMNIORB4_omniORB4_SUBDIR omniORB4/CORBA.h
  PATHS
  ${OMNIORB4_DIR}/include
  /usr/include
  /usr/local/include
)

SET( OMNIORB4_DEFAULT_LIB_PATH /usr/lib /usr/local/lib )
#### FIXME: what are the required libraries for proper usage ????
FIND_LIBRARY(  OMNIORB4_LIBRARY_omniORB4 omniORB4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_omnithread omnithread
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_COS4 COS4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_COSDynamic4 COSDynamic4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_PROGRAM(OMNIORB4_IDL_COMPILER
  NAMES omniidl
  PATHS ${OMNIORB4_DIR}/bin
        /usr/bin
        /usr/local/bin
  DOC "The idl compiler"
)
 
IF( OMNIORB4_omniORB4_SUBDIR )
IF( OMNIORB4_LIBRARY_omniORB4 )
IF( OMNIORB4_LIBRARY_omnithread )
IF( OMNIORB4_LIBRARY_COS4 )
IF( OMNIORB4_LIBRARY_COSDynamic4 )
IF( OMNIORB4_IDL_COMPILER )

  SET( OMNIORB4_FOUND "YES" )
  SET( OMNIORB4_INCLUDE_DIR
    ${OMNIORB4_omniORB4_SUBDIR} )
  SET( OMNIORB4_LIBRARIES
    ${OMNIORB4_LIBRARY_omniORB4}
    ${OMNIORB4_LIBRARY_omnithread}
    ${OMNIORB4_LIBRARY_COS4}
    ${OMNIORB4_LIBRARY_COSDynamic4} )

ENDIF( OMNIORB4_IDL_COMPILER )
ENDIF( OMNIORB4_LIBRARY_COSDynamic4 )
ENDIF( OMNIORB4_LIBRARY_COS4 )
ENDIF( OMNIORB4_LIBRARY_omnithread )
ENDIF( OMNIORB4_LIBRARY_omniORB4 )
ENDIF( OMNIORB4_omniORB4_SUBDIR )

IF( NOT OMNIORB4_FOUND )
  SET( OMNIORB4_DIR "" CACHE PATH "Root of omniORB instal tree (optional)." )
  MARK_AS_ADVANCED( OMNIORB4_DIR )
ENDIF( NOT OMNIORB4_FOUND )

