# We need to know where TAU has been installed
SET( TAU_DIR "" CACHE PATH "Root of TAU install directory." )

# We need a file to instrument the Diet code, by default use the one in Cmake/TAU
SET( TAU_INS_FILE "${CMAKE_MODULE_PATH}/TAU/instrument_rules.tau"
  CACHE FILEPATH "File containing the rules to instrument DIET with TAU." )

# we currently rely on the Makefile created during the TAU 
# installation process.
# This file contains the options for the tau_compiler.sh script
FIND_PATH(
  TAU_MAKEFILE_PATH Makefile
  PATHS
  ${TAU_DIR}/include
)


IF( NOT TAU_MAKEFILE_PATH )
  MESSAGE( SEND_ERROR "You have to provide TAU install directory. (a Makefile has to be present under TAU_DIR/include/)" )
ELSE( NOT TAU_MAKEFILE_PATH )
  MARK_AS_ADVANCED( TAU_MAKEFILE_PATH )
  MARK_AS_ADVANCED( TAU_DIR )
  MARK_AS_ADVANCED( TAU_KEEP_MOD_SRC )

  # Ugly, but for now on I have no other idea so as to retrieve the full
  # command line for both the CXX and C TAU compilers (to automatically
  # instrument the DIET code).
  # We modify the makefiles, each outputs on the standard output the 
  # full command to call the tau_compiler.sh script.
  #
  # we first copy the Makefile and change every ".*" by "\".*\""
  SET( TAU_MAKEFILE ${CMAKE_CURRENT_BINARY_DIR}/TAU_Makefile )
  FILE( READ ${TAU_MAKEFILE_PATH}/Makefile MF_TMP )
  STRING( REGEX REPLACE "\"([^\"]*)\"" "\"\\\\\"\\1\\\\\"\"" MF_MOD_TMP ${MF_TMP} )
  FILE( WRITE ${TAU_MAKEFILE} ${MF_MOD_TMP} )
 
  # CXX
  CONFIGURE_FILE(
    ${CMAKE_MODULE_PATH}/TAU/Makefile_CXX
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile_mod_CXX
    @ONLY
    )
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_BINARY_DIR}/Makefile_mod_CXX
    OUTPUT_VARIABLE TAU_CXX_CMD
    )
  STRING( REGEX REPLACE "[ \t\n]+" " " TAU_CXX_CMD2 ${TAU_CXX_CMD} )
  STRING( REGEX MATCH "^[^ ]+" TAU_CXX_COMPILER ${TAU_CXX_CMD2} )

  STRING( REPLACE "${TAU_CXX_COMPILER}" "" TAU_CXX_FLAGS ${TAU_CXX_CMD2})

  IF( TAU_KEEP_MOD_SRC) 
    SET( CMAKE_CXX_FLAGS "-optKeepFiles ${TAU_CXX_FLAGS} ${CMAKE_CXX_FLAGS}" CACHE STRING "Flags used by the compiler during all build types, with TAU flags." FORCE )
  ELSE( TAU_KEEP_MOD_SRC )
    SET( CMAKE_CXX_FLAGS "${TAU_CXX_FLAGS} ${CMAKE_CXX_FLAGS}" CACHE STRING "Flags used by the compiler during all build types, with TAU flags." FORCE )
  ENDIF( TAU_KEEP_MOD_SRC )

  # C
  CONFIGURE_FILE(
    ${CMAKE_MODULE_PATH}/TAU/Makefile_C
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile_mod_C
    @ONLY
    )
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_BINARY_DIR}/Makefile_mod_C
    OUTPUT_VARIABLE TAU_C_CMD
    )
  STRING( REGEX REPLACE "[ \t\n]+" " " TAU_C_CMD2 ${TAU_C_CMD} )
  STRING( REGEX MATCH "^[^ ]+" TAU_C_COMPILER ${TAU_C_CMD2} )

  STRING( REPLACE "${TAU_C_COMPILER}" "" TAU_C_FLAGS ${TAU_C_CMD2})
  IF( TAU_KEEP_MOD_SRC) 
    SET( CMAKE_C_FLAGS "-optKeepFiles ${TAU_C_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types, with TAU flags." FORCE )
  ELSE( TAU_KEEP_MOD_SRC )
    SET( CMAKE_C_FLAGS "${TAU_C_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types, with TAU flags." FORCE )
  ENDIF( TAU_KEEP_MOD_SRC )


  # we need to exlude some files
  SET( CMAKE_C_FLAGS "-optTauSelectFile=\"${TAU_INS_FILE}\" ${CMAKE_C_FLAGS}"
    CACHE STRING "Flags used by the compiler during all build types, with TAU flags."
    FORCE )
  SET( CMAKE_CXX_FLAGS "-optTauSelectFile=\"${TAU_INS_FILE}\" ${CMAKE_CXX_FLAGS}"
    CACHE STRING "Flags used by the compiler during all build types, with TAU flags."
    FORCE )

  # Verbose mode ?
  IF( CMAKE_VERBOSE_MAKEFILE )
    SET( CMAKE_C_FLAGS "-optVerbose ${CMAKE_C_FLAGS}"
      CACHE STRING "Flags used by the compiler during all build types, with TAU flags."
      FORCE )
    SET( CMAKE_CXX_FLAGS "-optVerbose ${CMAKE_CXX_FLAGS}"
      CACHE STRING "Flags used by the compiler during all build types, with TAU flags."
      FORCE )
  ENDIF( CMAKE_VERBOSE_MAKEFILE )


  CONFIGURE_FILE(
    ${CMAKE_MODULE_PATH}/TAU/TAU_preprocessor.py
    ${CMAKE_CURRENT_BINARY_DIR}/TAU_preprocessor.py
    @ONLY
    )
  SET( CMAKE_CXX_COMPILER "${CMAKE_CURRENT_BINARY_DIR}/TAU_preprocessor.py" CACHE FILEPATH "TAU compiler." FORCE )
  SET( CMAKE_C_COMPILER "${CMAKE_CURRENT_BINARY_DIR}/TAU_preprocessor.py" CACHE FILEPATH "TAU compiler." FORCE )

ENDIF( NOT TAU_MAKEFILE_PATH )

