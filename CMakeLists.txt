#############################################################################
# DIET main cmake entry point: comment or perish...                         #
#############################################################################

PROJECT(DIET)

SET(DIET_version 2.2)

# --------------------- OPTIONS OF THE PROJECT -----------------------------
OPTION( DIET_BUILD_BLAS_EXAMPLES "Build BLAS based DIET examples." OFF )
OPTION( DIET_BUILD_EXAMPLES      "Build DIET examples." OFF )
OPTION( DIET_BUILD_DOCUMENTATION "Build the documentation." OFF )
OPTION( DIET_USE_BATCH  "Build DIET with Batch support." OFF )
OPTION( DIET_USE_CORI   "Build DIET with CORI support." OFF )
OPTION( DIET_USE_DART   "Build DIET with dart reporting support." OFF)
OPTION( DIET_USE_FAST   "Build DIET with FAST support." OFF )
OPTION( DIET_USE_FD     "Build DIET with Fault Detector support." OFF)
OPTION( DIET_USE_JUXMEM "Build DIET with JuxMem support." OFF )
OPTION( DIET_USE_JXTA   "Build DIET with JXTA architecture support." OFF )
OPTION( DIET_USE_WORKFLOW    "Build DIET with workflow support..." OFF)
OPTION( DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF )
OPTION( DIET_WITH_MULTI_MA   "Build DIET with MULTI-Master-Agent support." OFF )
OPTION( DIET_WITH_ALTPREDICT "Build DIET with ALTernative performance PREDICTion support." OFF )

# --------------------- OPTIONS SET BY DEFAULT  ----------------------------
# One might wish not to compile DIET but just to build the docs:
OPTION( DIET_BUILD_LIBRARIES "Build DIET libraries." ON )
# Are libraries dynamic libraries as opposed to library archives ? 
# Note: this variable is a cmake internal, hence not prefixed with DIET_.
OPTION( BUILD_SHARED_LIBS "Build libraries as shared libraries." ON )

# --------------------- OPTIONS COHERENCE SANITY CHECKS --------------------
IF( NOT DIET_BUILD_LIBRARIES  AND DIET_BUILD_EXAMPLES )
  MESSAGE(SEND_ERROR "DIET_BUILD_LIBRARIES must be set for building examples.")
ENDIF( NOT DIET_BUILD_LIBRARIES AND DIET_BUILD_EXAMPLES )
# JXTA (see below) is used to provide functionalities similar to the ones
# of the DIET_WITH_MULTI_MA. For the time being those two extensions
# are not incompatible at compile time. Yet, requiring both probably
# means you are either an expert or a lost newbie...
# In order to prevent trouble, we assume the user is newbie and we
# explicitely require a disambiguation:
IF( DIET_WITH_MULTI_MA AND DIET_USE_JXTA )
 MESSAGE(SEND_ERROR "Either set DIET_WITH_MULTI_MA or DIET_USE_JXTA. Not both.")
ENDIF( DIET_WITH_MULTI_MA AND DIET_USE_JXTA )

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

### CORI (COllector of Resource Information) is heavily dependant on 
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

IF( DIET_BUILD_DOCUMENTATION )
  FIND_PACKAGE( LATEX )
  FIND_PACKAGE( Transfig )
  IF( NOT TRANSFIG_FOUND )
    MESSAGE("Transfig installation was not found. Please provide FIG2DEV.")
  ENDIF( NOT TRANSFIG_FOUND )
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
  SET( DIET_BATCH_WRITING_PATH $ENV{HOME} 
       CACHE STRING "Path where the SeD has writing access." )
ENDIF( DIET_USE_BATCH )

# Enable dynamic MULTI-Master-Agent support based on JXTA library
# (refer to http://www.jxta.org/ ). This is similar to functionalities offered
# the DIET_WITH_MULTI_MA option (see below) except that JXTA is used in place
# of CORBA. JXTA allows the creation of a new MA on demand and it's
# integration within the multi-hierarchy of existing MAs.
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

# Define for Fault Detection system
IF( DIET_USE_FD )
  ADD_DEFINITIONS( -DHAVE_FD )
ENDIF( DIET_USE_FD )

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

# Enable distributed shared memory based on Juxtaposed Memory (JuxMem)
# library (refer to http://juxmem.gforge.inria.fr/ ).
IF( DIET_USE_JUXMEM )
  FIND_PACKAGE( Juxmem )
  IF( JUXMEM_FOUND )
    ADD_DEFINITIONS( -DHAVE_JUXMEM )
  ELSE( JUXMEM_FOUND )
    SET( DIET_USE_JUXMEM OFF CACHE BOOL "Build DIET with JuxMem support." )
  ENDIF( JUXMEM_FOUND )
ENDIF( DIET_USE_JUXMEM )

