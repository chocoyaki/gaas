#****************************************************************************#
#* DIET client Makefile                                                     *#
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
#* Revision 1.11  2006/01/13 10:40:53  mjan
#* Updating DIET for next JuxMem (0.2)
#*
#* Revision 1.10  2005/05/27 08:18:17  mjan
#* Moving JuxMem in a more appropriate place (src/utils)
#* Added log messages for VizDIET
#* Added use of JuxMem in the client side
#*
#* Revision 1.9  2005/04/13 08:49:54  hdail
#* Cleaning up references to libUtilsCichlid.la.
#*
#* Revision 1.8  2004/12/08 15:02:52  alsu
#* plugin scheduler first-pass validation testing complete.  merging into
#* main CVS trunk; ready for more rigorous testing.
#*
#* Revision 1.7.2.1  2004/11/02 00:38:08  alsu
#* providing clients access to estVector_t structure
#*
#* Revision 1.7  2003/09/16 10:35:58  pcombes
#* Remove .hh files from libDIET_client_la_SOURCES.
#*
#* Revision 1.6  2003/08/01 19:20:23  pcombes
#* Reorganize FAST and omniORB flags. Include FASTMgr.
#*
#* Revision 1.5  2003/07/25 20:37:36  pcombes
#* Separate the DIET API (slightly modified) from the GridRPC API (version of
#* the draft dated to 07/21/2003)
#*
#* Revision 1.4  2003/06/18 07:33:48  pcombes
#* Add files for management of asynchronism into distribution (noinst_HEADERS)
#*
#* Revision 1.3  2003/06/02 08:09:55  cpera
#* Beta version of asynchronize DIET API.
#*
#* Revision 1.2  2003/04/10 13:12:38  pcombes
#* Update to new utils and CORBA AM libraries.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

include $(top_srcdir)/config.mk.in

lib_LTLIBRARIES = libDIET_client.la

libDIET_client_la_SOURCES = DIET_client.cc DIET_grpc.cc         \
			    CallAsyncMgr.cc CallbackImpl.cc

noinst_HEADERS = CallAsyncMgr.hh CallbackImpl.hh

instdir = $(sysconfdir)

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

libDIET_client_la_LIBADD = \
	$(utilsdir)/libUtilsCommon.la $(utilsdir)/libUtilsSeDClt.la	\
	$(utilsdir)/libUtilsVector.la	\
	$(corbadir)/libCorbaCommon.la $(corbadir)/idl/libIDLAgent.la	\
	$(corbadir)/idl/libIDLCommon.la $(corbadir)/idl/libIDLMA.la	\
	$(LIBS_OMNIORB)

AM_CXXFLAGS = @JUXMEM_CXXFLAGS@ $(CXXFLAGS_OMNIORB) -I$(top_srcdir)/src/utils \
	      -I$(top_srcdir)/src/CORBA -I$(corbadir)/idl

