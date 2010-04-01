#****************************************************************************#
#* DIET cmake local file                                                    *#
#* Example for dynamically adding and removing some services via the use of *#
#* a dynamic library sent to the SeD and dynamically called.                *#
#* ONLY WORKS WITH DAGDA                                                    *#
#****************************************************************************#

INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/include
)

# Client to connect an element to a new parent
ADD_EXECUTABLE( connect connect.cc )
TARGET_LINK_LIBRARIES( connect ${DIET_SERVER_LIBRARIES} AgentCommon)

# Client to disconnect an element from its parent
ADD_EXECUTABLE( disconnect disconnect.cc )
TARGET_LINK_LIBRARIES( disconnect ${DIET_SERVER_LIBRARIES} AgentCommon)

# Client to kill part of a hierarchy
ADD_EXECUTABLE( remove remove.cc )
TARGET_LINK_LIBRARIES( remove ${DIET_SERVER_LIBRARIES} AgentCommon)

INSTALL( TARGETS connect disconnect remove DESTINATION bin/examples/dynamic_hierarchy )
