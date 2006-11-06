#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_DOCUMENTATION )

  # The programmer's guide is not part of the User's distribution:
  IF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide )
    SUBDIRS( ProgrammersGuide )
  ENDIF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide )

  SUBDIRS( Tutorial )
  SUBDIRS( UsersManual )
  IF( DOXYGEN )
    SET( DIET_DOXYGEN_INPUT
      "${DIET_SOURCE_DIR}/include                                                      ${DIET_SOURCE_DIR}/src" )
    SET( DIET_DOXYGEN_EXCLUDE
      "${DIET_SOURCE_DIR}/src/examples" )
    SET( DIET_DOXYGEN_IMAGE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide/fig                                ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide/                                   ${CMAKE_CURRENT_BINARY_DIR}/UsersManual" )
    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      IMMEDIATE )
    ADD_CUSTOM_TARGET(doc-html ALL
      ${DOXYGEN}
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
  ENDIF( DOXYGEN )

ENDIF( DIET_BUILD_DOCUMENTATION )
