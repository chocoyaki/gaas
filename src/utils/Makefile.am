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
#* Revision 1.10  2004/09/29 13:35:32  sdahan
#* Add the Multi-MAs feature.
#*
#* Revision 1.9  2004/07/07 18:19:17  rbolze
#* correct a dummy bug : forgot \ at the end of a line :)
#*
#* Revision 1.8  2004/03/01 18:57:48  rbolze
#* change to take account of new  files for logservice
#*
#* Revision 1.7  2003/12/01 14:49:31  pcombes
#* Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
#*
#* Revision 1.6  2003/08/01 19:20:23  pcombes
#* Reorganize FAST and omniORB flags. Include FASTMgr.
#*
#* Revision 1.5  2003/06/24 16:04:18  cpera
#* Update ReaderWriter file name.
#*
#* Revision 1.4  2003/06/18 07:33:48  pcombes
#* Add files for management of asynchronism into distribution (noinst_HEADERS)
#*
#* Revision 1.3  2003/05/14 11:53:44  ecaron
#* update to take into account the statistic module in each DIET component
#*
#* Revision 1.2  2003/04/10 12:53:15  pcombes
#* Add Parsers ; reorganize libraries.
#*
#* Revision 1.1  2003/02/14 10:31:30  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

noinst_LTLIBRARIES = libUtilsCichlid.la libUtilsCommon.la \
		     libUtilsSeDClt.la  libUtilsNodes.la

libUtilsCichlid_la_SOURCES = com_tools.cc
libUtilsCommon_la_SOURCES  = debug.cc DIET_data.cc ms_function.cc \
			     Parsers.cc statistics.cc KeyString.cc
libUtilsSeDClt_la_SOURCES  = DIET_mutex.cc
libUtilsNodes_la_SOURCES   = Counter.cc ServiceTable.cc FASTMgr.cc \
			     DietLogComponent.cc MonitoringThread.cc

noinst_HEADERS = com_tools.hh Counter.hh debug.hh DIET_data_internal.hh \
		 FASTMgr.hh LinkedList.hh ms_function.hh Parsers.hh \
		 ReadersWriterLock.hh ServiceTable.hh ServiceTable.hh \
		 statistics.hh ts_container/ts_map.hh \
		 ts_container/ts_set.hh ts_container/ts_vector.hh \
		 DietLogComponent.hh MonitoringThread.hh KeyString.hh


AM_CXXFLAGS = $(CXXFLAGS_OMNIORB) $(CXXFLAGS_FAST) \
	      -I$(top_srcdir)/src/CORBA -I$(top_builddir)/src/CORBA/idl
