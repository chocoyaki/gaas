#****************************************************************************#
#* $Id$ 		    *#
#* DIET global Makefile                                                     *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Frederic 	LOMBARD       - LIFC Besançon (France)              *#
#*    - Philippe 	COMBES        - LIP ENS-Lyon (France)               *#
#*    - Romain   	LACROIX       - LIP ENS-Lyon (France)               *#
#*    - Christophe   	PERA          - LIP ENS-Lyon (France)               *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#*
#* $Log$
#* Revision 1.2  2003/02/17 15:33:23  cpera
#* Change SUBMUDULES_diet in SUBMODULES_DIET
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#*
#* Revision 1.5  2002/12/18 16:30:20  pcombes
#* Makefile.in: install does not depend on all.
#* configure:   bug fix in messages.
#*
#* Revision 1.4  2002/12/03 19:11:42  pcombes
#* Fix minor bug in uninstall (DIET_config.h was not removed ...)
#*
#* Revision 1.3  2002/12/03 19:08:23  pcombes
#* Update configure, update to FAST 0.3.15, clean CVS logs in files.
#* Put main Makefile in root directory.
#*
#****************************************************************************##

SUBDIRS = CORBA utils @SUBMODULES_DIET@ 
DIST_SUBDIRS = $(SUBDIRS) @DIST_SUBMODULES_DIET@
