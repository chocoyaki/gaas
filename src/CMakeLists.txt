#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
ADD_SUBDIRECTORY( CORBA )
ADD_SUBDIRECTORY( utils )
ADD_SUBDIRECTORY( agent )
ADD_SUBDIRECTORY( client )
ADD_SUBDIRECTORY( SeD )


IF( DIET_BUILD_EXAMPLES )
  ADD_SUBDIRECTORY( examples )
ENDIF( DIET_BUILD_EXAMPLES )
