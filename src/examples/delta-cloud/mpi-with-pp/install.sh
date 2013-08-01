puppet apply --modulepath ./puppet/modules ./puppet/manifests/build.pp

cd src
make
