#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.2  2005/04/29 10:36:37  eboix
#*     idl generated files are now taken into account.  --- Injay 2461
#*
#* Revision 1.1  2005/04/28 13:25:11  eboix
#*     Added cmake subdir configuration files. --- Injay 2461
#*
#****************************************************************************#

SET(AgentCommon_SOURCES
    AgentImpl.cc
    BindService.cc
    ExitClass.cc
    FloodRequest.cc
    FloodRequestsList.cc
    GlobalSchedulers.cc
    LocalAgentImpl.cc
    LocMgrImpl.cc
    MasterAgentImpl.cc
    ReferenceUpdateThread.cc
    Request.cc
    Schedulers.cc )

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/src/agent          # local dir .
  ${DIET_SOURCE_DIR}/src/CORBA          # for e.g. marshalling.hh
  ${DIET_BINARY_DIR}/include            # FIXME: must preceed source/include
                                        # to take cmake-generated DIET_config.h
  ${DIET_SOURCE_DIR}/include            # FIXME: for DIET_data.h 
  ${DIET_BINARY_DIR}/src/CORBA/idl      # FIXME: is this a lib ?
)

ADD_LIBRARY( AgentCommon ${AgentCommon_SOURCES} )

ADD_EXECUTABLE( dietAgent dietAgent.cc )
TARGET_LINK_LIBRARIES( dietAgent
  AgentCommon
  CorbaCommon
  IDLAgent
  IDLCommon
  IDLLA
  IDLMA
  UtilsCommon
  UtilsNodes
  UtilsVector
  ${OMNIORB4_LIBRARIES} )

### FIXME
### missing all the JXTA_MODE related stuff...
