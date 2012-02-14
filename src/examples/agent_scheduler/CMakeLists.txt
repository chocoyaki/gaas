# Scheduler example
include_directories(${DIET_DIR}/include
  ${DIET_DIR}/include/scheduler
  ${OMNIORB4_INCLUDE_DIR})

set(HostnameRR_SRCS
  ../../agent/GlobalSchedulers.cc
  ../../agent/Schedulers.cc
  ../../agent/UserScheduler.cc
  ../../utils/debug.cc
  ../../utils/est_internal.cc
  ../../utils/Vector.c
  ../../utils/config/CommonParser.cc
  ../../utils/config/constants.cc
  ../../utils/config/configuration.cc

  # FIXME: all the above sources shouldn't even be there!!! We need to have a library to easily create schedulers!
  HostnameRR.cc
  )

add_library(HostnameRR MODULE ${HostnameRR_SRCS})
target_link_libraries(HostnameRR ${Boost_LIBRARIES})
add_dependencies(HostnameRR dietAgent) # fake dependency, but we need files generated from IDLs

if(APPLE)
  target_link_libraries(HostnameRR ${OMNIORB4_LIBRARIES})
endif()

install(TARGETS HostnameRR DESTINATION ${EXAMPLE_INSTALL_DIR}/agent_scheduler)
