#****************************************************************************#
#* $Id$ 		    *#
#* DIET agent Makefile                                                      *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Frederic   LOMBARD          - LIFC Besançon (France)                *#
#*    - Philippe   COMBES           - LIP ENS-Lyon (France)                 *#
#*    - Romain     LACROIX          - LIP ENS-Lyon (France)                 *#
#*    - Christophe PERA 	    - LIP ENS-Lyon (France)                 *#
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
#* Revision 1.8  2002/12/18 08:50:17  sdahan
#* the sonIdCounter, nbAgentSons, nbServerSons, reqIdCounter types change
#* from int to Counter which is a thread safe counter.
#*
#* Revision 1.7  2002/12/03 19:08:23  pcombes
#* Update configure, update to FAST 0.3.15, clean CVS logs in files.
#* Put main Makefile in root directory.
#*
#* Revision 1.4  2002/10/03 17:58:15  pcombes
#* Add trace levels (for Bert): traceLevel = n can be added in cfg files.
#* An agent son can now be killed (^C) without crashing this agent.
#* DIET with FAST: compilation is OK, but run time is still to be fixed.
#*
#****************************************************************************#

include $(top_srcdir)/config.mk.in

bin_PROGRAMS = lagent magent

magent_SOURCES =  agent_impl.cc \
		agent_impl.hh \
               	DataID.cc \
               	Request.cc \
        	MasterAgent_impl.cc \
		magent_main.cc

lagent_SOURCES =  LocalAgent_impl.cc \
		agent_impl.cc \
              	DataID.cc \
              	Request.cc \
		lagent_main.cc

noinst_HEADERS = agent_impl.hh \
		AgentDescription.hh \
		DataID.hh \
		MasterAgent_impl.hh \
		LocalAgent_impl.hh \
		Request.hh \
		RequestID.hh \
		SonID.hh

lagent_LDADD = 	$(top_builddir)/src/CORBA/libStaticCorbaTSA.la \
		$(top_builddir)/src/CORBA/libStaticCorbaLA.la \
		$(top_builddir)/src/CORBA/libcorba-utils.la \
		$(top_builddir)/src/utils/libUtilsSD.la \
		$(top_builddir)/src/utils/libUtilsDevel.la \
		$(top_builddir)/src/utils/libUtilsCM.la \
		$(LIBS_DIET_USED)

magent_LDADD = 	$(top_builddir)/src/CORBA/libStaticCorbaTSA.la \
		$(top_builddir)/src/CORBA/libStaticCorbaLA.la \
		$(top_builddir)/src/CORBA/libStaticCorbaMA.la \
		$(top_builddir)/src/CORBA/libcorba-utils.la \
		$(top_builddir)/src/utils/libUtilsSD.la \
		$(top_builddir)/src/utils/libUtilsDevel.la \
		$(top_builddir)/src/utils/libUtilsCM.la \
		$(LIBS_DIET_USED)

AM_CPPFLAGS = -I$(top_srcdir)/include \
		-I$(top_srcdir)/src/utils \
		-I$(top_srcdir)/src/CORBA \
		-I$(top_builddir)/src/CORBA \
		$(INCLUDE)
