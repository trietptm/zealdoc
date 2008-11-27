#!/bin/sh
#
# cscope for source code reading.
#

CURRDIR=`pwd`
TOOLNAME=`basename $CURRDIR`

if [ `which $TOOLNAME`x = ""x ]; then
	echo "Please install <$TOOLNAME> first."
	exit 1
fi

if [ ! -e ~/.vim/plugin ]; then
	mkdir -p ~/.vim/plugin
fi

# link to current user home directory
if [ -e ~/.vim/plugin/cscope_maps.vim ]; then
	echo "###"
	echo "#	Have a older version in your system. We don't override it."
	echo "#	If your are sure what you are doing, please remove the older one first."
	echo "#	Then excute this script again."
	echo "###"
	exit 1
else
	# TODO: if the file has a bad symbol, also will get here.
	cd ~/.vim/plugin
	# do not copy it, I want to use the same one everywhere.
	ln -s $CURRDIR/cscope_maps.vim cscope_maps.vim
	exit 1
fi
