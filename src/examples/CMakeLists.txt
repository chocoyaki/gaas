#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

SET( DIET_CLIENT_LIBRARIES DIET_client DIET_Dagda)
SET( DIET_SERVER_LIBRARIES DIET_SeD DIET_Dagda)

IF( DIET_USE_DYNAMICS )
  if ( DIET_ADMIN_API )
    ADD_SUBDIRECTORY( dynamic_hierarchy )
  endif ( )
ENDIF( DIET_USE_DYNAMICS )

IF( DIET_USE_ALT_BATCH )
  ADD_SUBDIRECTORY( Batch )
  IF( DIET_USE_CLOUD)
    ADD_SUBDIRECTORY( cloud )
  ENDIF( DIET_USE_CLOUD )
ENDIF( DIET_USE_ALT_BATCH )

ADD_SUBDIRECTORY( BLAS )
ADD_SUBDIRECTORY( cfgs )
ADD_SUBDIRECTORY( cori )
ADD_SUBDIRECTORY( dmat_manips )
ADD_SUBDIRECTORY( file_transfer )
ADD_SUBDIRECTORY( scalars )
ADD_SUBDIRECTORY( GridRPC)

ADD_SUBDIRECTORY( allDataTransfers )
ADD_SUBDIRECTORY( containers )
ADD_SUBDIRECTORY( dyn_add_rem )
ADD_SUBDIRECTORY( dynamicServiceMgr )

IF ( DIET_USE_WORKFLOW )
  ADD_SUBDIRECTORY ( workflow )
  # Do not compile SeDWrapper example for as long as Diet 3.0 is not released:
  # this example requires BOOST.
  # ADD_SUBDIRECTORY ( SeDWrapper )
ENDIF ( DIET_USE_WORKFLOW )

# IF ( DIET_USE_CCS )
#   ADD_SUBDIRECTORY ( ccs )
# ENDIF ( DIET_USE_CCS )

IF ( DIET_USE_MULTICALL )
  ADD_SUBDIRECTORY ( multicall )
ENDIF ( DIET_USE_MULTICALL )
