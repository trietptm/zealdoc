#!/bin/sh
#
# cscope for source code reading.
#

if [ `which cscope` == "" ] ; then
	echo "Please install cscope first."
	exit 1
fi

if [ ! -e ~/.vim/plugin ]; then
	mkdir -p ~/.vim/plugin
fi

if [ -e ~/.vim/plugin/cscope_maps.vim ]; then
	echo "###"
	echo "#	Have a older version in your system. We don't override it."
	echo "#	If your are sure what you are doing, please remove the older one first."
	echo "#	Then excute this script again."
	echo "###"
	exit 1
else
	# TODO: if the file has a bad symbol, also will get here.
	DIR=`pwd`
	cd ~/.vim/plugin
	ln -s $DIR/cscope_maps.vim cscope_maps.vim
	exit 1
fi
