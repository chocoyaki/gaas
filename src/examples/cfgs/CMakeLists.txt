#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# Build (by configuration) all the configurations files of the examples by
# basically substituting with the proper "hostname".

SET( DIET_EXAMPLES_CONFIG_BUILD ON BOOL "Whether the configuration files were built" )

# FIXME: already defined in Cmake/FindTestingUtils.cmake 
EXECUTE_PROCESS(COMMAND hostname
  OUTPUT_VARIABLE DIET_CMAKE_HOSTNAME
)
FILE( GLOB DIET_CONFIG_FILES_GLOB 
  "${DIET_SOURCE_DIR}/src/examples/cfgs/*.cfg.in"
  "${DIET_SOURCE_DIR}/src/examples/cfgs/*.ldif.in"
  "${DIET_SOURCE_DIR}/src/examples/cfgs/ldap_schema/*.schema.in"
)
FOREACH( filename ${DIET_CONFIG_FILES_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoPath ${filename} NAME )
  STRING( REGEX REPLACE "[.]in$" "" filenameNoPath ${filenameNoPath} )
  SET( filenameDest ${DIET_BINARY_DIR}/etc/${filenameNoPath} )
  CONFIGURE_FILE(
    ${filename}
    ${filenameDest}
    IMMEDIATE
  )
  INSTALL( FILES ${filenameDest} DESTINATION etc )
ENDFOREACH( filename )

## FIXME: find out why we cannot test availability of the hostname command with
##        IF( COMMAND "hostname" ) or IF( COMMAND hostname )
## SET( DIET_EXAMPLES_CONFIG_BUILD OFF BOOL "Wether the configuration files were build" )
## MESSAGE( FATAL_ERROR "hostname command not available" )
