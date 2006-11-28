#****************************************************************************#
#* JXTA SeD loader script                                                   *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Cedric Tedeschi cedric.tedeschi@ens-lyon.fr                         *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.2  2006/11/28 17:52:27  ctedesch
#*  header
#*
#****************************************************************************#

#!/bin/sh
# Launch a JXTA SeD providing MatPROD with SeD2.cfg as configuration file for the DIET SeD

# To be configured
# export DIET_HOME=
# export JXTA_LIB=${DIET_HOME}/install/lib

java -cp ${DIET_HOME}/src/SeD:${JXTA_LIB}/jxtaext.jar:${JXTA_LIB}/jaxen-core.jar:/${JXTA_LIB}/jxtaSwing.jar:${JXTA_LIB}/jaxen-jdom.jar:${JXTA_LIB}/jdom.jar:${JXTA_LIB}/saxpath.jar:${JXTA_LIB}/bcprov-jdk14.jar:${JXTA_LIB}/jxta.jar:${JXTA_LIB}/jxtasecurity.jar:${JXTA_LIB}/junit.jar:${JXTA_LIB}/cryptix32.jar:${JXTA_LIB}/org.mortbay.jetty.jar:${JXTA_LIB}/cryptix-asn1.jar:${JXTA_LIB}/jxtaptls.jar:${JXTA_LIB}/log4j.jar:${JXTA_LIB}/javax.servlet.jar JXTASeD ${DIET_HOME}/src/examples/cfgs/SeD3.cfg MaSUM


