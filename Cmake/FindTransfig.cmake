#
# this module looks for dia the gtk+ based diagram creation program
# refer to http://www.gnome.org/projects/dia/
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
MARK_AS_ADVANCED(
  FIG2DEV
)

