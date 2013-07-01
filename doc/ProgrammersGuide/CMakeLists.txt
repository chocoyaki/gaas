set( LATEX_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_latex_document( ProgrammersGuide.tex
  INPUTS start.tex
  source_tree.tex
  CS.tex
  batch.tex
  cloud.tex
  cloudnv.tex
  debug.tex
  package.tex
  Annexe1.tex
  #IMAGE_DIRS fig
  IMAGES fig/DiagrammeGridRPCDietStructure.fig
  fig/logo_DIET.png
  fig/DiagrammeAclocal.fig
  fig/DiagrammeAutoheader.png
  fig/DiagrammeAutomakeLibtoolize.png
  fig/DiagrammeAutoconf.png
  fig/DiagrammeConfigure.png
  fig/CorbaClientClassDIagram.png
  fig/CallBackSynchronisationClassDiagram2.png
  fig/WaitRulesClassDiagram.png
  fig/CallAsyncSequenceDiagram.png
  fig/CallAsyncWaitSequenceDiagram.png
  fig/CallbackSynchronisationSequenceDiagram.png
  fig/CallAsyncProbeSequenceDiagram1.png
  fig/DietCancelSequenceDiagram.png
  fig/ActivityDiagramSedCloud.png
  fig/communicationDiagramSedCloud.png
  MANGLE_TARGET_NAMES
  DEFAULT_PDF )

add_dependencies( doc ProgrammersGuide_pdf )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/ProgrammersGuide.pdf
  DESTINATION ${DOC_INSTALL_DIR} )

