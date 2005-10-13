#****************************************************************************#
#* DIET main cmake entry point                                              *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#****************************************************************************#


PROJECT(DIET)
# Comment or perish

SET(DIET_version 1.1)

# --------------------- OPTIONS OF THE PROJECT -----------------------------
# Build dynamic lib by default
OPTION( BUILD_SHARED_LIBS "Build DIET with shared libraries." ON )
OPTION( DIET_BUILD_EXAMPLES "Build DIET examples." ON )
OPTION( DIET_BUILD_DOCUMENTATION "Build the users manual documentation." OFF )
OPTION( DIET_USE_JXTA "Build DIET with the JXTA architecture support." OFF )
OPTION( DIET_USE_FAST "Build DIET with the FAST support." OFF )
OPTION( DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF )
OPTION( DIET_MAINTAINER_MODE "Build additional documentation..." OFF )

# --------------------- DEPENDENCIES TO EXTERNALS --------------------------
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
# - what is the general purpose of src/CORBA/idl dir ?
#   ditto
## eof - CMakeLists.txt
