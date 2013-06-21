#!/bin/sh

if [ $# -lt 3 ]
then
	echo "usage $0 vms_count level output [nfs_vm_id ramses_vm_id (for opennebula)]"
	exit
fi

vms_count=$1
level=$2
output=$3

if [ $# -gt 3 ]
then
	nfs_vm_id=$4
	ramses_vm_id=$5
	xsltproc --param nb-vms $vms_count --param level $level --param nfs_vm_image_id $nfs_vm_id  --param ramses_vm_image_id $ramses_vm_id set-dag-params.xslt dag-expe-base-opennebula.xml > $output
else
	xsltproc --param nb-vms $vms_count --param level $level set-dag-params.xslt dag-expe-base.xml > $output

fi
