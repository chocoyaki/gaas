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
#* Revision 1.4  2006/11/28 17:46:15  ctedesch
#* header and update library path
#*
#****************************************************************************#

#!/bin/sh
# Launch a JXTA SeD providing MatSUM with SeD1.cfg as configuration file for the DIET SeD

export JXTA_LIB=${DIET_HOME}/src/lib
# To be configured
# export INSTALL_DIR=
# export EXECUTABLE_DIR=${INSTALL_DIR}/src/SeD
java -cp ${EXECUTABLE_DIR}:${JXTA_LIB}/jxtaext.jar:${JXTA_LIB}/jaxen-core.jar:/${JXTA_LIB}/jxtaSwing.jar:${JXTA_LIB}/jaxen-jdom.jar:${JXTA_LIB}/jdom.jar:${JXTA_LIB}/saxpath.jar:${JXTA_LIB}/bcprov-jdk14.jar:${JXTA_LIB}/jxta.jar:${JXTA_LIB}/jxtasecurity.jar:${JXTA_LIB}/junit.jar:${JXTA_LIB}/cryptix32.jar:${JXTA_LIB}/org.mortbay.jetty.jar:${JXTA_LIB}/cryptix-asn1.jar:${JXTA_LIB}/jxtaptls.jar:${JXTA_LIB}/log4j.jar:${JXTA_LIB}/javax.servlet.jar JXTASeD ${DIET_HOME}/src/examples/cfgs/SeD1.cfg MatSUM

