#!/bin/sh
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
	cp cscope_maps.vim ~/.vim/plugin
fi
