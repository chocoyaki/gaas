set( LATEX_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_latex_document(UsersManual.tex
  INPUTS description.tex
  install.tex
  data.tex
  client.tex
  server.tex
  batch.tex
  cloud.tex
  plugin.tex
  forwarder.tex
  prediction.tex
  deploy.tex
  dashboard.tex
  multima.tex
  workflow.tex
  xml_example.tex
  agent_scheduler.tex
  dagda.tex
  client_scheduler.tex
  dynamic.tex
  appendix.tex
  BIBFILES UsersManual.bib
  IMAGE_DIRS fig
  IMAGES fig/DIET_arch_request-2.pdf
  fig/init.pdf
  fig/data.pdf
  fig/global_platform.pdf
  fig/VizDIET.pdf
  fig/VizDIET_screenshot_2.png
  fig/logo_DIET_big.png
  fig/multima.pdf
  fig/overviewCori.pdf
  fig/smprod.fig
  fig/wf_example1.fig
  fig/wf_archi2.fig
  fig/wf_archi1.fig
  fig/DiagrammeGridRPCDietStructure.fig
  fig/logo_DIET.png
  fig/DIETScheduling.pdf
  fig/schdl00.pdf
  fig/schdl01.pdf
  fig/schdl02.pdf
  fig/schdl03.pdf
  fig/schdl04.pdf
  fig/schdl05.pdf
  fig/dagdaArch.pdf
  MANGLE_TARGET_NAMES
  DEFAULT_PDF )

add_dependencies( doc UsersManual_pdf )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.pdf
  DESTINATION ${DOC_INSTALL_DIR} )

## FIXME: if you want to compile the doc on windaube
#if( WIN32 )
#  exec_program( mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/Data )
#  exec_program( cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/client.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_client.c )
#  exec_program( cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/batch_server.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_batch_server.c )
#  exec_program( cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/sequential_server.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_sequential_server.c )
#endif()
