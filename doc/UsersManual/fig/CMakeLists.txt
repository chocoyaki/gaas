#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#

############################################################################
# In order to avoid the painfull process of passing the TEXINPUT environment
# variable copy all the required files to ${DIET_BINARY_DIR}:

### Existing ps files are simply copied:
FILE(GLOB DIET_USERS_MANUAL_PS_GLOB "*.ps")
FOREACH( filename ${DIET_USERS_MANUAL_PS_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoPath ${filename} NAME )
  SET( filenameDest "${CMAKE_CURRENT_BINARY_DIR}/${filenameNoPath}" )
  CONFIGURE_FILE(
    ${filename}
    ${filenameDest}
    COPYONLY
    IMMEDIATE
  )
ENDFOREACH( filename )

############# Convert figures to eps for latex target #####################
### In order to create a target corresponding to all what's get build
# within this directory:
SET( DIET_USERS_FIGURE_POSTSCRIPT_OUTPUTS )

### Convert the (transfig) fig figures to eps:
FILE(GLOB DIET_USERS_MANUAL_FIG_GLOB "*.fig")

FOREACH( filename ${DIET_USERS_MANUAL_FIG_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoExt ${filename} NAME_WE )
  SET( filenameDest "${CMAKE_CURRENT_BINARY_DIR}/${filenameNoExt}.eps" )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${filenameDest}
    COMMAND   ${FIG2DEV}
    ARGS      -L eps ${filename} ${filenameDest}
    DEPENDS   ${filename}
    )
  ADD_CUSTOM_TARGET(
    dummy${filename}_EPS
    ALL
    DEPENDS  ${filenameDest}
    )
  INSTALL(
    FILES ${filenameDest}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/UsersManual/fig
    )
  SET( DIET_USERS_FIGURE_POSTSCRIPT_OUTPUTS
    ${DIET_USERS_FIGURE_POSTSCRIPT_OUTPUTS} ${filenameDest} )
ENDFOREACH( filename )

### Convert the gif figures to eps:
FILE(GLOB DIET_USERS_MANUAL_GIF_GLOB "*.gif")
FOREACH( filename ${DIET_USERS_MANUAL_GIF_GLOB} )
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
  INSTALL(
    FILES ${filenameDest}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/UsersManual/fig
    )
  SET( DIET_USERS_FIGURE_POSTSCRIPT_OUTPUTS
    ${DIET_USERS_FIGURE_POSTSCRIPT_OUTPUTS} ${filenameDest} )
ENDFOREACH( filename )

############# Convert figures to png for html target ######################
### Convert the (transfig) fig figures to png:
SET( DIET_USERS_FIGURE_PNG_OUTPUTS )
FOREACH( filename ${DIET_USERS_MANUAL_FIG_GLOB} )
  GET_FILENAME_COMPONENT( filenameNoExt ${filename} NAME_WE )
  SET( filenameDest "${CMAKE_CURRENT_BINARY_DIR}/${filenameNoExt}.png" )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${filenameDest}
    COMMAND   ${FIG2DEV}
    ARGS      -L png ${filename} ${filenameDest}
    DEPENDS   ${filename}
    )
  ADD_CUSTOM_TARGET( dummy${filename}_PNG
      ALL
      DEPENDS  ${filenameDest}
    )
  SET( DIET_USERS_FIGURE_PNG_OUTPUTS
    ${DIET_USERS_FIGURE_PNG_OUTPUTS} ${filenameDest} )
ENDFOREACH( filename )

######################### Eventually create the target:
ADD_CUSTOM_TARGET( DIET_TARGET_USERS_FIGURES
  ALL DEPENDS
  ${DIET_USERS_FIGURE_POSTSCRIPT_OUTPUTS}
  ${DIET_USERS_FIGURE_PNG_OUTPUTS}
)
