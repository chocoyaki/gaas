#!/bin/bash

if [ $# -lt 3 ]
then
	echo "usage $0 max_vms folder result_out_folder"
	exit
fi



max_vms=$1
xml_folder=$2
result_out_folder=$3

#we are in the root of the project DIETCloud

for ((i=1; i<=$max_vms; i++))
do
	echo "test-ramses-client-wf-expe-opennebula etc/client_wf.cfg $xml_folder/$i.xml $result_out_folder/$i.txt" > $xml_folder/$i.sh
	chmod a+x $xml_folder/$i.sh
done
