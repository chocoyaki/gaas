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
#* Revision 1.3  2003/06/02 14:51:54  cpera
#* Applying coding standards on Callback.idl.
#*
#* Revision 1.2  2003/06/02 08:08:11  cpera
#* Beta version of asynchronize DIET API.
#*
#* Revision 1.1  2003/04/10 12:34:32  pcombes
#* IDL descriptions are back in their directory.
#*
#****************************************************************************#

# IDL source files
IDL_FILES = common_types.idl Callback.idl response.idl \
	    Agent.idl MasterAgent.idl LocalAgent.idl SeD.idl

# idl dependancies
AgentSK.cc AgentDynSK.cc Agent.hh: \
	common_types.hh response.hh SeD.hh
MasterAgentSK.cc MasterAgentDynSK.cc MasterAgent.hh: Agent.hh
LocalAgentSK.cc LocalAgentDynSK.cc LocalAgent.hh:    Agent.hh
SeDSK.cc SeDDynSK.cc SeD.hh:                         common_types.hh Callback.hh
responseSK.cc responseDynSK.cc response.hh:          SeD.hh
Callback.hh CallbackSK.cc CallbackDynSK.cc : 		common_types.hh	


# compiling and dependency rules for idl files
%.hh %SK.cc %DynSK.cc: %.idl $(IDL_COMPILER)
# FIXME: Why not -(rm -f $@) ???
#	@rm -f $(basename $(notdir $<)).hh   \
#	       $(basename $(notdir $<))SK.cc \
#	       $(basename $(notdir $<))DynSK.cc || true > /dev/null
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

