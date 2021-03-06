#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( NOT DIET_USE_DELTACLOUD )
  MESSAGE( STATUS "DELTACLOUD related example NOT compiled." )
ELSE( NOT DIET_USE_DELTACLOUD )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### ... and its respective client
  SET( CC_CLIENT_EXAMPLES test-delta-cloud test2-delta-cloud test3-delta-cloud test4-delta-cloud)

  FOREACH( loop_var ${CC_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
    TARGET_LINK_LIBRARIES( ${loop_var} DIET_DeltaCloud )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud )
  ENDFOREACH( loop_var )

  SET (CC_SERVER_EXAMPLES test-sed-cloud test-sed-cloud-without-vm test-sed-cloud-vm-launched-at-first-solve test-sed-cloud-vm-launched-at-solve-then-destroyed test-sed-cloud-with-vm-init-service test-sed-cloud-with-vm-destructor-service test-dagda-sed)
  FOREACH( loop_var ${CC_SERVER_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
    TARGET_LINK_LIBRARIES( ${loop_var} DIET_DeltaCloud )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud )
  ENDFOREACH( loop_var )

  SET( CC_DIET_CLIENT_EXAMPLES test-client-cloud test-client-cloud-with-vm-init-service test-dagda-client test-dagda-client-send-file)

  FOREACH( loop_var ${CC_DIET_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud )
  ENDFOREACH( loop_var )


  INSTALL(DIRECTORY matrixAdd DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud FILES_MATCHING PATTERN "*" PATTERN "*~" EXCLUDE)
  INSTALL(FILES mat1.txt mat2.txt DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud)

ENDIF( NOT DIET_USE_DELTACLOUD )
