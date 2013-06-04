#! /bin/bash

function printUsage {
  echo "Usage: $BASH_SOURCE -a AGENTNAME [OPTION...]"
  echo
  echo "  -a, --agentname AGENTNAME set the name of the agent"
  echo "  -n, --hostname HOSTNAME   set the hostname in the certificate"
  echo "  -w, --working-dir DIRECTORY the working directory (./ by default)"
  echo "  -h, --help                displays this help"  
  echo
  exit 0
  }

function error {
  echo "ERROR in $BASH_SOURCE :" $1
  exit 1
  }

set -e # Stop on first error
set -u # Stop if undefined variable

# Default values
scriptDir=$(cd `dirname $0` && pwd)
hostname=`hostname -I | cut -d" " -f1`

# Parsing arguments

i=0
argv=()
for arg in "$@"; do
    argv[$i]="$arg"
    i=$((i + 1))
done

i=0
while test $i -lt $# ; do
    arg="${argv[$i]}"
    case "$arg" in
        -h|--help) printUsage;;
        -n|--hostname) i=$((i + 1)); hostname="${argv[$i]}";;
        -a|--agentname) i=$((i + 1)); agentname="${argv[$i]}";;
        -w|--working-dir) i=$((i + 1)); workingDir="${argv[$i]}";;
        *) error "Unknown argument '$arg'";;
    esac
    i=$((i + 1))
done

dir="$workingDir/certificates/${agentname}"

if [ -e "${dir}" ]; then
  error "Folder '$dir' already exists: agent ${agentname} has got a certificate already"
fi

echo "Executing commands:"
echo "-------------------"
(
  PS4="> "
  set -x
  
  # Preparing the folder
  mkdir -p "$dir"
  cp "$scriptDir/openssl_agent.cnf" "${dir}/openssl.cnf"
  cd "$dir"
  mkdir "private"

  # Creating the certificate request
  openssl req -new    \
    -keyout private/$agentname.key.pem -out $agentname.req.pem \
    -config ./openssl.cnf -nodes \
    -subj "/C=FR/O=The Agent Company/OU=Certificates Service/CN=$agentname" &>/dev/null
)
echo "-------------------" 
echo "Certificate request created in dir $dir"
