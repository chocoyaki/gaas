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
#* Revision 1.12  2005/04/11 20:21:27  ycaniou
#* Allow the compilation of a elagi Batch enabled SeD
#*
#* Revision 1.11  2004/12/08 15:02:51  alsu
#* plugin scheduler first-pass validation testing complete.  merging into
#* main CVS trunk; ready for more rigorous testing.
#*
#* Revision 1.10.2.1  2004/10/27 22:35:50  alsu
#* include
#*
#* Revision 1.10  2004/07/09 19:51:58  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#*
#* Revision 1.9  2004/07/08 20:16:30  rbolze
#* add dmat_jxta_prods.h and remplace EXTA by EXTRA
#*
#* Revision 1.8  2004/07/08 17:47:51  rbolze
#* add java files to the distrib
#*
#* Revision 1.7  2004/06/11 15:45:39  ctedesch
#* add DIET/JXTA
#*
#* Revision 1.6  2003/09/22 21:07:30  pcombes
#* Add files for data persistency.
#*
#* Revision 1.4  2003/08/01 19:20:23  pcombes
#* Reorganize FAST and omniORB flags. Include FASTMgr.
#*
#* Revision 1.2  2003/04/10 13:17:05  pcombes
#* Update to new utils and CORBA AM libraries.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

if USE_BATCH
   include $(ELAGI_DIR)/include/elagi/elagi.make
endif

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_SeD.la

if JXTA_MODE
lib_LTLIBRARIES += libSeDJNI.la
#bin_PROGRAMS = JXTASeD.class
endif

libDIET_SeD_la_SOURCES = DIET_server.cc SeDImpl.cc DataMgrImpl.cc

noinst_HEADERS = SeDImpl.hh DataMgrImpl.hh dmat_jxta_progs.h 

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

libDIET_SeD_la_LIBADD = \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsSeDClt.la	\
	$(utilsdir)/libUtilsNodes.la $(utilsdir)/libUtilsVector.la	\
	$(corbadir)/libCorbaCommon.la $(corbadir)/idl/libIDLCommon.la	\
	$(corbadir)/idl/libIDLAgent.la $(LIBS_OMNIORB) $(LIBS_FAST) \
	$(ELAGI_LIBS)
	
AM_CXXFLAGS = $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) -I$(top_srcdir)/src/utils \
	      -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl 		     \
	      -I$(top_srcdir)/src/agent


if JXTA_MODE

AM_CPPFLAGS += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

libSeDJNI_la_SOURCES = $(top_srcdir)/include/DIET_server.h SeDJNI.cc SeDImpl.cc \
	DataMgrImpl.cc
libSeDJNI_la_DEPENDENCIES = JXTASeD.h
libSeDJNI_la_LIBADD = $(libDIET_SeD_la_LIBADD)

SeDJNI.cc: JXTASeD.h
JXTASeD.h: JXTASeD.class
	$(JAVAH) JXTASeD

JXTASeD.class: JXTASeD.java
	$(JAVAC) -classpath .:$(top_srcdir)/src/lib/jxta.jar JXTASeD.java

clean:
	rm -f *.class JXTASeD.h

endif

EXTRA_DIST = *.java
