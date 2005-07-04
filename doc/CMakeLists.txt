#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#****************************************************************************#

SUBDIRS( UsersManual )
IF( DIET_MAINTAINER_MODE )
  SUBDIRS( ProgrammersGuide Tutorial )
ENDIF( DIET_MAINTAINER_MODE )
