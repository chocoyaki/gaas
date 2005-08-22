#****************************************************************************#
#* $Id$ *#
#* DIET tutorial Makefile                                                   *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Eddy CARON (Eddy.Caron@ens-lyon.fr)                                 *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#* $Log$
#* Revision 1.4  2005/08/22 08:09:17  ycaniou
#* Added the references that are cited in the Tutorial, thus
#* Added the Tutorial.bib file containing the references and
#* Modified the Makefile.am so that a bibtex command is run when the
#*   tutorial is compiled.
#*
#* Revision 1.3  2003/12/15 17:27:09  pcombes
#* Use PG' Makefile with almost no changes.
#*
#* Revision 1.2  2003/12/14 07:59:47  ecaron
#* Create DIET_tutorial.ps
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#


mod = $(shell basename `pwd`)

moddir = $(prefix)/doc
mod_DATA = DIET_$(mod).ps


TEX_FILES = $(notdir $(wildcard $(srcdir)/*.tex))
FIGDIR    = ../UsersManual/fig
FIG_FILES = $(wildcard $(srcdir)/fig/*.fig)
EPS_FILES = $(foreach file, $(notdir $(FIG_FILES)), \
			    $(patsubst %.fig, fig/%.eps, $(file)))
PS_FILES  = $(FIGDIR)/logo_DIET.ps

BIB_FILE = $(mod).bib

EXTRA_DIST = $(mod_DATA) $(TEX_FILES) $(BIB_FILE) $(FIG_FILES)

$(mod_DATA): $(mod).dvi
	dvips $(mod) -o $@

if MAINTAINER_MODE
RED1  = 
RED12 = 
else
RED1  = > /dev/null
RED12 = > /dev/null 2>&1
endif

# If the example of header had true CVS fields, they would be replaced at each
# commit. That is why $@Id$ and $@Log$ are used, which are to be replaced before
# the compilation. The source to compile is stored in directory _tmp_.
$(mod).dvi: $(EPS_FILES) $(PS_FILES) $(TEX_FILES) $(BIB_FILE)
	@(export TEXINPUTS=:.:$(srcdir):;		\
          export BIBINPUTS=:.:$(srcdir):;       \
	  echo latex $(mod) $(RED1);		\
	  latex $(mod).tex $(RED1);		\
	  echo latex $(mod) $(RED1);		\
	  echo "(bibtex -terse $(mod) $(RED12) || exit 0)";\
          (bibtex -terse $(mod) $(RED12) || exit 0);    \
	  latex $(mod) $(RED1);			\
	  latex $(mod) $(RED1))

.fig.eps:
	@if [ ! -d fig ]; then mkdir fig; fi
	fig2dev -L eps $^ $@

$(FIGDIR)/logo_DIET.ps: $(FIGDIR)/logo_DIET.gif
	@if [ ! -d $(FIGDIR) ]; then mkdir $(FIGDIR); fi
	convert $^ $@


clean-local:
	rm -f $(mod_DATA) *.aux *.log *.dvi *.ps $(PS_FILES)
	@if [ "$(top_builddir)" != "$(top_srcdir)" ]; then	\
	  echo rmdir $(FIGDIR);					\
	  rmdir $(FIGDIR) $(RED12) || exit 0;			\
	fi

