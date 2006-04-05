#############################################################################
# DIET main cmake entry point: comment or perish...                         #
#############################################################################

PROJECT(DIET)

SET(DIET_version 2.0)

# --------------------- OPTIONS OF THE PROJECT -----------------------------
OPTION( DIET_BUILD_EXAMPLES "Build DIET examples." OFF )
OPTION( DIET_BUILD_DOCUMENTATION "Build the users manual documentation." OFF )
OPTION( DIET_USE_BATCH "Build DIET with Batch support." OFF )
OPTION( DIET_USE_BLAS "Build DIET with BLAS support." OFF )
OPTION( DIET_USE_CORI "Build DIET with CORI support." OFF )
OPTION( DIET_USE_JXTA "Build DIET with JXTA architecture support." OFF )
OPTION( DIET_USE_FAST "Build DIET with FAST support." OFF )
OPTION( DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF )
OPTION( DIET_WITH_MULTI_MA   "Build DIET with MULTI-Master-Agent support." OFF )
OPTION( DIET_MAINTAINER_MODE "Build additional documentation..." OFF )

# --------------------- BUILDING DEFAULT OPTIONS  --------------------------
# By default build dynamic libraries:
OPTION( BUILD_SHARED_LIBS "Build DIET with shared libraries." ON )

# --------------------- DEPENDENCIES TOWARDS "CLASSIC" C++ HEADERS ---------
# This is just a convenience to advance the detection of any potential missing
# C++ "classic" header to the cmake stage. If we were to forget the following
# tests (and such a header was missing), things would simply break at
# preprocessing stage of the compilation...
# [Those tests are a translation of the AC_CHECK_HEADER(...) that were expressed
# in the configure.ac of the autotools version].
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)

SET( DIET_INCLUDES_TO_CHECK
  assert.h
  iostream
  limits.h
  math.h
  stdlib.h
  string.h
  unistd.h
  sys/stat.h
  sys/types.h )

FOREACH( include_to_check ${DIET_INCLUDES_TO_CHECK} )
  CHECK_INCLUDE_FILE_CXX( ${include_to_check} dummyfound${include_to_check} )
  IF ( NOT dummyfound${include_to_check} )
    MESSAGE(FATAL_ERROR "Missing ${include_to_check} header file.")
  ENDIF( NOT dummyfound${include_to_check} )
ENDFOREACH( include_to_check )

# --------------------- DEPENDENCIES TOWARDS SYSTEM FUNCTIONS ------------
# This is just a convenience to advance the detection of any potential missing
# external function to the cmake stage. If we were to forget the following
# tests (and such an external function was missing), things would simply break
# at linking stage...
# [Those tests are a translation of the AC_CHECK_FUNCS(...) that were expressed
# in the configure.ac of the autotools version].
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

SET( DIET_FUNCTIONS_TO_CHECK
  gethostname
  gettimeofday
  strdup
  strtok_r )

FOREACH( function_to_check ${DIET_FUNCTIONS_TO_CHECK} )
  CHECK_FUNCTION_EXISTS( ${function_to_check} dummyfound${function_to_check} )
  IF ( NOT dummyfound${function_to_check} )
    MESSAGE(FATAL_ERROR "Missing ${function_to_check} function.")
  ENDIF( NOT dummyfound${function_to_check} )
ENDFOREACH( function_to_check )

### CORI (COllector of Ressource Information) is heavily dependant on 
# system calls for probing ressources:
IF( DIET_USE_CORI )
  ADD_DEFINITIONS( -DHAVE_CORI )
  INCLUDE( ${DIET_SOURCE_DIR}/Cmake/ConfigureCORI.cmake )
ENDIF( DIET_USE_CORI )

# --------------------- DEPENDENCIES TOWARDS EXTERNAL PACKAGES -------------
# Path to additional modules (i.e. used by FIND_PACKAGE commands making
# reference to non CMake defined "standard" modules):
SET( CMAKE_MODULE_PATH ${DIET_SOURCE_DIR}/Cmake )

# OmniORB is mandatory:
FIND_PACKAGE( OmniORB )
IF( OMNIORB4_FOUND )
  ADD_DEFINITIONS( -D__OMNIORB4__ )
ELSE( OMNIORB4_FOUND )
  MESSAGE("omniORB installation was not found. Please provide OMNIORB4_DIR:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DOMNIORB4_DIR:PATH=/usr/local/omniORB-4.0.5 ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g.  OMNIORB4_INCLUDE_DIR).")
  MESSAGE("    Providing OMNIORB4_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    omniORB installation...")
  SET( OMNIORB4_DIR "" CACHE PATH "Root of omniORB instal tree." )
