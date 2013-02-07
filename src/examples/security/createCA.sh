#! /bin/bash

function printUsage {
  echo "Usage: $BASH_SOURCE [OPTION...]"
  echo
  echo "  -d, --directory DIRECTORY set the output directory (default value is CA/)"
  echo "  -h, --help                displays this help"  
  echo
  exit 0
  }

function error {
  echo "ERROR in $BASH_SOURCE :" $1
  exit 1
  }

set -e # Stop on first error

# Default values

dir="CA"
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
        -d|--directory)   i=$((i + 1)); dir="${argv[$i]}";;
        -h|--help) printUsage;;
        -n|--hostname) i=$((i + 1)); hostname="${argv[$i]}";;
        *) error "Unknown argument '$arg'";;
    esac
    i=$((i + 1))
done


# Preparing the folder for storing certificates
if [ -e "${dir}" ]; then
  error "Folder '$dir' already exists"
fi

mkdir "$dir"
cp openssl_ca.cnf "${dir}/openssl.cnf"
cd "$dir"
mkdir newcerts private
echo '01' > serial
touch index.txt

# Creating the Authority certificate
openssl req -new -x509 -extensions v3_ca    \
  -keyout private/cakey.pem -out cacert.pem \
  -days 3650 -config ./openssl.cnf -nodes \
  -subj "/C=FR/O=DIET/OU=CA Management/CN=$hostname" 
 
echo "CA created in dir $dir"
