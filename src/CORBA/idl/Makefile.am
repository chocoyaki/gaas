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
#* Revision 1.3  2003/06/02 08:08:11  cpera
#* Beta version of asynchronize DIET API.
#*
#* Revision 1.2  2003/05/13 17:05:09  pcombes
#* Comment AM_CPPFLAGS - useless.
#*
#* Revision 1.1  2003/04/10 12:34:32  pcombes
#* IDL descriptions are back in their directory.
#****************************************************************************#

include $(top_srcdir)/config.mk.in
include ./idlRules.mk

# automake value to compile following files before everything else.
BUILT_SOURCES = AgentSK.cc       AgentDynSK.cc         Agent.hh \
		common_typesSK.cc common_typesDynSK.cc common_types.hh \
		MasterAgentSK.cc MasterAgentDynSK.cc   MasterAgent.hh \
		LocalAgentSK.cc  LocalAgentDynSK.cc    LocalAgent.hh \
		SeDSK.cc 	 SeDDynSK.cc	       SeD.hh \
		responseSK.cc    responseDynSK.cc      response.hh \
		callback.hh	callbackSK.cc		callbackDynSK.cc

noinst_LTLIBRARIES = libIDLCommon.la libIDLAgent.la libIDLLA.la libIDLMA.la

nodist_libIDLCommon_la_SOURCES = common_typesSK.cc common_typesDynSK.cc \
				 SeDSK.cc SeDDynSK.cc \
				 responseSK.cc responseDynSK.cc \
				 callbackSK.cc callbackDynSK.cc

nodist_libIDLAgent_la_SOURCES = AgentSK.cc       AgentDynSK.cc
nodist_libIDLLA_la_SOURCES    = LocalAgentSK.cc  LocalAgentDynSK.cc
nodist_libIDLMA_la_SOURCES    = MasterAgentSK.cc MasterAgentDynSK.cc

# automake include variables
#AM_CPPFLAGS += 

Makefile.am:./idlRules.mk
Makefile.in:./idlRules.mk
Makefile:./idlRules.mk

