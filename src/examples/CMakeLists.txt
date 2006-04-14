#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_SeD )

ADD_SUBDIRECTORY( Batch )
ADD_SUBDIRECTORY( BLAS )
ADD_SUBDIRECTORY( cfgs )
ADD_SUBDIRECTORY( dmat_manips )
ADD_SUBDIRECTORY( JXTA )
ADD_SUBDIRECTORY( file_transfer )
ADD_SUBDIRECTORY( scalars )

IF ( DIET_USE_WORKFLOW )
  ADD_SUBDIRECTORY ( workflow )
ENDIF ( DIET_USE_WORKFLOW )