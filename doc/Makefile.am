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
#* Revision 1.8  2004/07/08 18:05:54  rbolze
#* only have tutorial in maintainer mode => this is only because we don't want to distrib tutorial in the tarballs
#*
#* Revision 1.7  2003/12/17 16:12:05  ecaron
#* Change "tutorial" directory to "Tutorial"
#*
#* Revision 1.6  2003/12/15 13:48:07  ecaron
#* Add tutorial directory to the DIET distribution
#*
#* Revision 1.5  2003/09/09 12:38:54  pcombes
#* Reorganization of doc.
#*
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

SUBDIRS = UsersManual 
if MAINTAINER_MODE
SUBDIRS += ProgrammersGuide Tutorial
endif

DIST_SUBDIRS = $(SUBDIRS)

if MAINTAINER_MODE

# This is an artifact for automake to generate the targets necessary for
# generating the source code docmentation without attempting to install it.
BUILT_SOURCES = API
EXTRA_DIST = Doxyfile API
$(BUILT_SOURCES): $(srcdir)/Doxyfile
	(CURDIR=`pwd`; cd $(top_srcdir); \
	 sed "s|API|$$CURDIR/API|g" doc/Doxyfile | doxygen - || exit 0)

endif

Tutorial:
	$(MAKE) -C $@
