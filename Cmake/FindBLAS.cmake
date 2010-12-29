#
# Find the BLAS libraries and include dir
# (BLAS = Basic Linear Algebric Subroutines)
#
# The following variables are set:
# BLAS_LIBRARIES    - Files to link against to use BLAS
# BLAS_FOUND        - When false, don't try to use BLAS
# BLAS_DIR          - (optional) Suggested installation directory to search
#                     BLAS_DIR can be used to make it simpler to find the
#                     libraries when BLAS was not installed in the
#                     usual/well-known directories (e.g. because you made an
#                     in tree-source compilation or because you installed it in
#                     an "unusual" directory like "/home/user/your_name/lib").
#                     Just set BLAS_DIR to point your specific installation.
#
# Notes: this is a novice trial to port the "acx_blas m4 macro" to cmake. For
#        the original version (that is the m4 version) refer to e.g.
#        http://ac-archive.sourceforge.net/Installed_Packages/acx_blas.htm
#        This is a partial port (PhiPACK, CXML...are not coded), but this
#        should be easy to fix...
# Author: well, I'm doing my best to have a decent social life. ;-)
 
INCLUDE( ${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake )

SET( BLAS_DEFAULT_LIB_PATH /usr/lib /usr/local/lib /usr/lib64)
SET( BLAS_PATHS ${BLAS_DIR} ${BLAS_DEFAULT_LIB_PATH} )
SET( BLAS_FOUND "" )

############
# When the ARG_LIBRARY exists this macro checks if the ARG_FUNCTION function
# exists with this library.
# ARG_LIB       - searched library argument
# ARG_FUNCTION  - searched function
# ARG_LIB_PATHS - path[s] to the searched library argument
# VARIABLE_sh   - returned result: fully qualified shared library name (i.e. absolute
#                 path and library name) when found, "" when unfound.
# VARIABLE_st   - returned result: fully qualified static library name (i.e. absolute
#                 path and library name) when found, "" when unfound.
# Note: this can be seen as the autoconf AC_CHECK_LIB() equivalent...somehow.
MACRO(BLAS_CHECK_LIBRARY ARG_LIB ARG_FUNCTION ARG_LIB_PATHS VARIABLE_sh VARIABLE_st )
  # shared
  STRING(REGEX MATCH "^${VARIABLE_sh}$" LEADSPACE "${VARIABLE_sh}")
  IF(LEADSPACE)
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBlas.cmake: BLAS_CHECK_LIBRARY looking for shared library ${CMAKE_SHARED_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} " )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "with path ${ARG_LIB_PATHS}: " )
    FIND_LIBRARY( ${VARIABLE_sh}
      NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}
      # NAMES ${ARG_LIB}
      PATHS ${ARG_LIB_PATHS}
      NO_DEFAULT_PATH
      )
    FIND_LIBRARY( ${VARIABLE_sh}
      NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}
      )

    # Because FIND_LIBRARY has the side effect of marking it's first argument
    # as un-advanced:
    MARK_AS_ADVANCED( ${VARIABLE_sh} )
    IF( ${VARIABLE_sh} )
      FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
            "found ${${VARIABLE_sh}}.\n" )
      CHECK_LIBRARY_EXISTS( ${${VARIABLE_sh}} "${ARG_FUNCTION}" "" RESULT)
      IF( RESULT )
        SET( ${VARIABLE_sh} ${${VARIABLE_sh}} CACHE INTERNAL "boob" FORCE )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "FindBlas.cmake: BLAS_CHECK_LIBRARY ${ARG_FUNCTION} function " )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "found within ${${VARIABLE_sh}}.\n" )
      ELSE( RESULT )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "FindBlas.cmake: BLAS_CHECK_LIBRARY ${ARG_FUNCTION} function " )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "NOT found within ${${VARIABLE_sh}}.\n" )
      ENDIF( RESULT )
    ELSE( ${VARIABLE_sh} )
      FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log "not found.\n" )
    ENDIF( ${VARIABLE_sh} )
  ELSE(LEADSPACE)
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBlas.cmake: passed variable was prepended with a space?\n" )
  ENDIF(LEADSPACE)

  # static
  STRING(REGEX MATCH "^${VARIABLE_st}$" LEADSPACE "${VARIABLE_st}")
  IF(LEADSPACE)
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBlas.cmake: BLAS_CHECK_LIBRARY looking for static library ${CMAKE_STATIC_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX} " )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "with path ${ARG_LIB_PATHS}: " )
    FIND_LIBRARY( ${VARIABLE_st}
      NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}
      PATHS ${ARG_LIB_PATHS}
      NO_DEFAULT_PATH
      )
    FIND_LIBRARY( ${VARIABLE_st}
      NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}${ARG_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}
      )

    # Because FIND_LIBRARY has the side effect of marking it's first argument
    # as un-advanced:
    MARK_AS_ADVANCED( ${VARIABLE_st} )
    IF( ${VARIABLE_st} )
      FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
            "found ${${VARIABLE_st}}.\n" )
      CHECK_LIBRARY_EXISTS( ${${VARIABLE_st}} "${ARG_FUNCTION}" "" RESULT)
      IF( RESULT )
        SET( ${VARIABLE_st} ${${VARIABLE_st}} CACHE INTERNAL "boob" FORCE )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "FindBlas.cmake: BLAS_CHECK_LIBRARY ${ARG_FUNCTION} function " )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "found within ${${VARIABLE_st}}.\n" )
      ELSE( RESULT )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "FindBlas.cmake: BLAS_CHECK_LIBRARY ${ARG_FUNCTION} function " )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "NOT found within ${${VARIABLE_st}}.\n" )
      ENDIF( RESULT )
    ELSE( ${VARIABLE_st} )
      FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log "not found.\n" )
    ENDIF( ${VARIABLE_st} )
  ELSE(LEADSPACE)
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBlas.cmake: passed variable was prepended with a space?\n" )
  ENDIF(LEADSPACE)
