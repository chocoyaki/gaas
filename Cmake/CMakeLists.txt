### Build the configuration file for external projects using DIET and cmake:
CONFIGURE_FILE( DietConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/DietConfig.cmake
  @ONLY )
INSTALL_FILES( /share/ FILES DietConfig.cmake )

### Build the configuration file for external projects using DIET and make:
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.inc.in
  ${CMAKE_BINARY_DIR}/include/Makefile.inc
  IMMEDIATE
)
INSTALL_FILES(
  /include/
  FILES
  ${CMAKE_BINARY_DIR}/include/Makefile.inc
)


# --------------------- COPY THE CONFIGURATION FILE -----------------------
### FIXME: this is to avoid conflicting with the autotools generated version
### of the config file contained in directory ${DIET_SOURCE_DIR}/include
### and which depends on autoconf generated acDIET_config.h.
### When we get rid of the autotools stuff (if such thing happens)
### simply overwrite ${DIET_SOURCE_DIR}/include/DIET_config.h with
### Cmake/DIET_config.h.in
CONFIGURE_FILE(
  DIET_config.h.in
  ${DIET_BINARY_DIR}/include/DIET_config.h
  COPYONLY
  IMMEDIATE )

INSTALL_FILES( /include/ FILES ${DIET_BINARY_DIR}/include/DIET_config.h )

ADD_SUBDIRECTORY( Ctest )

# FIXME: wait until cmake disposes of a more powerful INSTALL_FILES
#        to add the equivalent of the following line (refer to thread
#        http://public.kitware.com/pipermail/cmake/2006-January/007867.html) :
# INSTALL_FILES( ${CMAKE_ROOT}/Modules/ FILES FindDiet.cmake )
