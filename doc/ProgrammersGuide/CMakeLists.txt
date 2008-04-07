#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

PROJECT(ProgrammersGuide NONE)

SET (LATEX_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../gendoc)
INCLUDE(${DIET_SOURCE_DIR}/Cmake/UseLATEX.cmake)
ADD_LATEX_DOCUMENT(ProgrammersGuide.tex 
		   INPUTS start.tex
			  source_tree.tex
			  CS.tex
			  batch.tex
			  debug.tex
			  Annexe1.tex
		   IMAGE_DIRS fig/ 
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
		   DEFAULT_PDF
		   )