IF( DIET_WITH_STATISTICS )
  # Enable generation of the statistics log:
  ADD_DEFINITIONS( -DHAVE_STATISTICS )
ENDIF( DIET_WITH_STATISTICS )

IF( DIET_WITH_ALTPREDICT )
  # Enable experimental ALTernative performance PREDICTion support:
  ADD_DEFINITIONS( -DHAVE_ALTPREDICT )
ENDIF( DIET_WITH_ALTPREDICT )

IF( DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES )
  # BLAS (Basic Linear Algebric Subroutines) examples subdir requires...BLAS !
  FIND_PACKAGE( BLAS )
ENDIF( DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES )

### Workflow support:
IF( DIET_USE_WORKFLOW )
  FIND_PACKAGE( Xerces )
  IF( XERCES_FOUND ) 
    ADD_DEFINITIONS( -DHAVE_WORKFLOW )
  ELSE( XERCES_FOUND ) 
    SET( DIET_USE_WORKFLOW OFF
      CACHE BOOL "Build DIET with workflow support..." )
  ENDIF( XERCES_FOUND ) 
ENDIF( DIET_USE_WORKFLOW )

# --------------- OFFER BUILD MODE EXTENSIONS AND DEFAULTS  ----------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/DietDefaultBuildTypes.cmake )

# ---------------- HANDLE SPECIFICITIES OF THE C COMPILER ------------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/ConfigureCCompiler.cmake )

# ---------------- SET THE LINKER SEARCH PATH (RPATH) ----------------------
SET( CMAKE_INSTALL_RPATH_USE_LINK_PATH ON
     CACHE BOOL "Wether to set an rpath for dynamic libraries." )
SET( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
     CACHE STRING "Rpath set at install stage." FORCE )

# ------------------- CTEST SUPPORT ----------------------------------------
IF( BUILD_TESTING OR DIET_USE_DART )
  INCLUDE( ${DIET_SOURCE_DIR}/Cmake/FindTestingUtils.cmake )
  # Caveat emptor:
  # 1/ Excerpt from docs about ENABLE_TESTING:
  #    [...] ctest expects to find a test file in the build directory root.
  #    Therefore, this command should be in the source directory root.
  # 2/ ENABLE_TESTING() has to be run prior to any ADD_TEST() command (or
  #    the ADD_TEST are simply ignored). Since the Testing directory is
  #    within the src subdir, the ENABLE_TESTING() must occur PRIOR to
  #    the ADD_SUBDIRECTORY( src ) command (see below).
  # 3/ Dart2 support also requires the ENABLE_TESTING() to be present
  #    and invoked prior to the INCLUDE( CTest ) command (see below).
  ENABLE_TESTING()
  # For comments on the following CXX_TEST_PATH refer to
  #   http://public.kitware.com/pipermail/cmake/2002-January/002643.html
  IF( EXECUTABLE_OUTPUT_PATH )
    SET( CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH} )
  ELSE( EXECUTABLE_OUTPUT_PATH )
    SET (CXX_TEST_PATH .)
  ENDIF (EXECUTABLE_OUTPUT_PATH)
ENDIF( BUILD_TESTING OR DIET_USE_DART )

# ---------------- OPTIONAL DART2 CLIENT SUPPORT ---------------------------
# Refer to http://na-mic.org/Wiki/index.php/Dart2Summary
IF( DIET_USE_DART )
  # With the following include you somehow engage yourself in providing
  # a CTestConfig.cmake file: refer to Modules/CTest.cmake for further details.
  INCLUDE( CTest )
ENDIF( DIET_USE_DART )

# ---------------- STATIC LINKING FIXES ------------------------------------
# When BUILD_SHARED_LIBS is NOT set, we assume that we not only expect
# static libraries but also statically linked binaries. Alas as stated by
#     http://public.kitware.com/pipermail/cmake/2005-March/006157.html
#   Unfortunately right now there is no variable that is automatically set
#   with the appropriate flags for the platform, so you need a bunch of IF
#   conditions (and maybe your own try-compiles) to set it. We should probably
#   add a target property for executables to specify whether it is linked
#   statically or shared.
# For the time being we hence use the following dirty non-portable trick:
IF( NOT BUILD_SHARED_LIBS )
  SET( CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static" )

  # For some mysterious, stange and yet unknown reason, the omnithread
  # library (from omniORB) requires to be linked against the pthread library
  # when building a static executable. Without further bitching on omniORB
  # and when we find the pthread library, we simply link against it...
  FIND_LIBRARY( LIBRARY_PTHREAD NAMES pthread )
  IF( LIBRARY_PTHREAD )
    SET( OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARIES} ${LIBRARY_PTHREAD} )
  ENDIF( LIBRARY_PTHREAD )
ENDIF( NOT BUILD_SHARED_LIBS )

