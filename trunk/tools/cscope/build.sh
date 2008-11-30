#!/bin/sh
#

CURRDIR=`pwd`
TOOLNAME=`basename $CURRDIR`

source ../../lib/lib.sh

cmd_not_exist_exit $TOOLNAME;

dir_not_exist_create ~/.vim/plugin;

file_exist_exit ~/.vim/plugin/cscope_maps.vim;
sym_file_exist_exit ~/.vim/plugin/cscope_maps.vim;

cd ~/.vim/plugin
# do not copy it, I want to use the same one everywhere.
ln -s $CURRDIR/cscope_maps.vim cscope_maps.vim
exit 1
