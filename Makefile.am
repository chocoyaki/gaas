#****************************************************************************#
#* $Id$                 *#
#* DIET global Makefile                                                     *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Frederic LOMBARD          - LIFC Besançon (France)                  *#
#*    - Philippe COMBES           - LIP ENS-Lyon (France)                   *#
#*    - Romain   LACROIX          - LIP ENS-Lyon (France)                   *#
#*    - Christophe   LACROIX          - LIP ENS-Lyon (France)               *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#*
#* $Log$
#* Revision 1.2  2003/02/19 09:32:24  cpera
#* Change dist rules for documentation.
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
#****************************************************************************#

        
SUBDIRS = src doc
DIST_SUBDIRS = src 

EXTRA_DIST = $(top_srcdir)/include/Makefile.inc.in \
             $(top_srcdir)/include/DIET_config.h.in \
             $(top_srcdir)/src/examples/BLAS/Makefile.in \
             $(top_srcdir)/src/examples/BLAS/client.c \
             $(top_srcdir)/src/examples/BLAS/dgemm_client.c \
             $(top_srcdir)/src/examples/BLAS/server.c \
             $(top_srcdir)/src/examples/BLAS/server.c \
             $(top_srcdir)/src/examples/cfgs/client.cfg \
             $(top_srcdir)/src/examples/cfgs/LA1.cfg \
             $(top_srcdir)/src/examples/cfgs/LA2.cfg \
             $(top_srcdir)/src/examples/cfgs/MA1.cfg \
             $(top_srcdir)/src/examples/cfgs/SeD.cfg \
             $(top_srcdir)/src/examples/dmat_manips/client.c \
             $(top_srcdir)/src/examples/dmat_manips/Makefile.in \
             $(top_srcdir)/src/examples/dmat_manips/progs.h \
             $(top_srcdir)/src/examples/dmat_manips/server.c \
             $(top_srcdir)/src/examples/file_transfer/client.c \
             $(top_srcdir)/src/examples/file_transfer/server.c \
             $(top_srcdir)/src/examples/file_transfer/Makefile.in \
             $(top_srcdir)/src/examples/SCALAPACK/Makefile.in \
             $(top_srcdir)/src/examples/SCALAPACK/client.c \
             $(top_srcdir)/src/examples/SCALAPACK/server.c \
             $(top_srcdir)/src/examples/SCALAPACK/pdgemm_client.c \
             $(top_srcdir)/src/examples/SCALAPACK/job.h \
             $(top_srcdir)/src/examples/SCALAPACK/job.c \
             $(top_srcdir)/doc/CS/DIET_Coding_Standard.ps \
             $(top_srcdir)/doc/UM/DIET_User_Manual.ps
if MAINTAINER_MODE
	EXTRA_DIST+= $(top_srcdir)/autogen.sh
endif

include_HEADERS = $(top_srcdir)/include/Makefile.inc \
		$(top_srcdir)/include/DIET_config.h \
		$(top_srcdir)/include/DIET_client.h \
		$(top_srcdir)/include/DIET_server.h \
		$(top_srcdir)/include/DIET_data.h \
		$(top_srcdir)/include/DIET_mutex.h \
		$(top_srcdir)/include/slimfast_api_local.h

MAINTAINERCLEANFILES= $(top_srcdir)/Makefile.in \
		$(top_srcdir)/aclocal.m4 \
		$(top_srcdir)/configure \
		$(top_srcdir)/config-h.in \
		$(top_srcdir)/stamp-h.in

distclean-local:
	-rm -rf $(MAINTAINERCLEANFILES)


.PHONY: CORBA utils agent SeD client examples 

help:
	@echo "====================================================================="
	@echo
	@echo "Usage :   make help     : shows this help screen"
	@echo
	@echo "          make agent    : builds DIET agent executable"
	@echo "          make SeD      : builds DIET SeD library"
	@echo "          make client   : builds DIET client library"
	@echo "          make examples : builds basic examples" 
	@echo "          make install  : copy files into $(INSTALL_DIR)"
	@echo "          make all      : builds everything but the examples"
	@echo
	@echo "====================================================================="

CORBA:	
	$(MAKE) -C src/CORBA

utils:	CORBA
	$(MAKE) -C src/utils	

agent:	utils
	$(MAKE) -C src/agent

SeD:	utils
	$(MAKE) -C src/SeD

client:	utils
	$(MAKE) -C src/client

examples:
	$(MAKE) -C src/examples


