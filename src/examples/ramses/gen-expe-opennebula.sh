#!/bin/bash

if [ $# -lt 5 ]
then
	echo "usage $0 max_vms level folder nfs_vm_id ramses_vm_id"
	exit
fi

max_vms=$1
level=$2
folder=$3
nfs_vm_id=$4
ramses_vm_id=$5

#echo $folder

for ((i=1; i <= $max_vms; i++))
do
	#echo $i
	output=$i.xml
	./set-dag-params-opennebula.sh $i $level $folder/$output $nfs_vm_id $ramses_vm_id
done
