#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

ADD_SUBDIRECTORY( fig )

# Treate file.tex.in 
FILE( GLOB DIET_USERS_MANUAL_FILES_TO_CONFIG 
  "${DIET_SOURCE_DIR}/doc/UsersManual/*.tex.in"
)
FOREACH( filename ${DIET_USERS_MANUAL_FILES_TO_CONFIG} )
  GET_FILENAME_COMPONENT( filenameNoPath ${filename} NAME )
  STRING( REGEX REPLACE "[.]in$" "" filenameNoPath ${filenameNoPath} )
  SET( filenameDest ${DIET_SOURCE_DIR}/doc/UsersManual/${filenameNoPath} )
  CONFIGURE_FILE(
    ${filename}
    ${filenameDest}
    IMMEDIATE
  )
ENDFOREACH( filename )

# In order to avoid the painfull process of passing the TEXINPUT environment
# variable (with "SET( ENV{TEXINPUTS} ...") copy all the required files
# to CMAKE_CURRENT_BINARY_DIR:
FILE(GLOB DIET_USERS_MANUAL_TEX_GLOB "*.tex")
FILE(GLOB DIET_USERS_MANUAL_BIB_GLOB "*.bib")
SET(TO_COPY ${DIET_USERS_MANUAL_TEX_GLOB} ${DIET_USERS_MANUAL_BIB_GLOB})
FOREACH( filename ${TO_COPY} )
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
# 1/ latex (first pass: for bibtex to have an aux file)
# 2/ bibtex 
# 3/ latex (second pass)
# 4/ latex (third pass)
# In order to achieve this pipeline, we pervert the bibtex and latex generated
# files (.aux, .log, .dvi, .bbl...) to create an artificial set of
# dependencies. The side-effect is hopefully the required behavior...
ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.aux
  DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/UsersManual.tex
  COMMAND   ${LATEX_COMPILER}  
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/UsersManual
  COMMENT   "Latex (first pass)"
  )

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.bbl
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.aux
  COMMAND   ${BIBTEX_COMPILER}  
  ARGS      -terse ${CMAKE_CURRENT_BINARY_DIR}/UsersManual
  COMMENT   "Bibtex"
  )

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.dvi
            ${DIET_TARGET_USERS_FIGURES}
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.bbl
  COMMAND   ${LATEX_COMPILER}  
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/UsersManual
  COMMENT   "Latex (second pass)"
  )

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.dvi
  DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/UsersManual
  )

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.log
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.bbl
            ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.dvi
  COMMAND   ${LATEX_COMPILER}  
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/UsersManual
  COMMENT   "Latex (third pass)"
  )

# Trigger the pipeline by requiring LaTeX's log file:
ADD_CUSTOM_TARGET(
  UsersManualDocument
  ALL echo
  DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.log
  )

# Whenever if possible also generate the pdf document:
IF( DVIPDF_CONVERTER )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.pdf
    DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.log
    COMMAND   ${DVIPDF_CONVERTER}
    ARGS      -o ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.pdf
              ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.dvi
    COMMENT   "Generating pdf version of UserManual."
  )
  ADD_CUSTOM_TARGET(
    UsersManualPdfDocument
    ALL echo
    DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.pdf
  )
  INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.pdf
    DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/UsersManual
  )
ENDIF( DVIPDF_CONVERTER )

### But FIRST, convert the required figures (various formats) to eps:
ADD_DEPENDENCIES( UsersManualDocument DIET_TARGET_USERS_FIGURES )
