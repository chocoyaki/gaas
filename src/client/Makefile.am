#****************************************************************************#
#* $Id$ 		    *#
#* DIET client Makefile                                                     *#
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
#* Revision 1.9  2002/12/03 19:08:24  pcombes
#* Update configure, update to FAST 0.3.15, clean CVS logs in files.
#* Put main Makefile in root directory.
#*
#* Revision 1.7  2002/11/25 10:31:24  lbertsch
#* Added statistics in client library
#*
#* Revision 1.4  2002/10/25 14:31:18  ecaron
#* FAST support: convertors implemented and compatible to --without-fast
#*               configure option, but still not tested with FAST !
#*
#****************************************************************************#

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_client.la

libDIET_client_la_SOURCES = $(top_srcdir)/include/DIET_client.h \
			DIET_client.cc

AM_CPPFLAGS =-I$(top_srcdir)/include \
		-I$(top_srcdir)/src/CORBA \
		-I$(top_builddir)/src/CORBA \
		-I$(top_srcdir)/src/utils \
		$(INCLUDE) 

libDIET_client_la_LIBADD = $(top_builddir)/src/utils/libUtilsDevel.la \
			$(top_builddir)/src/utils/libUtilsDdDm.la \
			$(top_builddir)/src/utils/libUtilsDemo.la \
			$(top_builddir)/src/utils/libUtilsServiceTable.la \
			$(top_builddir)/src/CORBA/libcorba-utils.la \
			$(top_builddir)/src/CORBA/libStaticCorbaTSA.la \
			$(top_builddir)/src/CORBA/libDynamicCorbaTSA.la \
			$(top_builddir)/src/CORBA/libStaticCorbaLA.la \
                        $(top_builddir)/src/CORBA/libStaticCorbaMA.la \
                        $(LIBS_DIET_USED) 
