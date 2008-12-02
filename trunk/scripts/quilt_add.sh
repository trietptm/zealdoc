#!/bin/sh
#
# quilt cmd:
# 	import
# 	push -a
# 	add 
# 	diff -z
#	files -val
#	refress [patch.name]
#

# $1 is files dir which you want to add to quilt.
if [ $1x = x ]; then
	echo "No dir"
	exit
fi
files=`ls $1`

for file in $files ; do
	quilt add $file
done
