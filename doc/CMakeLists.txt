#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_DOCUMENTATION )

  #### Distribution related work arounds: note that fixing distribution
  # choices (e.g. not distributing the TeX sources) at the cmake level
  # is an awkward choice (not to say kludgy or even sucky).
  #
  # The programmer's guide (both sources and the compiled manual) is not
  # part of the main distributions:
  IF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide )
    SUBDIRS( ProgrammersGuide )
  ENDIF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide )
  # The TeX sources of the user's guide are not part of the main
  # distribution. BUT the compiled manual (pdf version) is distributed
  # within the UsersManual directory. Thus checking for the existence
  # of the directory is not sufficient:
  IF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual )
    IF( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual/UsersManual.tex )
      SUBDIRS( UsersManual )
    ENDIF( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual/UsersManual.tex )
  ENDIF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual )

  # FIXME: following Tutorial subdir is not yet translated to cmake:
  # SUBDIRS( Tutorial )
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
