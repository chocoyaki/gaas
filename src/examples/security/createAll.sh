#! /bin/bash

function printUsage {
  echo "Usage: $BASH_SOURCE [OPTION...]"
  echo
  echo "  -l, --la NUMBER set the number of LAs (default = 0)"
  echo "  -c, --configfile"
  echo "  -h, --help                displays this help"  
  echo
  exit 0
}

function getType {
  type=$1
  case "$type" in
    MA) echo DIET_MASTER_AGENT;;
    LA) echo DIET_LOCAL_AGENT;;
    SeD) echo "";;
  esac
  }

function create() {
  num=$1
  
  # Create config and certificates
  echo "Creating $num : ${agentname[$num]}"
  parent="${parent[$num]}"
  name="${agentname[$num]}"
  cfg="configs/$name.cfg"
  > "$cfg"
  if [ -n "${agenttype[$num]}" ]; then
    echo "agentType = ${agenttype[$num]}" >> "$cfg"
  fi
  echo "name = $name" >> "$cfg"
  if [ -n "${parent}" ]; then
    echo "parentName = $parent" >> "$cfg"
  fi
  "$scriptDir/createCert.sh" --agentname "$name" -n "${hostname[$num]}"
  "$scriptDir/validateCert.sh" --agentname "$name" --authdir "$dir" --config-file "$cfg"

  set -x
  # Create launch script
  if [ -n "${binary[$num]}" ];then
    binary=`echo "${binary[$num]}" | sed 's:\/:\\\/:g'`
    args="${binargs[$num]}"
  else
    binary="dietAgent"
    args=""
  fi

  cat "$scriptDir/launch-scripts/Agent.launch" | sed "s/\#NAME\#/$name/g" \
    | sed "s/\#BINARY\#/$binary/g" \
    > "$name.launch"
  chmod +x "$name.launch"
 
  set +x
  
  }

function readFile {
  file=$1
  nb=0
  while IFS=";" read parentname agenttype agentname hostname bin_name bin_args
  do
    nb=$(($nb+1))
    parent[$nb]=$parentname
    agenttype[$nb]=$( getType "$agenttype" )
    agentname[$nb]=$agentname
    hostname[$nb]=$hostname
    binary[$nb]=$bin_name
    binargs[$nb]=$bin_args
    echo "${parent[$nb]} -> ${agentname[$nb]} ( ${agenttype[$nb]} ) : ${hostname[$nb]}"
  done < $file
  nbelements=$nb
 echo "No implemented yet"
}

set -e
set -u

nbLAs=0

i=0
argv=()
for arg in "$@"; do
    argv[$i]="$arg"
    i=$((i + 1))
done

agentname=()

i=0
while test $i -lt $# ; do
    arg="${argv[$i]}"
    case "$arg" in
        -h|--help) printUsage;;
        -l|--la) i=$((i + 1)); nbLAs=${argv[$i]};;
        -c|--configfile) i=$((i + 1)); readFile "${argv[$i]}" ;;
        *) error "Unknown argument '$arg'";;
    esac
    i=$((i + 1))
done

scriptDir=$(cd `dirname $0` && pwd)
dir="$PWD/Auth"
mkdir -p "$PWD/configs"
hostname=`hostname -I | cut -d" " -f1`

# Creating omniorb config file
cfg="configs/omniORB4.cfg"
> "$cfg"
echo "InitRef = NameService=corbaname::$hostname:2809" >> "$cfg"
echo "supportBootstrapAgent = 1" >> "$cfg"
echo "traceLevel = 5" >> "$cfg"
echo "maxGIOPConnectionPerServer = 1000" >> "$cfg"

# Creating Authority certificates
echo "Creating authority"
"$scriptDir/createAuth.sh" --hostname "$hostname" --directory "$dir"

echo $nbelements
nb=1
while test $nb -le $nbelements
do
  create $nb  
  if [ "${agenttype[$nb]}" == "DIET_MASTER_AGENT" ]; then
    maname="${agentname[$nb]}"
  fi
  nb=$(($nb +1))
done


# Creating client
echo "Creating client"
name="client"
cfg="configs/$name.cfg"
> "$cfg"
echo "MAName = $maname" >> "$cfg"
"$scriptDir/createCert.sh" --agentname "$name"
"$scriptDir/validateCert.sh" --agentname "$name" --authdir "$dir" --config-file "$cfg"

