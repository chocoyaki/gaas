#!/bin/sh

if [ $# -lt 3 ]
then
	echo "usage $0 vms_count level output"
	exit
fi

vms_count=$1
level=$2
output=$3


xsltproc --param nb-vms $vms_count --param level $level set-dag-params.xslt dag-expe-base.xml > $output

