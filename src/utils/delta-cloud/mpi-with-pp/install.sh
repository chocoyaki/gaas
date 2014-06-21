apt-get update
apt-get install puppet

puppet apply --modulepath ./puppet/modules ./puppet/manifests/build.pp

cd src
make
