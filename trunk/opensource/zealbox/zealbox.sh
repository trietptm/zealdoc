#!/bin/sh

SYS_PATH=/usr/bin
BOXDIR=`pwd`
BOXNAME="zealbox.sh"

CMD_LIST="fxg k9 csrb"

FXG_HELP='fxg $1- find ./ | xargs grep $1'
K9_HELP='k9 $task - kill pid'
CSRB_HELP='cscope -Rb and mkdir cscope directory'
CMD_HELPS="$FXG_HELP $K9_HELP $CSRB_HELP"

function usage()
{
	echo "usage: ./$BOXNAME command"
	echo "install command that supported"
	echo "supported command is {$CMD_LIST} now"
#	echo "$CMD_HELPS"
	echo "$FXG_HELP"
	echo "$K9_HELP"
	echo "$CSRB_HELP"
}

function cmds_usage()
{
	echo ":/"	
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
	if [ $# -ne 1 ]; then
		echo 'Usage: fxg $word_you_lookup'
		exit 1
	fi
	find . -name cscope -prune -o -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
	return 0
}

# cscope -Rb
function csrb()
{
	echo "cscoping..."
	cscope -Rb
	mkdir -p cscope
	mv cscope.out cscope/
	echo "finish"
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
