#****************************************************************************#
#* DIET agent Makefile                                                      *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   *#
#*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             *#
#*    - Christophe PERA (Christophe.Pera@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.2  2003/04/10 13:10:24  pcombes
#* Update to new files.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

bin_PROGRAMS = DIET_LA DIET_MA

DIET_LA_SOURCES = DIET_LA.cc LocalAgentImpl.cc AgentImpl.cc\
		  GlobalSchedulers.cc Schedulers.cc Request.cc qsort.cc

DIET_MA_SOURCES = DIET_MA.cc MasterAgentImpl.cc AgentImpl.cc \
		  GlobalSchedulers.cc Schedulers.cc Request.cc qsort.cc

noinst_HEADERS = AgentImpl.hh LocalAgentImpl.hh MasterAgentImpl.hh ChildID.hh \
		 GlobalSchedulers.hh NodeDescription.hh Schedulers.hh \
		 Request.hh RequestID.hh

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

DIET_LA_LDADD =	\
	$(corbadir)/libCorbaCommon.la $(corbadir)/idl/libIDLAgent.la \
	$(corbadir)/idl/libIDLCommon.la $(corbadir)/idl/libIDLLA.la  \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsNodes.la   \
	$(LIBS_OMNIORB) $(LIBS_FAST)


DIET_MA_LDADD =	\
	$(corbadir)/libCorbaCommon.la                                  \
	$(corbadir)/idl/libIDLAgent.la $(corbadir)/idl/libIDLCommon.la \
	$(corbadir)/idl/libIDLLA.la $(corbadir)/idl/libIDLMA.la        \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsNodes.la     \
	@LIBS_OMNIORB@ @LIBS_FAST@

AM_CPPFLAGS += -I$(top_srcdir)/src/utils \
	       -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

