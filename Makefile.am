#****************************************************************************#
#* DIET global Makefile                                                     *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             *#
#*    - Christophe PERA (Christophe.Pera@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.6  2003/06/23 13:13:40  pcombes
#* Update make help.
#*
#* Revision 1.5  2003/06/03 18:36:03  pcombes
#* Add a VERSION file to the distribution.
#*
#* Revision 1.4  2003/04/10 15:02:10  pcombes
#* Add doc and examples modules. Apply distrib_file.sh to distributed files.
#* Include autotools sources and doxygen documentation in maintainer distribs.
#*
#* Revision 1.3  2003/02/19 15:46:20  cpera
#* Change files because of portability errors.
#*
#* Revision 1.2  2003/02/19 09:32:24  cpera
#* Change dist rules for documentation.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

SUBDIRS = @SUBMODULES_DIET@
DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = $(top_srcdir)/include/Makefile.inc.in


# RM_FROM_DIST: files that should not be in the dist, but that automake includes
# DONT_DIST_FILES: files that should not be processed by distrib_file.sh before
#                  distributed
RM_FROM_DIST    = AUTHORS NEWS include/Makefile.inc
DONT_DIST_FILES = README COPYING ChangeLog INSTALL configure config.guess \
		  config.sub depcomp install-sh ltmain.sh mkinstalldirs   \
		  missing include/DIET_config.h.in

if MAINTAINER_MODE
DONT_DIST_FILES += aclocal.m4
else
RM_FROM_DIST += aclocal.m4 configure.ac `find . -name Makefile.am`
endif

include_HEADERS = $(top_builddir)/include/Makefile.inc \
		  $(top_srcdir)/include/DIET_client.h \
		  $(top_srcdir)/include/DIET_server.h \
		  $(top_srcdir)/include/DIET_data.h \
		  $(top_srcdir)/include/DIET_mutex.h

MAINTAINERCLEANFILES = $(top_srcdir)/Makefile.in \
		       $(top_srcdir)/aclocal.m4 \
		       $(top_srcdir)/configure \
		       $(top_srcdir)/config-h.in \
		       $(top_srcdir)/stamp-h.in

dist-hook:
	(cd $(distdir); rm -f $(RM_FROM_DIST))
	for f in `find $(distdir) -type f`; do				  \
	  export VERSION=$(PACKAGE_VERSION);                              \
	  $(top_srcdir)/bin/script/distrib_file.sh $$f $(DONT_DIST_FILES);\
	done
	echo $(PACKAGE_STRING) > $(distdir)/VERSION
	echo `LANG=C date +"%A %B %d %Y"` >> $(distdir)/VERSION
if MAINTAINER_MODE
	@cp $(top_srcdir)/Doxyfile $(distdir)
	(cd $(distdir); doxygen || exit 0)
	@rm -f $(distdir)/Doxyfile
endif

distclean-local:
	-rm -rf $(MAINTAINERCLEANFILES)


.PHONY: CORBA utils agent SeD client examples doc

help:
	@echo "===================================================================="
	@echo " Usage : make help     : shows this help screen"
	@echo "         make agent    : builds DIET agent executable"
	@echo "         make SeD      : builds DIET SeD library"
	@echo "         make client   : builds DIET client library"
	@echo "         make examples : builds basic examples" 
	@echo "         make install  : copy files into $(prefix)"
	@echo "===================================================================="

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

doc:
	$(MAKE) -C doc

distrib: doc dist
distribcheck: doc distcheck


