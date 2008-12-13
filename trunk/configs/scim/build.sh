#!/bin/sh
#

CURRDIR=`pwd`
TOOLNAME=`basename $CURRDIR`

source ../../lib/lib.sh

need_root;

cmd_not_exist_exit $TOOLNAME;

file_exist_exit /etc/X11/Xsession.d/95xinput
# check symbol is exist
sym_file_exist_exit /etc/X11/Xsession.d/95xinput

cd /etc/X11/Xsession.d
# do not copy it, I want to use the same one everywhere.
ln -s $CURRDIR/95xinput 95xinput

echo "restart x window: ctrl+alt+backapace"

exit 0
