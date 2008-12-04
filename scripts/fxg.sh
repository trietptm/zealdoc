#!/bin/sh

#
# fxg - find, xargs and grep
# find at current directory
#
if [ "$1"x = ""x ]; then
	echo "no grep param"
	exit 1;
fi
echo $1

find . -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
