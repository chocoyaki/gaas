#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.1  2005/04/28 13:25:10  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/CORBA/idl     # Current directory
)

ADD_LIBRARY( IDLAgent 
  AgentSK.cc
  AgentDynSK.cc
  DataMgrSK.cc
  DataMgrDynSK.cc
  LogComponentDynSK.cc
  LogComponentSK.cc
  LocMgrSK.cc
  LocMgrDynSK.cc
  LogTypesSK.cc
  LogTypesDynSK.cc )

ADD_LIBRARY( IDLCommon 
  CallbackDynSK.cc
  CallbackSK.cc
  common_typesSK.cc
  common_typesDynSK.cc
  responseDynSK.cc
  responseSK.cc
  SeDSK.cc
  SeDDynSK.cc )

ADD_LIBRARY( IDLLA
  LocalAgentSK.cc
  LocalAgentDynSK.cc )

ADD_LIBRARY( IDLMA
  MasterAgentSK.cc
  MasterAgentDynSK.cc )
