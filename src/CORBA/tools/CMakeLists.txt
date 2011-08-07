#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

# Generates some tools for developers and debugging:
#   * dietObjects: list the DIET objects currently declared

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl    
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/src/utils/config
)

set(DIETOBJECTS_Sources
  dietObjects.cc
)

set(DIETOBJECTS_Libraries
  CorbaCommon
  ${OMNIORB4_LIBRARIES}
  DIET_Utils
  LibForwarder
)

add_executable(dietObjects
  ${DIETOBJECTS_Sources}
)

target_link_libraries(dietObjects ${DIETOBJECTS_Libraries})

install(TARGETS dietObjects DESTINATION ${BIN_INSTALL_DIR})
