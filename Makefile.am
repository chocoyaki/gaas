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
#* Revision 1.9  2003/09/22 21:05:49  pcombes
#* Better management of the maintainer distribution.
#*
#* Revision 1.8  2003/09/09 12:55:17  pcombes
#* Reoraganization of doc + new distrib generation.
#*
#* Revision 1.7  2003/07/25 20:37:36  pcombes
#* Separate the DIET API (slightly modified) from the GridRPC API (version of
#* the draft dated to 07/21/2003)
#*
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

# Headers to install ...
include_HEADERS = $(top_builddir)/include/Makefile.inc \
		  $(top_srcdir)/include/DIET_grpc.h \
		  $(top_srcdir)/include/DIET_client.h \
		  $(top_srcdir)/include/DIET_server.h \
		  $(top_srcdir)/include/DIET_data.h \
		  $(top_srcdir)/include/DIET_mutex.h

# Some files that can be re-built from scratch (with bootstrap.sh)
MAINTAINERCLEANFILES = $(top_srcdir)/Makefile.in \
		       $(top_srcdir)/aclocal.m4 \
		       $(top_srcdir)/configure \
		       $(top_srcdir)/config-h.in \
		       $(top_srcdir)/stamp-h.in

# Some useful selective targets ...

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
	$(MAKE) -C src/$@

utils:	CORBA
	$(MAKE) -C src/$@

agent:	utils
	$(MAKE) -C src/$@

SeD:	utils
	$(MAKE) -C src/$@

client:	utils
	$(MAKE) -C src/$@

examples:
	$(MAKE) -C src/$@

doc:
	$(MAKE) -C $@


#****************************************************************************#
# Management of the distributions:
#  - the developer distribution if in maintainer mode,
#  - the user distribution else.
#****************************************************************************#

# Compile the doc first, in order to add it in the dist.
distrib: doc dist
distribcheck: doc distcheck


if MAINTAINER_MODE
# add the suffix "-dev"
override PACKAGE_VERSION = $(VERSION)-dev
override PACKAGE_STRING  = $(PACKAGE_NAME) $(VERSION)-dev
override distdir         = $(PACKAGE)-$(VERSION)-dev
endif

# Files to add in the dist, which are not already included by 
# automake.
EXTRA_DIST = $(top_srcdir)/include/Makefile.inc.in
if MAINTAINER_MODE
EXTRA_DIST += acmacros/aci.m4 acmacros/fast.m4 acmacros/libtool.m4 \
	      bin bootstrap.sh TO_DO
	      bin/scripts bootstrap.sh TO_DO
endif

# RM_FROM_DIST: files that should not be in the dist, but that automake includes
# DONT_DIST_FILES: files that should not be processed by distrib_file.sh before
RM_FROM_DIST    = AUTHORS NEWS aclocal.m4 include/Makefile.inc
RM_FROM_DIST    = aclocal.m4 include/Makefile.inc
DONT_DIST_FILES = README COPYING ChangeLog INSTALL configure config.guess \
		  config.sub depcomp install-sh ltmain.sh mkinstalldirs   \
		  missing include/DIET_config.h.in

DONT_DIST_FILES += doc/API acmacros bootstrap.sh bin TO_DO
DONT_DIST_FILES += doc/API acmacros bootstrap.sh bin/scripts TO_DO
RM_FROM_DIST += configure.ac doc/Programmer `find . -name Makefile.am`
RM_FROM_DIST += AUTHORS NEWS configure.ac doc/Programmer `find . -name Makefile.am`
endif

# The target to execute once the dist is ready to be tgz'd.
dist-hook:
	(cd $(distdir); rm -rf $(RM_FROM_DIST))
	for f in `ls -A $(distdir)`; do				  \
	  export VERSION=$(PACKAGE_VERSION);                              \
	  $(top_srcdir)/bin/scripts/distrib_file.sh $(distdir)/$$f	\
						    $(DONT_DIST_FILES);	\
	done
if MAINTAINER_MODE
	echo "$(PACKAGE_STRING) (maintainer distribution)" > $(distdir)/VERSION
else
	echo $(PACKAGE_STRING) > $(distdir)/VERSION
endif
	echo `LANG=C date +"%A %B %d %Y"` >> $(distdir)/VERSION

distclean-local:
	-rm -rf $(MAINTAINERCLEANFILES)


