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

SET( BLAS_DEFAULT_LIB_PATH /usr/lib /usr/local/lib )
SET( BLAS_PATHS ${BLAS_DIR} ${BLAS_DEFAULT_LIB_PATH} )
SET( BLAS_FOUND "" )
SET( BLAS_LIBRARIES "" )

############
# When the ARG_LIBRARY exists this macro checks if the ARG_FUNCTION function
# exists with this library.
# ARG_LIB       - searched library argument
# ARG_FUNCTION  - searched function
# ARG_LIB_PATHS - path[s] to the searched library argument
# VARIABLE      - returned result: fully qualified library name (i.e. absolute
#                 path and library name) when found, "" when unfound.
# Note: this can be seen as the autoconf AC_CHECK_LIB() equivalent...somehow.
MACRO(BLAS_CHECK_LIBRARY ARG_LIB ARG_FUNCTION ARG_LIB_PATHS VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBlas.cmake: BLAS_CHECK_LIBRARY looking for library ${ARG_LIB} " )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "with path ${ARG_LIB_PATHS}: " )
    FIND_LIBRARY( ${VARIABLE} ${ARG_LIB} PATHS ${ARG_LIB_PATHS} )
    IF( ${VARIABLE} )
      CHECK_LIBRARY_EXISTS( ${${VARIABLE}} "${ARG_FUNCTION}" "" RESULT)
      FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
            "found ${${VARIABLE}}.\n" )
      IF( RESULT )
        SET( ${VARIABLE} ${${VARIABLE}} CACHE INTERNAL "boob" FORCE )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "FindBlas.cmake: BLAS_CHECK_LIBRARY ${ARG_FUNCTION} function " )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "found within ${${VARIABLE}}.\n" )
      ELSE( RESULT )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "FindBlas.cmake: BLAS_CHECK_LIBRARY ${ARG_FUNCTION} function " )
        FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
              "NOT found within ${${VARIABLE}}.\n" )
      ENDIF( RESULT )
    ELSE( ${VARIABLE} )
      FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log "not found.\n" )
    ENDIF( ${VARIABLE} )
  ELSE("${VARIABLE}" MATCHES "^${VARIABLE}$")
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBlas.cmake: passed variable was prepended with a space?\n" )
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(BLAS_CHECK_LIBRARY)

### BLAS in ATLAS library ?
IF( NOT BLAS_FOUND )
  FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
        "FindBLAS.cmake: looking for Atlas version: \n" )
  BLAS_CHECK_LIBRARY( "atlas"   "ATL_xerbla"  "${BLAS_PATHS}" FOUND_ATLAS )
  BLAS_CHECK_LIBRARY( "f77blas" "sgemm"       "${BLAS_PATHS}" FOUND_f77BLAS )
  BLAS_CHECK_LIBRARY( "cblas"   "cblas_dgemm" "${BLAS_PATHS}" FOUND_CBLAS )
  IF( FOUND_ATLAS AND FOUND_f77BLAS AND FOUND_CBLAS )
    SET( BLAS_FOUND "YES" )
    SET( BLAS_LIBRARIES "${FOUND_CBLAS} ${FOUND_f77BLAS} ${FOUND_ATLAS}" )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log 
        "FindBLAS.cmake: Atlas version found.\n" )
  ELSE( FOUND_ATLAS AND FOUND_f77BLAS AND FOUND_CBLAS )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log 
        "FindBLAS.cmake: Atlas version NOT found.\n" )
  ENDIF( FOUND_ATLAS AND FOUND_f77BLAS AND FOUND_CBLAS )
ENDIF( NOT BLAS_FOUND )

### Generic BLAS library
IF( NOT BLAS_FOUND )
  FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
        "FindBLAS.cmake: looking for BLAS generic version: \n" )
  BLAS_CHECK_LIBRARY( "blas" "dgemm_" "${BLAS_PATHS}" FOUND_GENERIC_BLAS )
  IF( FOUND_GENERIC_BLAS )
    SET( BLAS_FOUND "YES" )
    SET( BLAS_LIBRARIES ${FOUND_GENERIC_BLAS} )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBLAS.cmake: BLAS generic version found.\n" )
  ELSE( FOUND_GENERIC_BLAS )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
          "FindBLAS.cmake: BLAS generic version not found.\n" )
  ENDIF( FOUND_GENERIC_BLAS )
ENDIF( NOT BLAS_FOUND )

### Eventually present the result of the search:
IF( BLAS_FOUND )
  MARK_AS_ADVANCED( BLAS_DIR )
  MARK_AS_ADVANCED( BLAS_FOUND )
  MARK_AS_ADVANCED( BLAS_LIBRARIES )
ELSE( BLAS_FOUND )
  MESSAGE("BLAS library was not found. Please provide BLAS_DIR:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DBLAS_DIR:PATH=/usr/local/omniORB-4.0.5 ")
  SET( BLAS_DIR "" CACHE PATH "Directory containing the BLAS library(ies)." )
ENDIF( BLAS_FOUND )
