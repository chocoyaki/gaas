#****************************************************************************#
#* DIET cmake local file                                                    *#
#****************************************************************************#

PROJECT(DIET_UsersManual NONE)

SET ( LATEX_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE(${DIET_DOC_SOURCE_DIR}/Cmake/UseLATEX.cmake)
ADD_LATEX_DOCUMENT(UsersManual.tex 
		   INPUTS description.tex 
		          install.tex 
 			  data.tex 
			  client.tex
			  server.tex 
			  batch.tex
			  plugin.tex
			  prediction.tex
			  deploy.tex
			  dashboard.tex
			  multima.tex
			  jxta.tex
			  juxmem.tex
			  workflow.tex
			  xml_example.tex
                          agent_scheduler.tex
                          dagda.tex
		   BIBFILES UsersManual.bib 
		   IMAGE_DIRS fig 
		   IMAGES fig/DIET_arch_request-2.ps
				fig/FAST.fig
				fig/JXTAConfig.ps
				fig/VizDIET.pdf
				fig/VizDIET_screenshot_2.png
				fig/data.fig
				fig/global_platform.fig
				fig/global_platform_jxta.fig
				fig/init.fig
				fig/logo_DIET_big.png
				fig/multima.fig
				fig/overviewCori.fig
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
		   DEFAULT_PDF
		   )
INSTALL_FILES(/doc FILES  ${CMAKE_CURRENT_BINARY_DIR}/UsersManual.pdf)
