#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( DIET_BUILD_LIBRARIES )
  SET( DIET_SeD_SOURCES
   DIET_server.cc
   SeDImpl.cc
  )
IF (NOT DIET_USE_DAGDA)
  SET(DIET_SeD_SOURCES
      ${DIET_SeD_SOURCES}
      DataMgrImpl.cc
  )
ENDIF(NOT DIET_USE_DAGDA)

  IF( DIET_USE_ALT_BATCH )    # in that case, put lib UtilsCoRI in lib DIET_SeD
    SET( DIET_SeD_SOURCES
	 ${DIET_SeD_SOURCES}
         ${DIET_SOURCE_DIR}/src/utils/Cori_batch.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/FASTMgr.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Data_Easy.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_CPU.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_Disk.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_Memory.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Fast.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Metric.cc
         ${DIET_SOURCE_DIR}/src/utils/nodes/CORIMgr.cc )
  ENDIF( DIET_USE_ALT_BATCH )
  
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/utils/nodes
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/src/agent        # for e.g. ChildID.hh
    ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
    ${DIET_SOURCE_DIR}/include          # for DIET_data.h 
    ${DIET_BINARY_DIR}/src/CORBA/idl
  )
  
  IF( DIET_USE_JUXMEM )
    INCLUDE_DIRECTORIES( ${JUXMEM_INCLUDE_DIR} )
  ENDIF( DIET_USE_JUXMEM )
  
  # To use Dagda instead of DTM.
  IF( DIET_USE_DAGDA )
    INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA ${UUID_PATH} )
    SET( DAGDA_LIBRARIES
         IDLDagda
	 Dagda
	 DIET_Dagda_static )
  ENDIF( DIET_USE_DAGDA )

  ADD_LIBRARY( DIET_SeD SHARED ${DIET_SeD_SOURCES} )
  ADD_LIBRARY( DIET_SeD_static STATIC ${DIET_SeD_SOURCES} )
  
  SET( DIET_SeDTmpLib
       CorbaCommon
       UtilsNodes
       IDLAgent
       IDLCommon
       UtilsCommon
       UtilsVector
       ${OMNIORB4_LIBRARIES}
       ${DAGDA_LIBRARIES} )

  IF( NOT DIET_USE_ALT_BATCH )
    IF( DIET_USE_CORI )      # Then the lib is compiled independantly
      SET( DIET_SeDTmpLib
	   ${DIET_SeDTmpLib}
	   UtilsCori )
    ENDIF( DIET_USE_CORI )
  ENDIF( NOT DIET_USE_ALT_BATCH )

  TARGET_LINK_LIBRARIES( DIET_SeD
			 ${DIET_SeDTmpLib} )
  TARGET_LINK_LIBRARIES( DIET_SeD_static
			 ${DIET_SeDTmpLib} ) 

  IF(DIET_USE_FD)
    TARGET_LINK_LIBRARIES( DIET_SeD fd )
    TARGET_LINK_LIBRARIES( DIET_SeD_static fd )
 ENDIF(DIET_USE_FD)
  
  IF( DIET_USE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( DIET_SeD UtilsSeDBatch )
    TARGET_LINK_LIBRARIES( DIET_SeD_static UtilsSeDBatch )
  ENDIF( DIET_USE_ALT_BATCH )

  IF( DIET_USE_JUXMEM )
    TARGET_LINK_LIBRARIES( DIET_SeD ${JUXMEM_LIBRARIES} )
    TARGET_LINK_LIBRARIES( DIET_SeD_static ${JUXMEM_LIBRARIES} )
  ENDIF( DIET_USE_JUXMEM )

  IF( NOT BUILD_SHARED_LIBS )
    TARGET_LINK_LIBRARIES( DIET_SeD stdc++ )
  ENDIF( NOT BUILD_SHARED_LIBS )
      
  
  INSTALL( TARGETS DIET_SeD DESTINATION lib )
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

    INSTALL( TARGETS SeDJNI DESTINATION lib )

  ENDIF( DIET_USE_JXTA )
###################################

ENDIF( DIET_BUILD_LIBRARIES )
