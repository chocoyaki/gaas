#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
set(DAGDA_SOURCES
  DagdaImpl.cc
  DagdaFactory.cc
  DagdaCatalog.cc
  CacheAlgorithms.cc
  NetworkStats.cc
  AdvancedDagdaComponent.cc
  Container.cc
  DIET_Dagda.cc
  ${DIET_IDL_OUTPUT}
  ${config_SRCS})

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/agent
  ${DIET_SOURCE_DIR}/src/utils/nodes
  ${DIET_SOURCE_DIR}/src/utils/config)

if (DIET_TRANSFER_PROGRESSION)
  set(DAGDA_SOURCES ${DAGDA_SOURCES} Transfers.cc)
endif (DIET_TRANSFER_PROGRESSION)

add_library(DIET_Dagda ${DAGDA_SOURCES})
set_target_properties(DIET_Dagda PROPERTIES VERSION ${DIET_VERSION})

set(DAGDA_Libs
  CorbaCommon
  UtilsNodes
  DIET_Utils
  ${OMNIORB4_LIBRARIES}
  ${Boost_LIBRARIES})

if (DIET_USE_LOG)
set(DAGDA_Libs
  ${DAGDA_Libs}
  DietLogLibrary)
endif (DIET_USE_LOG)

if (CYGWIN)
  set(DAGDA_Libs
    ${DAGDA_Libs}
  )
endif (CYGWIN)

target_link_libraries(DIET_Dagda ${DAGDA_Libs})

install(TARGETS DIET_Dagda DESTINATION ${LIB_INSTALL_DIR})
install( FILES DIET_Dagda.h DESTINATION ${INCLUDE_INSTALL_DIR})

if (DIET_USE_LOG)
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)
