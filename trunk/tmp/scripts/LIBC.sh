#!/bin/sh

build_libc()
{
	mkdir $DOT1X_DIR/build
	cd $DOT1X_DIR/build || exit 1
	../configure \
		--prefix=/ \
		--localstatedir=$LOCALSTATE_DIR \
		--sysconfdir=$SYSCONF_DIR \
		--with-headers=$DOT1X_PREFIX/include/linux \
		--enable-add-ons="linuxthreads" \
		--disable-profile \
		--enable-shared \
		--disable-omitfp \
		--disable-bounds \
		--disable-versioning \
		--disable-static-nss \
		--disable-hidden-plt \
		--without-gmp \
		--without-gd \
		--with-tls \
		--without-xcoff \
		--without-CVS \
		--with-elf \
		--with-fp \
	|| exit
	make clean
	make
	#make install
	make DESTDIR=$DOT1X_BUILD/glibc-linux install
	ldconfig
}

package_libc()
{
#		$DOT1X_PREFIX/lib/libdb.so \
	files="\
	"
	echo $files
}

clean_libc()
{
	echo
}

trap "exit -1" INT
case "$1" in
	build)
		build_libc
		;;
	package)
		package_libc
		;;
	clean)
		clean_libc
		;;
	*)
		;;
esac

exit 0
