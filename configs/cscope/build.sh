#!/bin/sh
#

CURRDIR=`pwd`

if [ ! -e ~/.vim/plugin ]; then
	mkdir -p ~/.vim/plugin
fi

if [ ! -e ~/.vim/plugin/cscope_maps.vim ]; then
	cd ~/.vim/plugin
	ln -s $CURRDIR/cscope_maps.vim cscope_maps.vim
fi

exit 1
