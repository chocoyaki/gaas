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



if (DIET_USE_DAGDA)
  set(DIET_Utils_SOURCES
    ${DIET_Utils_SOURCES}
    DataRelationMgr.cc
  )
  add_subdirectory(DAGDA)
endif (DIET_USE_DAGDA)

ADD_SUBDIRECTORY(events)

IF( DIET_USE_FD )
  ADD_SUBDIRECTORY( fd )
ENDIF (DIET_USE_FD)

  INCLUDE_DIRECTORIES(
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
  IF( DIET_USE_BATCH )
    CONFIGURE_FILE(
      batch_configure.h.in
      ${CMAKE_CURRENT_BINARY_DIR}/batch_configure.h
      IMMEDIATE
    )

    INCLUDE_DIRECTORIES( ${APPLESEEDS_INCLUDE_DIR}
			 ${CMAKE_CURRENT_BINARY_DIR} )
    ADD_LIBRARY( UtilsSeDBatch STATIC batch.c )
    TARGET_LINK_LIBRARIES( UtilsSeDBatch ${APPLESEEDS_LIBRARIES} -lpthread )
    add_dependencies(UtilsSeDBatch DIET_CORBA)
  ENDIF( DIET_USE_BATCH )

  IF( DIET_USE_ALT_BATCH )
    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

    IF( DIET_USE_CLOUD)
        ########################################################################
        # Build the Eucalyptus SOAP client library and its associated BatchSystem
        INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} ${DIET_SOURCE_DIR}/src/utils/EucaLib
            ${GSOAP_SRC_DIR}
            )
        ADD_DEFINITIONS( -DWITH_DOM -DWITH_OPENSSL )

        # Yes, part of gSOAP needs to be recompiled in order to use the WSSE plugin.
        # This is the way it was built to work. There is no automatic build for
        # the WSSE plugin and therefore this is the only way.
        #
        # This is also what the gSOAP community advises in their doc inside the package:
        # doc/wsse/html/wsse.html
        # Save yourself some time and just read this next time.
        ADD_LIBRARY( EucalyptusLib
            EucaLib/ec2wrapper.c
            EucaLib/soapC.c
            EucaLib/soapClient.c

            ${GSOAP_SRC_DIR}/stdsoap2.c
            ${GSOAP_SRC_DIR}/dom.c
            ${GSOAP_SRC_DIR}/plugin/smdevp.c
            ${GSOAP_SRC_DIR}/plugin/wsseapi.c
            )
        
        TARGET_LINK_LIBRARIES( EucalyptusLib -lssl -lcrypto
        )
            ADD_DEFINITIONS( -DWITH_DOM -DWITH_OPENSSL )
        INSTALL_TARGETS( /lib/ EucalyptusLib )
            
        # Build SeDBatch library with the Cloud component
        INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} ${DIET_SOURCE_DIR}/src/utils/EucaLib
            ${GSOAP_INCLUDE_DIRS} ${GSOAP_PLUGIN_DIR} ${GSOAP_PLUGIN_DIR}/.. )
        ADD_LIBRARY( UtilsSeDBatch STATIC
            ${GSOAP_SRC_DIR}
            )
        ADD_LIBRARY( UtilsSeDBatch
            BatchCreator.cc
            BatchSystem.cc
            OAR1_6BatchSystem.cc
            OAR2_XBatchSystem.cc
            Loadleveler_BatchSystem.cc
            PBS_BatchSystem.cc
            SGE_BatchSystem.cc
            Eucalyptus_BatchSystem.cc )
        TARGET_LINK_LIBRARIES( UtilsSeDBatch EucalyptusLib -lpthread -lssl -lcrypto )
    ELSE( DIET_USE_CLOUD )
        # Build SeDBatch library without the Cloud component
        ADD_LIBRARY( UtilsSeDBatch STATIC
            BatchCreator.cc
            BatchSystem.cc
            OAR1_6BatchSystem.cc
            OAR2_XBatchSystem.cc
            Loadleveler_BatchSystem.cc
            PBS_BatchSystem.cc
            SGE_BatchSystem.cc
        )
        add_dependencies(UtilsSeDBatch DIET_CORBA)

    ENDIF( DIET_USE_CLOUD)
  ENDIF( DIET_USE_ALT_BATCH )

  add_library(DIET_Utils STATIC
    ${DIET_Utils_SOURCES}
  )

  target_link_libraries(DIET_Utils UtilsNodes)
  if (DIET_USE_ALT_BATCH)
    target_link_libraries(DIET_Utils UtilsSeDBatch)
  endif (DIET_USE_ALT_BATCH)
  if (DIET_USE_WORKFLOW)
    target_link_libraries(DIET_Utils UtilsWf UtilsEvents)
  endif (DIET_USE_WORKFLOW)
  if (DIET_USE_CORI)
    target_link_libraries(DIET_Utils UtilsCori)
  endif (DIET_USE_CORI)

  add_dependencies(DIET_Utils DIET_CORBA)

########################################################################
## New : For scheduler load support.
## Install the files needed for scheduler development.
IF( DIET_USE_USERSCHED )
    INSTALL( FILES
        est_internal.hh
	debug.hh
	LinkedList.hh
	DIET_data_internal.hh
	Vector.h
	Parsers.hh
	ms_function.hh

	DESTINATION
        include/scheduler
    )
    INSTALL(FILES ts_container/ts_set.hh  DESTINATION include/scheduler/ts_container )
ENDIF( DIET_USE_USERSCHED )

