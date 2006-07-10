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
#* Revision 1.11  2006/07/10 09:01:43  aamar
#* Adding CltReoMan and WfLogService compilation rules.
#*
#* Revision 1.10  2006/04/14 14:09:14  aamar
#* Adding the workflow support compilation rules.
#*
#* Revision 1.9  2004/03/01 19:07:27  rbolze
#* change to take into account the new idl files for logservice
#*
#* Revision 1.8  2003/09/22 21:06:42  pcombes
#* Clean SOURCES variables. Add modules for data persistency.
#*
#* Revision 1.3  2003/06/02 08:08:11  cpera
#* Beta version of asynchronize DIET API.
#*
#* Revision 1.1  2003/04/10 12:34:32  pcombes
#* IDL descriptions are back in their directory.
#****************************************************************************#

include $(top_srcdir)/config.mk.in
include idlRules.mk

# automake value to compile following files before everything else.
BUILT_SOURCES = AgentSK.cc	  AgentDynSK.cc		\
		CallbackSK.cc	  CallbackDynSK.cc	\
		common_typesSK.cc common_typesDynSK.cc	\
		DataMgrSK.cc	  DataMgrDynSK.cc	\
		LocalAgentSK.cc	  LocalAgentDynSK.cc	\
		LocMgrSK.cc	  LocMgrDynSK.cc	\
		MasterAgentSK.cc  MasterAgentDynSK.cc	\
		responseSK.cc	  responseDynSK.cc	\
		SeDSK.cc	  SeDDynSK.cc		\
		LogTypesSK.cc	  LogTypesDynSK.cc	\
		LogComponentSK.cc LogComponentDynSK.cc

if USING_WORKFLOW
BUILT_SOURCES += MaDagSK.cc	  MaDagDynSK.cc \
		WfLogServiceSK.cc WfLogServiceDynSK.cc
endif

noinst_LTLIBRARIES = libIDLCommon.la libIDLAgent.la libIDLLA.la libIDLMA.la

if USING_WORKFLOW
noinst_LTLIBRARIES += libIDLMaDag.la libIDLWfLog.la
endif

nodist_libIDLCommon_la_SOURCES = common_typesSK.cc common_typesDynSK.cc \
				 SeDSK.cc SeDDynSK.cc 			\
				 responseSK.cc responseDynSK.cc 	\
				 CallbackSK.cc CallbackDynSK.cc

if USING_WORKFLOW
nodist_libIDLCommon_la_SOURCES += CltReoManSK.cc CltReoManDynSK.cc
endif

nodist_libIDLAgent_la_SOURCES = AgentSK.cc       AgentDynSK.cc		\
				DataMgrSK.cc     DataMgrDynSK.cc	\
				LocMgrSK.cc	 LocMgrDynSK.cc		\
		 		LogTypesSK.cc	 LogTypesDynSK.cc	\
				LogComponentSK.cc LogComponentDynSK.cc

nodist_libIDLLA_la_SOURCES    = LocalAgentSK.cc  LocalAgentDynSK.cc
nodist_libIDLMA_la_SOURCES    = MasterAgentSK.cc MasterAgentDynSK.cc

if USING_WORKFLOW
nodist_libIDLMaDag_la_SOURCES = MaDagSK.cc MaDagDynSK.cc
nodist_libIDLWfLog_la_SOURCES = WfLogServiceSK.cc WfLogServiceDynSK.cc
endif

# automake include variables
AM_CXXFLAGS = $(CXXFLAGS_OMNIORB) #-Wno-reorder

