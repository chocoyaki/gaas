#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

PROJECT(DIET_UsersFAQ NONE)

## FIXME: if you want to compile the doc on windaube
#EXEC_PROGRAM(mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/Data)
#EXEC_PROGRAM(cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/client.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_client.c)
#EXEC_PROGRAM(cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/batch_server.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_batch_server.c)
#EXEC_PROGRAM(cp ARGS ${DIET_DOC_SOURCE_DIR}/../src/examples/Batch/sequential_server.c ${CMAKE_CURRENT_BINARY_DIR}/Data/examples_Batch_sequential_server.c)

SET ( LATEX_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE(${DIET_DOC_SOURCE_DIR}/Cmake/UseLATEX.cmake)

# FAQ
ADD_LATEX_DOCUMENT(UsersFAQ.tex 
		   IMAGE_DIRS fig 
		   IMAGES       
				fig/logo_DIET_big.png
				fig/logo_DIET.png
		   DEFAULT_PDF
		   )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/UsersFAQ.pdf DESTINATION doc )
