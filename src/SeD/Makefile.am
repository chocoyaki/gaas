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
#* Revision 1.5  2003/09/16 10:37:19  pcombes
#* Remove .hh files from libDIET_SeD_la_SOURCES.
#*
#* Revision 1.4  2003/08/01 19:20:23  pcombes
#* Reorganize FAST and omniORB flags. Include FASTMgr.
#*
#* Revision 1.3  2003/05/05 14:15:53  pcombes
#* FAST flags were missing ...
#*
#* Revision 1.2  2003/04/10 13:17:05  pcombes
#* Update to new utils and CORBA AM libraries.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

#AUTOMAKE_OPTIONS = gnu

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_SeD.la

libDIET_SeD_la_SOURCES = DIET_server.cc SeDImpl.cc 

noinst_HEADERS = SeDImpl.hh

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

libDIET_SeD_la_LIBADD = \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsSeDClt.la	\
	$(utilsdir)/libUtilsNodes.la $(corbadir)/libCorbaCommon.la	\
	$(corbadir)/idl/libIDLCommon.la $(corbadir)/idl/libIDLAgent.la	\
	$(LIBS_OMNIORB) $(LIBS_FAST)

AM_CXXFLAGS = $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) -I$(top_srcdir)/src/utils \
	      -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

