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
		   DEFAULT_PDF
		   )


