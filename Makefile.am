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
#* Revision 1.14  2004/07/09 19:51:57  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#*
#* Revision 1.13  2004/07/08 20:02:47  rbolze
#* add src/lib dir in the distrib tarballs and correct some mistake from last commit
#*
#* Revision 1.12  2004/07/07 18:17:55  rbolze
#* ready to 1.1 version
#*
#* Revision 1.11  2003/12/15 13:13:59  pcombes
#* Add the 'make' line in the help.
#*
#* Revision 1.10  2003/09/26 14:04:24  pcombes
#* Add DIST_SUBMODULES_DIET in DIST_SUBDIRS
#*
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
#* Revision 1.4  2003/04/10 15:02:10  pcombes
#* Add doc and examples modules. Apply distrib_file.sh to distributed files.
#* Include autotools sources and doxygen documentation in maintainer distribs.
#****************************************************************************#

SUBDIRS = @SUBMODULES_DIET@
DIST_SUBDIRS = $(SUBDIRS) @DIST_SUBMODULES_DIET@

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
	@echo "         make          : builds everything configured" 
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
EXTRA_DIST += $(top_srcdir)/src/lib/*

if MAINTAINER_MODE
EXTRA_DIST += acmacros/aci.m4 acmacros/fast.m4 acmacros/libtool.m4 \
	      acmacros/omniorb.m4 acmacros/utils.m4 \
	      bin/scripts bootstrap.sh TO_DO
endif

# RM_FROM_DIST: files that should not be in the dist, but that automake includes
# DONT_DIST_FILES: files that should not be processed by distrib_file.sh before
#                  distributed
RM_FROM_DIST    = aclocal.m4 include/Makefile.inc
DONT_DIST_FILES = README COPYING ChangeLog INSTALL configure config.guess \
		  config.sub depcomp install-sh ltmain.sh mkinstalldirs   \
		  missing include/DIET_config.h.in $(top_srcdir)/src/lib/*

if MAINTAINER_MODE
DONT_DIST_FILES += doc/API acmacros bootstrap.sh bin/scripts TO_DO
else
RM_FROM_DIST += AUTHORS NEWS configure.ac doc/ProgrammerGuide doc/Tutorial `find . -name Makefile.am`
endif

# The target to execute once the dist is ready to be tgz'd.
dist-hook:
	(cd $(distdir) ;mv doc/UsersManual/*.ps doc/ ;rm -rf $(RM_FROM_DIST))
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


