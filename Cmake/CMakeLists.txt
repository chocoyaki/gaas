### Build the configuration file for external projects using DIET and cmake:
configure_file( DietConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/DietConfig.cmake
  @ONLY )
# install in CMake module path
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/DietConfig.cmake DESTINATION ${CMAKE_MOD_INSTALL_DIR} )

### Build the configuration file for external projects using DIET and make:
get_directory_property( COMPILE_FLAGS DEFINITIONS )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.inc.in
  ${CMAKE_BINARY_DIR}/include/Makefile.inc
  IMMEDIATE )
install( FILES ${CMAKE_BINARY_DIR}/include/Makefile.inc DESTINATION ${INCLUDE_INSTALL_DIR} )

# ADD_SUBDIRECTORY( Ctest )

# FIXME: wait until cmake disposes of a more powerful INSTALL_FILES
#        to add the equivalent of the following line (refer to thread
#        http://public.kitware.com/pipermail/cmake/2006-January/007867.html) :
# INSTALL_FILES( ${CMAKE_ROOT}/Modules/ FILES FindDiet.cmake )
