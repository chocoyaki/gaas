#****************************************************************************#
#* $Id$ 		    *#
#* DIET CORBA Makefile                                                      *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Frederic     LOMBARD          - LIFC Besançon (France)              *#
#*    - Philippe     COMBES           - LIP ENS-Lyon (France)               *#
#*    - Romain       LACROIX          - LIP ENS-Lyon (France)               *#
#*    - Christophe   PERA             - LIP ENS-Lyon (France)               *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#*
#* $Log$
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#*
#* Revision 1.4  2002/12/03 19:08:23  pcombes
#* Update configure, update to FAST 0.3.15, clean CVS logs in files.
#* Put main Makefile in root directory.
#*
#****************************************************************************#

include $(top_srcdir)/config.mk.in
include ./idlRules.mk

# automake value to compile following files before everything else.
BUILT_SOURCES = agentSK.cc \
		agentDynSK.cc \
		agent.hh \
		masterAgentSK.cc \
		masterAgentDynSK.cc \
		masterAgent.hh \
		localAgentSK.cc \
		localAgentDynSK.cc \
		localAgent.hh \
		SeDSK.cc \
		SeDDynSK.cc \
		SeD.hh \
		typesSK.cc \
		typesDynSK.cc \
		types.hh


noinst_LTLIBRARIES = libStaticCorbaTSA.la \
		libDynamicCorbaTSA.la \
		libStaticCorbaMA.la \
		libStaticCorbaLA.la \
		libDynamicCorbaMA.la \
		libDynamicCorbaLA.la \
		libcorba-utils.la \
		libStaticCorbaA.la \
		libDynamicCorbaA.la


# defining libtool libraries sources
libcorba_utils_la_SOURCES = omniorb.cc \
                            omniorb.hh \
                            marshalling.cc \
                            marshalling.hh \
                            ORBMgr.cc \
                            ORBMgr.hh \
                            $(top_srcdir)/src/utils/dietTypes.hh

nodist_libStaticCorbaTSA_la_SOURCES=agentSK.cc \
				typesSK.cc \
				SeDSK.cc 
                                        
nodist_libDynamicCorbaTSA_la_SOURCES=agentDynSK.cc \
				typesDynSK.cc \
				SeDDynSK.cc
                                
nodist_libStaticCorbaMA_la_SOURCES=masterAgentSK.cc

nodist_libDynamicCorbaMA_la_SOURCES=masterAgentDynSK.cc

nodist_libStaticCorbaLA_la_SOURCES=localAgentSK.cc

nodist_libDynamicCorbaLA_la_SOURCES=localAgentDynSK.cc

nodist_libStaticCorbaA_la_SOURCES=agentSK.cc

nodist_libDynamicCorbaA_la_SOURCES=agentDynSK.cc

# automake include variables
AM_CPPFLAGS =-I$(top_builddir)/include \
		-I$(top_srcdir)/include \
		-I$(top_builddir)/src/utils \
		$(WARN_CFLAGS) \
		$(INCLUDE) 

# AUTOMAKE variable for listing include header files distributed
include_HEADERS = omniorb.hh marshalling.hh ORBMgr.hh

Makefile.am:./idlRules.mk
Makefile.in:./idlRules.mk
Makefile:./idlRules.mk

