#****************************************************************************#
#* DIET cmake local file                                                    *#
#*                                                                          *#
#****************************************************************************#
#* $Author$
#* $LICENSE$
#* $Id$
#* $Log$
#* Revision 1.3  2005/04/29 16:18:02  eboix
#*     First example of JXTA dependant compilation.  --- Injay 2461
#*
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
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/src/agent          # local dir .
  ${DIET_BINARY_DIR}/include            # for e.g. DIET_config.h
  ${DIET_SOURCE_DIR}/include

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
  ${OMNIORB4_LIBRARIES}
)

IF( DIET_USE_JXTA )
  SET( TARGET_JAVA_DIR ${DIET_BINARY_DIR}/src/agent )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${DIET_BINARY_DIR}/src/agent/LA.class
    COMMAND   ${JAVA_COMPILE}
    ARGS      -d ${DIET_BINARY_DIR}/src/agent
              ${DIET_SOURCE_DIR}/src/agent/LA.java
    DEPENDS   ${DIET_SOURCE_DIR}/src/agent/LA.java
    COMMENT   "java compile"
  )
  ADD_CUSTOM_COMMAND(
    OUTPUT    ${DIET_BINARY_DIR}/src/agent/LA.h
    COMMAND   ${JAVA_HEADER_COMPILE}
    ARGS      -classpath ${DIET_BINARY_DIR}/src/agent
              -d ${DIET_BINARY_DIR}/src/agent
              LA
    DEPENDS   ${DIET_BINARY_DIR}/src/agent/LA.class
    COMMENT   "java header compile"
  )
  SET_SOURCE_FILES_PROPERTIES( 
    ${DIET_SOURCE_DIR}/src/agent/localAgentJNI.cc
    PROPERTIES
    OBJECT_DEPENDS ${DIET_BINARY_DIR}/src/agent/LA.h
  )

  INCLUDE_DIRECTORIES( ${DIET_BINARY_DIR}/src/agent )  # For LA.h ...
  ADD_LIBRARY( dietLocalAgent ${DIET_SOURCE_DIR}/src/agent/localAgentJNI.cc )

### missing all the dietMasterAgent_la part

ENDIF( DIET_USE_JXTA )
