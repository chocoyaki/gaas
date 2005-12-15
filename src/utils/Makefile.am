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
#* Revision 1.19  2005/12/15 10:12:17  pfrauenk
#* CoRI functionality added by Peter Frauenkron
#*
#* Revision 1.18  2005/08/31 15:05:24  alsu
#* New plugin scheduling interface: removal of lingering reference to
#* estVector.h
#*
#* Revision 1.17  2005/08/31 14:56:13  alsu
#* New plugin scheduling interface: removal of deprecated estVector.c
#* source, and addition of est_internal.cc (which provides the
#* DIET-internal interface to the new estimation vector)
#*
#* Revision 1.16  2005/07/13 09:07:30  rbolze
#* remove some JUXMEM_MODE tag and add estVector.h into noinst_HEADERS variable
#*
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
libUtilsSeDClt_la_SOURCES  = DIET_mutex.cc AccessController.cc \
                             est_internal.cc
libUtilsNodes_la_SOURCES   = Counter.cc ServiceTable.cc FASTMgr.cc \
			     DietLogComponent.cc MonitoringThread.cc 

if USING_CORI
libUtilsNodes_la_SOURCES += Cori_Fast.cc Cori_Data_Easy.cc \
			    Cori_Metric.cc Cori_Easy_CPU.cc \
			    Cori_Easy_Memory.cc Cori_Easy_Disk.cc\
			     CORIMgr.cc  
endif


libUtilsVector_la_SOURCES  = Vector.c
libJuxMemWrapper_la_SOURCES = JuxMemImpl.cc

noinst_HEADERS = Counter.hh debug.hh DIET_data_internal.hh \
		 FASTMgr.hh LinkedList.hh ms_function.hh Parsers.hh \
		 ReadersWriterLock.hh ServiceTable.hh ServiceTable.hh \
		 statistics.hh ts_container/ts_map.hh \
		 ts_container/ts_set.hh ts_container/ts_vector.hh \
		 DietLogComponent.hh MonitoringThread.hh KeyString.hh \
		 AccessController.hh Vector.h JuxMemImpl.hh 

if USING_CORI
noinst_HEADERS += Cori_Fast.hh Cori_Easy_CPU.hh  Cori_Easy_Disk.hh  \
		  Cori_Easy_Memory.hh Cori_Data_Easy.hh Cori_Metric.hh\
		 CORIMgr.hh 
endif


AM_CXXFLAGS = @JUXMEM_CXXFLAGS@ $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) \
	      -I$(top_srcdir)/src/CORBA -I$(top_builddir)/src/CORBA/idl
