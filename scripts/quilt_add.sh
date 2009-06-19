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


function add_file_to_quilt()
{
	file=$1;

	if [ -f ${file} ] ; then
		quilt add $file;
	fi
}

# add all files into *.patch include subdir
function add_all_files()
{
	d_or_f=`find $1`

	for file in $d_or_f ;
	do
		add_file_to_quilt $file;
	done
}

# add files into *.patch only include current dir
function add_files()
{
	d_or_f=`find $1 -maxdepth 1`

	for file in $d_or_f ;
	do
		add_file_to_quilt $file;
	done
}

# $1 is files' dir which you want to add to quilt.
add_all_files $1;
