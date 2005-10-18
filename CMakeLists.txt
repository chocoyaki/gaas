#****************************************************************************#
#* DIET main cmake entry point                                              *#
#****************************************************************************#

PROJECT(DIET)
# Comment or perish

SET(DIET_version 2.0)

# --------------------- OPTIONS OF THE PROJECT -----------------------------
# Build dynamic lib by default
OPTION( BUILD_SHARED_LIBS "Build DIET with shared libraries." ON )
OPTION( DIET_BUILD_EXAMPLES "Build DIET examples." ON )
OPTION( DIET_BUILD_DOCUMENTATION "Build the users manual documentation." OFF )
OPTION( DIET_USE_JXTA "Build DIET with the JXTA architecture support." OFF )
OPTION( DIET_USE_FAST "Build DIET with the FAST support." OFF )
OPTION( DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF )
OPTION( DIET_MAINTAINER_MODE "Build additional documentation..." OFF )

# --------------------- DEPENDENCIES TOWARDS "CLASSIC" C++ HEADERS ---------
# This is just a convenience to advance the detection of any potential missing
# C++ "classic" header to the cmake stage. If we were to forget the following
# tests (and such a header was missing), things would simply break at
# preprocessing stage of the compilation...
# [Those tests are a translation of the AC_CHECK_HEADER(...) that were expressed
# in the configure.ac of the autotools version].
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)

# CLEANME CLEANME
# The file alloca.h is only included in src/agent/qsort.cc with the following
# code (which is either useless or compilation failure prone):
#    #if HAVE_ALLOCA_H
#    #include <alloca.h>
#    #endif // HAVE_ALLOCA_H
CHECK_INCLUDE_FILE_CXX("alloca.h" CMAKE_FOUND_ALLOCA_HEADER)
IF (NOT CMAKE_FOUND_ALLOCA_HEADER)
  MESSAGE(FATAL_ERROR "Missing alloca.h header file.")
ENDIF(NOT CMAKE_FOUND_ALLOCA_HEADER)

CHECK_INCLUDE_FILE_CXX("assert.h" CMAKE_FOUND_ASSERT_HEADER)
IF (NOT CMAKE_FOUND_ASSERT_HEADER)
  MESSAGE(FATAL_ERROR "Missing assert.h header file.")
ENDIF(NOT CMAKE_FOUND_ASSERT_HEADER)

CHECK_INCLUDE_FILE_CXX("limits.h" CMAKE_FOUND_LIMITS_HEADER)
IF (NOT CMAKE_FOUND_LIMITS_HEADER)
  MESSAGE(FATAL_ERROR "Missing limits.h header file.")
ENDIF(NOT CMAKE_FOUND_LIMITS_HEADER)

CHECK_INCLUDE_FILE_CXX("math.h" CMAKE_FOUND_MATH_HEADER)
IF (NOT CMAKE_FOUND_MATH_HEADER)
  MESSAGE(FATAL_ERROR "Missing math.h header file.")
ENDIF(NOT CMAKE_FOUND_MATH_HEADER)

CHECK_INCLUDE_FILE_CXX("stdlib.h" CMAKE_FOUND_STDLIB_HEADER)
IF (NOT CMAKE_FOUND_STDLIB_HEADER)
  MESSAGE(FATAL_ERROR "Missing stdlib.h header file.")
ENDIF(NOT CMAKE_FOUND_STDLIB_HEADER)

# --------------------- DEPENDENCIES TO EXTERNAL PACKAGES -------------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/FindOmniORB.cmake  )
# Path to additional modules (i.e. used by FIND_PACKAGE commands making
# reference to non CMake defined "standard" modules):
SET( CMAKE_MODULE_PATH ${DIET_SOURCE_DIR}/Cmake )

IF( DIET_BUILD_DOCUMENTATION )
  # DIET_MAINTAINER_MODE is used within the doc subdirectory
  FIND_PACKAGE( LATEX )
  FIND_PACKAGE( Transfig )
  FIND_PACKAGE( ImageMagick )
  SUBDIRS( doc )
ENDIF( DIET_BUILD_DOCUMENTATION )

IF( DIET_USE_JXTA )
  FIND_PACKAGE( JavaExtended )
  IF( JAVA_FOUND )
    ADD_DEFINITIONS(-DHAVE_JXTA)
  ELSE( JAVA_FOUND )
    SET( DIET_USE_JXTA OFF )
  ENDIF( JAVA_FOUND )
ENDIF( DIET_USE_JXTA )

IF( DIET_USE_FAST )
  SET( CMAKE_HAVE_FAST 1 )   # for configuration file
ELSE( DIET_USE_FAST )
  SET( CMAKE_HAVE_FAST 0 )   # for configuration file
ENDIF( DIET_USE_FAST )

IF( DIET_WITH_STATISTICS )
  # Enable generation of the statistics log
  ADD_DEFINITIONS(-DHAVE_STATISTICS)
ENDIF( DIET_WITH_STATISTICS )

# --------------------- BUILD THE CONFIGURATION FILE -----------------------
### FIXME: consider using ADD_DEFINITIONS cmake feature instead of a configure
###        file !!!!
CONFIGURE_FILE( ${DIET_SOURCE_DIR}/Cmake/DIET_config.h.in
                ${DIET_BINARY_DIR}/include/DIET_config.h )

SUBDIRS(src)

# --------------------------------------------------------------------------
# QUESTIONS:
# - what is the general purpose of diet/include dir ? (sub lib or includes?)
#        (see src/agent/CMakeLists.txt INCLUDE_DIRECTORIES)
## eof - CMakeLists.txt
