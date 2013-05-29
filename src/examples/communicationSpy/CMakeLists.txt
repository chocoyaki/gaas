#****************************************************************************#
#* DIET Spy                                                                 *#
#* Spies DIET elements on hostname                                          *#
#****************************************************************************#

INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_EXECUTABLE( spy spyMain.cc Spy.cc Address.cc )
target_link_libraries(spy ${DIET_ADMIN_LIBRARIES} pcap)

INSTALL( TARGETS spy DESTINATION ${EXAMPLE_INSTALL_DIR}/spy )
