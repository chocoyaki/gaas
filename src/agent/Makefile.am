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
#* Revision 1.7  2003/10/06 10:15:38  cpontvie
#* Adding the ExitClass to the compile process
#*
#* Revision 1.6  2003/09/22 21:08:00  pcombes
#* Add files for data persistency.
#*
#* Revision 1.5  2003/08/01 19:20:23  pcombes
#* Reorganize FAST and omniORB flags. Include FASTMgr.
#*
#* Revision 1.4  2003/05/10 08:50:33  pcombes
#* New Parsers let us reunify the two executables into one dietAgent.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

bin_PROGRAMS = dietAgent


dietAgent_SOURCES = dietAgent.cc AgentImpl.cc LocalAgentImpl.cc \
		    MasterAgentImpl.cc GlobalSchedulers.cc Schedulers.cc \
		    Request.cc qsort.cc LocMgrImpl.cc ExitClass.cc

noinst_HEADERS = AgentImpl.hh LocalAgentImpl.hh MasterAgentImpl.hh ChildID.hh \
		 GlobalSchedulers.hh NodeDescription.hh Schedulers.hh \
		 Request.hh RequestID.hh LocMgrImpl.hh ExitClass.hh

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

dietAgent_LDADD = \
	$(utilsdir)/libUtilsNodes.la $(utilsdir)/libUtilsCommon.la       \
	$(corbadir)/libCorbaCommon.la $(corbadir)/idl/libIDLAgent.la     \
  $(corbadir)/idl/libIDLCommon.la $(corbadir)/idl/libIDLLA.la      \
  $(corbadir)/idl/libIDLMA.la                                      \
	$(LIBS_OMNIORB) $(LIBS_FAST)

AM_CXXFLAGS = $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) -I$(utilsdir) \
              -I$(corbadir) -I$(corbadir)/idl
