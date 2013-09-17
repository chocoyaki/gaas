#!/bin/sh

if [ $# -lt 1 ]
then
	echo "please specify the openstack service end point url"
	exit 1
fi

end_point=$1

OCCI_URL=$1 deltacloudd -i opennebula -r 0.0.0.0 &

sleep 2


. ~/ramses.src

cd /root/diet

dietAgent etc/MA1.cfg &

sleep 5

maDagAgent etc/MA_DAG.cfg &

sleep 5

test-sed-cloud-with-vm-destructor-service etc/SeDconnectToMa.cfg http://localhost:3001/api oneadmin passoneadmin &
test-ramses-sed-cloud-with-vm-init-service etc/SeDconnectToMa.cfg &
