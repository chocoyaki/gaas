#****************************************************************************#
#* DIET Programmer's Guide Makefile                                         *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*    - Christophe PERA (Christophe.Pera@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.1  2003/09/09 12:42:44  pcombes
#* Reorganization of doc: include CS in a programmer's guide.
#*
#* Revision 1.4  2003/04/10 11:14:16  pcombes
#* Include in autotools chain ; fix compilation in a build directory.
#*
#* Revision 1.3  2003/02/19 09:32:24  cpera
#* Change dist rules for documentation.
#*
#* Revision 1.2  2003/02/14 12:27:16  cpera
#* change spaces with tabs (make errors in Makefile target).
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

EXTRA_DIST = $(mod_DATA) $(TEX_FILES) $(FIG_FILES)

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
$(mod).dvi: $(EPS_FILES) $(PS_FILES) $(TEX_FILES)
	@if [ ! -d _tmp_ ]; then mkdir _tmp_; fi
	@sed -e "s/\$$@/\$$/g" $(srcdir)/$(mod).tex > _tmp_/$(mod).tex
	@(export TEXINPUTS=:.:$(srcdir):;	\
	  export BIBINPUTS=:.:$(srcdir):;	\
	  echo latex $(mod) $(RED1);		\
	  latex _tmp_/$(mod).tex $(RED1);	\
	  echo latex $(mod) $(RED1);		\
	  latex _tmp_/$(mod) $(RED1)		\
	  rm -rf _tmp_)

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
