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
#* Revision 1.18  2006/01/05 07:45:17  mjan
#* Fix bug reported by Eddy.
#*
#* Revision 1.17  2005/08/30 09:20:20  ycaniou
#* Corrected things in DIET_server.cc (diet_submit_batch...)
#* Link libDIET with Elagi and Appleseeds only if BATCH is asked in the
#*   configuration (corrected the Makefile)
#* Changed things in SeDImpl.[ch] for batch submission to work. Only synchronous
#*   mode made.
#*
#* Revision 1.16  2005/05/27 08:18:17  mjan
#* Moving JuxMem in a more appropriate place (src/utils)
#* Added log messages for VizDIET
#* Added use of JuxMem in the client side
#*
#* Revision 1.15  2005/05/18 14:18:09  mjan
#* Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
#*
#* Revision 1.14  2005/05/02 16:46:33  ycaniou
#* Added the function diet_submit_batch(), the stuff in the makefile to compile
#*  with appleseeds..
#*
#* Revision 1.13  2005/04/27 01:41:34  ycaniou
#* Added the stuff for a correct compilation, for a correct registration of
#* a batch profile, and for its execution.
#* Added the solve_batch() function
#*
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
	$(corbadir)/idl/libIDLAgent.la $(LIBS_OMNIORB) $(LIBS_FAST)

if USE_BATCH
libDIET_SeD_la_LIBADD += $(ELAGI_LIBS) $(AS_LIBS)
endif

if JUXMEM_MODE
libDIET_SeD_la_LIBADD += $(utilsdir)/libJuxMemWrapper.la
endif

AM_CXXFLAGS = @JUXMEM_CXXFLAGS@ $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) -I$(top_srcdir)/src/utils \
	      -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl 		     \
	      -I$(top_srcdir)/src/agent

if USE_BATCH
include $(APPLESEEDS_DIR)/include/appleseeds/appleseeds.make
include $(ELAGI_DIR)/include/elagi/elagi.make
AM_CPPFLAGS += $(ELAGI_INC) $(AS_INC)
endif

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
