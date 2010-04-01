#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#
# TODO (YC): when no appleseeds anymore, DIET_USE_ALT_BATCH only!

IF( NOT DIET_USE_BATCH AND NOT DIET_USE_ALT_BATCH )
  MESSAGE( STATUS "Batch related example NOT compiled." )
ELSE( NOT DIET_USE_BATCH AND NOT DIET_USE_ALT_BATCH )

IF( DIET_USE_BATCH )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### Two servers...
  SET( C_SERVER_EXAMPLES
       batch_server 
       sequential_server
       parallel_server
       )

  FOREACH( loop_var ${C_SERVER_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_SERVER_LIBRARIES} )
    SET_TARGET_PROPERTIES( ${loop_var} PROPERTIES LINKER_LANGUAGE CXX )
    INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/Batch )
  ENDFOREACH( loop_var )
  
  ### ... and their respective clients
  SET( C_CLIENT_EXAMPLES client async_client )
  
  FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/Batch )
  ENDFOREACH( loop_var )

  ### Do Lammps example
  #  ADD_SUBDIRECTORY( Lammps )

ELSE( DIET_USE_BATCH )

  INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/include )

  ### Two servers...
  SET( C_SERVER_EXAMPLES
    batch_server )

  FOREACH( loop_var ${C_SERVER_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_SERVER_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/Batch )
  ENDFOREACH( loop_var )
  
  ### ... and their respective clients
  SET( C_CLIENT_EXAMPLES client async_client )
  
  FOREACH( loop_var ${C_CLIENT_EXAMPLES} )
    ADD_EXECUTABLE( ${loop_var} ${loop_var}.c )
    TARGET_LINK_LIBRARIES( ${loop_var} ${DIET_CLIENT_LIBRARIES} )
    INSTALL( TARGETS ${loop_var} DESTINATION bin/examples/Batch )
  ENDFOREACH( loop_var )

  ### Do Lammps example, Decrypthon suites, SparseSolver/PasTiX prototype
  # ADD_SUBDIRECTORY( Lammps )
  ADD_SUBDIRECTORY( TestAllBatch )
  # ADD_SUBDIRECTORY( Cori_cycle_stealing )

ENDIF( DIET_USE_BATCH )

ENDIF( NOT DIET_USE_BATCH AND NOT DIET_USE_ALT_BATCH )
