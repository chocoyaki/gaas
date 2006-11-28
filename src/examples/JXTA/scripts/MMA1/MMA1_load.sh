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
#* Revision 1.6  2006/11/28 17:23:51  ctedesch
#* change library path
#*
#* Revision 1.5  2006/11/28 17:09:00  ctedesch
#* add old logs in header
#*
#* Revision 1.4  2006/11/28 17:06:27  ctedesch
#* header
#*
#* Revision 1.3 2004/07/09 19:51:58  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#* 
#* Revision 1.2 2004/07/06 13:52:27  ctedesch
#* modify the scripts loading JXTA components (useless settings)
#* Revision 1.1 2004/06/11 15:45:39  ctedesch
#* add DIET/JXTA
#****************************************************************************#
#!/bin/sh
# Launch a JXTA MA with MA1.cfg as configuration file for the DIET MA

export JXTA_LIB=${DIET_HOME}/src/lib
# To be configured
# export INSTALL_DIR=
# export EXECUTABLE_DIR=${INSTALL_DIR}/src/agent

java -cp ${EXECUTABLE_DIR}:${JXTA_LIB}/bcprov-jdk14.jar:${JXTA_LIB}/jxtasecurity.jar:${JXTA_LIB}/cryptix32.jar:${JXTA_LIB}/jxtaext.jar:${JXTA_LIB}/jaxen-core.jar:/${JXTA_LIB}/jxtaSwing.jar:${JXTA_LIB}/jaxen-jdom.jar:${JXTA_LIB}/jdom.jar:${JXTA_LIB}/jxta.jar:${JXTA_LIB}/junit.jar:${JXTA_LIB}/org.mortbay.jetty.jar:${JXTA_LIB}/cryptix-asn1.jar:${JXTA_LIB}/jxtaptls.jar:${JXTA_LIB}/log4j.jar:${JXTA_LIB}/javax.servlet.jar JXTAMultiMA ${DIET_HOME}/src/examples/cfgs/MA1.cfg

