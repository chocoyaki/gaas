# DIET cmake local file

set(DIET_Utils_SOURCES
  debug.cc
  DIET_data.cc
  KeyString.cc
  ms_function.cc
  Parsers.cc
  statistics.cc
  AccessController.cc
  JobQueue.cc
  DIET_mutex.cc
  est_internal.cc
  Vector.c
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

add_subdirectory(events)

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
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    if (DIET_USE_CLOUD)
        ########################################################################
        # Build the Eucalyptus SOAP client library and its associated BatchSystem
        include_directories(${CMAKE_CURRENT_BINARY_DIR} ${DIET_SOURCE_DIR}/src/utils/EucaLib
            ${GSOAP_SRC_DIR}
            )
        add_definitions( -DWITH_DOM -DWITH_OPENSSL )

        # Yes, part of gSOAP needs to be recompiled in order to use the WSSE plugin.
        # This is the way it was built to work. There is no automatic build for
        # the WSSE plugin and therefore this is the only way.
        #
        # This is also what the gSOAP community advises in their doc inside the package:
        # doc/wsse/html/wsse.html
        # Save yourself some time and just read this next time.
        add_library(EucalyptusLib
          EucaLib/ec2wrapper.c
          EucaLib/soapC.c
          EucaLib/soapClient.c

          ${GSOAP_SRC_DIR}/stdsoap2.c
          ${GSOAP_SRC_DIR}/dom.c
          ${GSOAP_SRC_DIR}/plugin/smdevp.c
          ${GSOAP_SRC_DIR}/plugin/wsseapi.c
        )
        
        target_link_libraries(EucalyptusLib -lssl -lcrypto)
        add_definitions( -DWITH_DOM -DWITH_OPENSSL )
        install(TARGETS EucalyptusLib DESTINATION ${LIB_INSTALL_DIR})
            
        # Build SeDBatch library with the Cloud component
        include_directories(
          ${CMAKE_CURRENT_BINARY_DIR}
          ${DIET_SOURCE_DIR}/src/utils/EucaLib
          ${GSOAP_INCLUDE_DIRS}
          ${GSOAP_PLUGIN_DIR}
          ${GSOAP_PLUGIN_DIR}/..
        )
#        add_library(UtilsSeDBatch STATIC
#            ${GSOAP_SRC_DIR}
#        )
        add_library(UtilsSeDBatch STATIC
            BatchCreator.cc
            BatchSystem.cc
            OAR1_6BatchSystem.cc
            OAR2_XBatchSystem.cc
            Loadleveler_BatchSystem.cc
            PBS_BatchSystem.cc
            SGE_BatchSystem.cc
            Eucalyptus_BatchSystem.cc
        )
        target_link_libraries(UtilsSeDBatch
          EucalyptusLib
          -lpthread
          -lssl
          -lcrypto
        )
    else (DIET_USE_CLOUD)
        # Build SeDBatch library without the Cloud component
        add_library(UtilsSeDBatch STATIC
            BatchCreator.cc
            BatchSystem.cc
            OAR1_6BatchSystem.cc
            OAR2_XBatchSystem.cc
            Loadleveler_BatchSystem.cc
            PBS_BatchSystem.cc
            SGE_BatchSystem.cc
        )
        add_dependencies(UtilsSeDBatch DIET_CORBA)
    endif (DIET_USE_CLOUD)
  endif (DIET_USE_ALT_BATCH)

  add_library(DIET_Utils STATIC
    ${DIET_Utils_SOURCES}
  )

  target_link_libraries(DIET_Utils UtilsNodes)
  if (DIET_USE_ALT_BATCH)
    target_link_libraries(DIET_Utils UtilsSeDBatch)
  endif (DIET_USE_ALT_BATCH)
  if (DIET_USE_WORKFLOW)
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
    Parsers.hh
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

