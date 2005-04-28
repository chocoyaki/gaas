#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.1  2005/04/28 13:25:11  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

SET( @host bozo )
CONFIGURE_FILE(
      ${DIET_SOURCE_DIR}/src/examples/cfgs/LA1.cfg
      ${DIET_BINARY_DIR}/etc/LA1.cfg
      IMMEDIATE
  )
