#!/bin/sh

PID=`ps -a | grep netsvc | awk {'print $1'}`
echo $PID
sudo kill -9 $PID
