#****************************************************************************#
#* DIET CORBA Makefile                                                      *#
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
#* Revision 1.3  2003/05/05 14:00:37  pcombes
#* Add FAST flags for the compilation of marshalling.
#*
#* Revision 1.2  2003/04/10 12:38:30  pcombes
#* IDL descriptions are back in their directory.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

SUBDIRS = idl
DIST_SUBDIRS = idl

include $(top_srcdir)/config.mk.in

noinst_LTLIBRARIES = libCorbaCommon.la libCorbaSeDClt.la

# defining libtool libraries sources
libCorbaCommon_la_SOURCES = ORBMgr.cc ORBMgr.hh
libCorbaSeDClt_la_SOURCES = marshalling.cc marshalling.hh \
                            $(top_srcdir)/src/utils/dietTypes.hh

noinst_HEADERS = marshalling.hh ORBMgr.hh

# automake include variables
AM_CPPFLAGS += $(CXXFLAGS_FAST) -I$(top_srcdir)/src/utils -I./idl

