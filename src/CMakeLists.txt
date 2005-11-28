#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#

SUBDIRS( CORBA utils agent client SeD )

IF( DIET_BUILD_EXAMPLES )
  SUBDIRS( examples )
ENDIF( DIET_BUILD_EXAMPLES )
