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
 
INCLUDE( ${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake )

SET( BLAS_DEFAULT_LIB_PATH /usr/lib /usr/local/lib )
SET( BLAS_PATHS ${BLAS_DIR} ${BLAS_DEFAULT_LIB_PATH} )
SET( BLAS_FOUND "" )
SET( BLAS_LIBRARIES "" )

############
# When the ARG_LIBRARY exists this macro checks if the ARG_FUNCTION function
# exists with this library.
# RESULT_VAR    - returned result: "YES" when found, "" when unfound.
# ARG_LIBRARY   - searched library argument
# ARG_SYMBOL    - searched function
# ARG_LIB_PATHS - path to the searched library argument
# Note: this can be seen as the autoconf AC_CHECK_LIB() equivalent...somehow.
MACRO( BLAS_CHECK_LIBRARY RESULT_VAR ARG_LIBRARY ARG_FUNCTION ARG_LIB_PATHS )
  FIND_LIBRARY( LIBRARY_FOUND ${ARG_LIBRARY} PATHS ${ARG_LIB_PATHS} )
  IF( LIBRARY_FOUND )
    CHECK_LIBRARY_EXISTS( "${LIBRARY_FOUND}" "${ARG_FUNCTION}" "" RESULT)
    IF( RESULT )
      SET( ${RESULT_VAR} "YES" CACHE INTERNAL "Blas library ${LIBRARY_FOUND}" )
    ELSE( RESULT )
      SET( ${RESULT_VAR} ""    CACHE INTERNAL "Blas library ${LIBRARY_FOUND}" )
    ENDIF( RESULT )
    SET( ${RESULT} "" )
  ELSE( LIBRARY_FOUND )
      SET( ${RESULT_VAR} ""    CACHE INTERNAL "Blas library ${LIBRARY_FOUND}" )
  ENDIF( LIBRARY_FOUND )
ENDMACRO( BLAS_CHECK_LIBRARY )

### BLAS in ATLAS library ?
IF( NOT BLAS_FOUND )
  BLAS_CHECK_LIBRARY( FOUND_ATLAS   "atlas"   "ATL_xerbla"  ${BLAS_PATHS} )
  BLAS_CHECK_LIBRARY( FOUND_f77BLAS "f77blas" "sgemm"       ${BLAS_PATHS} )
  BLAS_CHECK_LIBRARY( FOUND_CBLAS   "cblas"   "cblas_dgemm" ${BLAS_PATHS} )
  IF( FOUND_ATLAS AND FOUND_f77BLAS AND FOUND_CBLAS )
    SET( BLAS_FOUND "YES" )
    SET( BLAS_LIBRARIES "-lcblas -lf77blas -latlas" )
  ENDIF( FOUND_ATLAS AND FOUND_f77BLAS AND FOUND_CBLAS )
ENDIF( NOT BLAS_FOUND )

### Generic BLAS library
IF( NOT BLAS_FOUND )
  BLAS_CHECK_LIBRARY( FOUND "blas" "dgemm_" ${BLAS_PATHS} )
  SET( BLAS_FOUND "YES" )
  SET( BLAS_LIBRARIES "-lblas" )
ENDIF( NOT BLAS_FOUND )

IF( BLAS_FOUND )
  MARK_AS_ADVANCED( BLAS_DIR )
  MARK_AS_ADVANCED( BLAS_FOUND )
  MARK_AS_ADVANCED( BLAS_LIBRARIES )
ENDIF( BLAS_FOUND )

IF( NOT OMNIORB4_FOUND )
  MESSAGE("omniORB installation was not found. Please provide OMNIORB4_DIR:")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DOMNIORB4_DIR:PATH=/usr/local/omniORB-4.0.5 ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. OMNIORB4_INCLUDE_DIR).")
  MESSAGE("    Providing OMNIORB4_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    omniORB installation...")
  SET( OMNIORB4_DIR "" CACHE PATH "Root of omniORB instal tree." )
ENDIF( NOT OMNIORB4_FOUND )

