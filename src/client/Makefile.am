#****************************************************************************#
#* DIET client Makefile                                                     *#
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
#* Revision 1.2  2003/04/10 13:12:38  pcombes
#* Update to new utils and CORBA AM libraries.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_client.la

libDIET_client_la_SOURCES = $(top_srcdir)/include/DIET_client.h DIET_client.cc

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

libDIET_client_la_LIBADD = \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsSeDClt.la \
	$(utilsdir)/libUtilsCichlid.la                              \
	$(corbadir)/libCorbaCommon.la $(corbadir)/libCorbaSeDClt.la \
	$(corbadir)/idl/libIDLAgent.la                              \
	$(corbadir)/idl/libIDLCommon.la $(corbadir)/idl/libIDLMA.la \
	$(LIBS_OMNIORB)

AM_CPPFLAGS += -I$(top_srcdir)/src/utils \
	       -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

