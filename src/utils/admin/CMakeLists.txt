include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/include
)

add_library(DIET_admin 
  remove.cc
  connect.cc
  disconnect.cc
)

target_link_libraries( DIET_admin DIET_CORBA DIET_SeD )

set_target_properties( DIET_admin  PROPERTIES VERSION ${DIET_VERSION} )

install( TARGETS DIET_admin DESTINATION lib/)
