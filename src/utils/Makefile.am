#****************************************************************************#
#* $Id$ 		    *#
#* DIET utils Makefile                                                      *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Frederic   LOMBARD          - LIFC Besançon (France)                *#
#*    - Philippe   COMBES           - LIP ENS-Lyon (France)                 *#
#*    - Romain     LACROIX          - LIP ENS-Lyon (France)                 *#
#*    - Christophe PERA             - LIP ENS-Lyon (France)                 *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#*
#* $Log$
#* Revision 1.1  2003/02/14 10:31:30  cpera
#* add files used by autoconf/automake/libtool.
#*
#* Revision 1.8  2002/12/17 17:07:32  sdahan
#* Add a new thread safe CORBA::Long counter and a new thread safe
#* LinkedList objects.
#*
#* Revision 1.7  2002/12/03 19:08:24  pcombes
#* Update configure, update to FAST 0.3.15, clean CVS logs in files.
#* Put main Makefile in root directory.
#*
#* Revision 1.6  2002/11/22 13:36:12  lbertsch
#* Added alpha linux support
#* Added a package for statistics and some traces
#*
#****************************************************************************#

include $(top_srcdir)/config.mk.in

noinst_LTLIBRARIES = libUtilsDevel.la \
		libUtilsDemo.la \
		libUtilsCM.la \
		libUtilsSD.la \
		libUtilsDdDm.la \
		libUtilsServiceTable.la

libUtilsDdDm_la_SOURCES = DIET_data.cc \
			DIET_mutex.cc
                      
libUtilsDemo_la_SOURCES = com_tools.cc

libUtilsDevel_la_SOURCES = statistics.cc \
			debug.cc

libUtilsCM_la_SOURCES = ms_function.cc \
			Counter.cc

libUtilsSD_la_SOURCES = ServiceTable.cc \
			DIET_data.cc

libUtilsServiceTable_la_SOURCES = ServiceTable.cc

noinst_HEADERS = ms_function.hh \
		ServiceTable.hh \
		statistics.hh \
		dlist.hh \
		LinkedList.hh \
		dietTypes.hh \
		debug.hh \
		Counter.hh \
		./ts_container/ts_map.hh \
		./ts_container/ts_set.hh \
		./ts_container/ts_vector.hh \
		com_tools.hh

AM_CPPFLAGS =-I$(top_srcdir)/include \
		-I$(top_srcdir)/src/utils \
		$(WARN_CFLAGS) \
		-I$(top_builddir)/src/CORBA \
		$(INCLUDE)
