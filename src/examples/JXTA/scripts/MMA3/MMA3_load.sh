#!/bin/sh
# Launch a JXTA MA with MA3.cfg as configuration file for the DIET MA

# To be configured
#export DIET_HOME=

java -cp ${DIET_HOME}/src/agent:${JXTA_LIB}/jxtaext.jar:${JXTA_LIB}/jaxen-core.jar:/${JXTA_LIB}/jxtaSwing.jar:${JXTA_LIB}/jaxen-jdom.jar:${JXTA_LIB}/jdom.jar:${JXTA_LIB}/saxpath.jar:${JXTA_LIB}/bcprov-jdk14.jar:${JXTA_LIB}/jxta.jar:${JXTA_LIB}/jxtasecurity.jar:${JXTA_LIB}/junit.jar:${JXTA_LIB}/cryptix32.jar:${JXTA_LIB}/org.mortbay.jetty.jar:${JXTA_LIB}/cryptix-asn1.jar:${JXTA_LIB}/jxtaptls.jar:${JXTA_LIB}/log4j.jar:${JXTA_LIB}/javax.servlet.jar JXTAMultiMA ${DIET_HOME}/src/examples/cfgs/MA3.cfg
