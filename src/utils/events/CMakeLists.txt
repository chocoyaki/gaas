IF( DIET_BUILD_LIBRARIES )
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/include
    ${DIET_SOURCE_DIR}/src/utils
  )

  ########################################################################
  ### UtilsEvents library

  SET(   UtilsEventsSource
	 EventBase.cc
	 EventDispatcher.cc
# 	 EventObserver.cc
	 EventManager.cc )

  ADD_LIBRARY( UtilsEvents STATIC ${UtilsEventsSource})


ENDIF( DIET_BUILD_LIBRARIES )
