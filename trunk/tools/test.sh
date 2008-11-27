#!/bin/sh

source ../lib/lib.sh

# $? - status 
cmd_isexist abd; ret=$?
if [ $ret = 0 ]; then
	echo "not exist cmd"
fi

file_isexist abd; ret=$?
if [ $ret = 0 ]; then
	echo "exist file"
	log_fail FILE_EXIST
	exit 1
fi
