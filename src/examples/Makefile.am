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
#* Revision 1.5  2003/09/22 21:09:08  pcombes
#* Scripts for managing a local platform with LDAP + NWS + CORBA Name server.
#*
#* Revision 1.4  2003/06/16 17:15:54  pcombes
#* Move asynchronous examples in dmat_manips.
#*
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

SUBDIRS = dmat_manips file_transfer @SUBMODULES_examples@
DIST_SUBDIRS = $(SUBDIRS) @DIST_SUBMODULES_examples@

EXTRA_DIST = $(wildcard $(srcdir)/cfgs/*.cfg)					\
	     $(wildcard $(srcdir)/cfgs/*.ldif) $(srcdir)/cfgs/ldap_schema

noinst_bindir = `if [ ! -d bin ]; then mkdir bin; fi; cd bin; pwd`
# The generated bin/local_corba_ns.sh is already "installed" by configure.
noinst_bin_SCRIPTS = $(srcdir)/bin/local_platform.sh	\
		     $(srcdir)/bin/local_ldap.sh $(srcdir)/bin/local_nws.sh

HOSTNAME = @HOST_NAME@
ex_datadir = ./etc

install-data-hook:
	@if [ ! -d $(sysconfdir) ]; then mkdir -p $(sysconfdir); fi
#	Configure does not maintain the mode of the files ...
	chmod +x bin/local_corba_ns.sh
#	Generate config files in the build directories
#	+ copy generated config files into $(sysconfdir)
	if [ ! -d $(ex_datadir) ]; then						\
	  mkdir -p $(ex_datadir);						\
	  for f in $(EXTRA_DIST); do						\
	    if [ -d $$f ]; then\
	      cp -R $$f $(ex_datadir);						\
	    else								\
	      sed -e "s|@host|$(HOSTNAME)|g" $$f |				\
	      sed -e "s|@pwd|`pwd`|g" $$f > $(ex_datadir)/`basename $$f`;	\
	    fi;									\
	  done;									\
	  for f in $(ex_datadir)/*.cfg; do					\
	    $(INSTALL_DATA) $$f $(sysconfdir);					\
	  done;									\
	  rm -f $(sysconfdir)/slapd.cfg;					\
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



