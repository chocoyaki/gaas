#
# This module looks for transfig's fig2dev conversion utility
# (fig stands for "Facility for Interactive Generation of figures")
#
# The following variables are set:
# FIG2DEV               - Path to fig2dev utility
# TRANSFIG_FOUND        - When false, don't try to use TRANSFIG
#
 
FIND_PROGRAM(FIG2DEV
  NAMES
  fig2dev
  PATH
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)

SET( TRANSFIG_FOUND FALSE )
IF( FIG2DEV )
  SET( TRANSFIG_FOUND TRUE )
  MARK_AS_ADVANCED( FIG2DEV )
ENDIF( FIG2DEV )
