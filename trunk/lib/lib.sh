#!/bin/sh
#
# library for shell scripts
#

cmd_isexist()
{
	local cmd=$1

	if [ `which $cmd`x = ""x ]; then
		return 0
	else
		return 1
	fi
}

file_isexist()
{
	local file=$1

	if [ -e $file ]; then
		return 1
	else
		return 0
	fi
}

log_fail()
{
	local var=$1

	case $var in 
	FILE_EXIST)
		echo "file_exist_error"
		;;
	CMD_NOT_EXIST)
		echo "cmd_not_exist_error"
		;;
	esac
}
