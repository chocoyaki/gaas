#****************************************************************************#
#* jxta dmat_manips example: Makefile                                       *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Cedric TEDESCHI (Cedric.Tedeschi@insa-lyon.fr)                      *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#

include $(top_srcdir)/config.mk.in

utilsdir = $(top_builddir)/src/utils
corbadir = $(top_builddir)/src/CORBA

EXTRA_DIST = JXTAClient.java scripts

all:  JXTAClient.class

JXTAClient.class: JXTAClient.java
	$(JAVAC) -classpath .:$(top_srcdir)/src/lib/jxta.jar JXTAClient.java

clean:
	rm -f *.class

