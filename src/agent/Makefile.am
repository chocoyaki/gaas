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
#* Revision 1.4  2003/05/10 08:50:33  pcombes
#* New Parsers let us reunify the two executables into one dietAgent.
#*
#* Revision 1.3  2003/05/05 14:30:42  pcombes
#* FAST flags were missing ...
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

bin_PROGRAMS = dietAgent


dietAgent_SOURCES = dietAgent.cc AgentImpl.cc LocalAgentImpl.cc \
		    MasterAgentImpl.cc GlobalSchedulers.cc Schedulers.cc \
		    Request.cc qsort.cc

noinst_HEADERS = AgentImpl.hh LocalAgentImpl.hh MasterAgentImpl.hh ChildID.hh \
		 GlobalSchedulers.hh NodeDescription.hh Schedulers.hh \
		 Request.hh RequestID.hh

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

dietAgent_LDADD = \
	$(corbadir)/libCorbaCommon.la                                  \
	$(corbadir)/idl/libIDLAgent.la $(corbadir)/idl/libIDLCommon.la \
	$(corbadir)/idl/libIDLLA.la $(corbadir)/idl/libIDLMA.la        \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsNodes.la     \
	$(LIBS_OMNIORB) $(LIBS_FAST)

AM_CPPFLAGS += $(CXXFLAGS_FAST) -I$(top_srcdir)/src/utils \
	       -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

