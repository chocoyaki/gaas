INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/utils
)

########################################################################
### UtilsEvents library

set(UtilsEventsSource
  EventBase.cc
  EventDispatcher.cc
  EventManager.cc )

add_library(UtilsEvents ${UtilsEventsSource})
add_dependencies(UtilsEvents DIET_CORBA)