ENDMACRO(BLAS_CHECK_LIBRARY)

### BLAS in ATLAS library ?
IF( NOT BLAS_FOUND )
  FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
        "FindBLAS.cmake: looking for Atlas version: \n" )
  BLAS_CHECK_LIBRARY( "atlas"   "ATL_xerbla"  "${BLAS_PATHS}" FOUND_ATLAS_sh FOUND_ATLAS_st )
  BLAS_CHECK_LIBRARY( "f77blas" "sgemm"       "${BLAS_PATHS}" FOUND_f77BLAS_sh FOUND_f77BLAS_st  )
  BLAS_CHECK_LIBRARY( "cblas"   "cblas_dgemm" "${BLAS_PATHS}" FOUND_CBLAS_sh FOUND_CBLAS_st )
  IF(APPLE)
     BLAS_CHECK_LIBRARY( "g2c"   "_do_fio" "${BLAS_PATHS}" FOUND_G2C_sh FOUND_G2C_st )
  ENDIF(APPLE)

  IF( FOUND_ATLAS_sh AND FOUND_f77BLAS_sh AND FOUND_CBLAS_sh )	
    SET( BLAS_FOUND "YES" )
    SET( BLAS_LIBRARIES_sh ${FOUND_CBLAS_sh} ${FOUND_G2C_sh} ${FOUND_f77BLAS_sh} ${FOUND_ATLAS_sh} CACHE FILEPATH "BLAS shared libraries" )
    SET( BLAS_LIBRARIES_st ${FOUND_CBLAS_st} ${FOUND_G2C_st} ${FOUND_f77BLAS_st} ${FOUND_ATLAS_st} CACHE FILEPATH "BLAS static libraries" )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log 
        "FindBLAS.cmake: Atlas version found.\n" )
  ELSE( FOUND_ATLAS_sh AND FOUND_f77BLAS_sh AND FOUND_CBLAS_sh )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log 
      "FindBLAS.cmake: Atlas version NOT found.\n" )
  ENDIF( FOUND_ATLAS_sh AND FOUND_f77BLAS_sh AND FOUND_CBLAS_sh )
ENDIF( NOT BLAS_FOUND )

IF (APPLE AND BLAS_FOUND_sh AND NOT FOUND_G2C_sh)
  SET( BLAS_FOUND "NOT" )
ENDIF(APPLE AND BLAS_FOUND_sh AND NOT FOUND_G2C_sh)

### Generic BLAS library
IF( NOT BLAS_FOUND )
  FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
        "FindBLAS.cmake: looking for BLAS generic version: \n" )
  BLAS_CHECK_LIBRARY( "blas" "dgemm_" "${BLAS_PATHS}" FOUND_GENERIC_BLAS_sh FOUND_GENERIC_BLAS_st )
  IF( FOUND_GENERIC_BLAS_sh )
    SET( BLAS_FOUND "YES" )
    SET( BLAS_LIBRARIES_sh ${FOUND_GENERIC_BLAS_sh} CACHE FILEPATH "BLAS shared libraries" )
    SET( BLAS_LIBRARIES_st ${FOUND_GENERIC_BLAS_st} CACHE FILEPATH "BLAS static libraries" )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBLAS.cmake: BLAS generic version found.\n" )
  ELSE( FOUND_GENERIC_BLAS_sh )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBLAS.cmake: BLAS generic version not found.\n" )
  ENDIF( FOUND_GENERIC_BLAS_sh )
ENDIF( NOT BLAS_FOUND )

### Eventually present the result of the search:
IF( NOT BLAS_FOUND )
  MESSAGE(SEND_ERROR "BLAS library was not found. Please provide BLAS_DIR: \n    - through the GUI when working with ccmake,\n    - as a command line argument when working with cmake e.g. \n      cmake .. -DBLAS_DIR:PATH=/usr/lib ")
  SET( BLAS_DIR "" CACHE PATH "Directory containing the BLAS library(ies)." )
ENDIF( NOT BLAS_FOUND )
