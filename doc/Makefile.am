#****************************************************************************#
#* DIET documentation global Makefile                                       *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Christophe PERA (Christophe.Pera@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.4  2003/06/16 17:41:27  pcombes
#* Fix header.
#*
#* Revision 1.3  2003/04/10 11:11:03  pcombes
#* Fix compilation in a build directory.
#*
#* Revision 1.2  2003/02/14 12:27:16  cpera
#* change spaces with tabs (make errors in Makefile target).
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#*
#* Revision 1.2  2003/01/13 12:09:00  pcombes
#* UM: client part complete for users's day ...
#*
#* Revision 1.1  2003/01/07 18:08:43  pcombes
#* Add install targets in Makefiles.
#****************************************************************************#

SUBDIRS = CS UM
DIST_SUBDIRS = $(SUBDIRS)

.PHONY:	CS UM

CS: 
	$(MAKE) -C CS

UM: 
	$(MAKE) -C UM

