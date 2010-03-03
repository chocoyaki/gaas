#############################################################################
# DIET main cmake entry point: comment or perish...                         #
#############################################################################

PROJECT(DIET)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF( COMMAND CMAKE_POLICY )
  # CMAKE policy CMP0002: CMake 2.6 introduces policy CMP0002 which
  # requires all logical target names to be globally unique (duplicate
  # target names previously worked in some cases by accident but were
  # not diagnosed). Projects using duplicate target names and working
  # accidentally will receive warnings referencing the policy.
  # Thus we set to 'OLD' this policy in order to remove the warnings.
  # THIS SHOULD BE SET TO 'NEW', but it requires that all targets have a 
  # different name, which currently isn't the case.
  CMAKE_POLICY( SET CMP0002 OLD )

  # See `cmake --help-policy CMP0003`
  CMAKE_POLICY( SET CMP0003 NEW )
ENDIF( COMMAND CMAKE_POLICY )


SET( DIET_MAJOR_VERSION 2 )
SET( DIET_MINOR_VERSION 4 )

SET( DIET_REVISION_VERSION rc2 )
SET( DIET_VERSION ${DIET_MAJOR_VERSION}.${DIET_MINOR_VERSION})
IF (DIET_REVISION_VERSION)
	SET( DIET_VERSION ${DIET_VERSION}.${DIET_REVISION_VERSION})
ENDIF (DIET_REVISION_VERSION)
EXEC_PROGRAM(date ARGS "+%Y/%m/%d-%H:%M:%S" OUTPUT_VARIABLE BUILD_DATE )
SET( DIET_BUILD_VERSION ${BUILD_DATE})

# --------------------- OPTIONS OF THE PROJECT -----------------------------
OPTION( DIET_BUILD_BLAS_EXAMPLES "Build BLAS based DIET examples." OFF )
OPTION( DIET_BUILD_EXAMPLES      "Build DIET examples." OFF )
#OPTION( DIET_BUILD_DOCUMENTATION "Build the documentation." OFF ) ## go in the doc dir
OPTION( DIET_USE_ALT_BATCH  "Build DIET with Batch support." OFF )
OPTION( DIET_USE_CORI   "Build DIET with CORI support." OFF )
OPTION( DIET_USE_DART   "Build DIET with dart reporting support." OFF)
OPTION( DIET_USE_DYNAMICS   "Build DIET with dynamic hierarchy modifications features." OFF)
OPTION( DIET_USE_FAST   "Build DIET with FAST support." OFF )
OPTION( DIET_USE_FD     "Build DIET with Fault Detector support." OFF)
OPTION( DIET_USE_JUXMEM "Build DIET with JuxMem support." OFF )
OPTION( DIET_USE_JXTA   "Build DIET with JXTA architecture support." OFF )
OPTION( DIET_USE_WORKFLOW    "Build DIET with workflow support..." OFF)
OPTION( DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF )
OPTION( DIET_WITH_MULTI_MA   "Build DIET with MULTI-Master-Agent support." OFF )
# New : User scheduler at agents level support.
OPTION( DIET_USE_USERSCHED   "Build DIET with user scheduler support (Experimental)." OFF )
OPTION( DIET_USE_ENDIANNESS  "Build DIET with endianness management  (Experimental)." OFF )
OPTION( DIET_USE_CCS         "Build DIET with client custom scheduling  (Experimental)." OFF )
# Ackfile support
OPTION( DIET_USE_ACKFILE     "Build DIET with acknowledge file." OFF)
## Use Dagda as data manager.
OPTION( DIET_USE_DAGDA "Build DIET with Dagda as data manager. (In developpment)." OFF )
# Profiling, used to add counters with TAU (http://http://www.cs.uoregon.edu/research/tau/home.php)
OPTION( DIET_TAU_PROFILING "Build DIET with TAU profiling support. (In developpment)." OFF )
MARK_AS_ADVANCED( DIET_TAU_PROFILING )
OPTION( DIET_USE_MULTICALL "Build DIET with the multiple async call (In development)" OFF)
MARK_AS_ADVANCED( DIET_USE_MULTICALL )
# ---- get C and CXX compiler version -----#
EXEC_PROGRAM(${CMAKE_C_COMPILER} ARGS "-dumpversion" OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION )
EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS "-dumpversion" OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION )


