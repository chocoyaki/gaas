project( diet-doc )
cmake_minimum_required( VERSION 2.6 )

# update module path
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Cmake )

# set documentation install dir
set( DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}"
  CACHE PATH "Directory where doc will be installed" FORCE )

# create doc target if non-existant
add_custom_target( doc ALL )

# cleanup
set_property( DIRECTORY APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES doc )

##############################################################################
# Doxygen is required for building API doc
# no Doxygen, no API doc
#
##############################################################################
find_package( Doxygen )
if( NOT DOXYGEN_FOUND )
  message( WARNING " doxygen not found." )
  message( WARNING " API documentation will not be generated." )
else()
  # tell doxygen where our sources lives
  get_filename_component( SRC_DIR ${PROJECT_SOURCE_DIR} PATH )

  # setup our configuration file
  # since CMake manage list as semi-colons separated strings
  # we have to preprocess them in order to have something
  # understandable by Doxygen
  set( DIET_DOXYGEN_INPUT_IN ${SRC_DIR}/include
    ${SRC_DIR}/src )
  foreach( ENTRY ${DIET_DOXYGEN_INPUT_IN} )
    set( DIET_DOXYGEN_INPUT "${DIET_DOXYGEN_INPUT} ${ENTRY}" )
  endforeach()
  
  set( DIET_DOXYGEN_IMAGE_PATH_IN ${SRC_DIR}/src/examples
    ${SRC_DIR}/doc/ProgrammersGuide
    ${SRC_DIR}/doc/ProgrammersGuide/fig
    ${SRC_DIR}/doc/UsersManual )
  foreach( ENTRY ${DIET_DOXYGEN_IMAGE_PATH_IN} )
    set( DIET_DOXYGEN_IMAGE_PATH "${DIET_DOXYGEN_IMAGE_PATH} ${ENTRY}" )
  endforeach()
  
  set( DIET_DOXYGEN_EXCLUDE ${SRC_DIR}/examples )
  
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
  
  # add doxygen target
  add_custom_target( doc-html ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
  
  # add doc-html as dependency to doc target
  add_dependencies( doc doc-html )

  # install generated documentation
  install( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/API
    DESTINATION ${DOC_INSTALL_DIR} )


  ## generate manpages
  # setup our configuration file
  # since CMake manage list as semi-colons separated strings
  # we have to preprocess them in order to have something
  # understandable by Doxygen
  set( DIET_DOXYGEN_INPUT ${SRC_DIR}/include )
    
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-man.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-man )
  
  # add doxygen target
  add_custom_target( doc-man-doxygen ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-man )
  
  # add doc-html as dependency to doc target
  add_dependencies( doc doc-man-doxygen )

  # TODO install
endif()


#############################################################################
# Guides require latex, transfig and ImageMagick to be installed
#
#############################################################################
find_package( LATEX )
find_package( Transfig )
find_package( ImageMagick )

set( ENABLE_GUIDE ON )
if( NOT PDFLATEX_COMPILER )
  message( WARNING " Latex installation was not found." )
  set( ENABLE_GUIDE OFF )
endif()

if( NOT TRANSFIG_FOUND )
  message( WARNING " Transfig installation was not found. Please provide FIG2DEV." )
  set( ENABLE_GUIDE OFF )
endif()

if( NOT IMAGEMAGICK_CONVERT_EXECUTABLE )
  message( "Convert ImageMagick utility was not found." )
  message( "Please provide IMAGEMAGICK_CONVERT_EXECUTABLE." )
  set( ENABLE_GUIDE OFF )
endif()

if( ENABLE_GUIDE )
  include( Cmake/UseLATEX.cmake )
  add_subdirectory( ProgrammersGuide )
  # UsersManual correctly built but cmake may raise an error
  # due to latex number of warnings
  add_subdirectory( UsersManual )
  add_subdirectory( UsersFAQ )
else()
  message( WARNING " Guides will not be built due to missing requirements." )
endif()


#############################################################################
# man pages generation
# we need rst2man for the generation
#
#############################################################################
find_program( RST2MAN_BIN rst2man )

if( RST2MAN_BIN )
  add_subdirectory( man )
else()
  message( WARNING " Man pages will not be built due to missing requirements." )
  message( WARNING " Please install rst2man (usually included in python-docutils package)" )
endif()
