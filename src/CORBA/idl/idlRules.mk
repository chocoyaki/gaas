#****************************************************************************#
#* common Makefile rules and variables for IDL                              *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Christophe PERA (Christophe.Pera@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.6  2003/09/22 21:06:48  pcombes
#* Clean useless targets. Add modules for data persistency.
#*
#* Revision 1.5  2003/09/19 07:48:57  bdelfabr
#* adding locMgr.idl and dataMgr.idl files to idlRules
#*
#* Revision 1.4  2003/07/04 09:47:56  pcombes
#* Indentation. Remove useless comments.
#*
#* Revision 1.2  2003/06/02 08:08:11  cpera
#* Beta version of asynchronize DIET API.
#*
#* Revision 1.1  2003/04/10 12:34:32  pcombes
#* IDL descriptions are back in their directory.
#****************************************************************************#

# IDL source files
IDL_FILES = common_types.idl Callback.idl response.idl \
	    Agent.idl MasterAgent.idl LocalAgent.idl SeD.idl \
	    DataMgr.idl LocMgr.idl

# compiling and dependency rules for the files generated from IDL
%.hh %SK.cc %DynSK.cc: %.idl $(IDL_COMPILER)
	$(IDL_COMPILER) $(IDLFLAGS) $<
# FIXME: The following two lines are a trick for omniORB 4 compiled with
#        PYTHON 2.3.
#	 It should be detected by configure, since omniORB creates a pythonX.X
#	 directory in the omniORB libraries directory.
	sed -e "s/False/false/g" $(basename $(notdir $<))SK.cc > tmp.cc
	@mv tmp.cc $(basename $(notdir $<))SK.cc
	sed -e "s/False/false/g" $(basename $(notdir $<))DynSK.cc > tmp.cc
	@mv tmp.cc $(basename $(notdir $<))DynSK.cc

EXTRA_DIST = $(IDL_FILES)

# AUTOMAKE variables for cleaning generated idl files  
CLEANFILES =$(foreach file,$(basename $(IDL_FILES)),$(patsubst %,%.hh,$(file)))
CLEANFILES+=$(foreach file,$(basename $(IDL_FILES)),$(patsubst %,%SK.cc,$(file)))
CLEANFILES+=$(foreach file,$(basename $(IDL_FILES)),$(patsubst %,%DynSK.cc,$(file)))

