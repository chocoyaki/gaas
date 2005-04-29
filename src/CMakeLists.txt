#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.3  2005/04/29 16:18:02  eboix
#*     First example of JXTA dependant compilation.  --- Injay 2461
#*
#* Revision 1.2  2005/04/29 10:36:37  eboix
#*     idl generated files are now taken into account.  --- Injay 2461
#*
#* Revision 1.1  2005/04/28 13:25:10  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

SUBDIRS( CORBA utils agent client SeD )

IF( DIET_BUILD_EXAMPLES )
  SUBDIRS( examples )
ENDIF( DIET_BUILD_EXAMPLES )
