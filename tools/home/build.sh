#!/bin/sh
#

CURRDIR=`pwd`

source ../../lib/lib.sh

file_exist_exit ~/.bashrc
sym_file_exist_exit ~/.bashrc

cd ~
# do not copy it, I want to use the same one everywhere.
ln -s $CURRDIR/bashrc .bashrc
exit 1
