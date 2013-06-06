#!/bin/sh

vms_count=$1
level=$2
output=$3

xsltproc --param nb-vms $vms_count --param level $level set-dag-params.xslt dag-expe-base.xml > $output
