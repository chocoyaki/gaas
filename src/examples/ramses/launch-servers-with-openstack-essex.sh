#!/bin/sh

if [ $# -lt 1 ]
then
	echo "please specify the openstack service end point url"
	exit 1
fi

end_point=$1

deltacloudd -i openstack -r 0.0.0.0 -P $end_point &

sleep 2


. ~/ramses.src

cd /root/diet

dietAgent etc/MA1.cfg &

sleep 5

maDagAgent etc/MA_DAG.cfg &

sleep 5

test-sed-cloud-with-vm-destructor-service etc/SeDconnectToMa.cfg http://localhost:3001/api admin+openstack keystone_admin &
test-ramses-sed-cloud-with-vm-init-service etc/SeDconnectToMa.cfg &
