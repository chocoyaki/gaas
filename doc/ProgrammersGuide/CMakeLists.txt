#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#

ADD_SUBDIRECTORY( fig )

# In order to avoid the painfull process of passing the TEXINPUT environment
# variable copy all the required files to CMAKE_CURRENT_BINARY_DIR:
FILE(GLOB DIET_PROGRAMMERS_MANUAL_TEX_GLOB "*.tex")
FOREACH( filename ${DIET_PROGRAMMERS_MANUAL_TEX_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoPath ${filename} NAME )
  SET( filenameDest "${CMAKE_CURRENT_BINARY_DIR}/${filenameNoPath}" )
  CONFIGURE_FILE(
    ${filename}
    ${filenameDest}
    COPYONLY
    IMMEDIATE
  )
ENDFOREACH( filename )

### Then we need to invoke:
# 1/ latex (first pass)
# 3/ latex (second pass)
# In order to achieve this pipeline, we pervert the latex generated
# files (.aux, .log, .dvi, .bbl...) to create an artificial set of
# dependencies. The side-effect is hopefully the required behavior...
ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.aux
  DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/ProgrammersGuide.tex
  COMMAND   ${LATEX_COMPILER}  
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide
  COMMENT   "Latex (first pass)"
)

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.dvi
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.aux
            ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.tex
  COMMAND   ${LATEX_COMPILER}  
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide
  COMMENT   "Latex (second pass)"
)

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.dvi
  DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/ProgrammersGuide
)

ADD_CUSTOM_TARGET(
  ProgrammersGuideDocument
  ALL echo
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.dvi
)

# Whenever if possible also generate the pdf document:
IF( DVIPDF_CONVERTER )
  ADD_CUSTOM_COMMAND(
    OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.pdf
    DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.log
    COMMAND  ${DVIPDF_CONVERTER}
    ARGS     -o ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.pdf
             ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.dvi
    COMMENT  "Generating pdf version of ProgrammersGuide."
  )
  ADD_CUSTOM_TARGET(
    ProgrammersGuidePdfDocument
    ALL echo
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.pdf
  )
  INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.pdf
    DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/ProgrammersGuide
  )
ENDIF( DVIPDF_CONVERTER )

### But FIRST, convert the required figures (various formats) to eps:
ADD_DEPENDENCIES(
  ProgrammersGuideDocument
  DIET_TARGET_PROGRAMMERSGUIDE_FIGURES
  )
