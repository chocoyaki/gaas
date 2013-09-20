#!/bin/sh

if [ $# -lt 3 ]
then
	echo "usage $0 vms_count level output"
	exit
fi

vms_count=$1
level=$2
output=$3


output_directory=$(dirname $output)
output_file=$(basename $output)

xsltproc --param level $level --param nb-vms $vms_count set-dag-params.xslt dag-expe-base.xml > $output
xsltproc --param nb-vms $vms_count  set-dag-params.xslt dag-expe-base-bootstrap-essex.xml > $output_directory/b_$output_file
