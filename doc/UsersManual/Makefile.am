#****************************************************************************#
#* DIET User's Manual Makefile                                              *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Eddy CARON (Eddy.Caron@ens-lyon.fr)                                 *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.1  2003/09/09 12:38:20  pcombes
#* Reorganization of doc: UM becomes UsersManual.
#*
#* Revision 1.5  2003/05/23 09:25:15  pcombes
#* Add *~ in clean target.
#*
#* Revision 1.4  2003/05/15 14:17:43  pcombes
#* Enable compilaiton output in maintainer mode.
#*
#* Revision 1.3  2003/04/10 11:16:40  pcombes
#* Include in autotools chain ; fix compilation in a build directory.
#*
#* Revision 1.2  2003/02/19 09:32:24  cpera
#* Change dist rules for documentation.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

mod = $(shell basename `pwd`)


moddir = $(prefix)/doc
mod_DATA = DIET_$(mod).ps

TEX_FILES = $(notdir $(wildcard $(srcdir)/*.tex))
FIG_FILES = $(wildcard $(srcdir)/fig/*.fig)
EPS_FILES = $(foreach file, $(notdir $(FIG_FILES)), \
			    $(patsubst %.fig, fig/%.eps, $(file)))
GIF_FILES = $(wildcard $(srcdir)/fig/*.gif)
PS_FILES =  $(foreach file, $(notdir $(GIF_FILES)), \
			    $(patsubst %.gif, fig/%.ps, $(file)))

BIB_FILE = $(mod).bib

EXTRA_DIST = $(mod_DATA) $(TEX_FILES) $(BIB_FILE) $(FIG_FILES) $(GIF_FILES)

$(mod_DATA): $(mod).dvi
	dvips $(mod) -o $@ > /dev/null

if MAINTAINER_MODE
RED1  = 
RED12 = 
else
RED1  = > /dev/null
RED12 = > /dev/null 2>&1
endif

$(mod).dvi: $(EPS_FILES) $(PS_FILES) $(TEX_FILES) $(BIB_FILE)
	@(export TEXINPUTS=:.:$(srcdir):;		\
	  export BIBINPUTS=:.:$(srcdir):;		\
	  echo latex $(mod) $(RED1);			\
	  latex $(mod) $(RED1);				\
	  echo "(bibtex -terse $(mod) $(RED12) || exit 0)";\
	  (bibtex -terse $(mod) $(RED12) || exit 0);	\
	  latex $(mod) $(RED1);				\
	  latex $(mod) $(RED1))

.fig.eps:
	@if [ ! -d fig ]; then mkdir fig; fi
	fig2dev -L eps $^ $@

.gif.ps:
	@if [ ! -d fig ]; then mkdir fig; fi
	convert $^ $@


clean-local:
	rm -f $(mod_DATA) *~ *.ps fig/*.eps $(PS_FILES)		\
	      *.aux *.log *.dvi *.bbl *.blg *.toc *.lof *.lot
	@if [ "$(top_builddir)" != "$(top_srcdir)" ]; then 	\
	  echo rmdir fig;					\
	  rmdir fig > /dev/null 2>&1 || exit 0;			\
	fi

