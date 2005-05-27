#****************************************************************************#
#* DIET utils Makefile                                                      *#
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
#* Revision 1.15  2005/05/27 08:18:17  mjan
#* Moving JuxMem in a more appropriate place (src/utils)
#* Added log messages for VizDIET
#* Added use of JuxMem in the client side
#*
#* Revision 1.14  2005/04/08 09:54:50  hdail
#* Removing unused cichlid usage from DIET.
#*
#* Revision 1.13  2004/12/08 15:02:52  alsu
#* plugin scheduler first-pass validation testing complete.  merging into
#* main CVS trunk; ready for more rigorous testing.
#*
#* Revision 1.12  2004/11/25 08:59:09  rbolze
#* add AccessController.hh header
#*
#* Revision 1.11.2.2  2004/10/27 22:35:51  alsu
#* include
#*
#* Revision 1.11.2.1  2004/10/26 14:12:52  alsu
#* (Tag: AS-plugin-sched)
#*  - branch created to avoid conflicting with release 1.2 (imminent)
#*  - initial commit on branch, new dynamic performance info structure in
#*    the profile
#*
#* Revision 1.11  2004/10/04 13:55:06  hdail
#* - Added AccessController class, an enhanced counting semaphore.
#* - Added config file options for controlling concurrent SeD access.
#*
#* Revision 1.10  2004/09/29 13:35:32  sdahan
#* Add the Multi-MAs feature.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

noinst_LTLIBRARIES = libUtilsCommon.la libUtilsSeDClt.la \
		     libUtilsNodes.la libUtilsVector.la
if JUXMEM_MODE
noinst_LTLIBRARIES += libJuxMemWrapper.la
endif

libUtilsCommon_la_SOURCES  = debug.cc DIET_data.cc ms_function.cc \
			     Parsers.cc statistics.cc KeyString.cc
libUtilsSeDClt_la_SOURCES  = DIET_mutex.cc AccessController.cc
libUtilsNodes_la_SOURCES   = Counter.cc ServiceTable.cc FASTMgr.cc \
			     DietLogComponent.cc MonitoringThread.cc
libUtilsVector_la_SOURCES  = Vector.c estVector.c
if JUXMEM_MODE
libJuxMemWrapper_la_SOURCES = JuxMemImpl.cc
endif

noinst_HEADERS = Counter.hh debug.hh DIET_data_internal.hh \
		 FASTMgr.hh LinkedList.hh ms_function.hh Parsers.hh \
		 ReadersWriterLock.hh ServiceTable.hh ServiceTable.hh \
		 statistics.hh ts_container/ts_map.hh \
		 ts_container/ts_set.hh ts_container/ts_vector.hh \
		 DietLogComponent.hh MonitoringThread.hh KeyString.hh \
		 AccessController.hh Vector.h
if JUXMEM_MODE
noinst_HEADERS += JuxMemImpl.hh
endif

AM_CXXFLAGS = @JUXMEM_CXXFLAGS@ $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) \
	      -I$(top_srcdir)/src/CORBA -I$(top_builddir)/src/CORBA/idl
