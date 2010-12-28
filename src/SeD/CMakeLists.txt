#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

## Sources
set(DIET_SeD_SOURCES
  DIET_server.cc
  SeDImpl.cc
)


if (NOT DIET_USE_DAGDA)
  set(DIET_SeD_SOURCES
      ${DIET_SeD_SOURCES}
      DataMgrImpl.cc
  )
endif(NOT DIET_USE_DAGDA)

if (DIET_USE_ALT_BATCH)  # in that case, put lib UtilsCoRI in lib DIET_SeD
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
  set(DIET_SeD_SOURCES
    ${DIET_SeD_SOURCES}
    ${DIET_SOURCE_DIR}/src/utils/batch/Cori_batch.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/FASTMgr.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Data_Easy.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_CPU.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_Disk.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_Memory.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Fast.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Metric.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/CORIMgr.cc
  )
endif (DIET_USE_ALT_BATCH)
  
include_directories(${OMNIORB4_INCLUDE_DIR}
  ${DIET_SOURCE_DIR}/src/utils
  ${DIET_SOURCE_DIR}/src/utils/nodes
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DIET_SOURCE_DIR}/src/agent        # for e.g. ChildID.hh
  ${DIET_SOURCE_DIR}/src/CORBA        # for e.g. marshalling.hh
  ${DIET_SOURCE_DIR}/include          # for DIET_data.h 
  ${DIET_BINARY_DIR}/src/CORBA/idl
)
  
if (DIET_USE_LOG)
  include_directories(
    ${LOG_SRC_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)

if (DIET_USE_JUXMEM)
  include_directories(${JUXMEM_INCLUDE_DIR})
endif (DIET_USE_JUXMEM)

if (DIET_USE_DAGDA)
  include_directories(${DIET_SOURCE_DIR}/src/utils/DAGDA ${UUID_PATH})
endif (DIET_USE_DAGDA)


add_library(DIET_SeD ${DIET_SeD_SOURCES})
SET_TARGET_PROPERTIES (DIET_SeD PROPERTIES VERSION ${DIET_VERSION})

# DIET_Sed shared & static libraries depends on CORBA files.
add_dependencies(DIET_SeD CORBAFiles)


## Dependencies towards other libraries  
# To use Dagda instead of DTM.
if (DIET_USE_DAGDA)
  # /!\ DIET_Dagda has to be included first (before any other DIET
  # library), otherwise it won't work on APPLE.
  set(DIET_SeD_Libs
    ${DIET_SeD_Libs}
    ${UUID_LIB}
    DIET_Dagda
  )
endif (DIET_USE_DAGDA)

# Link with log only if log enabled
if (DIET_USE_LOG)
  set(DIET_SeD_Libs
    ${DIET_SeD_Libs}
    DietLogLibrary)
endif (DIET_USE_LOG)

# common DIET libs
set(DIET_SeD_Libs
  ${DIET_SeD_Libs}
  DIET_CORBA
  CorbaCommon
  UtilsNodes
  DIET_Utils
  ${OMNIORB4_LIBRARIES}
)


# CORI
if (NOT DIET_USE_ALT_BATCH)
  if (DIET_USE_CORI)      # Then the lib is compiled independantly
    set(DIET_SeD_Libs
      ${DIET_SeD_Libs}
      UtilsCori
    )
  endif (DIET_USE_CORI)
endif (NOT DIET_USE_ALT_BATCH)

# Fault detection
if (DIET_USE_FD)
  set(DIET_SeD_Libs
    ${DIET_SeD_Libs}
    fd
  )
endif (DIET_USE_FD)



target_link_libraries(DIET_SeD ${DIET_SeD_Libs})
  
if (DIET_USE_ALT_BATCH)
  target_link_libraries(DIET_SeD UtilsSeDBatch)
endif (DIET_USE_ALT_BATCH)

if (DIET_USE_JUXMEM)
  target_link_libraries(DIET_SeD ${JUXMEM_LIBRARIES})
endif (DIET_USE_JUXMEM)

if (NOT BUILD_SHARED_LIBS)
  target_link_libraries(DIET_SeD stdc++)
endif (NOT BUILD_SHARED_LIBS)


# Install DIET_SeD library only, not the static one
install(TARGETS DIET_SeD DESTINATION ${LIB_INSTALL_DIR})



## JXTA
if (DIET_USE_JXTA)
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.class
    COMMAND   ${JAVA_COMPILE}
    ARGS      -d ${CMAKE_CURRENT_BINARY_DIR}
    -classpath ${CMAKE_CURRENT_SOURCE_DIR}:${DIET_SOURCE_DIR}/src/lib/jxta.jar
              ${CMAKE_CURRENT_SOURCE_DIR}/JXTASeD.java
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/JXTASeD.java
    COMMENT   "java compile SeD"
  )

  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.h
    COMMAND   ${JAVA_HEADER_COMPILE}
    ARGS      -classpath ${CMAKE_CURRENT_BINARY_DIR}
              -d ${CMAKE_CURRENT_BINARY_DIR}
              JXTASeD
    DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.class
    COMMENT   "java header compile SeD"
  )
  set_source_files_properties( 
    ${CMAKE_CURRENT_SOURCE_DIR}/SeDJNI.cc
    PROPERTIES
      OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/JXTASeD.h
      COMPILE_FLAGS "-I${JAVA_INCLUDE_PATH}"
  )

  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_library(SeDJNI ${CMAKE_CURRENT_SOURCE_DIR}/SeDJNI.cc)

  target_link_libraries(SeDJNI
    DIET_SeD
    ${OMNIORB4_LIBRARIES}
  )

  install(TARGETS SeDJNI DESTINATION lib)

endif (DIET_USE_JXTA)
