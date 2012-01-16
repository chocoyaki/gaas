#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

set(DIET_CLIENT_LIBRARIES DIET_client DIET_Dagda ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
set(DIET_SERVER_LIBRARIES DIET_SeD DIET_Dagda DIET_client ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
set(DIET_ADMIN_LIBRARIES DIET_admin DIET_Dagda DIET_client)

if(DIET_USE_WORKFLOW)
  set(DIET_CLIENT_LIBRARIES ${DIET_CLIENT_LIBRARIES} ${XERCES_LIBRARY} ${XQILLA_LIBRARY})
  set(DIET_SERVER_LIBRARIES ${DIET_SERVER_LIBRARIES} ${XERCES_LIBRARY} ${XQILLA_LIBRARY})
  set(DIET_ADMIN_LIBRARIES ${DIET_ADMIN_LIBRARIES} ${XERCES_LIBRARY} ${XQILLA_LIBRARY})
endif()

if(AIX)
  set(CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
  set(CMAKE_C_LINKER_PREFERENCE "CXX")
endif()

set(EXAMPLE_INSTALL_DIR "${BIN_INSTALL_DIR}/examples"
  CACHE INTERNAL "Directory where examples will be installed")

add_subdirectory(dynamic_hierarchy)

if(DIET_USE_ALT_BATCH)
  add_subdirectory(Batch)
  if(DIET_USE_CLOUD)
    add_subdirectory(cloud)
  endif(DIET_USE_CLOUD)
endif(DIET_USE_ALT_BATCH)

if(DIET_USE_USERSCHED)
  add_subdirectory(agent_scheduler)
endif()
add_subdirectory(BLAS)
add_subdirectory(cfgs)
add_subdirectory(cori)
add_subdirectory(dmat_manips)
add_subdirectory(file_transfer)
add_subdirectory(scalars)
add_subdirectory(GridRPC)

add_subdirectory(allDataTransfers)
add_subdirectory(containers)
add_subdirectory(dyn_add_rem)
add_subdirectory(dynamicServiceMgr)

if (DIET_USE_WORKFLOW)
  add_subdirectory (workflow)
  # Do not compile SeDWrapper example for as long as Diet 3.0 is not released:
  # this example requires BOOST.
  # ADD_SUBDIRECTORY (SeDWrapper)
endif (DIET_USE_WORKFLOW)

# IF (DIET_USE_CCS)
#   ADD_SUBDIRECTORY (ccs)
# ENDIF (DIET_USE_CCS)

if (DIET_USE_MULTICALL)
  add_subdirectory (multicall)
endif (DIET_USE_MULTICALL)
