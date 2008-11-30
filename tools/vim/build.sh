#!/bin/sh
#

CURRDIR=`pwd`
TOOLNAME=`basename $CURRDIR`

source ../../lib/lib.sh

cmd_not_exist_exit $TOOLNAME;

file_exist_exit ~/.vimrc
sym_file_exist_exit ~/.vimrc

cd ~
# do not copy it, I want to use the same one everywhere.
ln -s $CURRDIR/vimrc .vimrc
exit 1
