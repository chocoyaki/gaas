#! /bin/bash

function printUsage {
  echo "Usage: $BASH_SOURCE -a AGENTNAME [OPTION...]"
  echo
  echo "  -a, --agentname AGENTNAME   set the name of the agent"
  echo "  -c, --config-file FILENAME  set the cfg file to add ssl parameters"
  echo "  -d, --authdir DIRECTORY     the directory in which the authority has its certificates (CA by default)"
  echo "  -w, --working-dir DIRECTORY the working directory (./ by default)"
  echo "  -h, --help                  displays this help"  
  echo
  exit 0
  }

function error {
  set +x
  echo "ERROR in $BASH_SOURCE :" $1
  exit 1
  }

set -e # Stop on first error
set -u # Stop if undefined variable

# Default values
scriptDir=$(cd `dirname $0` && pwd)
workingDir="$PWD"
cfg=""
debug=0

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
        -c|--config-file) i=$((i + 1)); cfg="${argv[$i]}";;
        -d|--authdir) i=$((i + 1)); authdir="${argv[$i]}";;
        -a|--agentname) i=$((i + 1)); agentname="${argv[$i]}";;
        -v|--verbose) debug=1;;
        -w|--working-dir) i=$((i + 1)); workingDir="${argv[$i]}";;
        *) error "Unknown argument '$arg'";;
    esac
    i=$((i + 1))
done

authdir="$workingDir/CA"

agentdir="$workingDir/certificates/${agentname}"
if [[ ! "$authdir" == "/"* ]]; then
  authdir="$PWD/$authdir"
fi
if [[ ! "$agentdir" == "/"* ]]; then
  agentdir="$PWD/$agentdir"
fi
if [ -e "$cfg" ]; then
  if [[ ! "$cfg" == "/"* ]]; then
    cfg="$workingDir/$cfg"
  fi
fi
if [ ! -e "${agentdir}" ]; then
  error "Folder '$agentdir' does not exist: wrong agent name"
fi

echo "Executing commands:"
echo "-------------------"
(
  PS4="> "

  if [ $debug == 1 ]; then
        set -x
  fi

  # Sending CA certificate
   cp "$authdir/cacert.pem" "$agentdir"

  # Sending cert request
  mkdir -p "$authdir/tmp"
  cp "$agentdir/$agentname.req.pem" "$authdir/tmp"
  
  # Authority
  cd "$authdir"
  certfile="$authdir/tmp/$agentname.cert.pem"
  # Automatically accept certificate : echo -e "y\ny"
  echo -e "y\ny" | openssl ca -out "$certfile" -config openssl.cnf -infiles "$authdir/tmp/$agentname.req.pem"
  

  cp "$certfile" "$agentdir"
  cat "$agentdir/private/$agentname.key.pem" "$agentdir/$agentname.cert.pem" > "$agentdir/private/$agentname.pem"
  
  if [ -n "$cfg" ]; then
    if [ -f "$cfg" ]; then
      echo "sslRootCertificate = $agentdir/cacert.pem" >> "$cfg"
      echo "sslPrivateKey = $agentdir/private/$agentname.pem" >> "$cfg"
    else
      error "File '$cfg' does not exist !"
    fi
  else
    echo "You can now use $agentdir/cacert.pem as a root certificate"
    echo "and $agentdir/private/$agentname.pem as a private key"
    echo "in the configuration of $agentname"
  fi
  
  
)
echo "-------------------" 
echo "Certificate validated for agent $agentdir"
