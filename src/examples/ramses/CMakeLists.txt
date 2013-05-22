#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

IF( NOT DIET_USE_DELTACLOUD )
  MESSAGE( STATUS "DELTACLOUD related example NOT compiled." )
ELSE( NOT DIET_USE_DELTACLOUD )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### ... and its respective client
  SET( CC_SERVER_EXAMPLES test-ramses-sed-cloud-without-vm test-ramses-sed-cloud-with-vm-init-service)

  FOREACH( loop_var ${CC_SERVER_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
    TARGET_LINK_LIBRARIES( ${loop_var} DIET_DeltaCloud )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/ramses )
  ENDFOREACH( loop_var )

  SET( CC_DIET_CLIENT_EXAMPLES test-ramses-client-cloud test-ramses3d-client-cloud test-ramses-client-wf)

  FOREACH( loop_var ${CC_DIET_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.cc )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud )
  ENDFOREACH( loop_var )


#  INSTALL(DIRECTORY matrixAdd DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud FILES_MATCHING PATTERN "*" PATTERN "*~" EXCLUDE)
#  INSTALL(FILES mat1.txt mat2.txt DESTINATION ${EXAMPLE_INSTALL_DIR}/delta-cloud)

ENDIF( NOT DIET_USE_DELTACLOUD )
