set( LATEX_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_latex_document( UsersFAQ.tex
  IMAGE_DIRS fig
  IMAGES
  fig/logo_DIET_big.png
  fig/logo_DIET.png
  MANGLE_TARGET_NAMES
  DEFAULT_PDF )

add_dependencies( doc UsersFAQ_pdf )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/UsersFAQ.pdf
  DESTINATION ${DOC_INSTALL_DIR} )

## FIXME: if you want to compile the doc on windaube
#if( WIN32 )
#  exec_program( mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/Data )
#  exec_program( cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/client.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_client.c )
#  exec_program( cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/batch_server.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_batch_server.c )
#  exec_program( cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/sequential_server.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_sequential_server.c )
#endif()