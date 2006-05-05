#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

SUBDIRS( UsersManual )
IF( DIET_MAINTAINER_MODE )
  SUBDIRS( ProgrammersGuide Tutorial )
  IF( DOXYGEN )
    SET( DIET_DOXYGEN_INPUT
      "${DIET_SOURCE_DIR}/include ${DIET_SOURCE_DIR}/src                               ${DIET_SOURCE_DIR}/doc/source_tree.txt" )
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

ENDIF( DIET_MAINTAINER_MODE )
