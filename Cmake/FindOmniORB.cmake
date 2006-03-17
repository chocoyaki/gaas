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
#### The list of required libraries for omniORB is not so clear. See
# http://www.omniorb-support.com/pipermail/omniorb-list/2005-May/026666.html
# The answer should basically be in
#    - omniORB-4.0.X/README.*
#    - omniORB-4.0.X/readmes/*
# but doesn't seem that obvious to newbies.
# The logic we took is here to impose the detection of omniORB4, omnithread,
# and omniDynamic4 libraries. Optionaly we search for libraries COS4 and
#
 
FIND_PATH( OMNIORB4_INCLUDE_DIR omniORB4/CORBA.h
  PATHS
  ${OMNIORB4_DIR}/include
  /usr/include
  /usr/local/include
)

SET( OMNIORB4_DEFAULT_LIB_PATH /usr/lib /usr/local/lib )

FIND_LIBRARY(  OMNIORB4_LIBRARY_omniORB4 omniORB4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_omnithread omnithread
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_omniDynamic4 omniDynamic4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_COS4 COS4      # Optional library
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_LIBRARY( OMNIORB4_LIBRARY_COSDynamic4 COSDynamic4 # Optional library
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

FIND_PROGRAM(OMNIORB4_IDL_COMPILER
  NAMES omniidl
  PATHS ${OMNIORB4_DIR}/bin
        /usr/bin
        /usr/local/bin
  DOC "What is the path where omniidl (the idl compiler) can be found"
)
 
SET( OMNIORB4_FOUND "NO" )

IF( OMNIORB4_INCLUDE_DIR )
IF( OMNIORB4_LIBRARY_omniORB4 )
IF( OMNIORB4_LIBRARY_omnithread )
IF( OMNIORB4_LIBRARY_omniDynamic4 )
IF( OMNIORB4_IDL_COMPILER )

  SET( OMNIORB4_FOUND "YES" )
  MARK_AS_ADVANCED( OMNIORB4_DIR )
  MARK_AS_ADVANCED( OMNIORB4_INCLUDE_DIR )
  MARK_AS_ADVANCED( OMNIORB4_LIBRARY_omniORB4 )
  MARK_AS_ADVANCED( OMNIORB4_LIBRARY_omnithread )
  MARK_AS_ADVANCED( OMNIORB4_LIBRARY_omniDynamic4 )
  MARK_AS_ADVANCED( OMNIORB4_IDL_COMPILER )
  SET( OMNIORB4_LIBRARIES
    ${OMNIORB4_LIBRARY_omniORB4}
    ${OMNIORB4_LIBRARY_omnithread}
    ${OMNIORB4_LIBRARY_omniDynamic4}
  )

  # We do not require the existence of the libraries COS4 and COSDynamic4
  # since they exist in some distribution packages (e.g. Fedora Core) and
  # not in others (e.g. Debian). When present we add them to the list of
  # libraries we link against. When absent we hide away (in the advanced
  # parameters) the NOT-FOUND associated variable in order not to deconcert
  # the newbie:
  IF( OMNIORB4_LIBRARY_COS4 )
    SET( OMNIORB4_LIBRARIES
      ${OMNIORB4_LIBRARIES}
      ${OMNIORB4_LIBRARY_COS4}
    )
  ENDIF( OMNIORB4_LIBRARY_COS4 )
  MARK_AS_ADVANCED( OMNIORB4_LIBRARY_COS4 )

  IF( OMNIORB4_LIBRARY_COSDynamic4 )
    SET( OMNIORB4_LIBRARIES
      ${OMNIORB4_LIBRARIES}
      ${OMNIORB4_LIBRARY_COSDynamic4}
    )
  ENDIF( OMNIORB4_LIBRARY_COSDynamic4 )
  MARK_AS_ADVANCED( OMNIORB4_LIBRARY_COSDynamic4 )

ENDIF( OMNIORB4_IDL_COMPILER )
ENDIF( OMNIORB4_LIBRARY_omniDynamic4 )
ENDIF( OMNIORB4_LIBRARY_omnithread )
ENDIF( OMNIORB4_LIBRARY_omniORB4 )
ENDIF( OMNIORB4_INCLUDE_DIR )
