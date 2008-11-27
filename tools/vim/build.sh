#!/bin/sh
#

CURRDIR=`pwd`
TOOLNAME=`basename $CURRDIR`

source ../../lib/lib.sh

cmd_not_exist_exit $TOOLNAME;

file_exist_exit ~/.vimrc

# TODO: if the file has a bad symbol, also will get here.
cd ~
# do not copy it, I want to use the same one everywhere.
ln -s $CURRDIR/vimrc .vimrc
exit 1
