INCLUDE_DIRECTORIES(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA
  ${DIET_SOURCE_DIR}/src/utils/DAGDA
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/include
)

generate_diet_tests( DIETDynamicsConnectTests DynamicAddRemoveServiceSeDFixture )
generate_diet_tests( DIETDynamicsDisconnectTests DynamicAddRemoveServiceSeDFixture )
generate_diet_tests( DIETDynamicsRemoveTests DynamicAddRemoveServiceSeDFixture )
