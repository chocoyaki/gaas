#****************************************************************************#
#* DIET CORBA IDL Makefile                                                  *#
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
#* Revision 1.6  2003/08/01 19:20:22  pcombes
#* Reorganize FAST and omniORB flags. Include FASTMgr.
#*
#* Revision 1.5  2003/07/04 09:47:56  pcombes
#* Indentation. Remove useless comments.
#*
#* Revision 1.3  2003/06/02 08:08:11  cpera
#* Beta version of asynchronize DIET API.
#*
#* Revision 1.1  2003/04/10 12:34:32  pcombes
#* IDL descriptions are back in their directory.
#****************************************************************************#

include $(top_srcdir)/config.mk.in
include ./idlRules.mk

# automake value to compile following files before everything else.
BUILT_SOURCES = AgentSK.cc       AgentDynSK.cc         Agent.hh \
		CallbackSK.cc    CallbackDynSK.cc      Callback.hh \
		common_typesSK.cc common_typesDynSK.cc common_types.hh \
		LocalAgentSK.cc  LocalAgentDynSK.cc    LocalAgent.hh \
		MasterAgentSK.cc MasterAgentDynSK.cc   MasterAgent.hh \
		responseSK.cc    responseDynSK.cc      response.hh \
		SeDSK.cc 	 SeDDynSK.cc	       SeD.hh

noinst_LTLIBRARIES = libIDLCommon.la libIDLAgent.la libIDLLA.la libIDLMA.la

nodist_libIDLCommon_la_SOURCES = common_typesSK.cc common_typesDynSK.cc \
				 SeDSK.cc SeDDynSK.cc \
				 responseSK.cc responseDynSK.cc \
				 CallbackSK.cc CallbackDynSK.cc

nodist_libIDLAgent_la_SOURCES = AgentSK.cc       AgentDynSK.cc
nodist_libIDLLA_la_SOURCES    = LocalAgentSK.cc  LocalAgentDynSK.cc
nodist_libIDLMA_la_SOURCES    = MasterAgentSK.cc MasterAgentDynSK.cc

# automake include variables
AM_CXXFLAGS = -Wno-reorder $(CXXFLAGS_OMNIORB)

Makefile.am:./idlRules.mk
Makefile.in:./idlRules.mk
Makefile:./idlRules.mk

