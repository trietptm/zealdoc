#!/bin/sh
# This script do the find function for 
# cscope.files
# As a input parameter is source directory.
# 
# only kernel need it
# other small project just use: cscope -R

usage()
{
	echo "We need 2 parameter:"
	echo '$1 - kernel source direcotry.'
	echo '$2 - kernel arch which you want to search.'
	echo "Example: ./cs_find.sh /kernel/source/dir/ x86"
}

# we need know source and ARCH(dir name in srctree/arch/)
if [ $# -ne 2 ]; then
	usage
	exit 1;
fi

LNX=$1
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
        -name "*.[chxsS]" -print > $LNX/cscope/cscope.files

cd $LNX/cscope 
echo "Finding files now, just wait & be patient"
cscope -b -k -q
echo "Cscope Find Finish..."
exit
