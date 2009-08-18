#!/bin/sh
# This script do the find function for 
# cscope.files
# As a input parameter is source directory.
# 
# only kernel need it
# other small project just use: cscope -R
#
# cscope -Rbkq
# -R recursion all (sub)directorys
# -b do not run interface
# -k do not search system path(e.g. /usr/include)
# -q generate cscope.in.out cscope.po.out for searching speed up

usage()
{
	echo "usage:"
	echo "2 parameters needed:"
	echo '$1 - kernel source direcotry(need absolute path).'
	echo '$2 - kernel arch which you want to search.'
	echo "Example: ./cs_find.sh /kernel/source/dir/ x86"
}

# we need know source and ARCH(dir name in srctree/arch/)
if [ $# -ne 2 ]; then
	usage
	exit 1;
fi

DIR=`dirname $1`
BASE=`basename $1`

LNX=$DIR/$BASE
ARCH=$2

mkdir -p $LNX/cscope

cd / 	
    find  $LNX                                                                \
	-path "$LNX/arch/*" ! -path "$LNX/arch/$ARCH*" -prune -o              \
	-path "$LNX/include/asm-*" \
		! \( -path "$LNX/include/asm-$ARCH*" -o -path "$LNX/include/asm-generic*" \) \
		-prune -o  	      					      \
	-path "$LNX/tmp*" -prune -o                                           \
	-path "$LNX/Documentation*" -prune -o                                 \
	-path "$LNX/scripts*" -prune -o                                       \
	-path "$LNX/sound*" -prune -o                                         \
	-path "$LNX/patches*" -prune -o                                       \
	\( -name .svn -o -name .pc -o -name CVS -o -name .git \) -prune -o    \
	\( -name .cmd.c \) -prune -o    \
        -name "*.[chxsS]" -print > $LNX/cscope/cscope.files

cd $LNX/cscope 
echo "Finding files now, just wait & be patient"
cscope -b -k -q
echo "Cscope Find Finish..."
exit
