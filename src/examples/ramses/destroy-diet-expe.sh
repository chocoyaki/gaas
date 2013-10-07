#!/bin/sh


#suppression de deltacloud
deltacloudd_pid=$(ps aux | grep deltacloudd | grep -v grep | awk '{print $2}')
kill -9 $deltacloudd_pid


#suppression sed-cloud (+ sed-appli eventuels)
sedcloud_pid=$(ps aux | grep sed-cloud | grep -v grep | awk '{print $2}')
kill -9 $sedcloud_pid

#suppresion madag
madag_pid=$(ps aux | grep maDagAgent | grep -v grep | awk '{print $2}')
kill -9 $madag_pid

#suppresion ma
agent_pid=$(ps aux | grep dietAgent | grep -v grep | awk '{print $2}')
kill -9 $agent_pid
