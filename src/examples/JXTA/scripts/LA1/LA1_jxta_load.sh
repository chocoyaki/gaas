#!/bin/sh
# Launch a LA via JNI with LA3.cfg as configuration file

# To be configured
#export DIET_HOME=


java -cp ${DIET_HOME}/src/agent LA ${DIET_HOME}/src/examples/cfgs/LA3.cfg 
