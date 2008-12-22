#!/bin/sh

SYS_PATH=/usr/bin
BOXDIR=`pwd`
BOXNAME="zealbox.sh"

function usage()
{
	echo "usage: ./$BOXNAME \$command"
	echo "\$command - which one you want to install"
}

#
# fxg - find, xargs and grep
# Usage:
# $1 - word you want to lookup
#
function fxg()
{
	if [ $# -ne 1 ]; then
		echo 'Usage:'
		echo 'fxg $word_you_lookup'
		exit 1
	fi
	find . -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
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