# --------------------- OPTIONS SET BY DEFAULT  ----------------------------
# One might wish not to compile DIET but just to build the docs:
OPTION( DIET_BUILD_LIBRARIES "Build DIET libraries." ON )
# Are libraries dynamic libraries as opposed to library archives ?
# Note: this variable is a cmake internal, hence not prefixed with DIET_.
OPTION( BUILD_SHARED_LIBS "Build libraries as shared libraries." ON )

# --------------------- OPTIONS COHERENCE SANITY CHECKS --------------------

IF( DIET_BUILD_BLAS_EXAMPLES AND NOT DIET_BUILD_EXAMPLES )
  MESSAGE( SEND_ERROR "For DIET_BUILD_BLAS_EXAMPLES to be effective you MUST also set DIET_BUILD_EXAMPLES" )
ENDIF( DIET_BUILD_BLAS_EXAMPLES AND NOT DIET_BUILD_EXAMPLES )

IF( NOT DIET_BUILD_LIBRARIES  AND DIET_BUILD_EXAMPLES )
  MESSAGE(SEND_ERROR "DIET_BUILD_LIBRARIES must be set for building examples.")
ENDIF( NOT DIET_BUILD_LIBRARIES AND DIET_BUILD_EXAMPLES )

IF( DIET_USE_WORKFLOW AND NOT DIET_USE_DAGDA )
  MESSAGE(SEND_ERROR "DIET_USE_WORKFLOW requires DIET_USE_DAGDA.")
ENDIF( DIET_USE_WORKFLOW AND NOT DIET_USE_DAGDA )

IF( DIET_USE_DYNAMICS AND NOT DIET_USE_DAGDA )
  MESSAGE(SEND_ERROR "DIET_USE_DYNAMICS requires DIET_USE_DAGDA.")
ENDIF( DIET_USE_DYNAMICS AND NOT DIET_USE_DAGDA )

# JXTA (see below) is used to provide functionalities similar to the ones
# of the DIET_WITH_MULTI_MA. For the time being those two extensions
# are not incompatible at compile time. Yet, requiring both probably
# means you are either an expert or a lost newbie...
# In order to prevent trouble, we assume the user is newbie and we
# explicitely require a disambiguation
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

IF( DIET_USE_ALT_BATCH )
    # In that case, CORI must also be present because of CORI_BATCH
    IF( NOT DIET_USE_CORI )
      MESSAGE( "Because of needed batch estimations, CORI must also be selected." )
      SET( DIET_USE_CORI ON )
    ENDIF( NOT DIET_USE_CORI )

    # The HAVE_ALT_BATCH compiler flags has too many occurences to be cleanly
    # associated with all the concerned files by using
    # SET_SOURCE_FILES_PROPERTIES combined with PROPERTIES COMPILE_FLAGS.
    # [For example ServiceTable.hh gets included in many places...]
    ADD_DEFINITIONS( -DHAVE_ALT_BATCH )
ENDIF( DIET_USE_ALT_BATCH )

### CORI (COllector of Resource Information) is heavily dependant on
# system calls for probing ressources:
IF( DIET_USE_CORI )
  ADD_DEFINITIONS( -DHAVE_CORI )
  INCLUDE( ${DIET_SOURCE_DIR}/Cmake/ConfigureCORI.cmake )
ENDIF( DIET_USE_CORI )


