#****************************************************************************#
#* $Id$ *#
#* DIET Coding Standards Makefile                                           *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES           - LIP ENS-Lyon (France)                   *#
#*    - Christophe PERA           - LIP ENS-Lyon (France)                   *#
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
#* Revision 1.3  2003/01/21 12:16:03  pcombes
#* Hide latex compilation logs
#*
#* Revision 1.2  2003/01/13 12:09:00  pcombes
#* UM: client part complete for users's day ...
#*
#* Revision 1.1.1.1  2003/01/07 18:06:18  pcombes
#* Add Coding Standards for DIET.
#*
#****************************************************************************#


#TOP = ../..
#INSTALL_DIR = @PREFIX@

#export MODULE = $(shell basename `pwd`)
#export TEX_FILES = $(shell ls *.tex)
#export PS_FILES  = ../UM/fig/logo_DIET.ps

MODULE = $(shell basename `pwd`)
TEX_FILES = $(shell ls *.tex)
PS_FILES  = ../UM/fig/logo_DIET.ps

#export EPS_FILES = $(shell if [ -d fig ]; then ls fig/*.fig | sed "s/\.fig/\.eps/g"; fi)
#export BIB = $(shell ls | grep $(MODULE).bib)

TARGET = Diet_Coding_Standard.ps

all:		$(TARGET)

$(PS_FILES):../UM/fig/logo_DIET.fig

#EXTRA_DIST = tutorial.tex  

$(TARGET):	$(MODULE).dvi
	dvips $(MODULE) -o

$(MODULE).dvi:	$(PS_FILES) $(TEX_FILES) # $(EPS_FILES) $(BIB)
	latex $(MODULE) > /dev/null
#	bibtex -terse $(MODULE) || exit 0
#	latex $(MODULE)
	latex $(MODULE) > /dev/null

#%.eps:	%.fig
#	fig2dev -L eps $< $@

%.ps:	%.gif
	convert $^ $@

clean-local:
	rm -f *.aux *.log *.dvi *.bbl *.blg *.toc *.lof *.lot *.ps *.pdf
distclean-local:clean-local

#install-data-local-diet: $(TARGET)
#	[ -f $(TARGET) ] || (echo "No $(MODULE) to install"); \
#  $(INSTALL) -d ${prefix}/doc; \
#	if [ $(TARGET) -nt ${prefix}/doc/$(TARGET) ]; then \
#	  $(INSTALL_DATA) $(TARGET) ${prefix}/doc; \
#	fi
#install-data-local:install-data-local-diet

#uninstall-local-diet:
#	rm -f ${prefix}/doc/$(TARGET)
#	if [ -d ${prefix}/doc ]; then                    \
#	  rmdir --ignore-fail-on-non-empty ${prefix}/doc;\
#	fi
#
#uninstall-local:uninstall-local-diet
