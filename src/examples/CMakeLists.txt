#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF ( DIET_USE_DAGDA )
   SET( DIET_CLIENT_LIBRARIES DIET_client Dagda UtilsNodes UtilsCommon)
   SET( DIET_SERVER_LIBRARIES DIET_SeD Dagda UtilsNodes UtilsCommon)
ELSE ( DIET_USE_DAGDA )
   SET( DIET_CLIENT_LIBRARIES DIET_client UtilsNodes UtilsCommon)
   SET( DIET_SERVER_LIBRARIES DIET_SeD UtilsNodes UtilsCommon)
ENDIF ( DIET_USE_DAGDA )

IF( DIET_USE_DYNAMICS )
  ADD_SUBDIRECTORY( dynamic_hierarchy )
ENDIF( DIET_USE_DYNAMICS )

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

IF ( DIET_USE_DAGDA )
  ADD_SUBDIRECTORY( containers )
  ADD_SUBDIRECTORY( dyn_add_rem )
  ADD_SUBDIRECTORY( dynamicServiceMgr )
ENDIF ( DIET_USE_DAGDA )

IF ( DIET_USE_WORKFLOW )
  ADD_SUBDIRECTORY ( workflow )
ENDIF ( DIET_USE_WORKFLOW )

# IF ( DIET_USE_CCS )
#   ADD_SUBDIRECTORY ( ccs )
# ENDIF ( DIET_USE_CCS )

IF ( DIET_USE_MULTICALL )
	ADD_SUBDIRECTORY ( multicall )
ENDIF ( DIET_USE_MULTICALL )