ENDIF( OMNIORB4_FOUND )

# Maintainer mode basically means building the developper's guide
# documentation and compiling/linking with gobs of found warnings...
IF( DIET_MAINTAINER_MODE )
  SET( DIET_BUILD_EXAMPLES ON )
  IF(NOT CMAKE_BUILD_TYPE)
    SET( CMAKE_BUILD_TYPE Maintainer CACHE STRING
    "Build mode"
    FORCE
  )
  ENDIF(NOT CMAKE_BUILD_TYPE)
ENDIF( DIET_MAINTAINER_MODE )

IF( DIET_BUILD_DOCUMENTATION )
  # DIET_MAINTAINER_MODE is used within the doc subdirectory
  FIND_PACKAGE( LATEX )
  FIND_PACKAGE( Transfig )
  FIND_PACKAGE( ImageMagick )
  FIND_PACKAGE( Doxygen )
  ADD_SUBDIRECTORY( doc )
ENDIF( DIET_BUILD_DOCUMENTATION )

IF( DIET_USE_BATCH )
  FIND_PACKAGE( AppleSeeds )
  IF( APPLESEEDS_FOUND )
    # The HAVE_BATCH compiler flags has too many occurences to be cleanly
    # associated with all the concerned files by using
    # SET_SOURCE_FILES_PROPERTIES combined with PROPERTIES COMPILE_FLAGS.
    # [For example ServiceTable.hh gets included in many places...]
    ADD_DEFINITIONS( -DHAVE_BATCH )
  ELSE( APPLESEEDS_FOUND )
    MESSAGE( SEND_ERROR "Properly provide the AppleSeeds library.")
    SET( DIET_USE_BATCH OFF )
  ENDIF( APPLESEEDS_FOUND )
ENDIF( DIET_USE_BATCH )

# JXTA (see below) is used to provide functionalities similar to the ones
# of the DIET_WITH_MULTI_MA. For the time being those two extensions
# are not incompatible at compile time. Yet, requiring both probably
# means you are either an expert or a lost newbie... Allthough 
# the flag DIET_MAINTAINER_MODE doesn't convey any expertise semantics
# we consider the newbie won't set that mode on. Hence when not
# in MAINTAINER_MODE, we assume the user is newbie and we explicitely
# require an explicit choice:
IF( DIET_WITH_MULTI_MA AND DIET_USE_JXTA AND NOT DIET_MAINTAINER_MODE )
 MESSAGE(SEND_ERROR "Either set DIET_WITH_MULTI_MA or DIET_USE_JXTA. Not both.")
ENDIF( DIET_WITH_MULTI_MA AND DIET_USE_JXTA AND NOT DIET_MAINTAINER_MODE )

IF( DIET_USE_JXTA )
  FIND_PACKAGE( JavaExtended )
  IF( JAVA_FOUND )
    ADD_DEFINITIONS(-DHAVE_JXTA)
  ELSE( JAVA_FOUND )
    SET( DIET_USE_JXTA OFF )
  ENDIF( JAVA_FOUND )
ENDIF( DIET_USE_JXTA )

# So called MULTI-Master-Agent
IF( DIET_WITH_MULTI_MA )
  ADD_DEFINITIONS( -DHAVE_MULTI_MA )
ENDIF( DIET_WITH_MULTI_MA )

IF( DIET_USE_FAST )
  FIND_PACKAGE( FAST )
  IF( FAST_FOUND )
    ADD_DEFINITIONS( -DHAVE_FAST )
    ### FIXME: we only ported the FAST 0.8.x API way of doing things.
    ### Nevertheless the code still makes references to the FAST 0.4.x API
    ### (search for defines __FAST_0_4__ versus __FAST_0_8__. Should we trash
    ### the FAST 0.4.x API support ?
    ADD_DEFINITIONS( -D__FAST_0_8__ )
  ENDIF( FAST_FOUND )
ENDIF( DIET_USE_FAST )

IF( DIET_WITH_STATISTICS )
  # Enable generation of the statistics log
  ADD_DEFINITIONS( -DHAVE_STATISTICS )
ENDIF( DIET_WITH_STATISTICS )

IF( DIET_BUILD_EXAMPLES AND DIET_USE_BLAS )
  # BLAS (Basic Linear Algebric Subroutines) examples subdir requires...BLAS !
  FIND_PACKAGE( BLAS )
ENDIF( DIET_BUILD_EXAMPLES AND DIET_USE_BLAS )

