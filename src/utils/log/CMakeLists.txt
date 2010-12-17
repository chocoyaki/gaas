#include_directories(
#  ${OMNIORB4_INCLUDE_DIR}
#  ${DIET_BINARY_DIR}/src/CORBA/idl
#  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
#  ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
#)
#
########################################################################

include_directories(
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${LOG_SRC_DIR}
  ${LOG_SRC_DIR}/monitor
  ${LOG_SRC_DIR}/idl
  ${OMNIORB4_INCLUDE_DIR})


set(LogSources
  DietLogComponent.cc)

add_library(DietLogLibrary STATIC ${LogSources})

add_dependencies(DietLogLibrary DIET_CORBA)

target_link_libraries(DietLogLibrary
  ${LOG_LIBRARY}
)

