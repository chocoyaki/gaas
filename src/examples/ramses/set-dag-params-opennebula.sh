#!/bin/sh

if [ $# -lt 6 ]
then
	echo "usage $0 vms_count level output nfs_vm_id ramses_vm_id deltacloud_hostname(port 3001)"
	exit
fi

vms_count=$1
level=$2
output=$3
nfs_vm_id=$4
ramses_vm_id=$5
deltacloud_hostname=$6

output_directory=$(dirname $output)
output_file=$(basename $output)

xsltproc --param nb-vms $vms_count --param level $level --param nfs_vm_image_id $nfs_vm_id  --param ramses_vm_image_id $ramses_vm_id set-dag-params-for-opennebula.xslt dag-expe-base-bootstrap-opennebula.xml > $output_directory/b_$output_file
	
xsltproc --param level $level --param nb-vms $vms_count set-dag-params.xslt dag-expe-base.xml > $output
sed -i "s/localhost:3001\/api/$deltacloud_hostname:3001\/api/g" $output

