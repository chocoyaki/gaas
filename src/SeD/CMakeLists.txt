#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  SET( DIET_SeD_SOURCES
   DIET_server.cc
   DataMgrImpl.cc
   SeDImpl.cc
  )
  
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/src/agent        # for e.g. ChildID.hh
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${DIET_SOURCE_DIR}/include          # for DIET_data.h 
    ${DIET_BINARY_DIR}/src/CORBA/idl
  )
  
  IF( DIET_USE_BATCH )
    INCLUDE_DIRECTORIES( ${APPLESEEDS_INCLUDE_DIR} )
  ENDIF( DIET_USE_BATCH )

  IF( DIET_USE_JUXMEM )
    INCLUDE_DIRECTORIES( ${JUXMEM_INCLUDE_DIR} )
  ENDIF( DIET_USE_JUXMEM )
  
  ADD_LIBRARY( DIET_SeD ${DIET_SeD_SOURCES} )
  
  TARGET_LINK_LIBRARIES( DIET_SeD
    CorbaCommon
    IDLAgent
    IDLCommon
    UtilsCommon
    UtilsNodes
    UtilsSeDClt
    UtilsVector
    ${OMNIORB4_LIBRARIES}
  )
  
  IF(DIET_USE_FD)
    TARGET_LINK_LIBRARIES( DIET_SeD fd)
  ENDIF(DIET_USE_FD)
  
  IF( DIET_USE_BATCH )
    TARGET_LINK_LIBRARIES( DIET_SeD ${APPLESEEDS_LIBRARIES} )
    TARGET_LINK_LIBRARIES( DIET_SeD UtilsSeDBatch )
  ENDIF( DIET_USE_BATCH )


  IF( DIET_USE_JUXMEM )
    TARGET_LINK_LIBRARIES( DIET_SeD ${JUXMEM_LIBRARIES} )
  ENDIF( DIET_USE_JUXMEM )
  
  INSTALL_TARGETS( /lib/ DIET_SeD )
###################################

  IF( DIET_USE_JXTA )

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.class
      COMMAND   ${JAVA_COMPILE}
      ARGS      -d ${CMAKE_CURRENT_BINARY_DIR}
      -classpath ${CMAKE_CURRENT_SOURCE_DIR}:${DIET_SOURCE_DIR}/src/lib/jxta.jar
                ${CMAKE_CURRENT_SOURCE_DIR}/JXTASeD.java
      DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/JXTASeD.java
      COMMENT   "java compile SeD"
    )

    ADD_CUSTOM_COMMAND(
      OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.h
      COMMAND   ${JAVA_HEADER_COMPILE}
      ARGS      -classpath ${CMAKE_CURRENT_BINARY_DIR}
                -d ${CMAKE_CURRENT_BINARY_DIR}
                JXTASeD
      DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.class
      COMMENT   "java header compile SeD"
    )
    SET_SOURCE_FILES_PROPERTIES( 
      ${CMAKE_CURRENT_SOURCE_DIR}/SeDJNI.cc
      PROPERTIES
        OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.h
        COMPILE_FLAGS "-I${JAVA_INCLUDE_PATH}"
    )

    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
    ADD_LIBRARY( SeDJNI ${CMAKE_CURRENT_SOURCE_DIR}/SeDJNI.cc )

    TARGET_LINK_LIBRARIES( SeDJNI
      DIET_SeD
      ${OMNIORB4_LIBRARIES}
    )

    INSTALL_TARGETS( /lib/ SeDJNI )

  ENDIF( DIET_USE_JXTA )
###################################

ENDIF( DIET_BUILD_LIBRARIES )
