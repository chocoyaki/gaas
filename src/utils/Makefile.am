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
libUtilsCommon_la_SOURCES  = debug.cc DIET_data.cc ms_function.cc Parsers.cc	\
			     statistics.cc
libUtilsSeDClt_la_SOURCES  = DIET_mutex.cc
libUtilsNodes_la_SOURCES   = Counter.cc ServiceTable.cc

noinst_HEADERS = com_tools.hh Counter.hh debug.hh dietTypes.hh			\
		 LinkedList.hh ms_function.hh Parsers.hh ReadersWritersLock.hh	\
		 ServiceTable.hh ServiceTable.hh statistics.hh			\
		 ts_container/ts_map.hh	ts_container/ts_set.hh			\
		 ./ts_container/ts_vector.hh

AM_CPPFLAGS += -I$(top_builddir)/src/CORBA -I$(top_builddir)/src/CORBA/idl
