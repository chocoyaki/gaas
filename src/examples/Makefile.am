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
#* Revision 1.8  2004/06/11 15:45:39  ctedesch
#* add DIET/JXTA
#*
#* Revision 1.7  2003/09/29 09:16:25  pcombes
#* Fix bug in generation of ldap_schema.
#*
#* Revision 1.6  2003/09/26 14:07:01  pcombes
#* Get the examples out of the automake chain, to make Makefiles more readable.
#* The generated before.mk includes the right Makefile.inc.
#*
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

SUBDIRS = dmat_manips file_transfer scalars @SUBMODULES_examples@
if JXTA_MODE
SUBDIRS += JXTA
endif

# DIST_SUBDIRS is forced to all examples, so that it is still possible to build
# a (user) dsitribution in user mode.
DIST_SUBDIRS = BLAS dmat_manips file_transfer ScaLAPACK scalars
if JXTA_MODE
DIST_SUBDIRS += JXTA
endif

.PHONY:	$(DIST_SUBDIRS)

##
# Manage "manually" the additional configuration files,
# ie the cfgs/*.cfg, cfgs/*.ldif and cfgs/ldap_schema/*.schema files.
#

HOSTNAME = @HOST_NAME@
ex_datadir = ./etc
SRC_CONFIG_FILES = $(wildcard $(srcdir)/cfgs/*.cfg)	\
		   $(wildcard $(srcdir)/cfgs/*.ldif)	\
		   $(wildcard $(srcdir)/cfgs/ldap_schema/*.schema)
CONFIG_FILES = $(patsubst $(srcdir)/cfgs/%, $(ex_datadir)/%, $(SRC_CONFIG_FILES))

config-files:	$(CONFIG_FILES)

$(ex_datadir)/%.cfg:	$(srcdir)/cfgs/%.cfg
	@echo "Creating $@"
	@sed -e "s|@host|$(HOSTNAME)|g" $< | sed -e "s|@pwd|`pwd`|g"	\
	  > $(ex_datadir)/`basename $@`
$(ex_datadir)/%.ldif:	$(srcdir)/cfgs/%.ldif
	@echo "Creating $@"
	@sed -e "s|@host|$(HOSTNAME)|g" $< | sed -e "s|@pwd|`pwd`|g"	\
	  > $(ex_datadir)/`basename $@`
$(ex_datadir)/%.schema:	$(srcdir)/cfgs/%.schema
	@if [ ! -d $(ex_datadir)/ldap_schema ]; then	\
	  echo "Creating $(ex_datadir)/ldap_schema";	\
	  mkdir -p $(ex_datadir)/ldap_schema;		\
	fi
	@echo "Creating $@"
	@cp -f $< $@
clean-config-files:
	@for f in $(CONFIG_FILES); do	\
	  echo "rm -f $$f"; rm -f $$f;	\
	done

#
##

##
# Main targets
#

EXTRA_DIST = $(SRC_CONFIG_FILES) $(wildcard $(srcdir)/bin/*.sh.in)

all-am:	Makefile before.mk all-data

all-data:	$(ex_datadir) config-files

$(ex_datadir):
	@if [ ! -d $(ex_datadir) ]; then	\
	  mkdir -p $(ex_datadir);		\
	fi

clean: clean-am clean-config-files clean-recursive
	 rmdir $(ex_datadir)/ldap_schema $(ex_datadir) > /dev/null 2>&1 || exit 0

install-data-hook:
#	Copy generated config files into $(sysconfdir)
	@if [ ! -d $(sysconfdir) ]; then		\
	  mkdir -p $(sysconfdir);			\
	  for f in $(ex_datadir)/*.cfg; do		\
	    if [ "`basename $$f`" != slapd.cfg ]; then	\
	      echo "$(INSTALL_DATA) $$f $(sysconfdir)";	\
	      $(INSTALL_DATA) $$f $(sysconfdir);	\
	    fi;						\
	  done;						\
	fi

uninstall-hook:
	@for f in $(wildcard $(srcdir)/cfgs/*.cfg); do	\
	  rm -f $(sysconfdir)/`basename $$f`;		\
	done;

