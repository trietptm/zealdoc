#!/bin/sh
# This script do the find function for 
# cscope.files
# As a input parameter is source directory.

if [ x"$1" = "x" ]; then
	echo "Please input your source directory"
	exit 1
fi

SRC_DIR=$1
mkdir -p $SRC_DIR/cscope
cd /
find $1 	\
	\( -name .pc -o -name Doc* -o -name scripts -o -name .svn \) -prune -o \
	! -path $1/arch/arm* -a -path $1/arch/ -prune -o \
	-name "*.[chxsS]"  -print > $SRC_DIR/cscope/cscope.files \

cd $SRC_DIR/cscope 
echo "Finding files now, just wait & be patient"
cscope -b -k -q
echo "Cscope Find Finish..."
exit
