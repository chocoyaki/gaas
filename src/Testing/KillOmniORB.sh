#!/bin/sh

if [ $# -ne 1 ]; then
    echo "$0 <omniNames>"
    exit 1
fi

OMNINAMESCMD=$1

USER=`logname`

if [ "$USER" == "" ]; then
    PID=`ps x | grep "$OMNINAMESCMD" | grep -v "grep" | grep -v "$0" | awk '{print \$1}'`
else
    PID=`ps x -U $USER | grep "$OMNINAMESCMD" | grep -v "grep" | grep -v "$0" | awk '{print \$1}'`
fi 

if [ $? -ne 0 ]; then
    exit 1
fi

echo "Pid: $PID"

if [ "$PID" == "" ];then
    exit 1
fi


kill $PID
if [ $? -ne 0 ]; then
    exit 1
fi

exit 0