# --------------------------------------------------------------------------
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( include )
ADD_SUBDIRECTORY( Cmake )

# --------------------------------------------------------------------------
MESSAGE( STATUS
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") 
MESSAGE( STATUS
  "XXXXXXXXXXXXXXXXXXXXXX  DIET configuration summary  XXXXXXXXXXXXXXXXXXXXXX") 
MESSAGE( STATUS
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") 
MESSAGE( STATUS "XXX - Install prefix: ${CMAKE_INSTALL_PREFIX}" ) 
MESSAGE( STATUS "XXX - OmniORB found: ${OMNIORB4_FOUND}" ) 
IF( OMNIORB4_FOUND )
  MESSAGE( STATUS "XXX   * OmniORB directory: ${OMNIORB4_DIR}" ) 
  MESSAGE( STATUS "XXX   * OmniORB includes: ${OMNIORB4_INCLUDE_DIR}" ) 
  MESSAGE( STATUS "XXX   * OmniORB libraries: ${OMNIORB4_LIBRARIES}" ) 
ENDIF( OMNIORB4_FOUND )

##############
MESSAGE( STATUS "XXX - General options: " ) 
MESSAGE( STATUS "XXX   * Documentation: ${DIET_BUILD_DOCUMENTATION} " ) 
IF( DIET_BUILD_LIBRARIES )
  IF( BUILD_SHARED_LIBS )
     MESSAGE( STATUS "XXX   * Dynamics Libraries: ON " ) 
  ELSE( BUILD_SHARED_LIBS )
     MESSAGE( STATUS "XXX   * Static Libraries: ON " ) 
  ENDIF( BUILD_SHARED_LIBS )
ENDIF( DIET_BUILD_LIBRARIES )
MESSAGE( STATUS "XXX   * Examples: ${DIET_BUILD_EXAMPLES} " ) 
IF( DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES )
  MESSAGE( STATUS "XXX   * BLAS Examples: ${DIET_BUILD_BLAS_EXAMPLES} " ) 
ENDIF( DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES )

##############
MESSAGE( STATUS "XXX - Options set: " ) 
IF( DIET_USE_BATCH )
  MESSAGE( STATUS "XXX   * Batch: ${DIET_USE_BATCH} " ) 
  MESSAGE( STATUS "XXX     -- Appleseeds directory: ${APPLESEEDS_DIR} " ) 
  MESSAGE( STATUS "XXX     -- Appleseeds includes: ${APPLESEEDS_INCLUDE_DIR}") 
  MESSAGE( STATUS "XXX     -- Appleseeds library: ${APPLESEEDS_LIBRARY}") 
ENDIF( DIET_USE_BATCH )
IF( DIET_USE_CORI )
  MESSAGE( STATUS "XXX   * CORI: ${DIET_USE_CORI} " ) 
ENDIF( DIET_USE_CORI )
IF( DIET_USE_DART )
  MESSAGE( STATUS "XXX   * Dart: ${DIET_USE_DART} " ) 
ENDIF( DIET_USE_DART )
IF( DIET_USE_FD )
  MESSAGE( STATUS "XXX   * FD: ${DIET_USE_FD} " ) 
ENDIF( DIET_USE_FD )
IF( DIET_USE_FAST )
  MESSAGE( STATUS "XXX   * FAST: ${DIET_USE_FAST} " ) 
ENDIF( DIET_USE_FAST )
IF( DIET_USE_JXTA )
  MESSAGE( STATUS "XXX   * JXTA: ${DIET_USE_JXTA} " ) 
ENDIF( DIET_USE_JXTA )
IF( DIET_USE_JUXMEM )
  MESSAGE( STATUS "XXX   * JuxMem: ${DIET_USE_JUXMEM} " ) 
ENDIF( DIET_USE_JUXMEM )
IF( DIET_WITH_STATISTICS )
  MESSAGE( STATUS "XXX   * Statistics: ${DIET_WITH_STATISTICS} " ) 
ENDIF( DIET_WITH_STATISTICS )
IF( DIET_WITH_MULTI_MA )
  MESSAGE( STATUS "XXX   * Multi-MA: ${DIET_WITH_MULTI_MA} " ) 
ENDIF( DIET_WITH_MULTI_MA )
IF( DIET_USE_WORKFLOW )
  MESSAGE( STATUS "XXX   * Workflow: ${DIET_USE_WORKFLOW} " ) 
ENDIF( DIET_USE_WORKFLOW )
MESSAGE( STATUS
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") 

# --------------------------------------------------------------------------
# QUESTIONS:
# - what is the general purpose of diet/include dir ? (sub lib or includes?)
#        (see src/agent/CMakeLists.txt INCLUDE_DIRECTORIES)
## eof - CMakeLists.txt
