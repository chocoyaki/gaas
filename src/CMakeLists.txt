#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
include(UseOmniORB4)
set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -Wbuse_quotes")

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${IDL_INCLUDE_DIRECTORIES}
  ../include
  CORBA
  CORBA/security
  utils
  utils/config
  utils/nodes
  utils/DAGDA
  agent
  client
  server
  SeD)

## includes and compilation directives
if(DIET_USE_LOG)
  include_directories(${LOG_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils/log)
endif()
if(DIET_USE_ALT_BATCH)
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_ALT_BATCH")
endif()
if(DIET_USE_WORKFLOW)
  include_directories(${XERCES_INCLUDE_DIR} ${XQILLA_INCLUDE_DIR}
    agent/workflow
    client/workflow
    utils/workflow
    utils/events)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_WORKFLOW")
endif()
if(DIET_USE_LOG)
  include_directories(utils/log)
endif()
if(DIET_WITH_MULTI_MA)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DHAVE_MULTI_MA")
endif()
if(DIET_USE_USERSCHED)
  set(DIET_IDL_FLAGS "${DIET_IDL_FLAGS} -DUSERSCHED")
endif()
if(DIET_USE_CLOUD_EC2)
  include_directories(${GSOAP_INCLUDE_DIRS} utils/batch/EucaLib)
  add_definitions("-DWITH_DOM -DWITH_OPENSSL")
endif()

if(DIET_USE_DELTACLOUD)
    include_directories(${APACHE_DELTACLOUD_INCLUDE_DIR}
      utils/cloud
      utils/delta-cloud
      utils/cloud/deltacloud)
	SET(DELTACLOUD_MODULE_SRCS
    utils/cloud/RemoteAccess.cc
			utils/cloud/CloudAPIConnection.cc
			utils/delta-cloud/CloudServiceBinary.cc
      utils/cloud/IaasInterface.cc
			utils/cloud/deltacloud/Iaas_deltacloud.cc
			utils/cloud/Instance.cc
      utils/cloud/VMsDeployment.cc
			utils/delta-cloud/SeDCloud.cc
			utils/delta-cloud/SeDCloudActions.cc
			utils/delta-cloud/ServiceStatisticsMap.cc
			utils/delta-cloud/ServiceWrapper.cc
			utils/delta-cloud/ServiceWrapperArgument.cc
			utils/delta-cloud/Tools.cc
			utils/delta-cloud/server_metrics.cc
		)
    add_library(DIET_DeltaCloud ${DELTACLOUD_MODULE_SRCS})
	target_link_libraries(DIET_DeltaCloud ${APACHE_DELTACLOUD_LIBRARY} ${XERCES_LIB} DIET_SeD)
	install(TARGETS DIET_DeltaCloud DESTINATION ${LIB_INSTALL_DIR})
		
endif()

## generate CORBA idl required by almost everyone
set(DIET_IDL_SRCS
  CORBA/idl/common_types.idl
  CORBA/idl/Agent.idl
  CORBA/idl/Callback.idl
  CORBA/idl/LocalAgent.idl
  CORBA/idl/MasterAgent.idl
  CORBA/idl/response.idl
  CORBA/idl/SeD.idl
  CORBA/idl/AgentFwdr.idl
  CORBA/idl/CallbackFwdr.idl
  CORBA/idl/LocalAgentFwdr.idl
  CORBA/idl/MasterAgentFwdr.idl
  CORBA/idl/SeDFwdr.idl
  CORBA/idl/Forwarder.idl
  CORBA/idl/Dagda.idl
  CORBA/idl/DagdaFwdr.idl)

if(DIET_USE_WORKFLOW)
  set(DIET_IDL_SRCS ${DIET_IDL_SRCS}
    CORBA/idl/MaDag.idl
    CORBA/idl/CltMan.idl
    CORBA/idl/CltManFwdr.idl
    CORBA/idl/MaDagFwdr.idl
    CORBA/idl/WfLogService.idl
    CORBA/idl/WfLogServiceFwdr.idl)

  set(ClientWf_SRCS
    client/workflow/CltDagNodeLauncher.cc
    client/workflow/CltWfLogCentralDispatcher.cc
    client/workflow/CltWfMgr.cc)

  set(UtilsWf_SRCS
    utils/workflow/WfLogDispatcher.cc
    utils/workflow/WfNode.cc
    utils/workflow/Dag.cc
    utils/workflow/MetaDag.cc
    utils/workflow/FWorkflow.cc
    utils/workflow/FNode.cc
    utils/workflow/WfExpression.cc
    utils/workflow/WfDataWriter.cc
    utils/workflow/FActivityNode.cc
    utils/workflow/FIfNode.cc
    utils/workflow/FLoopNode.cc
    utils/workflow/WfPort.cc
    utils/workflow/WfPortAdapter.cc
    utils/workflow/FNodePort.cc
    utils/workflow/DagNode.cc
    utils/workflow/DagNodePort.cc
    utils/workflow/DagNodeLauncher.cc
    utils/workflow/DagWfParser.cc
    utils/workflow/NodeQueue.cc
    utils/workflow/FDataHandle.cc
    utils/workflow/InputIterator.cc
    utils/workflow/Thread.cc
    utils/workflow/WfUtils.cc
    utils/workflow/WfLogServiceImpl.cc)

  set(UtilsEvents_SRCS
    utils/events/EventBase.cc
    utils/events/EventDispatcher.cc
    utils/events/EventManager.cc)
endif()

if(DIET_USE_ALT_BATCH)
  set(UtilsBatch_SRCS
    utils/batch/BatchCreator.cc
    utils/batch/BatchSystem.cc
    utils/batch/OAR1_6BatchSystem.cc
    utils/batch/OAR2_XBatchSystem.cc
    utils/batch/Loadleveler_BatchSystem.cc
    utils/batch/PBS_BatchSystem.cc
    utils/batch/SGE_BatchSystem.cc
    utils/batch/Slurm_BatchSystem.cc)
  if(WIN32)
    set(UtilsBatch_SRCS ${UtilsBatch_SRCS}
      utils/batch/mkstemp.cc)
  endif()
endif()

if(DIET_USE_USERSCHED)
  # Install the files needed for scheduler development.
  # TODO: Create a DIET_scheduler.hpp file
  install(FILES
    ${IDL_INCLUDE_DIRECTORIES}/response.hh
    ${IDL_INCLUDE_DIRECTORIES}/common_types.hh
    ${IDL_INCLUDE_DIRECTORIES}/SeD.hh
    DESTINATION ${INCLUDE_INSTALL_DIR}/scheduler)
  install(FILES
    utils/est_internal.hh
    utils/debug.hh
    utils/LinkedList.hh
    utils/DIET_data_internal.hh
    utils/Vector.h
    utils/ms_function.hh
    utils/OSIndependance.hh
    DESTINATION ${INCLUDE_INSTALL_DIR}/scheduler)
  install(FILES
    utils/ts_container/ts_set.hh
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/scheduler/ts_container)
endif()

generate_idl(OUTPUT DIET_IDL_OUTPUT
  IDLS ${DIET_IDL_SRCS}
  FLAGS ${DIET_IDL_FLAGS}
  LANG C++)

## configuration
set(config_SRCS
  utils/config/CmdParser.cc
  utils/config/CommonParser.cc
  utils/config/EnvParser.cc
  utils/config/FileParser.cc
  utils/config/constants.cc
  utils/config/configuration.cc)

## UtilsLog_SRCS
set(UtilsLog_SRCS utils/log/DietLogComponent.cc)

## dietUtils_SRCS
set(dietUtils_SRCS
  utils/debug.cc
  utils/DIET_data.cc
  utils/DIET_uuid.cc
  utils/KeyString.cc
  utils/ms_function.cc
  utils/statistics.cc
  utils/AccessController.cc
  utils/JobQueue.cc
  utils/DIET_mutex.cc
  utils/est_internal.cc
  utils/Vector.c
  utils/DIET_compat.cc
  utils/DataRelationMgr.cc)

if(WIN32)
  set(dietUtils_SRCS ${dietUtils_SRCS} utils/OSIndependance.cc)
endif()


if(DIET_USE_CLOUD_EC2)
  set(cloud_SRCS
    utils/batch/EucaLib/ec2wrapper.c
    utils/batch/EucaLib/soapC.c
    utils/batch/EucaLib/soapClient.c
    utils/batch/Eucalyptus_BatchSystem.cc
    ${GSOAP_SRC_DIR}/gsoap/dom.c
    ${GSOAP_SRC_DIR}/gsoap/stdsoap2.c
    ${GSOAP_SRC_DIR}/gsoap/plugin/smdevp.c
    ${GSOAP_SRC_DIR}/gsoap/plugin/wsseapi.c)
endif()


if(DIET_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

## CorbaCommon
set(CorbaCommon_SRCS CORBA/ORBMgr.cc CORBA/marshalling.cc)

if (DIET_USE_SECURITY)
  set (CorbaCommon_SRCS ${CorbaCommon_SRCS} "CORBA/security/DIETSecurityManager.cc")
endif(DIET_USE_SECURITY)

## AgentCommon
set(AgentCommon_SRCS
  agent/AgentImpl.cc
  agent/BindService.cc
  agent/ExitClass.cc
  agent/FloodRequest.cc
  agent/FloodRequestsList.cc
  agent/GlobalSchedulers.cc
  agent/LocalAgentImpl.cc
  agent/MasterAgentImpl.cc
  agent/ReferenceUpdateThread.cc
  agent/Request.cc
  agent/Schedulers.cc)

## MaDagCommon
set(MaDagCommon_SRCS
  agent/workflow/MaDagNodeLauncher.cc
  agent/workflow/MaDag_impl.cc
  agent/workflow/MaDagLogCentralDispatcher.cc
  agent/workflow/WfScheduler.cc
  agent/workflow/HEFTScheduler.cc
  agent/workflow/MultiWfScheduler.cc
  agent/workflow/MultiWfBasicScheduler.cc
  agent/workflow/MultiWfFOFT.cc
  agent/workflow/MultiWfHEFT.cc
  agent/workflow/MultiWfSRPT.cc
  agent/workflow/MultiWfFCFS.cc)

## UtilsNode
set(UtilsNode_SRCS
  utils/nodes/Counter.cc
  utils/nodes/ServiceTable.cc)

if(DIET_USE_LOG)
  set(UtilsNode_SRCS ${UtilsNode_SRCS}
    utils/nodes/MonitoringThread.cc)
endif()

## Cori
set(cori_SRCS
  utils/nodes/Cori_Data_Easy.cc
  utils/nodes/Cori_Easy_CPU.cc
  utils/nodes/Cori_Easy_Disk.cc
  utils/nodes/Cori_Easy_Memory.cc
  utils/nodes/Cori_Metric.cc
  utils/nodes/CORIMgr.cc)

set(CORI_BATCH_CLEAN_COMPILE_FLAG "")
if(DIET_USE_ALT_BATCH)      # Then extract from the lib SeD related
  set(CORI_BATCH_CLEAN_COMPILE_FLAG ${CORI_BATCH_CLEAN_COMPILE_FLAG}
    "-D CLEAN_CORILIB_FROM_BATCH_STAFF")
  set(cori_SRCS ${cori_SRCS} utils/batch/Cori_batch.cc)
endif()
set_source_files_properties(Cori_Metric.cc
  PROPERTIES COMPILE_FLAGS "${CORI_BATCH_CLEAN_COMPILE_FLAG}")
# Refer to ConfigureCORI.cmake for definition of CORI_HAVE_* variables:
set(CORI_EASY_CPU_COMPILE_FLAGS  "")
set(CORI_EASY_MEMORY_COMPILE_FLAGS  "")
if(CORI_HAVE_PROCCPU)
  set( CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_PROCCPU")
endif()
if(CORI_HAVE_PROCMEM)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_PROCMEM")
endif()
if(CORI_HAVE_SYS_TYPES)
  set( CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_TYPES")
endif()
if(CORI_HAVE_SYS_SYSCTL)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSCTL")
endif()
if(CORI_HAVE_SYS_SYSINFO)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSINFO")
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_SYS_SYSINFO")
endif()
if(CORI_HAVE_get_avphys_pages)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_get_avphys_pages")
endif()
if(CORI_HAVE_get_nprocs)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_get_nprocs")
endif()
# FIXME: looks like never used !
if(CORI_HAVE_get_phys_pages)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_get_phys_pages")
endif()
if(CORI_HAVE_getloadavg)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_getloadavg")
endif()
if(CORI_HAVE_getpagesize)
  set(CORI_EASY_MEMORY_COMPILE_FLAGS
    "${CORI_EASY_MEMORY_COMPILE_FLAGS} -D CORI_HAVE_getpagesize")
endif()
if(CORI_HAVE_sysconf)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysconf")
endif()
if(CORI_HAVE_sysctl)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysctl")
endif()
if(CORI_HAVE_sysctlbyname)
  set(CORI_EASY_CPU_COMPILE_FLAGS
    "${CORI_EASY_CPU_COMPILE_FLAGS} -D CORI_HAVE_sysctlbyname")
endif()
set_source_files_properties(Cori_Easy_CPU.cc
  PROPERTIES COMPILE_FLAGS "${CORI_EASY_CPU_COMPILE_FLAGS}")
set_source_files_properties( Cori_Easy_Memory.cc
  PROPERTIES COMPILE_FLAGS "${CORI_EASY_MEMORY_COMPILE_FLAGS}")


## library: DIET_Dagda
set(CORE_SRCS
  utils/DAGDA/DagdaImpl.cc
  utils/DAGDA/DagdaFactory.cc
  utils/DAGDA/DagdaCatalog.cc
  utils/DAGDA/CacheAlgorithms.cc
  utils/DAGDA/NetworkStats.cc
  utils/DAGDA/AdvancedDagdaComponent.cc
  utils/DAGDA/Container.cc
  utils/DAGDA/DIET_Dagda.cc
  ${DIET_IDL_OUTPUT}
  ${config_SRCS}
  ${CorbaCommon_SRCS}
  ${dietUtils_SRCS}
  ${UtilsNode_SRCS}
  )
if(DIET_TRANSFER_PROGRESSION)
  set(CORE_SRCS ${CORE_SRCS} utils/DAGDA/Transfers.cc)
endif()

set(CORE_LIBS ${OMNIORB4_LIBRARIES} ${Boost_LIBRARIES})
if(WIN32)
  set(CORE_LIBS ${CORE_LIBS} Shlwapi ws2_32)
endif()

if(DIET_USE_LOG)
  set(CORE_SRCS ${CORE_SRCS} ${UtilsLog_SRCS})
  set(CORE_LIBS ${CORE_LIBS} ${LOG_LIBRARIES})
endif()

if(DIET_USE_WORKFLOW)
  set(CORE_LIBS ${CORE_LIBS} ${XQILLA_LIBRARY} ${XERCES_LIBRARY})
endif()

if(DIET_USE_DELTACLOUD)
  set(CORE_LIBS ${CORE_LIBS} ${XERCES_LIBRARY})
endif()

add_library(DIET_Dagda ${CORE_SRCS})
set_target_properties(DIET_Dagda PROPERTIES VERSION ${DIET_VERSION})
target_link_libraries(DIET_Dagda ${CORE_LIBS})

install(TARGETS DIET_Dagda DESTINATION ${LIB_INSTALL_DIR})
install(FILES utils/DAGDA/DIET_Dagda.h DESTINATION ${INCLUDE_INSTALL_DIR})

## library: DIET_admin
set(admin_SRCS
  utils/admin/remove.cc
  utils/admin/connect.cc
  utils/admin/disconnect.cc)

add_library(DIET_admin ${admin_SRCS})
target_link_libraries(DIET_admin DIET_SeD)
set_target_properties(DIET_admin  PROPERTIES VERSION ${DIET_VERSION})
install(TARGETS DIET_admin DESTINATION ${LIB_INSTALL_DIR})

## ForwarderCommon
set(ForwarderCommon_SRCS
  CORBA/dietFwdr.cc
  CORBA/SSHTunnel.cc
  CORBA/Options.cc
  CORBA/DIETForwarder.cc
  CORBA/forwarders/Agent.cc
  CORBA/forwarders/Callback.cc
  CORBA/forwarders/MasterAgent.cc
  CORBA/forwarders/SeD.cc
  CORBA/forwarders/Dagda.cc)
if(DIET_USE_WORKFLOW)
  set(ForwarderCommon_SRCS ${ForwarderCommon_SRCS}
    CORBA/forwarders/CltMan.cc
    CORBA/forwarders/MaDag.cc
    CORBA/forwarders/WfLogService.cc)
endif()

## executable: dietForwarder
set(dietForwarder_SRCS
  ${ForwarderCommon_SRCS}
  ${AgentCommon_SRCS}
  )

set(dietForwarder_LIBS DIET_Dagda DIET_SeD DIET_client ${CMAKE_DL_LIBS})
if(WIN32)
  set(dietForwarder_LIBS ${dietForwarder_LIBS} ws2_32 Iphlpapi)
endif()

if(DIET_USE_WORKFLOW)
  set(dietForwarder_SRCS ${dietForwarder_SRCS}
    ${UtilsWf_SRCS} ${MaDagCommon_SRCS})
  set(dietForwarder_LIBS ${dietForwarder_LIBS}
    ${XQILLA_LIBRARY} ${XERCES_LIBRARY})
endif()

if(DIET_USE_DELTACLOUD)
    set(dietForwarder_LIBS ${dietForwarder_LIBS} ${APACHE_DELTACLOUD_LIBRARY} ${XERCES_LIBRARY})
endif()

if(DIET_USE_USERSCHED)
  set(dietForwarder_SRCS ${dietForwarder_SRCS} agent/UserScheduler.cc )
  install(FILES CORBA/marshalling.hh
    DESTINATION ${INCLUDE_INSTALL_DIR}/scheduler)
endif()
add_executable(dietForwarder ${dietForwarder_SRCS})
target_link_libraries(dietForwarder ${dietForwarder_LIBS})
install(TARGETS dietForwarder DESTINATION ${BIN_INSTALL_DIR})


## executatble: dietObjects
if(DIET_BUILD_TOOLS)
  set(dietObjects_SRCS CORBA/tools/dietObjects.cc)
  set(dietObjects_LIBS DIET_client DIET_SeD ${CMAKE_DL_LIBS})
  if(WIN32)
    set(dietObjects_LIBS ${dietObjects_LIBS} ws2_32)
  endif()

  if(DIET_USE_WORKFLOW)
    set(dietObjects_LIBS ${dietObjects_LIBS}
      ${XERCES_LIBRARY} ${XQILLA_LIBRARY})
  endif()

  add_executable(dietObjects ${dietObjects_SRCS})
  target_link_libraries(dietObjects ${dietObjects_LIBS})
  install(TARGETS dietObjects DESTINATION ${BIN_INSTALL_DIR})
endif()

## library: DIET_client
set(DIET_client_SRCS
  client/DIET_client.cc
  client/DIET_grpc.cc
  client/DIETCall.cc
  client/CallAsyncMgr.cc
  client/CallbackImpl.cc)

set(DIET_client_LIBS DIET_Dagda)

if(DIET_USE_MULTICALL)
  set(DIET_client_SRCS ${DIET_client_SRCS} client/MultiCall.cc)
endif()
if(DIET_USE_CCS)
  set(DIET_client_SRCS ${DIET_client_SRCS} client/SpecificClientScheduler.cc)
endif()
if(DIET_USE_ALT_BATCH)
  set(DIET_client_SRCS ${DIET_client_SRCS})
endif()
if(DIET_USE_WORKFLOW)
  set(DIET_client_SRCS ${DIET_client_SRCS}
    ${ClientWf_SRCS} ${UtilsWf_SRCS} ${UtilsEvents_SRCS})

  set(DIET_client_LIBS ${DIET_client_LIBS}
    ${XQILLA_LIBRARY} ${XERCES_LIBRARY})

  install(FILES utils/workflow/FWorkflow.dtd DESTINATION ${DATA_INSTALL_DIR})
endif()

if(WIN32)
  set(DIET_client_LIBS ${DIET_client_LIBS} ws2_32 Shlwapi Iphlpapi)
endif()

add_library(DIET_client ${DIET_client_SRCS})
set_target_properties(DIET_client  PROPERTIES VERSION ${DIET_VERSION})
target_link_libraries(DIET_client ${DIET_client_LIBS})
install(TARGETS DIET_client DESTINATION ${LIB_INSTALL_DIR})

## executable: agent
set(dietAgent_LIBS
  DIET_Dagda
  DIET_client
  DIET_SeD
  ${CMAKE_DL_LIBS})

if(DIET_USE_USERSCHED)
  set(AgentCommon_SRCS ${AgentCommon_SRCS} agent/UserScheduler.cc)
  install(FILES
    agent/GlobalSchedulers.hh
    agent/UserScheduler.hh
    agent/Schedulers.hh
    DESTINATION ${INCLUDE_INSTALL_DIR}/scheduler)
endif()
set(dietAgent_SRCS ${AgentCommon_SRCS} agent/dietAgent.cc)
if(DIET_USE_WORKFLOW)
  set(dietAgent_SRCS ${dietAgent_SRCS} ${ClientWf_SRCS} ${MaDagCommon_SRCS})
  set(dietAgent_LIBS ${dietAgent_LIBS} ${XQILLA_LIBRARY} ${XERCES_LIBRARY})
endif()
if(DIET_USE_ALT_BATCH)
  set(dietAgent_SRCS ${dietAgent_SRCS})
endif()
if(DIET_USE_CLOUD_EC2)
  set(dietAgent_SRCS ${dietAgent_SRCS} ${cloud_SRCS})
  set(dietAgent_LIBS ${dietAgent_LIBS} ${GSOAP_LIBRARIES})
endif()
if(WIN32)
  set(dietAgent_LIBS ${dietAgent_LIBS} Shlwapi  ws2_32)
endif()

add_executable(dietAgent ${dietAgent_SRCS})
target_link_libraries(dietAgent ${dietAgent_LIBS})
install(TARGETS dietAgent DESTINATION ${BIN_INSTALL_DIR})

## executable: maDagAgent
if(DIET_USE_WORKFLOW)
  set(maDagAgent_SRCS agent/workflow/maDagAgent.cc ${MaDagCommon_SRCS}
     ${UtilsEvents_SRCS})
  set(maDagAgent_LIBS DIET_Dagda ${XQILLA_LIBRARY} ${XERCES_LIBRARY}
    DIET_SeD DIET_client ${CMAKE_DL_LIBS})
  if(WIN32)
    set(maDagAgent_LIBS ${maDagAgent_LIBS} ws2_32 Shlwapi)
  endif()
  if(DIET_USE_LOG)
    set(maDagAgent_LIBS ${maDagAgent_LIBS} ${LOG_LIBRARIES})
  endif()

  add_executable(maDagAgent ${maDagAgent_SRCS})
  target_link_libraries(maDagAgent ${maDagAgent_LIBS})
  install(TARGETS maDagAgent DESTINATION ${BIN_INSTALL_DIR})
endif()

## library: DIET_SeD
set(DIET_SeD_SRCS SeD/DIET_server.cc SeD/SeDImpl.cc ${cori_SRCS})
if(DIET_USE_ALT_BATCH)
  set(DIET_SeD_SRCS ${DIET_SeD_SRCS} ${UtilsBatch_SRCS})
endif()
set(DIET_SeD_LIBS DIET_Dagda)
if(WIN32)
  set(DIET_SeD_LIBS ${DIET_SeD_LIBS} ws2_32)
endif()
if(DIET_USE_CLOUD_EC2)
  set(DIET_SeD_SRCS ${DIET_SeD_SRCS} ${cloud_SRCS})
  set(DIET_SeD_LIBS ${DIET_SeD_LIBS} ${GSOAP_LIBRARIES})
endif()
if (DIET_USE_DELTACLOUD)
  set(DIET_SeD_SRCS ${DIET_SeD_SRCS} ${DELTACLOUD_MODULE_SRCS})
  set(DIET_SeD_LIBS ${DIET_SeD_LIBS} ${XERCES_LIBRARY} ${APACHE_DELTACLOUD_LIBRARY})
endif()

add_library(DIET_SeD ${DIET_SeD_SRCS})
set_target_properties(DIET_SeD  PROPERTIES VERSION ${DIET_VERSION})
target_link_libraries(DIET_SeD ${DIET_SeD_LIBS})
install(TARGETS DIET_SeD DESTINATION ${LIB_INSTALL_DIR})

## Tests
add_subdirectory(Testing)
