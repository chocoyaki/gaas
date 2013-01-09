#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( NOT DIET_USE_DELTACLOUD )
  MESSAGE( STATUS "DELTACLOUD related example NOT compiled." )
ELSE( NOT DIET_USE_DELTACLOUD )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### ... and its respective client
  SET( CC_CLIENT_EXAMPLES test-delta-cloud test2-delta-cloud test3-delta-cloud)
  
  FOREACH( loop_var ${CC_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
    #ADD_DEFINITIONS( -DHAVE_ALT_BATCH )
    TARGET_LINK_LIBRARIES( ${loop_var} DIET_DeltaCloud )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud )
  ENDFOREACH( loop_var )

  #INSTALL( FILES A.in DESTINATION ${EXAMPLE_INSTALL_DIR}/cloud )
  #INSTALL( FILES B.in DESTINATION ${EXAMPLE_INSTALL_DIR}/cloud )
ENDIF( NOT DIET_USE_DELTACLOUD )
