#****************************************************************************#
#* $Id$ *#
#* DIET Coding Standards Makefile                                           *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Eddy CARON (Eddy.Caron@ens-lyon.fr)                                 *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Christophe PERA (pera.christophe@ens-lyon.fr)                       *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#*
#* $Log$
#* Revision 1.2  2003/12/14 07:59:47  ecaron
#* Create DIET_tutorial.ps
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#*
#****************************************************************************#


#TOP = ../..
#INSTALL_DIR = @PREFIX@

#export MODULE = $(shell basename `pwd`)
#export TEX_FILES = $(shell ls *.tex)
#export PS_FILES  = ../UsersManual/fig/logo_DIET.ps

MODULE = $(shell basename `pwd`)
TEX_FILES = $(shell ls *.tex)
PS_FILES  = ../UsersManual/fig/logo_DIET.ps

#export EPS_FILES = $(shell if [ -d fig ]; then ls fig/*.fig | sed "s/\.fig/\.eps/g"; fi)
#export BIB = $(shell ls | grep $(MODULE).bib)

TARGET = DIET_tutorial.ps

all: $(TARGET)

$(PS_FILES):../UsersManual/fig/logo_DIET.gif 

$(TARGET):	$(MODULE).dvi
	dvips $(MODULE) -o  $@

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

