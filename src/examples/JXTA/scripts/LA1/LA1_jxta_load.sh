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
#* Revision 1.4  2006/11/28 17:36:05  ctedesch
#* header
#*
#****************************************************************************#
#!/bin/sh
# Launch a LA via JNI with LA3.cfg as configuration file

# To be configured
#export DIET_HOME=


java -cp ${DIET_HOME}/src/agent LA ${DIET_HOME}/src/examples/cfgs/LA3.cfg 
