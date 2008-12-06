#!/bin/sh

#
# fxg - find, xargs and grep
# find at current directory
#
function fxg()
{
	echo "in fxg $1"
	exit
	find . -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
}

if [ "$1"x = ""x ]; then
	echo "no grep param"
	exit 1;
fi
echo $0 $1
cmd=$0
$1 a
exit
