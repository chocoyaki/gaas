#****************************************************************************#
#* DIET cmake local file                                                    *#
#* Example for dynamically adding and removing some services via the use of *#
#* a dynamic library sent to the SeD and dynamically called.                *#
#* ONLY WORKS WITH DAGDA                                                    *#
#****************************************************************************#
# Client to connect an element to a new parent
add_executable(connect connect.cc)
target_link_libraries(connect ${DIET_ADMIN_LIBRARIES})

# Client to disconnect an element from its parent
add_executable(disconnect disconnect.cc)
target_link_libraries(disconnect ${DIET_ADMIN_LIBRARIES})

# Client to kill part of a hierarchy
add_executable(remove remove.cc)
target_link_libraries(remove ${DIET_ADMIN_LIBRARIES})

install(TARGETS connect disconnect remove DESTINATION ${EXAMPLE_INSTALL_DIR}/dynamic_hierarchy)
