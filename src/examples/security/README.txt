In this folder, you can use scripts to generate certificates and keys
to test the security extension of DIET. They generate files for deploying a
local DIET hierarchy with a Master Agent, a Local Agent and a Server Daemon.

To create all files, run:

    $ mkdir test
    $ cd test
    $ ../createAll.sh -c ../simpleconfig.txt

In test/CA, you will find the authority certificate, and the database of all
signed certificates
In test/certificates, you will find one folder per entity (i.e. agent or client)
containing several files. You can find cacert.pem, a copy of the authority
certificate, private/<agentname>.pem which is a concatenation of the private key
and the certificate of the entity
In test/configs, you will find the cfg files for every agent, and for a client,
and the cfg file for omniorb.

You can now test the configuration:

    Don't forget to set your PATH variable properly. In the simpleconfig.txt file
    there are only binary names. Either you expand them with the path, or you set
    your PATH variable. For instance:
    $ export PATH=~/git/diet/build/bin:$PATH

    If you ran it before, remove the logs of the Naming Service
    $ rm -f omninames.log omninames.bak

    Start the Naming service:
    $ OMNINAMES_LOGDIR=. omniNames -start -ignoreport -ORBendPoint giop:tcp::2809 -nohostname &

    Start the agents:
    $ ./MA1.launch
    $ ./LA1.launch

    Start the Server Daemon
    $ ./sed_dmat.launch

The agents outputs are in the files .out and .err. Check them in case it doesn't work.

There is no script for the client, but the cfg file is provided.
You need to run by its command and specify the omniorb config file.
    $ OMNIORB_CONFIG=configs/omniORB4.cfg dmat_manips_client configs/client.cfg T

