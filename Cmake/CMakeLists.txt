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

# install FindDiet.cmake in CMake module path
install( FILES FindDiet.cmake DESTINATION ${CMAKE_MOD_INSTALL_DIR} )
