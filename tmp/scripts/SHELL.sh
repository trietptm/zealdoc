#!/bin/sh

PKG="SHELL"

build_shell()
{
	echo $DOT1X_DIR
	cd $DOT1X_DIR || exit 1
	cp $DOT1X_ROOT/conf/busybox.$ARCH.config ./.config
	make oldconfig
	make
	make PREFIX=$DOT1X_TEMP/$PKG install
	chmod u+s $DOT1X_TEMP/$PKG/bin/busybox
	(
	cd $DOT1X_TEMP/$PKG
	tar -zcf $PKG.tgz --exclude=$PKG.tgz .
	)
	cp $DOT1X_TEMP/$PKG/$PKG.tgz $DOT1X_BINARY/$PKG.tgz
}

package_shell()
{
	echo
}

clean_shell()
{
	echo
}

trap "exit -1" INT
case "$1" in
	build)
		build_shell
		;;
	package)
		package_shell
		;;
	clean)
		clean_shell
		;;
	*)
		;;
esac

exit 0
