# DIET cmake local file

set(DIET_Utils_SOURCES
  debug.cc
  DIET_data.cc
  KeyString.cc
  ms_function.cc
  statistics.cc
  AccessController.cc
  JobQueue.cc
  DIET_mutex.cc
  est_internal.cc
  Vector.c
  DIET_compat.cc
  )

if (DIET_USE_LOG)
  add_subdirectory (log)
endif (DIET_USE_LOG)

if (DIET_USE_DAGDA)
  set(DIET_Utils_SOURCES
    ${DIET_Utils_SOURCES}
    DataRelationMgr.cc
    )
  add_subdirectory(DAGDA)
endif (DIET_USE_DAGDA)


add_subdirectory(config)

if (DIET_USE_FD)
  add_subdirectory(fd)
endif (DIET_USE_FD)

include_directories(
  ${OMNIORB4_INCLUDE_DIR}
  ${DIET_BINARY_DIR}/src/CORBA/idl
  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
  ${CMAKE_CURRENT_SOURCE_DIR}         # for DIET_data_internal.hh
  ${DIET_SOURCE_DIR}/include
  ${DIET_SOURCE_DIR}/src/utils	# for BatchSystems.hh in Cori_batch
  ${DIET_SOURCE_DIR}/src/SeD		# for SeDImpl.hh in Cori_batch
  ${DIET_SOURCE_DIR}/src/agent	# for ChildID.hh in SeDImpl
  )

########################################################################
### UtilsSedBatch library:
if (DIET_USE_ALT_BATCH)
  include_directories(
    ${DIET_SOURCE_DIR}/src/utils/batch
    )
  add_subdirectory(batch)
endif (DIET_USE_ALT_BATCH)

add_library(DIET_Utils STATIC
  ${DIET_Utils_SOURCES}
  )

target_link_libraries(DIET_Utils UtilsNodes)
if (DIET_USE_ALT_BATCH)
  target_link_libraries(DIET_Utils UtilsSeDBatch)
endif (DIET_USE_ALT_BATCH)
if (DIET_USE_WORKFLOW)
  add_subdirectory(events)
  target_link_libraries(DIET_Utils UtilsEvents)
endif (DIET_USE_WORKFLOW)
if (DIET_USE_CORI)
  target_link_libraries(DIET_Utils UtilsCori)
endif (DIET_USE_CORI)

if (DIET_USE_USERSCHED)
  install(TARGETS DIET_Utils DESTINATION ${LIB_INSTALL_DIR})
endif (DIET_USE_USERSCHED)


########################################################################
## Install the files needed for scheduler development.
if (DIET_USE_USERSCHED)
  INSTALL(FILES
    est_internal.hh
    debug.hh
    LinkedList.hh
    DIET_data_internal.hh
    Vector.h
    ms_function.hh
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/scheduler
    )
  install(FILES
    ts_container/ts_set.hh
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/scheduler/ts_container
    )
endif (DIET_USE_USERSCHED)


## Admin API
if ( DIET_ADMIN_API )
  add_subdirectory( admin )
endif()