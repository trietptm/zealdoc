#!/bin/sh
#
# library for shell scripts
#

# return 1 means exist
cmd_isexist()
{
	local cmd=$1

	# only find command in system path
	if [ `which $cmd`x = ""x ]; then
		return 0
	else
		return 1
	fi
}

cmd_exist_exit()
{
	local cmd=$1
	cmd_isexist $cmd; ret=$?
	if [ $ret = 1 ]; then
		echo "cmd_exist_exit <$cmd>"
		exit 1
	fi
}

cmd_not_exist_exit()
{
	local cmd=$1
	cmd_isexist $cmd; ret=$?
	if [ $ret = 0 ]; then
		echo "cmd_not_exist_exit <$cmd>"
		exit 1
	fi
}

# file and directory
# return 1 means exist
fd_isexist()
{
	local file=$1

	if [ -e $file ]; then
		return 1
	else
		return 0
	fi
}

sym_isexist()
{
	local file=$1

	# -h file
	# True if file exist and is a symbolic link.
	if [ -h $file ]; then
		return 1
	else
		return 0
	fi

}

sym_file_exist_exit()
{
	sym_isexist $1; ret=$?
	if [ $ret = 1 ]; then
		echo "sym_file_exist_exit <$1>"
		exit 1
	fi
}

file_exist_exit()
{
	fd_isexist $1; ret=$?
	if [ $ret = 1 ]; then
		echo "file_exist_exit <$1>"
		exit 1
	fi
}

file_not_exist_create()
{
	fd_isexist $1; ret=$?
	if [ $ret = 0 ]; then
		touch $1;
		return 0;
	fi
}

dir_not_exist_create()
{
	fd_isexist $1; ret=$?
	if [ $ret = 0 ]; then
		mkdir -p $1;
		return 0;
	fi
}

file_not_exist_exit()
{
	fd_isexist $1; ret=$?
	if [ $ret = 0 ]; then
		echo "file_exist_exit <$1>"
		exit 1
	fi
}

is_root()
{
	if [ `whoami`x = "root"x ]; then
		return 1
	else
		return 0
	fi
}

need_root()
{
	is_root; ret=$?
	if [ $ret = 0 ]; then
		echo "Need privilege, try with <root>"
		exit 1
	fi
}

log_fail()
{
	local var=$1
	local name=$2

	case $var in 
	FILE_EXIST)
		echo "file_exist_error"
		;;
	CMD_NOT_EXIST)
		echo "command <$2> is not exist"
		;;
	esac
}
