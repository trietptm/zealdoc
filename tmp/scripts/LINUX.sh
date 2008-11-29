#!/bin/sh

build_linux()
{
	echo $DOT1X_DIR
	cd $DOT1X_DIR || exit 1
	cp $DOT1X_ROOT/conf/linux.$ARCH.$MACH.config ./.config
	make oldconfig
	make
	if [ ! -x $DOT1X_ROOT/pkgs ]; then
		mkdir -p $DOT1X_ROOT/pkgs
	fi
	cp arch/$ARCH/boot/bzImage $DOT1X_BINARY/vmlinuz
	cp .config $DOT1X_BINARY/config
	cp System.map $DOT1X_BINARY/System.map
	
	# copy cpio tools from linux source code, only for intermediate data processing.
	if [ -d $DOT1X_BINARY/cpio ] ; then
		rm -Rf $DOT1X_BINARY/cpio
	fi
	mkdir $DOT1X_BINARY/cpio
	cp -p scripts/gen_initramfs_list.sh $DOT1X_BINARY/cpio
	mkdir $DOT1X_BINARY/cpio/usr
	cp -p usr/gen_init_cpio	$DOT1X_BINARY/cpio/usr

	rm -rf $DOT1X_PREFIX/include/linux
	mkdir -p $DOT1X_PREFIX/include/linux
	cp -rf include/linux $DOT1X_PREFIX/include/linux/linux
	cp -rf include/asm-generic $DOT1X_PREFIX/include/linux/asm-generic
	cp -rf include/asm-$ARCH $DOT1X_PREFIX/include/linux/asm-$ARCH
	ln -s $DOT1X_PREFIX/include/linux/asm-$ARCH $DOT1X_PREFIX/include/linux/asm
}

package_linux()
{
	echo
}

clean_linux()
{
	echo
}

trap "exit -1" INT
case "$1" in
	build)
		build_linux
		;;
	package)
		package_linux
		;;
	clean)
		clean_linux
		;;
	*)
		;;
esac

exit 0
