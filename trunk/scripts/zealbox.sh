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

function check_argc()
{
	if [ "$1"x = ""x ]; then
		echo "no grep param"
		exit 1;
	fi
}

cmd=`basename $0`
$cmd $* 2>/dev/null

# command return 0 when finished.
if [ $? -ne 0 ]; then
	echo "command not found"
	exit
fi
