#****************************************************************************#
#* $Id$                  *#
#* DIET SeD Makefile                                                        *#
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
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#*
#* Revision 1.5  2002/12/18 19:02:46  pcombes
#* Bug fix in statistics integration.
#*
#* Revision 1.4  2002/12/03 19:08:23  pcombes
#* Update configure, update to FAST 0.3.15, clean CVS logs in files.
#* Put main Makefile in root directory.
#*
#****************************************************************************#

#AUTOMAKE_OPTIONS = gnu

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_SeD.la

libDIET_SeD_la_SOURCES = $(top_srcdir)/include/DIET_server.h \
			SeD_impl.cc \
                        DIET_server.cc

noinst_HEADERS = $(top_srcdir)/src/SeD/SeD_impl.hh

AM_CPPFLAGS = -I$(top_srcdir)/include \
		-I$(top_srcdir)/src/utils \
		-I$(top_srcdir)/src/CORBA \
		-I$(top_builddir)/src/CORBA \
		$(INCLUDE) 

libDIET_SeD_la_LIBADD = $(top_builddir)/src/utils/libUtilsDevel.la \
			$(top_builddir)/src/utils/libUtilsDdDm.la \
			$(top_builddir)/src/utils/libUtilsServiceTable.la \
			$(top_builddir)/src/CORBA/libcorba-utils.la \
			$(top_builddir)/src/CORBA/libStaticCorbaTSA.la \
			$(top_builddir)/src/CORBA/libDynamicCorbaTSA.la \
			$(LIBS_DIET_USED)

