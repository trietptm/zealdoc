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
	echo "Usage: ***Need 2 or more parameters***"
	echo '$1 (M) - kernel source direcotry(need absolute path).'
	echo '$2 (M) - kernel arch which you want to search.'
	echo '$3 (O) - directory you want to ignore. Now only support one dir.'
	echo "Example: ./cs_find.sh /kernel/source/dir/ x86 drivers"
}

# we need know source and ARCH(dir name in srctree/arch/)
if [ $# -lt 2 ]; then
	usage
	exit 1;
fi

rm -rf $LNX/cscope 

DIR=`dirname $1`
BASE=`basename $1`

LNX=$DIR/$BASE
ARCH=$2
# now only support one dir.
if [ x$3 != x'' ]; then
	IGN="-path \"$LNX/$3\" -prune -o"
else
	IGN=""
fi

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
	-path "$LNX/$3" -prune -o                                             \
	\( -name .svn -o -name .pc -o -name CVS -o -name .git \) -prune -o    \
	\( -name .cmd.c \) -prune -o    \
        -name "*.[chxsS]" -print > $LNX/cscope/cscope.files

cd $LNX/cscope 
echo "Finding files now, just wait & be patient"
cscope -b -k -q
echo "Cscope Find Finish..."
exit

# Add some find command for finding out what you want
#
# 1. find . \( -name *.[sch] -o -name *.asm \) > cscope/cscope.files 
# 2. find | egrep '\.(c|h|asm)$'
