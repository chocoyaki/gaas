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

# Client
ADD_EXECUTABLE( client_DynamicServiceMgr client.cc )
TARGET_LINK_LIBRARIES( client_DynamicServiceMgr ${DIET_CLIENT_LIBRARIES} )

# Server
SET( DYNAMICSERVICEMGR_SERVER_SRC
  DynamicServiceMgr.cc
  server.cc
  )
ADD_EXECUTABLE( server_DynamicServiceMgr ${DYNAMICSERVICEMGR_SERVER_SRC} )
TARGET_LINK_LIBRARIES( server_DynamicServiceMgr ${DIET_SERVER_LIBRARIES} dl)

# install client and server 
INSTALL( TARGETS client_DynamicServiceMgr server_DynamicServiceMgr DESTINATION ${EXAMPLE_INSTALL_DIR}/dynamicServiceMgr )

# Library for Dynamic Service Mgr
ADD_LIBRARY( DynamicServiceMgr_example
  service.cc
  )
TARGET_LINK_LIBRARIES( DynamicServiceMgr_example ${DIET_SERVER_LIBRARIES} )

ADD_LIBRARY( DynamicServiceMgr_example2
  service2.cc
  )
TARGET_LINK_LIBRARIES( DynamicServiceMgr_example2 ${DIET_SERVER_LIBRARIES} )

# install libraries
INSTALL( TARGETS DynamicServiceMgr_example DynamicServiceMgr_example2 DESTINATION ${EXAMPLE_INSTALL_DIR}/dynamicServiceMgr )


# README
INSTALL( FILES README DESTINATION ${EXAMPLE_INSTALL_DIR}/dynamicServiceMgr )

