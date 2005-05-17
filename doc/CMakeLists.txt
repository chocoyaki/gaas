#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.1  2005/05/17 11:58:53  eboix
#*     Added a Cmake/README for cmake starters.
#*     First try on compiling the tex documentation.
#*     Cosmetic changes to CMakeLists.txt.  --- Injay 2461
#*
#*
#****************************************************************************#

IF( LATEX_COMPILER )
   SUBDIRS( UsersManual )
   IF( DIET_MAINTAINER_MODE )
      SUBDIRS( ProgrammersGuide Tutorial )
   END( DIET_MAINTAINER_MODE )
ENDIF( LATEX_COMPILER )
