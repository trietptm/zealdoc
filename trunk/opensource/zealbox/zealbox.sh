#!/bin/sh

SYS_PATH=/usr/bin
BOXDIR=`pwd`
# DO NOT CHANGE IT.
BOXNAME="zealbox.sh"
#
# If you want to add a new command, do as following: $xxx
#

#
# $1: add your command name here.
#
CMD_LIST="fxg, k9"

#
# $2: add command's usage/description here. 
#
FXG_HELP='fxg $1- find ./ | xargs grep $1'
K9_HELP='k9 $task - kill pid'

#
# $3: add it into list. 
#
CMD_HELPS=`printf "\t$FXG_HELP\n\t$K9_HELP\n\t"`

#
# $4: Right here you will only need one function more which name is your
# command.
#

function usage()
{
	echo ""
	echo "Usage: ./$BOXNAME \$command"
	echo ""
	echo "Currently supported commands are {$CMD_LIST}"
	echo "$CMD_HELPS"
}

#
# kill task quickly
# kill -9 PID
#
function k9()
{
	local task=$1

	if [ $# -eq 0 ]; then
		echo "ERROR: no task to kill."
		exit 1;
	fi
	PID=`ps -a | grep $task | awk {'print $1'}`
	echo "closing $task($PID), say 88 "
	sudo kill -9 $PID
}

#
# fxg - find, xargs and grep
# Usage:
# $1 - word you want to lookup
#
function fxg()
{
	if [ $# -gt 2 ]; then
		echo 'Usage: fxg $word_you_lookup'
		exit 1
	fi

#	if [ $? -eq 2 ]; then
#		if [ $2 -eq "d" ]; then
#			echo "dd"
#		fi
#	fi
	find . -name cscope -prune -o -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
	return 0
}

function fxg1()
{
	if [ $# -gt 2 ]; then
		echo 'Usage: fxg $word_you_lookup'
		exit 1
	fi

#	if [ $? -eq 2 ]; then
#		if [ $2 -eq "d" ]; then
#			echo "dd"
#		fi
#	fi
	find . -maxdepth 1 -name cscope -prune -o -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
	return 0
}

function fxg2()
{
	if [ $# -gt 2 ]; then
		echo 'Usage: fxg $word_you_lookup'
		exit 1
	fi

#	if [ $? -eq 2 ]; then
#		if [ $2 -eq "d" ]; then
#			echo "dd"
#		fi
#	fi
	find . -maxdepth 2 -name cscope -prune -o -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
	return 0
}


#####################
function do_install()
{
	local cmd=$1

	cd $SYS_PATH;
	sudo rm -f $1
	sudo ln -s $BOXDIR/$BOXNAME $1
}

cmd=`basename $0`
if [ $cmd = $BOXNAME ]; then
	if [ $# -lt 1 ]; then
		usage
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

#
#
#find -name '*.java' -type f -exec cat {} ; | wc -l

# find . -name '*.h' -type f | xargs cat | wc -l

# LOC: code of line (.c && .h)
# find . -type f -iname *.c -o -iname *.h -exec cat {} \; | wc -l
#