# --------------------------------------------------------------------------
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( include )
ADD_SUBDIRECTORY( Cmake )

# --------------- HANDLE BUILD TYPE DEFAULT AND EXTENSIONS  ------------------
# First offer an extra maintainer build type i.e. a build mode for which
# the compiler and linker go paranoid and report about most of the warning
# they are aware of.
# Note: the real test for offering this option should be against the compiler
#       and not against the system, since the flags that we set below are only
#       valid for gcc.
SET( CMAKE_BUILD_TYPE_DOCSTRING
  "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." )
  
IF( UNIX )
  # -Wold-style-cast generates too many warning due to omniORB includes
  # -pedantic generates ISO C++ unsupported 'long long' errors for omniORB
  # includes
  SET( CMAKE_CXX_FLAGS_MAINTAINER
    "-Wall -Wabi -Wstrict-null-sentinel -Woverloaded-virtual"
    CACHE STRING 
    "Flags used by the C++ compiler during maintainer builds."
    FORCE
  )
  SET( CMAKE_C_FLAGS_MAINTAINER
    "-Wall -pedantic"
    CACHE STRING 
    "Flags used by the C compiler during maintainer builds."
    FORCE
  )
  SET( CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    "--no-undefined --unresolved-symbols=report-all --warn-common --warn-once"
    CACHE STRING 
    "Flags used for linking binaries during maintainer builds."
    FORCE
  )
  SET( CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    "--unresolved-symbols=report-all --warn-common --warn-once"
    CACHE STRING 
    "Flags used by the shared libraries linker during maintainer builds."
    FORCE
  )
  SET( CMAKE_MODULE_LINKER_FLAGS_MAINTAINER
    "-total_module_bozo"
    CACHE STRING 
    "What the hack is a module anyhow..."
    FORCE
  )
  MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_MAINTAINER
    CMAKE_C_FLAGS_MAINTAINER
    CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    CMAKE_MODULE_LINKER_FLAGS_MAINTAINER
  )
  SET( CMAKE_BUILD_TYPE_DOCSTRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Maintainer." )
ENDIF( UNIX )

# Set the default build mode to compile c++ and c code with optimizations
# and debug info (roughly speaking "-O2 -g" on Un*x):
IF(NOT CMAKE_BUILD_TYPE)
  SET( CMAKE_BUILD_TYPE
    RelWithDebInfo CACHE STRING
    ${CMAKE_BUILD_TYPE_DOCSTRING}
    FORCE
  )
ENDIF(NOT CMAKE_BUILD_TYPE)

# --------------------- HANDLE COMPILER SPECIFICITIES  --------------------
INCLUDE(${DIET_SOURCE_DIR}/Cmake/CheckCCompiler.cmake)

# Reset the values before testing:
SET( CMAKE_C_FLAGS ${CMAKE_C_FLAGS_INIT}
  CACHE STRING "Flags for C compiler"
  FORCE
  )
  
### Does the C compiler support inlining option ?
CHECK_C_COMPILER_SUPPORTS_INLINE( INLINE_VALUE INLINE_SUPPORTED )

SET( DIET_CMAKE_C_FLAGS "")
IF( INLINE_SUPPORTED )
  IF( NOT "${INLINE_VALUE}" MATCHES "^inline$" )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "Adding exotic C inline support: ${INLINE_VALUE}\n\n" )
    SET( DIET_CMAKE_C_FLAGS "-Dinline=${INLINE_VALUE}")
  ENDIF( NOT "${INLINE_VALUE}" MATCHES "^inline$" )
ENDIF( INLINE_SUPPORTED )

### Is the C compiler ANSI C-conforming for const ?
CHECK_C_COMPILER_SUPPORTS_CONST( CONST_SUPPORTED )
IF( NOT CONST_SUPPORTED )
  SET( DIET_CMAKE_C_FLAGS ${DIET_CMAKE_C_FLAGS} "-Dconst=\"\"" )
ENDIF( NOT CONST_SUPPORTED )

### Pass the results to cmake internal flags:
IF( DIET_CMAKE_C_FLAGS )
  SET( CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS_INIT} ${DIET_CMAKE_C_FLAGS}"
    CACHE
    STRING "Flags for C compiler"
    FORCE
  )
ENDIF( DIET_CMAKE_C_FLAGS )

# --------------------------------------------------------------------------
# QUESTIONS:
# - what is the general purpose of diet/include dir ? (sub lib or includes?)
#        (see src/agent/CMakeLists.txt INCLUDE_DIRECTORIES)
## eof - CMakeLists.txt
