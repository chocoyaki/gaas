#
# Find the AppleSeeds libraries and include dir
#
# For AppleSeeds reference: 
#     http://grail.sdsc.edu/projects/appleseeds/install.html
#
 
# APPLESEEDS_INCLUDE_DIR - Directories to include to use AppleSeeds
# APPLESEEDS_LIBRARIES   - Files to link against to use AppleSeeds
# APPLESEEDS_FOUND       - When false, don't try to use AppleSeeds
# APPLESEEDS_DIR         - (optional) Suggested installation directory to search
#
# APPLESEEDS_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when AppleSeeds was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set APPLESEEDS_DIR it to your specific installation directory
#
 
FIND_PATH( APPLESEEDS_INCLUDE_DIR appleseeds/appleseeds.h
  PATHS
  ${APPLESEEDS_DIR}/include
  /usr/include
  /usr/local/include
)

FIND_LIBRARY( APPLESEEDS_LIBRARY appleseeds
  PATHS
  ${APPLESEEDS_DIR}/lib
  /usr/lib
  /usr/local/lib
)

IF( APPLESEEDS_INCLUDE_DIR )
IF( APPLESEEDS_LIBRARY )

  SET( APPLESEEDS_FOUND "YES" )
  SET( APPLESEEDS_INCLUDE_DIR ${APPLESEEDS_INCLUDE_DIR}/appleseeds )
  MARK_AS_ADVANCED( APPLESEEDS_DIR )
  MARK_AS_ADVANCED( APPLESEEDS_INCLUDE_DIR )
  SET( APPLESEEDS_LIBRARIES
    ${APPLESEEDS_LIBRARY}  ### FIXME: ajouter -lpthread
  )
  MARK_AS_ADVANCED( APPLESEEDS_LIBRARY )

ENDIF( APPLESEEDS_LIBRARY )
ENDIF( APPLESEEDS_INCLUDE_DIR )

IF( NOT APPLESEEDS_FOUND )
  MESSAGE("AppleSeeds library was not found. Please provide APPLESEEDS_DIR:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DAPPLESEEDS_DIR:PATH=/usr/local/AppleSeeds ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. APPLESEEDS_INCLUDE_DIR)")
  MESSAGE("    Providing APPLESEEDS_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    AppleSeeds installation...")
  SET( APPLESEEDS_DIR "" CACHE PATH "Root of AppleSeeds instal tree." )
ENDIF( NOT APPLESEEDS_FOUND )

