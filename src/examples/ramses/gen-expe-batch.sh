#!/bin/bash

max_vms=$1
level=$2
xml_folder=$3
result_out_folder=$4

#we are in the root of the project DIETCloud

for ((i=1; i<=$max_vms; i++))
do
	echo "test-ramses-client-wf-expe etc/client_wf.cfg $xml_folder/$i.xml $result_out_folder/$i.txt" > $xml_folder/$i.sh
	chmod a+x $xml_folder/$i.sh
done
