#!/bin/bash


max_vms=$1
level=$2
folder=$3

echo $folder

for ((i=1; i <= $max_vms; i++))
do
	#echo $i
	output=$i.xml
	./set-dag-params.sh $i $level $folder/$output
done
