In this folder, you can use scripts to generate certificates and keys
to test the security extension of DIET. They generate files for deploying a
local DIET hierarchy with a Master Agent, a Local Agent and a Server Daemon.

To create all files, run:

    mkdir test
    cd test
    ../createAll.sh

In test/Auth, you will find the authority certificate, and the database of all
signed certificates
In test/certificates, you will find one folder per entity (i.e. agent or client)
containing several files. You can find cacert.pem, a copy of the authority
certificate, private/<agentname>.pem which is the private key plus the certificate
of the authority
In test/configs, you will find the cfg files for every agent, and for a client,
and the cfg file for omniorb.
