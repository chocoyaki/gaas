#****************************************************************************#
#* DIET SeD Makefile                                                        *#
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
#* Revision 1.2  2003/04/10 13:17:05  pcombes
#* Update to new utils and CORBA AM libraries.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

#AUTOMAKE_OPTIONS = gnu

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_SeD.la

libDIET_SeD_la_SOURCES = \
	$(top_srcdir)/include/DIET_server.h DIET_server.cc SeDImpl.cc 

noinst_HEADERS = SeDImpl.hh

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

libDIET_SeD_la_LIBADD = \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsSeDClt.la    \
	$(utilsdir)/libUtilsNodes.la                                   \
	$(corbadir)/libCorbaCommon.la $(corbadir)/libCorbaSeDClt.la    \
	$(corbadir)/idl/libIDLCommon.la $(corbadir)/idl/libIDLAgent.la \
	$(LIBS_OMNIORB) $(LIBS_FAST)

AM_CPPFLAGS += -static -static-libgcc -I$(top_srcdir)/src/utils \
	       -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

