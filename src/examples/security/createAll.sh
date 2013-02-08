#! /bin/bash

set -e
set -u

scriptDir=$(cd `dirname $0` && pwd)
dir="$PWD/Auth"
hostname=`hostname -I | cut -d" " -f1`

mkdir -p "$PWD/configs"

# Creating omniorn config file
cfg="configs/omniORB4.cfg"
> "$cfg"
echo "InitRef = NameService=corbaname::$hostname:2809" >> "$cfg"
echo "supportBootstrapAgent = 1" >> "$cfg"
echo "traceLevel = 5" >> "$cfg"
echo "maxGIOPConnectionPerServer = 1000" >> "$cfg"

# Creating Authority certificates
echo "Creating authority"
"$scriptDir/createAuth.sh" --hostname "$hostname" --directory "$dir"

# Creating MA
echo "Creating MA"
name="MA_0"
maname="$name"
cfg="configs/$name.cfg"
> "$cfg"
echo "agentType = DIET_MASTER_AGENT" >> "$cfg"
echo "name = $name" >> "$cfg"
"$scriptDir/createCert.sh" --agentname "$name"
"$scriptDir/validateCert.sh" --agentname "$name" --authdir "$dir" --config-file "$cfg"

# Creating LA
echo "Creating LA"
parent="$name"
name="LA_0"
cfg="configs/$name.cfg"
> "$cfg"
echo "agentType = DIET_LOCAL_AGENT" >> "$cfg"
echo "name = $name" >> "$cfg"
echo "parentName = $parent" >> "$cfg"
"$scriptDir/createCert.sh" --agentname "$name"
"$scriptDir/validateCert.sh" --agentname "$name" --authdir "$dir" --config-file "$cfg"

# Creating SeD
echo "Creating SeD"
parent="$name"
name="SeD_0"
cfg="configs/$name.cfg"
> "$cfg"
echo "name = $name" >> "$cfg"
echo "parentName = $parent" >> "$cfg"
"$scriptDir/createCert.sh" --agentname "$name"
"$scriptDir/validateCert.sh" --agentname "$name" --authdir "$dir" --config-file "$cfg"

# Creating client
echo "Creating client"
parent="$name"
name="client"
cfg="configs/$name.cfg"
> "$cfg"
echo "MAName = $parent" >> "$cfg"
"$scriptDir/createCert.sh" --agentname "$name"
"$scriptDir/validateCert.sh" --agentname "$name" --authdir "$dir" --config-file "$cfg"

