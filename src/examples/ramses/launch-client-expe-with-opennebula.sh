#!/bin/bash

if [ $# -lt 4 ]
then
	echo -e "usage :\n\t $0 shared_out_folder_name[!_iX] nb_vms dag_folder_name j"
	exit 1
fi

shared_out_folder_name=$1
nb_vms=$2
dag_folder_name=$3
j=$4

mount -t nfs nfs:/home/latoch /mnt

. ~/ramses.src

cd /root/diet

for((i=1;i <= $nb_vms; i++))
do
	test-ramses-client-wf-expe-opennebula etc/client_wf.cfg src/examples/ramses/expe/opennebula/$dag_folder_name/b_$i.xml src/examples/ramses/expe/opennebula/$dag_folder_name/$i.xml /mnt/${shared_out_folder_name}_i$j/${i}_i$j.txt
	echo "sleep 100s"
	sleep 100
	echo "killall residual seds"
	killall test-ramses-sed-cloud-launched-after-vm-instanciation
done
