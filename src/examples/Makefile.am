#****************************************************************************#
#* DIET examples Makefile                                                   *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.3  2003/06/02 09:10:07  cpera
#* Samples illustrate use of asynchronize api.
#*
#* Revision 1.2  2003/05/19 17:15:35  pcombes
#* Change install behaviour: never erase cfg files already installed.
#*
#* Revision 1.1  2003/04/10 13:38:13  pcombes
#* Include examples in the autotools chain, with two optional modules
#* (BLAS and ScaLAPACK) and two default examples (dmat_manips and file_transfer)
#* NB: installation directory is $(top_builddir)/src/examples/bin.
#****************************************************************************#

SUBDIRS = dmat_manips dmat_manips_async file_transfer @SUBMODULES_examples@
DIST_SUBDIRS = $(SUBDIRS) @DIST_SUBMODULES_examples@

EXTRA_DIST = $(wildcard $(srcdir)/cfgs/*.cfg)

HOSTNAME = @HOST_NAME@
ex_datadir = ./etc

install-data-hook:
	@if [ ! -d $(sysconfdir) ]; then mkdir -p $(sysconfdir); fi
#	Generate config files in the build directories
#	+ copy generated config files into $(sysconfdir)
	@if [ ! -d $(ex_datadir) ]; then				     \
	  mkdir -p $(ex_datadir);					     \
	  for f in $(wildcard $(srcdir)/cfgs/*.cfg); do     		     \
	    sed -e 's/toto/$(HOSTNAME)/g' $$f > $(ex_datadir)/`basename $$f`;\
	  done;								     \
	  for f in $(ex_datadir)/*.cfg; do				     \
	    $(INSTALL_DATA) $$f $(sysconfdir)/`basename $$f`;		     \
	  done;								     \
	fi

uninstall-hook:
	@for f in $(wildcard $(srcdir)/cfgs/*.cfg); do	\
	  rm -f $(ex_datadir)/`basename $$f`;		\
	done;
	@rmdir $(ex_datadir)
	@for f in $(wildcard $(srcdir)/cfgs/*.cfg); do	\
	  rm -f $(sysconfdir)/`basename $$f`;		\
	done;

.PHONY: dmat_manips file_transfer BLAS ScaLAPACK

dmat_manips:
	$(MAKE) -C dmat_manips

file_transfer:
	$(MAKE) -C file_transfer

BLAS:
	$(MAKE) -C BLAS

ScaLAPACK:
	$(MAKE) -C ScaLAPACK



