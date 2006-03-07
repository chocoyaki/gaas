#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_client DIET_SeD )

SUBDIRS( Batch BLAS cfgs dmat_manips JXTA file_transfer scalars )
