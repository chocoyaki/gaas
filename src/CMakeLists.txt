#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
ADD_SUBDIRECTORY( CORBA )
ADD_SUBDIRECTORY( utils )
ADD_SUBDIRECTORY( client )
ADD_SUBDIRECTORY( agent )
ADD_SUBDIRECTORY( SeD )
ADD_SUBDIRECTORY( Testing )


IF( DIET_BUILD_EXAMPLES )
  ADD_SUBDIRECTORY( examples )
ENDIF( DIET_BUILD_EXAMPLES )
