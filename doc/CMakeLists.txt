PROJECT ( DIET_DOC ) 
CMAKE_MINIMUM_REQUIRED( VERSION 2.4 )

# Policy CMP0002 is set to use the old behavior in order
# to remove warnings with UseLATEX.cmake
# (it uses variables in ADD_CUSTOM_TARGET where a constant
# would be a better choice: uniq IDs are required).
#
# Example of the obtained warnings:
#  CMake Warning (dev) at Cmake/UseLATEX.cmake:548 (ADD_CUSTOM_TARGET):
#    Policy CMP0002 is not set: Logical target names must be globally unique.
#    Run "cmake --help-policy CMP0002" for policy details.  Use the cmake_policy
#    command to set the policy and suppress this warning.
#  Call Stack (most recent call first):
#    Cmake/UseLATEX.cmake:647 (ADD_LATEX_TARGETS)
#    UsersManual/CMakeLists.txt:15 (ADD_LATEX_DOCUMENT)
#  This warning is for project developers.  Use -Wno-dev to suppress it.
CMAKE_POLICY( SET CMP0002 OLD )

OPTION( BUILD_DOCUMENTATION "Build the documentation." ON )
SET( CMAKE_MODULE_PATH ${DIET_DOC_SOURCE_DIR}/Cmake )
  FIND_PACKAGE( LATEX )

  FIND_PACKAGE( Transfig )

  IF( NOT TRANSFIG_FOUND )
    MESSAGE( "Transfig installation was not found. Please provide FIG2DEV." )
    SET( BUILD_DOCUMENTATION OFF )
  ENDIF( NOT TRANSFIG_FOUND )

  FIND_PACKAGE( ImageMagick )
  IF( NOT IMAGEMAGICK_CONVERT_EXECUTABLE )
    MESSAGE( "Convert ImageMagick utility was not found." )
    MESSAGE( "Please provide IMAGEMAGICK_CONVERT_EXECUTABLE." )
    SET( BUILD_DOCUMENTATION OFF )
  ENDIF( NOT IMAGEMAGICK_CONVERT_EXECUTABLE )

  FIND_PACKAGE( Doxygen )
  IF( NOT DOXYGEN_EXECUTABLE OR NOT DOXYGEN_DOT_EXECUTABLE )
    MESSAGE( "Doxygen or Dot (from graphviz package) were not found." )
    SET( BUILD_DOCUMENTATION OFF )
  ENDIF( NOT DOXYGEN_EXECUTABLE OR NOT DOXYGEN_DOT_EXECUTABLE )

IF( BUILD_DOCUMENTATION )
  IF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide )
    SET( PROGRAMMERSGUIDE "ProgrammersGuide.pdf")
    SUBDIRS( ProgrammersGuide )
  ENDIF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide )

  IF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual )
    SET( USERSMANUAL "UsersManual.pdf")
      SUBDIRS( UsersManual )
  ENDIF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual )

INCLUDE( ${DIET_DOC_SOURCE_DIR}/Cmake/DisplayDocSummary.cmake )

## FIXME find a way to manage Doxygen
  SET( DOXYGEN OFF )
  # FIXME: following Tutorial subdir is not yet translated to cmake:
  # SUBDIRS( Tutorial )
  IF( DOXYGEN )
    SET( DIET_DOXYGEN_INPUT
      "${DIET_SOURCE_DIR}/include                                                      ${DIET_SOURCE_DIR}/src" )
    SET( DIET_DOXYGEN_EXCLUDE
      "${DIET_SOURCE_DIR}/src/examples" )
    SET( DIET_DOXYGEN_IMAGE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide/fig                                ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide/                                   ${CMAKE_CURRENT_BINARY_DIR}/UsersManual" )
    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      IMMEDIATE )
    ADD_CUSTOM_TARGET(doc-html ALL
      ${DOXYGEN}
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
  ENDIF( DOXYGEN )
ENDIF( BUILD_DOCUMENTATION )
