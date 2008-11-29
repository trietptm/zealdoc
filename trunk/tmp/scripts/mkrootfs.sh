#!/bin/sh

get_root()
{
	local bindir=`dirname $1`
	export DOT1X_ROOT=`(cd $bindir; cd ..; pwd)`
	source $DOT1X_ROOT/scripts/libdot1x.sh
	dot1x_init
}

edit_rootfs()
{
	rm -rf $DOT1X_TEMP/rootfs
	mkdir $DOT1X_TEMP/rootfs
	(
	cd $DOT1X_TEMP/rootfs
	tar -zxf $DOT1X_ROOT/pkgs/rootfs.tgz
	)
	chroot $DOT1X_TEMP/rootfs
}

update_rootfs()
{
	(
	cd $DOT1X_TEMP/rootfs
	tar -zcf rootfs.tgz --exclude=rootfs.tgz .
	)
	cp -f $DOT1X_TEMP/rootfs/rootfs.tgz $DOT1X_ROOT/pkgs/rootfs.tgz
}

usage()
{
	echo "Usage:"
	echo "  $1 command"
	echo "Where \"command\" can be follows:"
	echo "  edit        edit root filesystem with chroot"
	echo "  update      update modified root filesystem"
}

get_root $0
case "$1" in
	edit)
		edit_rootfs
		;;
	update)
		update_rootfs
		;;
	*)
		usage `basename $0` && exit 1
		;;
esac

