#
# Check the C compiler for unstable features
#
# VARIABLE - variable to store the result
#

#           vim /usr/share/autoconf/autoconf/c.m4

# AC_C_INLINE
# -----------
# Do nothing if the compiler accepts the inline keyword.
# Otherwise define inline to __inline__ or __inline if one of those work,
# otherwise define inline to be empty.
#
# HP C version B.11.11.04 doesn't allow a typedef as the return value for an
# inline function, only builtin types.
#
MACRO( CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE TO_TEST RESULT )
  TRY_COMPILE( ${RESULT}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Cmake/CheckCCompilerInline.c
    COMPILE_DEFINITIONS "-DPOSSIBLE_INLINE_KEYWORD=${TO_TEST}"
    OUTPUT_VARIABLE DUMMY_OUTPUT_FOR_DEBUG )
ENDMACRO( CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE )

# CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "__inline__" TEST )
# MESSAGE( SEND_ERROR "OUTSIDE: ${TEST}")
# CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "__inline" TEST )
# MESSAGE( SEND_ERROR "OUTSIDE: ${TEST}")
# CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "inline" TEST )
# MESSAGE( SEND_ERROR "OUTSIDE: ${TEST}")
# CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "zob" TEST )
# MESSAGE( SEND_ERROR "OUTSIDE: ${TEST}")


MACRO( CHECK_C_COMPILER_SUPPORTS_INLINE INLINE_VALUE INLINE_SUPPORTED )
  MESSAGE( SEND_ERROR "Checking C compiler for inlining support.")
  SET( SUPPORTED FALSE )

  MESSAGE( SEND_ERROR "Checking C compiler for inline.")
  CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "zinline" TEST_INLINE )
  IF( TEST_INLINE )
    SET( SUPPORTED TRUE )
    SET( ${INLINE_VALUE} "inline" )
  ENDIF( TEST_INLINE )

  IF( NOT SUPPORTED )
    MESSAGE( SEND_ERROR "Checking C compiler for __inline__.")
    CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "__inline__" TEST_INLINE )
    IF( TEST_INLINE )
      SET( SUPPORTED TRUE )
      SET( ${INLINE_VALUE} "__inline__" )
    ENDIF( TEST_INLINE )
  ENDIF( NOT SUPPORTED )

  IF( NOT SUPPORTED )
    MESSAGE( SEND_ERROR "Checking C compiler for __inline.")
    CHECK_C_COMPILER_SUPPORTS_INLINE_COMPILE( "__inline" TEST_INLINE )
    IF( TEST_INLINE )
      SET( SUPPORTED TRUE )
      SET( ${INLINE_VALUE} "__inline" )
    ENDIF( TEST_INLINE )
  ENDIF( NOT SUPPORTED )

  IF( SUPPORTED )
    SET( ${INLINE_SUPPORTED} TRUE CACHE INTERNAL "C compiler inlining support")
    FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "The C compiler was found to support inlining with keyword: "
      "${INLINE_VALUE}\n\n")
  ELSE( SUPPORTED )
    SET( ${INLINE_SUPPORTED} "" CACHE INTERNAL "C compiler inlining support")
    FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "The C compiler was found to support inlining with keyword: "
      "${INLINE_VALUE}\n\n")
  ENDIF( SUPPORTED )
ENDMACRO( CHECK_C_COMPILER_SUPPORTS_INLINE )

CHECK_C_COMPILER_SUPPORTS_INLINE( VALUE ZOB )
MESSAGE( SEND_ERROR "WAZAAAAAAAAAAAAAAA ${VALUE} ${ZOB}")
