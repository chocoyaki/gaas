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
)

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/include
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/agent
  ${DIET_SOURCE_DIR}/src/utils/nodes
)

if (DIET_TRANSFER_PROGRESSION)
  set(DAGDA_SOURCES ${DAGDA_SOURCES} Transfers.cc)
endif (DIET_TRANSFER_PROGRESSION)

add_library(DIET_Dagda ${DAGDA_SOURCES})
  
set(DAGDA_Libs
  DIET_CORBA
  CorbaCommon
  UtilsNodes
#  DIET_Utils
  ${UUID_LIB}
  pthread
  ${OMNIORB4_LIBRARIES}
)

if (CYGWIN)
  set(DAGDA_Libs
    ${DAGDA_Libs}
    /lib/e2fsprogs/libuuid.a
  )
endif (CYGWIN)

target_link_libraries(DIET_Dagda
  ${DAGDA_Libs}
)

install(TARGETS DIET_Dagda DESTINATION lib)
install( FILES DIET_Dagda.h DESTINATION include)
