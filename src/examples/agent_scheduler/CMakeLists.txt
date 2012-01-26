# Scheduler example
include_directories(${DIET_DIR}/include
  ${DIET_DIR}/include/scheduler
  ${OMNIORB4_INCLUDE_DIR})

set(HostnameRR_SRCS HostnameRR.cc)

add_library(HostnameRR MODULE ${HostnameRR_SRCS})

if(APPLE)
  target_link_libraries(HostnameRR ${OMNIORB4_LIBRARIES})
endif()

install(TARGETS HostnameRR DESTINATION ${EXAMPLE_INSTALL_DIR}/agent_scheduler)
