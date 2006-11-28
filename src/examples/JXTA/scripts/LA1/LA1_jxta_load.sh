#****************************************************************************#
#* JXTA MA loader script                                                    *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Cedric Tedeschi cedric.tedeschi@ens-lyon.fr                         *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.5  2006/11/28 17:44:01  ctedesch
#* add old logs in header and update library path
#*
#* Revision 1.4  2006/11/28 17:36:05  ctedesch
#* header
#*
#* Revision 1.3 2004/08/27 16:28:18  ctedesch
#* - Use of the asynchronous PIF scheme for propagation of the requests inside
#* the DIET J multi-hierarchy
#* - DIET/JXTA -> DIET J
#* - Change the JXTA examples scripts to build a whole multi-hierarchy
#* 
#* Revision 1.2 2004/07/06 13:52:27  ctedesch
#* modify the scripts loading JXTA components (useless settings)
#* 
#* Revision 1.1 2004/06/11 15:45:39  ctedesch
#* add DIET/JXTA
#****************************************************************************#
#!/bin/sh
# Launch a LA via JNI with LA3.cfg as configuration file

export JXTA_LIB=${DIET_HOME}/src/lib
# To be configured
# export INSTALL_DIR=
# export EXECUTABLE_DIR=${INSTALL_DIR}/src/agent
java -cp ${EXECUTABLE_DIR} LA ${DIET_HOME}/src/examples/cfgs/LA3.cfg 
