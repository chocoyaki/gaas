#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.3  2005/12/05 17:05:40  eboix
#*     - First trial for BLAS detection --- Injay2461
#*
#* Revision 1.2  2005/11/28 17:12:53  eboix
#*    src/examples following subdirs JXTA, dmat_manips, file_transfer and scalars
#*    are now properly compiled and linked (not tested though) --- Injay2461
#*
#* Revision 1.1  2005/04/28 13:25:11  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

SET( DIET_CLIENT_LIBRARIES DIET_client )
SET( DIET_SERVER_LIBRARIES DIET_client DIET_SeD )

SUBDIRS( BLAS cfgs dmat_manips JXTA file_transfer scalars )
