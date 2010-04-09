#
# This module checks wether FAST (Fast Agent System Timers, refer to 
# http:///www.loria.fr/~quinson/fast.html ) tools are installed. When they are it
# determines the location of the FAST libraries and include files.
# This code sets the following variables:
#   - FAST_FOUND set to either "YES" or "NO"
#   - FAST_LIBRARIES full path to the FAST libraries (and depending libraries)
#     additional linker flags in a un*x fashion (i.e. basically the result of
#     the command "fast-config --libs fast").
#   - FAST_INCLUDE_DIR full path to the FAST include directories (and depending
#     package directories) preprocessing flags in a un*x fashion (i.e.
#     basically the result of the command "fast-config --cflags fast"
#   - FAST_VERSION essentially the result of the command 
#     "fast-config --version fast"

FIND_PROGRAM( FAST_CONFIG_EXECUTABLE
  NAMES
  fast-config
  PATH
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
  DOC "Path to program fast-config (FAST tools)"
)

IF( FAST_CONFIG_EXECUTABLE )
  EXECUTE_PROCESS(COMMAND ${FAST_CONFIG_EXECUTABLE}
    "--libs fast"
    OUTPUT_VARIABLE TEMP_OUTPUT )
  SET( FAST_LIBRARIES ${TEMP_OUTPUT}
    CACHE STRING "FAST libraries link directives" )
  EXECUTE_PROCESS(COMMAND ${FAST_CONFIG_EXECUTABLE}
    "--cflags fast"
    OUTPUT_VARIABLE TEMP_OUTPUT )
  SET( FAST_INCLUDE_DIR ${TEMP_OUTPUT}
    CACHE STRING "FAST preprocessor include directives" )
  # fast.m4 elaborates a lot more on the version number of FAST as obtained
  # through "fast-config --vesion fast": it extracts major and minor version
  # number but alas the parsing requires some shell functionality... For the
  # time being, we will skip this part:
  EXECUTE_PROCESS(COMMAND ${FAST_CONFIG_EXECUTABLE}
    "--version fast"
    OUTPUT_VARIABLE TEMP_OUTPUT )
  SET( FAST_VERSION ${TEMP_OUTPUT}
    CACHE STRING "FAST version number" )
  IF( FAST_LIBRARIES AND FAST_INCLUDE_DIR )
    MARK_AS_ADVANCED(
      FAST_CONFIG_EXECUTABLE
      FAST_LIBRARIES
      FAST_INCLUDE_DIR
      FAST_VERSION )
    SET( FAST_FOUND "YES" )
  ELSE( FAST_LIBRARIES AND FAST_INCLUDE_DIR )
    SET( FAST_FOUND "NO" )
  ENDIF( FAST_LIBRARIES AND FAST_INCLUDE_DIR )
ELSE( FAST_CONFIG_EXECUTABLE )
  SET( FAST_FOUND "NO" )
ENDIF( FAST_CONFIG_EXECUTABLE )

