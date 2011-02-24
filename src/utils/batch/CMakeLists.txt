########################################################################
### UtilsSedBatch library:
if (DIET_USE_ALT_BATCH)
  include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/src/utils/config
    )

  set(UtilsSeDBatch_SOURCES
      BatchCreator.cc
      BatchSystem.cc
      OAR1_6BatchSystem.cc
      OAR2_XBatchSystem.cc
      Loadleveler_BatchSystem.cc
      PBS_BatchSystem.cc
      SGE_BatchSystem.cc
      )

  if (DIET_USE_CLOUD)
    ########################################################################
    # Build the Eucalyptus SOAP client library and its associated BatchSystem
    include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/EucaLib
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
      ${GSOAP_INCLUDE_DIRS}
      ${GSOAP_PLUGIN_DIR}
      ${GSOAP_PLUGIN_DIR}/..
      )
    #        add_library(UtilsSeDBatch STATIC
    #            ${GSOAP_SRC_DIR}
    #        )
    set(UtilsSeDBatch_SOURCES
      ${UtilsSeDBatch_SOURCES}
      Eucalyptus_BatchSystem.cc
      )
  endif (DIET_USE_CLOUD)

  # Build SeDBatch library
  add_library(UtilsSeDBatch STATIC
    ${UtilsSeDBatch_SOURCES}
    )
  add_dependencies(UtilsSeDBatch DIET_CORBA)

  if (DIET_USE_CLOUD)
    target_link_libraries(UtilsSeDBatch
      EucalyptusLib
      -lpthread
      -lssl
      -lcrypto
      )
  endif (DIET_USE_CLOUD)
endif (DIET_USE_ALT_BATCH)
