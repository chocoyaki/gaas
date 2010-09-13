#!/bin/sh

if [ $# -ne 3 ]; then
    echo "$0 <omniNames> <hostname> <port>"
    exit 1
fi

OMNINAMESCMD=$1
HOSTNAME=$2
PORT=$3

OMNIORB_CONFIG=""

echo "nohup $OMNINAMESCMD -start $PORT -always -ignoreport -ORBendPoint giop:tcp:$HOSTNAME:$PORT < /dev/null > /dev/null 2>&1 &"
nohup $OMNINAMESCMD -start $PORT -always -ignoreport -ORBendPoint giop:tcp:$HOSTNAME:$PORT < /dev/null > /dev/null 2>&1 &

if [ $? -ne 0 ]; then
    exit 1
fi

exit 0
