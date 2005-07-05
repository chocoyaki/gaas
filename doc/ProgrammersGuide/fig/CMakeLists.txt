#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#****************************************************************************#

############################################################################
# In order to avoid the painfull process of passing the TEXINPUT environment
# variable copy all the required files to ${DIET_BINARY_DIR}:


### Convert the (transfig) fig figures to eps:
FILE(GLOB DIET_PROGRAMMERS_MANUAL_FIG_GLOB "*.fig")

FOREACH( filename ${DIET_PROGRAMMERS_MANUAL_FIG_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoExt ${filename} NAME_WE )
  SET( filenameDest "${CMAKE_CURRENT_BINARY_DIR}/${filenameNoExt}.eps" )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${filenameDest}
    COMMAND   ${FIG2DEV}
    ARGS      -L eps ${filename} ${filenameDest}
    DEPENDS   ${filename}
    )
    ADD_CUSTOM_TARGET(dummy${filename}_EPS
      ALL
      DEPENDS  ${filenameDest}
    )
ENDFOREACH( filename )

### Convert the png and the gif figures to eps:
# FIXME: make the converted extension eps as opposed to ps (this implies
# changing the includegraphics in the TeX source).
FILE(GLOB DIET_PROGRAMMER_MANUAL_PNG_GLOB "*.png")
FILE(GLOB DIET_PROGRAMMER_MANUAL_GIF_GLOB "*.gif")
SET( DIET_PROGRAMMER_MANUAL_PNG_GIF_GLOB
  ${DIET_PROGRAMMER_MANUAL_PNG_GLOB}
  ${DIET_PROGRAMMER_MANUAL_GIF_GLOB}
  )
FOREACH( filename ${DIET_PROGRAMMER_MANUAL_PNG_GIF_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoExt ${filename} NAME_WE )
  SET( filenameDest "${CMAKE_CURRENT_BINARY_DIR}/${filenameNoExt}.ps" )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${filenameDest}
    COMMAND   ${IMAGEMAGICK_CONVERT_EXECUTABLE}
    ARGS      ${filename} ${filenameDest}
    DEPENDS   ${filename}
    )
  ADD_CUSTOM_TARGET(dummy${filename}_PS
    ALL
    DEPENDS  ${filenameDest}
  )
ENDFOREACH( filename )
