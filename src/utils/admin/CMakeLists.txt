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
  remove.cc)

INSTALL( FILES admin.hh DESTINATION include/)
INSTALL( TARGETS DIET_admin DESTINATION lib/)
