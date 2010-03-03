#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
ADD_SUBDIRECTORY( CORBA/idl )
ADD_SUBDIRECTORY( utils )
ADD_SUBDIRECTORY( CORBA )
ADD_SUBDIRECTORY( utils/nodes )
ADD_SUBDIRECTORY( client )
IF ( DIET_USE_WORKFLOW )
  ADD_SUBDIRECTORY( utils/workflow )
ENDIF (DIET_USE_WORKFLOW )
ADD_SUBDIRECTORY( agent )
ADD_SUBDIRECTORY( SeD )
ADD_SUBDIRECTORY( Testing )


IF( DIET_BUILD_EXAMPLES )
  ADD_SUBDIRECTORY( examples )
ENDIF( DIET_BUILD_EXAMPLES )
