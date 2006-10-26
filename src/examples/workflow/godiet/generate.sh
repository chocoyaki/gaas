#! /bin/sh

# The following variable represents the services used 
# to execute workflows
scalar_single_srv="scalar_server"
scalar_mul_srv="succ double sum square"
file_srv="greyscale flip duplicate"
string_srv="strlen sum int2str"
matrix_srv="dmat"

# For services (or SeDs) with parameters, we need to 
# define the corresponding value.
# To add new service <toto>, just add a value toto_params with the right value
dmat_params="all"

examples="scalar_single_srv scalar_mul_srv file_srv string_srv matrix_srv"

# Check if all needed variables are defined
checkVar() {
    if [ x$HOME = x ]
	then
	echo "HOME not defined"
	exit 1
    fi

    if [ x$OMNIORB_HOME = x ]
	then
	echo "OMNIORB_HOME not defined"
	exit 1
    fi

    if [ x$DIET_DIR = x ]
	then
	echo "DIET_DIR not defined"
	exit 1
    fi

}

# create an intermediary xml file by replacing the HOME, 
# OMNIORB_HOME and DIET_DIR with the corresponding values
#
# $1 fileName
putVar() {
    fileName=$1
    cat godiet-template.xml | sed -e "s|\$HOME|$HOME|g" > $fileName.1
    cat $fileName.1 | sed -e "s|\$OMNIORB_HOME|$OMNIORB_HOME|g" > $fileName.2
    cat $fileName.2 | sed -e "s|\$DIET_DIR|$DIET_DIR|g" > $fileName.3
    rm -f $fileName.1 $fileName.2
}

# Main

checkVar
echo "All needed variables defined"

for ex in $examples
  do
  eval "ex_srv=\$$ex"
  echo "Generating the XML file for " $ex "..."
  putVar $ex
  for srv in $ex_srv
    do
    echo "                <SeD>" >> $ex.3
    echo "                    <config server=\"localHost\" remote_binary=\"$srv\"/>" >> $ex.3
    eval "param=\$${srv}_params"
    if [ x$param != x ]
	then
	echo $srv $param
	echo "                    <parameters string=\"$param\"/>" >> $ex.3
    fi
    echo "                </SeD>" >> $ex.3
  done
  echo "        </master_agent>" >> $ex.3
  echo "    </diet_hierarchy>" >> $ex.3
  echo "" >> $ex.3
  echo "</diet_configuration>" >> $ex.3
  
  mv $ex.3 $ex.xml
  rm -f $ex.3

echo " done"
done