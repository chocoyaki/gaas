#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

if (DIET_USE_ALT_BATCH) 
  include_directories(${DIET_SOURCE_DIR}/src/utils/batch)
endif (DIET_USE_ALT_BATCH)  

  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/utils
    ${DIET_SOURCE_DIR}/src/utils/events
    ${DIET_SOURCE_DIR}/src/utils/workflow
    ${DIET_BINARY_DIR}/src/CORBA/idl
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include
    ${DIET_SOURCE_DIR}/src/SeD
    )

  INCLUDE_DIRECTORIES( ${XERCES_INCLUDE_DIR} )

  ADD_LIBRARY( CltWf STATIC
    CltDagNodeLauncher.cc
    CltWfLogCentralDispatcher.cc
    CltWfMgr.cc )

# CltWf depends on CORBAFiles
add_dependencies(CltWf CORBAFiles)

  TARGET_LINK_LIBRARIES( CltWf
    DIET_Utils
  )
