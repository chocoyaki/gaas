#****************************************************************************#
#* DIET src Makefile                                                        *#
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
#* Revision 1.4  2004/07/09 19:51:58  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#*
#* Revision 1.3  2003/04/10 13:17:31  pcombes
#* Apply Coding Standards.
#*
#* Revision 1.2  2003/02/17 15:33:23  cpera
#* Change SUBMUDULES_diet in SUBMODULES_DIET
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

SUBDIRS = CORBA utils @SUBMODULES_src@ 
DIST_SUBDIRS = $(SUBDIRS) @DIST_SUBMODULES_src@

jardir = $(prefix)/lib
jar_DATA = lib/*.jar