#-------------- PLATFORM SPECIFIC COMPILATION FLAGS -----------------------
# Requires CMake >= 2.4.7
# Used within omniORB, when set remove some warnings/errors
# Not tested under SunOS & FreeBSD
#
STRING( COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "AIX" AIX )
STRING( COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Darwin" APPLE )
STRING( COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Linux" LINUX )
STRING( COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "SunOS" SUNOS )
STRING( COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "FreeBSD" FREEBSD )
STRING( COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "CYGWIN" CYGWIN )

IF( CYGWIN )
  MESSAGE( STATUS "XXX System name Cygwin on Windows" )
  ADD_DEFINITIONS( -D__cygwin__ )

  # Workflow compilation on Windows inside the Cygwin environment
  # is not permitted until the good compilation of the libraries of
  # XQilla and Xerces are available.
  # For instance it is not possible so we have to wait for the 
  # problem resolution
  # TODO : solve the problem.
  SET( DIET_USE_WORKFLOW OFF CACHE BOOL "Unable to compile workflow on Windows with Cygwin. " FORCE )

ELSEIF( AIX )
  MESSAGE( STATUS "XXX System name Aix" )
  ADD_DEFINITIONS( -D__aix__ )

ELSEIF( APPLE )
  MESSAGE( STATUS "XXX System name Darwin" )
  ADD_DEFINITIONS( -D__darwin__ )

  # Static linking for a binary is forbidden on mac systems...
  # Static compilation of libraries is authorized...
  # So, if we are on a mac system let's just disable the static
  # compilation option.
  # http://developer.apple.com/qa/qa2001/qa1118.html
  MARK_AS_ADVANCED( BUILD_SHARED_LIBS )
  SET( BUILD_SHARED_LIBS ON CACHE BOOL "Unable to compile static binaries on APPLE systems." FORCE )

ELSEIF( LINUX )
  MESSAGE( STATUS "XXX System name Linux" )
  ADD_DEFINITIONS( -D__linux__ )

ELSEIF( SUNOS )
  MESSAGE( STATUS "XXX System name SunOS" )
  ADD_DEFINITIONS( -D__sunos__ )

ELSEIF( FREEBSD )
  MESSAGE( STATUS "XXX System name FreeBSD" )
  ADD_DEFINITIONS( -D__freebsd__ )
ENDIF( CYGWIN )



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
  MESSAGE("    cmake .. -DOMNIORB4_DIR:PATH=/usr/local/omniORB-4.0.7 ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g.  OMNIORB4_INCLUDE_DIR).")
  MESSAGE("    Providing OMNIORB4_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    omniORB installation...")
  SET( OMNIORB4_DIR "" CACHE PATH "Root of omniORB instal tree." )
ENDIF( OMNIORB4_FOUND )



## No mor
#IF( DIET_BUILD_DOCUMENTATION )
#  FIND_PACKAGE( LATEX )
#
#  FIND_PACKAGE( Transfig )
#  IF( NOT TRANSFIG_FOUND )
#    MESSAGE( "Transfig installation was not found. Please provide FIG2DEV." )
#    SET( DIET_BUILD_DOCUMENTATION OFF )
#  ENDIF( NOT TRANSFIG_FOUND )
#
#  FIND_PACKAGE( ImageMagick )
#  IF( NOT IMAGEMAGICK_CONVERT_EXECUTABLE )
#    MESSAGE( "Convert ImageMagick utility was not found." )
#    MESSAGE( "Please provide IMAGEMAGICK_CONVERT_EXECUTABLE." )
#    SET( DIET_BUILD_DOCUMENTATION OFF )
#  ENDIF( NOT IMAGEMAGICK_CONVERT_EXECUTABLE )
#
#  FIND_PACKAGE( Doxygen )
#  IF( NOT DOXYGEN_EXECUTABLE OR NOT DOXYGEN_DOT_EXECUTABLE )
#    MESSAGE( "Doxygen or Dot were not found." )
#    SET( DIET_BUILD_DOCUMENTATION OFF )
#  ENDIF( NOT DOXYGEN_EXECUTABLE OR NOT DOXYGEN_DOT_EXECUTABLE )
#
#  IF( DIET_BUILD_DOCUMENTATION )
#    ADD_SUBDIRECTORY( doc )
#  ENDIF( DIET_BUILD_DOCUMENTATION )
#ENDIF( DIET_BUILD_DOCUMENTATION )

# Enable dynamic MULTI-Master-Agent support based on JXTA library
# (refer to http://www.jxta.org/ ). This is similar to functionalities offered
# the DIET_WITH_MULTI_MA option (see below) except that JXTA is used in place
# of CORBA. JXTA allows the creation of a new MA on demand and it's
# integration within the multi-hierarchy of existing MAs.
IF( DIET_USE_JXTA )
  # Looking for the java compiler, javac, javah...
  FIND_PACKAGE( JavaExtended )
  IF( JAVA_FOUND )
    ADD_DEFINITIONS(-DHAVE_JXTA)
  ELSE( JAVA_FOUND )
    SET( DIET_USE_JXTA OFF )
  ENDIF( JAVA_FOUND )
  # For locating the jni.h:
  FIND_PACKAGE( JNI )
  IF( NOT JAVA_INCLUDE_PATH )
    MESSAGE( SEND_ERROR "JXTA exension: could not find jni.h.")
    SET( DIET_USE_JXTA OFF )
  ENDIF( NOT JAVA_INCLUDE_PATH )
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

IF( DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES )
  # BLAS (Basic Linear Algebric Subroutines) examples subdir requires...BLAS !
  FIND_PACKAGE( BLAS )
ENDIF( DIET_BUILD_EXAMPLES AND DIET_BUILD_BLAS_EXAMPLES )

### Workflow support:
IF( DIET_USE_WORKFLOW )
  FIND_PACKAGE( Xerces )
  IF( XERCES_FOUND )
    IF( XERCES_VERSION LESS 30 )
      MESSAGE( "Diet Workflow support requires at leat version 3.0 of Xerces." )
      MESSAGE( "Workflow support will be disabled" )
      SET( DIET_USE_WORKFLOW OFF)
    ELSE( XERCES_VERSION LESS 30 )
      FIND_PACKAGE( XQilla )
      IF( XQILLA_FOUND )
        ADD_DEFINITIONS( -DHAVE_WORKFLOW )
        SET ( DIET_USE_DAGDA ON )
      ELSE ( XQILLA_FOUND )
        MESSAGE( "Diet Workflow support requires XQilla library." )
        MESSAGE( "Workflow support will be disabled" )
        SET( DIET_USE_WORKFLOW OFF)
      ENDIF ( XQILLA_FOUND )
    ENDIF( XERCES_VERSION LESS 30 )
  ELSE( XERCES_FOUND )
    SET( DIET_USE_WORKFLOW OFF
         CACHE BOOL "Build DIET with workflow support..." )
  ENDIF( XERCES_FOUND )
ENDIF( DIET_USE_WORKFLOW )

# --------------------------- Ackfile support ------------------------
IF( DIET_USE_ACKFILE )
  ADD_DEFINITIONS( -DHAVE_ACKFILE )
ENDIF( DIET_USE_ACKFILE )

# --------------- New : For users agents scheduler support ------------------
IF( DIET_USE_USERSCHED )
  ADD_DEFINITIONS( -DUSERSCHED )
ENDIF( DIET_USE_USERSCHED )

# --------------- New : Dagda as data manager support ---------------
IF( DIET_USE_DAGDA )
  ADD_DEFINITIONS( -DHAVE_DAGDA )
  # Advance data ID depending on the uuid lib.
  FIND_FILE( UUIDLIB
    NAMES uuid.h
    PATHS /usr/include /usr/include/uuid
      /include /include/uuid
      /usr/local/include /usr/local/include/uuid
      ${UUID_PATH}/uuid
  )
  IF ( UUIDLIB )
    ADD_DEFINITIONS( -DHAVE_ADVANCED_UUID )
    IF ( NOT APPLE )
      # This library is not needed on Mac OS X.
       SET( UUIDLIB uuid )
    ELSE( NOT APPLE )
      SET( UUIDLIB "")
    ENDIF( NOT APPLE )
  ELSE( UUIDLIB )
    SET( UUIDLIB "")
  ENDIF( UUIDLIB )
ENDIF( DIET_USE_DAGDA )

# --------------- DIET Endianness support  ------------------
IF( DIET_USE_ENDIANNESS )
  ADD_DEFINITIONS( -DWITH_ENDIANNESS )
ENDIF( DIET_USE_ENDIANNESS )

# --------------- DIET custom client scheduling support  ------------------
IF( DIET_USE_CCS )
  ADD_DEFINITIONS( -DHAVE_CCS )
ENDIF( DIET_USE_CCS )

# --------------- DIET multiple async call support  ------------------
IF( DIET_USE_MULTICALL )
  ADD_DEFINITIONS( -DHAVE_MULTICALL )
ENDIF( DIET_USE_MULTICALL )

# --------------- OFFER BUILD MODE EXTENSIONS AND DEFAULTS  ----------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/DietDefaultBuildTypes.cmake )

# ---------------- HANDLE SPECIFICITIES OF THE C COMPILER ------------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/ConfigureCCompiler.cmake )

# ---------------- SET THE LINKER SEARCH PATH (RPATH) ----------------------
SET( CMAKE_INSTALL_RPATH_USE_LINK_PATH ON
     CACHE BOOL "Whether to set an rpath for dynamic libraries." )
SET( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
     CACHE STRING "Rpath set at install stage." FORCE )
SET(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)

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


# ---------------- TAU PROFILING SUPPORT -----------------------------------
# Refer to http://www.cs.uoregon.edu/research/tau/home.php for TAU installation
IF( DIET_TAU_PROFILING )
  # Still testing this part
  # the following include will add the path to the installation of TAU, and
  # will determine the configuration used in TAU.
  # The CXX and C compilers will be replaced with the TAU script: tau_compiler.sh
  OPTION( TAU_KEEP_MOD_SRC "Keep generated instrumented files." OFF )
  INCLUDE( ${CMAKE_MODULE_PATH}/TAU/TAU_Profiling.cmake )
ENDIF( DIET_TAU_PROFILING )



# ---------------- DYNAMIC HIERARCHY MODIFICATION FEATURES ----------------
IF( DIET_USE_DYNAMICS )
  ADD_DEFINITIONS( -DHAVE_DYNAMICS )
ENDIF( DIET_USE_DYNAMICS )


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

  # For some mysterious, strange and yet unknown reason, the omnithread
  # library (from omniORB) requires to be linked against the pthread library
  # when building a static executable. Without further bitching on omniORB
  # and when we find the pthread library, we simply link against it...
  FIND_LIBRARY( LIBRARY_PTHREAD NAMES pthread )
  IF( LIBRARY_PTHREAD )
    SET( OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARIES} ${LIBRARY_PTHREAD} )
  ENDIF( LIBRARY_PTHREAD )
ENDIF( NOT BUILD_SHARED_LIBS )
# --------------------------------------------------------------------------

IF( CYGWIN )
  IF( BUILD_SHARED_LIBS )  
    SET( DIET_INSTALL_LIBS_DIR ${CMAKE_INSTALL_PREFIX}/bin )
  ELSE( BUILD_SHARED_LIBS )
    SET( DIET_INSTALL_LIBS_DIR ${CMAKE_INSTALL_PREFIX}/lib )
  ENDIF( BUILD_SHARED_LIBS )
ELSE( CYGWIN )
  SET( DIET_INSTALL_LIBS_DIR ${CMAKE_INSTALL_PREFIX}/lib )
ENDIF( CYGWIN )


# --------------------------------------------------------------------------
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( include )
ADD_SUBDIRECTORY( Cmake )

# --------------------------------------------------------------------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/DisplaySummary.cmake )

## eof - CMakeLists.txt
