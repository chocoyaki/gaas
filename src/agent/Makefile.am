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
#* Revision 1.11  2004/07/09 19:51:58  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#*
#* Revision 1.10  2004/07/08 17:47:51  rbolze
#* add java files to the distrib
#*
#* Revision 1.9  2004/06/11 15:45:39  ctedesch
#* add DIET/JXTA
#*
#* Revision 1.8  2003/12/15 17:25:12  pcombes
#* Add ExitClass, without compilation bugs...
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
lib_LTLIBRARIES = libAgentCommon.la
if JXTA_MODE
lib_LTLIBRARIES += libdietMasterAgent.la libdietLocalAgent.la
#bin_PROGRAMS +=JXTAMultiMA.class LA.class 
endif

libAgentCommon_la_SOURCES = AgentImpl.cc ExitClass.cc LocalAgentImpl.cc	\
			LocMgrImpl.cc MasterAgentImpl.cc GlobalSchedulers.cc	\
		    	Schedulers.cc Request.cc qsort.cc

dietAgent_SOURCES = dietAgent.cc

noinst_HEADERS = AgentImpl.hh ExitClass.hh LocalAgentImpl.hh LocMgrImpl.hh	\
		 MasterAgentImpl.hh ChildID.hh GlobalSchedulers.hh		\
		 NodeDescription.hh Schedulers.hh Request.hh RequestID.hh

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

dietAgent_LDADD = \
	libAgentCommon.la	\
	$(utilsdir)/libUtilsNodes.la $(utilsdir)/libUtilsCommon.la     \
	$(corbadir)/libCorbaCommon.la                                  \
	$(corbadir)/idl/libIDLAgent.la $(corbadir)/idl/libIDLCommon.la \
	$(corbadir)/idl/libIDLLA.la $(corbadir)/idl/libIDLMA.la        \
	$(LIBS_OMNIORB) $(LIBS_FAST)

AM_CXXFLAGS = $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) -I$(top_srcdir)/src/utils \
	      -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

if JXTA_MODE

AM_CPPFLAGS += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

libdietMasterAgent_la_SOURCES = masterAgentJNI.cc
libdietMasterAgent_la_DEPENDENCIES = JXTAMultiMA.h
libdietMasterAgent_la_LIBADD = $(dietAgent_LDADD)

libdietLocalAgent_la_SOURCES = localAgentJNI.cc
libdietLocalAgent_la_DEPENDENCIES = LA.h
libdietLocalAgent_la_LIBADD = $(dietAgent_LDADD)

masterAgentJNI.cc: JXTAMultiMA.h
JXTAMultiMA.h: JXTAMultiMA.class
	$(JAVAH) JXTAMultiMA
JXTAMultiMA.class: JXTAMultiMA.java
	$(JAVAC) -classpath .:$(top_srcdir)/src/lib/jxta.jar JXTAMultiMA.java

localAgentJNI.cc: LA.h
LA.h: LA.class
	$(JAVAH) LA
LA.class: LA.java
	$(JAVAC) LA.java

clean:
	rm -f *.class JXTAMultiMA.h LA.h

endif

EXTRA_DIST = *.java
