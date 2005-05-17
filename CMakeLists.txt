#****************************************************************************#
#* DIET main cmake entry point                                              *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.3  2005/05/17 11:58:52  eboix
#*     Added a Cmake/README for cmake starters.
#*     First try on compiling the tex documentation.
#*     Cosmetic changes to CMakeLists.txt.  --- Injay 2461
#*
#* Revision 1.2  2005/04/29 16:18:01  eboix
#*     First example of JXTA dependant compilation.  --- Injay 2461
#*
#* Revision 1.1  2005/04/28 12:26:39  eboix
#*     Adding configuration files (trial version) of CMake building mecanism.
#*     This will NOT interfere with current autotools.  --- Injay 2461
#*
#****************************************************************************#


PROJECT(DIET)
# Comment or perish

SET(DIET_version 1.1)

# --------------------- OPTIONS OF THE PROJECT -----------------------------
# Build dynamic lib by default
OPTION( BUILD_SHARED_LIBS "Build DIET with shared libraries." ON )
OPTION( DIET_BUILD_EXAMPLES "Build DIET examples." ON )
OPTION( DIET_BUILD_DOCUMENTATION "Build the users manual documentation." OFF )
OPTION( DIET_USE_JXTA "Build the JXTA mode." OFF )
OPTION( DIET_USE_FAST "Build DIET with the FAST support." OFF )
OPTION( DIET_MAINTAINER_MODE "Build additional documentation..." OFF )

# --------------------- DEPENDENCIES TO EXTERNALS --------------------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/FindOmniORB.cmake )

IF( DIET_BUILD_DOCUMENTATION )
  FIND_PACKAGE( LATEX )
  SUBDIRS( doc )
ENDIF( DIET_BUILD_DOCUMENTATION )

IF( DIET_USE_JXTA )
  INCLUDE( ${DIET_SOURCE_DIR}/Cmake/FindJava.cmake )
  IF( NOT JAVA_FOUND )
     SET( DIET_UST_JXTA OFF )
  ENDIF( NOT JAVA_FOUND )
ENDIF( DIET_USE_JXTA )

IF( DIET_USE_FAST )
  SET( CMAKE_HAVE_FAST 1 )   # for configuration file
ELSE( DIET_USE_FAST )
  SET( CMAKE_HAVE_FAST 0 )   # for configuration file
ENDIF( DIET_USE_FAST )

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
