#!/bin/sh

#
# fxg - find, xargs and grep
# find at current directory
#
function fxg()
{
	if [ $# -ne 1 ]; then
		echo 'Usage: fxg $word_you_find'
		exit 1
	fi
	find . -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
	return 0
}

###
###
function do_install()
{
	local cmd=$1

	cd $SYS_PATH;
	set -e
	sudo ln -s $BOXDIR/zealbox.sh $1
}

cmd=`basename $0`
SYS_PATH=/usr/bin
BOXDIR=`pwd`

if [ $cmd = "zealbox.sh" ]; then
	if [ $# -lt 1 ]; then
		echo "which command(s) you want to install?"
		exit 1
	fi
	echo "Install $1 now."
	# TODO: detect if cmd is valid and handle cmd_list
	do_install $1;
	exit 0;
else
	$cmd $* 2>/dev/null
fi

# command return 0 when finished.
if [ $? -ne 0 ]; then
	echo "command not found"
	exit
fi
