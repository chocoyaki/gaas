#****************************************************************************#
#* JXTA client loader script                                                *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Cedric Tedeschi cedric.tedeschi@ens-lyon.fr                         *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$#
#* $Log$
#* Revision 1.6  2006/11/28 17:04:35  ctedesch
#* update the path for JXTA libraries
#*
#* Revision 1.5  2006/11/28 17:02:03  ctedesch
#* add old logs in header
#*
#* Revision 1.4  2006/11/28 16:57:57  ctedesch
#* header
#*
#* Revision 1.3
#* date: 2004/07/09 19:51:58  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#*
#* Revision 1.2
#* date: 2004/07/06 13:52:27  ctedesch
#* modify the scripts loading JXTA components (useless settings)
#* 
#*  Revision 1.1
#* date: 2004/06/11 15:45:40  ctedesch
#* add DIET/JXTA
#****************************************************************************#

#!/bin/sh
# Launch a JXTA client requesting for MatSUM

export JXTA_LIB=${DIET_HOME}/src/lib

java -cp .:${DIET_HOME}/src/examples/JXTA:${JXTA_LIB}/jxtaext.jar:${JXTA_LIB}/jaxen-core.jar:/${JXTA_LIB}/jxtaSwing.jar:${JXTA_LIB}/jaxen-jdom.jar:${JXTA_LIB}/jdom.jar:${JXTA_LIB}/saxpath.jar:${JXTA_LIB}/bcprov-jdk14.jar:${JXTA_LIB}/jxta.jar:${JXTA_LIB}/jxtasecurity.jar:${JXTA_LIB}/junit.jar:${JXTA_LIB}/cryptix32.jar:${JXTA_LIB}/org.mortbay.jetty.jar:${JXTA_LIB}/cryptix-asn1.jar:${JXTA_LIB}/jxtaptls.jar:${JXTA_LIB}/log4j.jar:${JXTA_LIB}/javax.servlet.jar JXTAClient MatSUM

