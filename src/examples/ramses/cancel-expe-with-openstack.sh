#!/bin/sh


if [ $# -lt 3 ]
then
	echo -e "usage:\n\t $0 site controlerFQDN.]site.grid5000.fr[ dietNode.]site.grid5000.fr["
	exit 1
fi

site=$1
controler=$2
dietHost=$3


ssh root@$dietHost.$site$.g5k "/root/diet/src/examples/ramses/destroy-diet-expe.sh"
ssh root@$controler.$site.g5k "./destroy-all-vms.sh"

