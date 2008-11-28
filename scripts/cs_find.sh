#!/bin/sh
# This script do the find function for 
# cscope.files
# As a input parameter is source directory.
# 
# only kernel need it
# other small project just use: cscope -R

if [ x"$1" = "x" ]; then
	echo "Please input your source directory"
	exit 1
fi

SRC_DIR=$1
mkdir -p $SRC_DIR/cscope
cd /
find $1 	\
	\( -name .svn -o -name .pc -o -name CVS -o -name .git \) -prune -o \
	-path "$SRC_DIR/arch/*" ! -path  "$SRC_DIR/arch/i386*" -prune -o	\
	-path "$SRC_DIR/include/asm-*" ! -path  "$SRC_DIR/include/asm-i386*" -prune -o \
	-path "$SRC_DIR/tmp*"  -prune -o \
	-path "$SRC_DIR/Document*"  -prune -o \
	-path "$SRC_DIR/script*"  -prune -o \
	-name "*.[chxsS]"  -print > $SRC_DIR/cscope/cscope.files \

cd $SRC_DIR/cscope 
echo "Finding files now, just wait & be patient"
cscope -b -k -q
echo "Cscope Find Finish..."
exit
