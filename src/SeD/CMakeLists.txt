#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

## Sources
set(DIET_SeD_SOURCES
  DIET_server.cc
  SeDImpl.cc
)


if (DIET_USE_ALT_BATCH)  # in that case, put lib UtilsCoRI in lib DIET_SeD
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
  set(DIET_SeD_SOURCES
    ${DIET_SeD_SOURCES}
    ${DIET_SOURCE_DIR}/src/utils/batch/Cori_batch.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Data_Easy.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_CPU.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_Disk.cc
    ${DIET_SOURCE_DIR}/src/utils/nodes/Cori_Easy_Memory.cc
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
  ${DIET_SOURCE_DIR}/src/utils/config
)
  
if (DIET_USE_LOG)
  include_directories(
    ${LOG_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils/log)
endif (DIET_USE_LOG)

include_directories(${DIET_SOURCE_DIR}/src/utils/DAGDA)


add_library(DIET_SeD ${DIET_SeD_SOURCES})
SET_TARGET_PROPERTIES (DIET_SeD PROPERTIES VERSION ${DIET_VERSION})

# DIET_Sed shared & static libraries depends on CORBA files.
add_dependencies(DIET_SeD CORBAFiles)


## Dependencies towards other libraries  
# /!\ DIET_Dagda has to be included first (before any other DIET
# library), otherwise it won't work on APPLE.
set(DIET_SeD_Libs
  ${DIET_SeD_Libs}
  DIET_Dagda
  )

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
set(DIET_SeD_Libs
  ${DIET_SeD_Libs}
  UtilsCori
  )

target_link_libraries(DIET_SeD ${DIET_SeD_Libs})
  
if (DIET_USE_ALT_BATCH)
  target_link_libraries(DIET_SeD UtilsSeDBatch)
endif (DIET_USE_ALT_BATCH)


# Install DIET_SeD library only, not the static one
install(TARGETS DIET_SeD DESTINATION ${LIB_INSTALL_DIR})
