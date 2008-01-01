#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_SeD )

IF( DIET_USE_BATCH OR DIET_USE_ALT_BATCH )
  ADD_SUBDIRECTORY( Batch )
ENDIF( DIET_USE_BATCH OR DIET_USE_ALT_BATCH )

ADD_SUBDIRECTORY( BLAS )
ADD_SUBDIRECTORY( cfgs )
ADD_SUBDIRECTORY( cori )
ADD_SUBDIRECTORY( dmat_manips )
ADD_SUBDIRECTORY( JXTA )
ADD_SUBDIRECTORY( file_transfer )
ADD_SUBDIRECTORY( scalars )
ADD_SUBDIRECTORY( GridRPC)

IF ( DIET_USE_WORKFLOW )
  ADD_SUBDIRECTORY ( workflow )
ENDIF ( DIET_USE_WORKFLOW )

IF ( DIET_USE_CCS )
  ADD_SUBDIRECTORY ( ccs )
ENDIF ( DIET_USE_CCS )
