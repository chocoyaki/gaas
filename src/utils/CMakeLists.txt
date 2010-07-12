# DIET cmake local file
## Use Dagda as data manager.
IF( DIET_USE_DAGDA )
  SET (DIET_DAGDA_LIBRARY DIET_Dagda_static)
  ADD_SUBDIRECTORY( DAGDA )
  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA ${UUID_PATH} )
  ADD_LIBRARY( UtilsDagda STATIC
    DataRelationMgr.cc
  )
  TARGET_LINK_LIBRARIES( UtilsDagda UtilsCommon ${OMNIORB4_LIBRARIES})
#  INSTALL( TARGETS UtilsDagda DESTINATION lib)
ENDIF( DIET_USE_DAGDA )

IF( DIET_USE_FD )
  ADD_SUBDIRECTORY( fd )
ENDIF (DIET_USE_FD)

IF( DIET_BUILD_LIBRARIES )
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


  ########################################################################
  ### UtilsCommon library:
  ADD_LIBRARY( UtilsCommon STATIC
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
  )
  # The following dependency expresses that before compiling UtilsCommon one
  # first needs to generate the header files [like common_types.h] with idl.
  # Track down the definition of the commidity target
  # DIET_TARGET_IDL_GENERATED_HEADER_FILES for more details.
  ADD_DEPENDENCIES( UtilsCommon ALL DIET_TARGET_IDL_GENERATED_HEADER_FILES )
  TARGET_LINK_LIBRARIES( UtilsCommon IDLCommon ${OMNIORB4_LIBRARIES} )
#  INSTALL( TARGETS UtilsCommon DESTINATION lib )
  ########################################################################
  ### UtilsVector library:
  ADD_LIBRARY( UtilsVector STATIC
    Vector.c
  )
#  INSTALL( TARGETS UtilsVector DESTINATION lib )

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
#    INSTALL( TARGETS UtilsSeDBatch DESTINATION lib )
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
        TARGET_LINK_LIBRARIES( UtilsSeDBatch EucalyptusLib -lpthread UtilsCommon -lssl -lcrypto )
    ELSE( DIET_USE_CLOUD )
        # Build SeDBatch library without the Cloud component
        ADD_LIBRARY( UtilsSeDBatch STATIC
            BatchCreator.cc
            BatchSystem.cc
            OAR1_6BatchSystem.cc
            OAR2_XBatchSystem.cc
            Loadleveler_BatchSystem.cc
            PBS_BatchSystem.cc
            SGE_BatchSystem.cc )
        TARGET_LINK_LIBRARIES( UtilsSeDBatch -lpthread UtilsCommon )
    ENDIF( DIET_USE_CLOUD)
#    INSTALL( TARGETS UtilsSeDBatch DESTINATION lib )
  ENDIF( DIET_USE_ALT_BATCH )

ENDIF( DIET_BUILD_LIBRARIES )


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
    INSTALL( FILES ts_container/ts_set.hh  DESTINATION include/scheduler/ts_container )
ENDIF( DIET_USE_USERSCHED )

